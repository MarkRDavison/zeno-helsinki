#include <catch2/catch_test_macros.hpp>
#include <helsinki/System/Utils/ServiceProvider.hpp>
#include <iostream>

namespace hl
{
    namespace test
    {
        struct NoInterface {};
        struct IServiceB { virtual void hello() = 0; virtual ~IServiceB() = default; };
        struct IServiceC { virtual void hello() = 0; virtual ~IServiceC() = default; };

        struct IServiceA
        {
            virtual void hello() = 0;
            virtual ~IServiceA() = default;
            virtual IServiceC& getC() = 0;
            virtual IServiceB& getB() = 0;
        };

        struct IServiceH { virtual void hello() = 0; virtual ~IServiceH() = default; virtual IServiceC& getC() = 0; virtual IServiceB& getB() = 0; };
        struct IServiceI { virtual void hello() = 0; virtual ~IServiceI() = default; virtual IServiceC& getC() = 0; };


        struct ServiceB : IServiceB { void hello() override { std::cout << "B\n"; } };
        struct ServiceC : IServiceC { void hello() override { std::cout << "C\n"; } };
        struct ServiceA : IServiceA
        {
            ServiceA(IServiceB& b, IServiceC& c) : b(b), c(c) {}
            void hello() override
            {
                std::cout << "A\n";
                b.hello();
                c.hello();
            }

            IServiceB& getB() override { return b; }
            IServiceC& getC() override { return c; }

            IServiceB& b;
            IServiceC& c;
        };

        struct ServiceD : IServiceC
        {
            ServiceD() = default;
            void hello() override { std::cout << "D\n"; }
        };

        struct ServiceE : IServiceA
        {
            ServiceE(IServiceC& c) : c(c) {}
            void hello() override { std::cout << "E\n"; c.hello(); }
            IServiceB& getB() override { throw std::runtime_error("E has no B"); }
            IServiceC& getC() override { return c; }
            IServiceC& c;
        };

        struct ServiceF : IServiceA
        {
            ServiceF(IServiceB& b, IServiceC& c) : b(b), c(c) {}
            void hello() override { std::cout << "F\n"; b.hello(); c.hello(); }
            IServiceB& getB() override { return b; }
            IServiceC& getC() override { return c; }

            IServiceB& b;
            IServiceC& c;
        };

        struct ServiceG : IServiceC
        {
            void hello() override { std::cout << "G\n"; }
        };

        struct ServiceH : IServiceH
        {
            ServiceH(IServiceB& b, IServiceC& c) : b(b), c(c) {}
            void hello() override { std::cout << "H\n"; b.hello(); c.hello(); }
            IServiceB& getB() override { return b; }
            IServiceC& getC() override { return c; }

            IServiceB& b;
            IServiceC& c;
        };

        struct ServiceI : IServiceI
        {
            ServiceI(IServiceH& h, IServiceC& c) : h(h), c(c) {}
            void hello() override { std::cout << "I\n"; h.hello(); c.hello(); }
            IServiceC& getC() override { return c; }

            IServiceH& h;
            IServiceC& c;
        };

        struct IServiceJ
        {
            virtual void hello() = 0;
            virtual ~IServiceJ() = default;
            virtual IServiceH& getH() = 0;
            virtual IServiceC& getC() = 0;
        };

        struct ServiceJ : IServiceJ
        {
            ServiceJ(IServiceH& h, IServiceC& c) : h(h), c(c) {}
            void hello() override { std::cout << "J\n"; h.hello(); c.hello(); }
            IServiceH& getH() override { return h; }
            IServiceC& getC() override { return c; }

            IServiceH& h;
            IServiceC& c;
        };

        struct IServiceL
        {
            virtual void hello() = 0;
            virtual ~IServiceL() = default;
        };

        struct IServiceK
        {
            virtual void hello() = 0;
            virtual ~IServiceK() = default;
            virtual IServiceC& getC() = 0;
            virtual IServiceL& getL() = 0;
        };

        struct ServiceL : IServiceL
        {
            void hello() override { std::cout << "L\n"; }
        };

        struct ServiceK : IServiceK
        {
            ServiceK(IServiceL& l, IServiceC& c) : l(l), c(c) {}
            void hello() override { std::cout << "K\n"; l.hello(); c.hello(); }
            IServiceC& getC() override { return c; }
            IServiceL& getL() override { return l; }

            IServiceL& l;
            IServiceC& c;
        };

        TEST_CASE("ServiceProvider basic registration and retrieval", "[ServiceProvider]")
        {
            hl::ServiceProvider root;

            root.registerService<IServiceB, ServiceB>(ServiceLifetime::Singleton);
            root.registerService<IServiceC, ServiceC>(ServiceLifetime::Scoped);
            root.registerService<IServiceA, ServiceA, IServiceB, IServiceC>(ServiceLifetime::Scoped);

            SECTION("Scoped service returns valid reference")
            {
                auto scope1 = root.createScope();
                IServiceA& a1 = scope1.get<IServiceA>();
                REQUIRE_NOTHROW(a1.hello());
            }

            SECTION("Same scope returns same instance for scoped service")
            {
                auto scope1 = root.createScope();
                IServiceA& a1 = scope1.get<IServiceA>();
                IServiceA& a2 = scope1.get<IServiceA>();
                REQUIRE(&a1 == &a2);
            }

            SECTION("Different scope returns different instance for scoped service")
            {
                auto scope1 = root.createScope();
                auto scope2 = root.createScope();
                IServiceA& a1 = scope1.get<IServiceA>();
                IServiceA& a2 = scope2.get<IServiceA>();
                REQUIRE(&a1 != &a2);
            }

            SECTION("Singleton service returns same instance across scopes")
            {
                IServiceB& b1 = root.get<IServiceB>();
                IServiceB& b2 = root.get<IServiceB>();
                REQUIRE(&b1 == &b2);

                auto scope2 = root.createScope();
                IServiceB& b3 = scope2.get<IServiceB>();
                REQUIRE(&b1 == &b3);
            }

            SECTION("Transient service returns different instances")
            {
                auto c1 = root.getTransient<IServiceC>();
                auto c2 = root.getTransient<IServiceC>();
                REQUIRE(c1 != c2);
            }

            SECTION("Invalid lifetime injection throws runtime error")
            {
                REQUIRE_THROWS_AS(
                    (root.registerService<IServiceA, ServiceA, IServiceB, IServiceC>(ServiceLifetime::Singleton)),
                    std::runtime_error
                );
            }
        }

        TEST_CASE("Singleton service shared across nested scopes", "[ServiceProvider]")
        {
            hl::ServiceProvider root;
            root.registerService<IServiceB, ServiceB>(ServiceLifetime::Singleton);

            auto scope1 = root.createScope();
            auto scope2 = scope1.createScope();

            IServiceB& b1 = root.get<IServiceB>();
            IServiceB& b2 = scope1.get<IServiceB>();
            IServiceB& b3 = scope2.get<IServiceB>();

            REQUIRE(&b1 == &b2);
            REQUIRE(&b1 == &b3);
        }

        TEST_CASE("Scoped services are unique per nested scope", "[ServiceProvider]")
        {
            hl::ServiceProvider root;
            root.registerService<IServiceC, ServiceC>(ServiceLifetime::Scoped);

            auto scope1 = root.createScope();
            auto scope2 = scope1.createScope();

            IServiceC& c1 = scope1.get<IServiceC>();
            IServiceC& c2 = scope2.get<IServiceC>();

            REQUIRE(&c1 != &c2);
        }

        TEST_CASE("Singleton cannot depend on shorter-lived service", "[ServiceProvider]")
        {
            struct ShortLived {};
            struct LongLived
            {
                LongLived(ShortLived&) {}
            };


            hl::ServiceProvider root;

            root.registerService<ShortLived, ShortLived>(ServiceLifetime::Scoped);

            REQUIRE_THROWS_AS(
                (root.registerService<LongLived, LongLived, ShortLived>(ServiceLifetime::Singleton)),
                std::runtime_error
            );
        }

        TEST_CASE("Transient services are always new across scopes", "[ServiceProvider]")
        {
            hl::ServiceProvider root;
            root.registerService<IServiceC, ServiceC>(ServiceLifetime::Transient);

            auto scope1 = root.createScope();
            auto scope2 = root.createScope();

            auto t1 = root.getTransient<IServiceC>();
            auto t2 = scope1.getTransient<IServiceC>();
            auto t3 = scope2.getTransient<IServiceC>();

            REQUIRE(t1 != t2);
            REQUIRE(t1 != t3);
            REQUIRE(t2 != t3);
        }

        TEST_CASE("Scoped services are isolated from parent and sibling scopes", "[ServiceProvider]")
        {
            hl::ServiceProvider root;
            root.registerService<IServiceC, ServiceC>(ServiceLifetime::Scoped);

            auto parentScope = root.createScope();
            auto childScope = parentScope.createScope();
            auto siblingScope = root.createScope();

            IServiceC& parentInstance = parentScope.get<IServiceC>();
            IServiceC& childInstance = childScope.get<IServiceC>();
            IServiceC& siblingInstance = siblingScope.get<IServiceC>();

            REQUIRE(&parentInstance != &childInstance);
            REQUIRE(&parentInstance != &siblingInstance);
            REQUIRE(&childInstance != &siblingInstance);
        }


        TEST_CASE("Scoped service can depend on another scoped service", "[ServiceProvider]")
        {
            hl::ServiceProvider root;

            root.registerService<IServiceC, ServiceD>(ServiceLifetime::Scoped);
            root.registerService<IServiceA, ServiceE, IServiceC>(ServiceLifetime::Scoped);

            auto scope1 = root.createScope();
            auto scope2 = root.createScope();

            IServiceA& e1 = scope1.get<IServiceA>();
            IServiceA& e2 = scope2.get<IServiceA>();
            IServiceC& d1 = scope1.get<IServiceC>();
            IServiceC& d2 = scope2.get<IServiceC>();

            REQUIRE(&e1 != &e2);
            REQUIRE(&d1 != &d2);

            REQUIRE(&e1.getC() == &d1);
            REQUIRE(&e2.getC() == &d2);
        }

        TEST_CASE("Transient service creates new instances but receives proper dependencies", "[ServiceProvider]")
        {
            hl::ServiceProvider root;

            root.registerService<IServiceB, ServiceB>(ServiceLifetime::Singleton);
            root.registerService<IServiceC, ServiceC>(ServiceLifetime::Scoped);
            root.registerService<IServiceA, ServiceF, IServiceB, IServiceC>(ServiceLifetime::Transient);

            auto scope1 = root.createScope();
            auto scope2 = root.createScope();

            auto f1 = scope1.getTransient<IServiceA>();
            auto f2 = scope1.getTransient<IServiceA>();
            auto f3 = scope2.getTransient<IServiceA>();

            REQUIRE(f1 != f2);
            REQUIRE(f1 != f3);
            REQUIRE(f2 != f3);

            REQUIRE(&f1->getB() == &f2->getB());
            REQUIRE(&f1->getC() == &scope1.get<IServiceC>());
            REQUIRE(&f3->getC() == &scope2.get<IServiceC>());
        }

        TEST_CASE("Scoped services with multi-level dependencies respect lifetimes", "[ServiceProvider]")
        {
            hl::ServiceProvider root;

            root.registerService<IServiceB, ServiceB>(ServiceLifetime::Singleton);
            root.registerService<IServiceC, ServiceG>(ServiceLifetime::Scoped);
            root.registerService<IServiceH, ServiceH, IServiceB, IServiceC>(ServiceLifetime::Scoped);
            root.registerService<IServiceI, ServiceI, IServiceH, IServiceC>(ServiceLifetime::Scoped);

            auto scope1 = root.createScope();
            auto scope2 = root.createScope();

            IServiceH& h1 = scope1.get<IServiceH>();
            IServiceH& h2 = scope2.get<IServiceH>();
            IServiceI& i1 = scope1.get<IServiceI>();
            IServiceI& i2 = scope2.get<IServiceI>();

            REQUIRE(&h1 != &h2);
            REQUIRE(&i1 != &i2);

            REQUIRE(&h1.getC() == &scope1.get<IServiceC>());
            REQUIRE(&i1.getC() == &scope1.get<IServiceC>());
            REQUIRE(&h2.getC() == &scope2.get<IServiceC>());
            REQUIRE(&i2.getC() == &scope2.get<IServiceC>());

            REQUIRE(&h1.getB() == &h2.getB());
        }

        TEST_CASE("Transient services in multi-level dependency chain", "[ServiceProvider]")
        {
            hl::ServiceProvider root;

            root.registerService<IServiceB, ServiceB>(ServiceLifetime::Singleton);
            root.registerService<IServiceC, ServiceG>(ServiceLifetime::Scoped);
            root.registerService<IServiceH, ServiceH, IServiceB, IServiceC>(ServiceLifetime::Scoped);
            root.registerService<IServiceJ, ServiceJ, IServiceH, IServiceC>(ServiceLifetime::Transient);

            auto scope1 = root.createScope();
            auto scope2 = root.createScope();

            auto j1 = scope1.getTransient<IServiceJ>();
            auto j2 = scope1.getTransient<IServiceJ>();
            auto j3 = scope2.getTransient<IServiceJ>();

            REQUIRE(j1 != j2);
            REQUIRE(j1 != j3);
            REQUIRE(j2 != j3);

            REQUIRE(&j1->getH() == &scope1.get<IServiceH>());
            REQUIRE(&j2->getH() == &scope1.get<IServiceH>());
            REQUIRE(&j3->getH() == &scope2.get<IServiceH>());

            REQUIRE(&j1->getC() == &scope1.get<IServiceC>());
            REQUIRE(&j2->getC() == &scope1.get<IServiceC>());
            REQUIRE(&j3->getC() == &scope2.get<IServiceC>());
        }

        TEST_CASE("Transient depending on transient", "[ServiceProvider]")
        {
            hl::ServiceProvider root;

            root.registerService<IServiceC, ServiceG>(ServiceLifetime::Scoped);
            root.registerService<IServiceL, ServiceL>(ServiceLifetime::Transient);
            root.registerService<IServiceK, ServiceK, IServiceL, IServiceC>(ServiceLifetime::Transient);

            auto scope1 = root.createScope();
            auto scope2 = root.createScope();

            auto k1 = scope1.getTransient<IServiceK>();
            auto k2 = scope1.getTransient<IServiceK>();
            auto k3 = scope2.getTransient<IServiceK>();

            REQUIRE(k1 != k2);
            REQUIRE(k1 != k3);
            REQUIRE(k2 != k3);

            REQUIRE(&k1->getL() != &k2->getL());
            REQUIRE(&k1->getL() != &k3->getL());
            REQUIRE(&k2->getL() != &k3->getL());

            REQUIRE(&k1->getC() == &scope1.get<IServiceC>());
            REQUIRE(&k2->getC() == &scope1.get<IServiceC>());
            REQUIRE(&k3->getC() == &scope2.get<IServiceC>());
        }

        TEST_CASE("Scoped service depends on singleton", "[ServiceProvider]")
        {
            hl::ServiceProvider root;

            root.registerService<IServiceB, ServiceB>(ServiceLifetime::Singleton);
            root.registerService<IServiceC, ServiceC>(ServiceLifetime::Scoped);
            root.registerService<IServiceA, ServiceA, IServiceB, IServiceC>(ServiceLifetime::Scoped);

            auto scope1 = root.createScope();
            auto scope2 = root.createScope();

            IServiceA& a1 = scope1.get<IServiceA>();
            IServiceA& a2 = scope2.get<IServiceA>();

            REQUIRE(&a1 != &a2);

            REQUIRE(&a1.getB() == &a2.getB());

            REQUIRE(&a1.getC() != &a2.getC());
        }

        TEST_CASE("Transient services produce new instances with correct dependency lifetimes", "[ServiceProvider]")
        {
            hl::ServiceProvider root;

            root.registerService<IServiceB, ServiceB>(ServiceLifetime::Singleton);
            root.registerService<IServiceC, ServiceC>(ServiceLifetime::Scoped);
            root.registerService<IServiceA, ServiceA, IServiceB, IServiceC>(ServiceLifetime::Transient);

            auto scope1 = root.createScope();
            auto scope2 = root.createScope();

            auto t1 = scope1.getTransient<IServiceA>();
            auto t2 = scope1.getTransient<IServiceA>();
            auto t3 = scope2.getTransient<IServiceA>();

            REQUIRE(t1 != t2);
            REQUIRE(t1 != t3);
            REQUIRE(t2 != t3);

            REQUIRE(&t1->getB() == &t2->getB());
            REQUIRE(&t1->getB() == &t3->getB());

            REQUIRE(&t1->getC() == &scope1.get<IServiceC>());
            REQUIRE(&t2->getC() == &scope1.get<IServiceC>());
            REQUIRE(&t3->getC() == &scope2.get<IServiceC>());
        }

        TEST_CASE("Nested scopes isolate scoped services but share singletons", "[ServiceProvider]")
        {
            hl::ServiceProvider root;

            root.registerService<IServiceB, ServiceB>(ServiceLifetime::Singleton);
            root.registerService<IServiceC, ServiceC>(ServiceLifetime::Scoped);
            root.registerService<IServiceA, ServiceA, IServiceB, IServiceC>(ServiceLifetime::Scoped);

            auto parentScope = root.createScope();
            auto childScope = parentScope.createScope();
            auto siblingScope = root.createScope();

            IServiceA& parentA = parentScope.get<IServiceA>();
            IServiceA& childA = childScope.get<IServiceA>();
            IServiceA& siblingA = siblingScope.get<IServiceA>();

            REQUIRE(&parentA != &childA);
            REQUIRE(&parentA != &siblingA);
            REQUIRE(&childA != &siblingA);

            REQUIRE(&parentA.getB() == &childA.getB());
            REQUIRE(&parentA.getB() == &siblingA.getB());

            REQUIRE(&parentA.getC() != &childA.getC());
            REQUIRE(&parentA.getC() != &siblingA.getC());
            REQUIRE(&childA.getC() != &siblingA.getC());
        }

        TEST_CASE("Registering concrete service without interface works", "[ServiceProvider]")
        {
            hl::ServiceProvider root;

            root.registerService<NoInterface, NoInterface>(hl::ServiceLifetime::Singleton);

            SECTION("Singleton concrete service returns same instance from root")
            {
                NoInterface& e1 = root.get<NoInterface>();
                NoInterface& e2 = root.get<NoInterface>();
                REQUIRE(&e1 == &e2);
            }

            SECTION("Scopes share same singleton instance")
            {
                auto scope1 = root.createScope();
                auto scope2 = root.createScope();

                NoInterface& e1 = scope1.get<NoInterface>();
                NoInterface& e2 = scope2.get<NoInterface>();

                REQUIRE(&e1 == &e2);
            }

            SECTION("Transient creation still supported if lifetime changed to transient")
            {
                hl::ServiceProvider sp;
                sp.registerService<NoInterface, NoInterface>(hl::ServiceLifetime::Transient);

                auto t1 = sp.getTransient<NoInterface>();
                auto t2 = sp.getTransient<NoInterface>();

                REQUIRE(t1 != t2);
            }
        }
    }
}
