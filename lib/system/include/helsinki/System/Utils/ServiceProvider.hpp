#pragma once

#include <helsinki/System/Utils/ServiceLifetime.hpp>
#include <unordered_map>
#include <type_traits>
#include <functional>
#include <stdexcept>
#include <typeindex>
#include <iostream>
#include <memory>
#include <tuple>
#include <any>

namespace hl
{
    class ServiceProvider
    {
    private:
        template<typename Impl, typename... DepPtrs>
        struct ImplHolder
        {
            std::tuple<std::shared_ptr<std::remove_reference_t<DepPtrs>>...> deps;
            Impl impl;

            template<std::size_t... I>
            ImplHolder(std::tuple<std::shared_ptr<std::remove_reference_t<DepPtrs>>...> d, std::index_sequence<I...>)
                : deps(std::move(d)),
                impl(*std::get<I>(deps)...)
            {
            }
        };
    public:
        explicit ServiceProvider(ServiceProvider* parent = nullptr)
            : parent(parent), root(parent ? parent->root : this)
        {
        }

        template<typename Interface, typename Impl, typename... Deps>
        void registerService(ServiceLifetime lifetime = ServiceLifetime::Singleton)
        {
            validateDependenciesLifetime<std::tuple<Deps...>>(lifetime);

            registerFactory<Interface>(
                [lifetime](ServiceProvider& sp) -> std::shared_ptr<Interface>
                {
                    auto depsTuple = std::make_tuple(getDependency<Deps>(sp)...);

                    using HolderT = ImplHolder<Impl, Deps...>;
                    constexpr std::size_t N = sizeof...(Deps);

                    auto holder = std::make_shared<HolderT>(std::move(depsTuple), std::make_index_sequence<N>{});

                    Interface* implPtr = &holder->impl;
                    return std::shared_ptr<Interface>(holder, implPtr);
                },
                lifetime
            );
        }

        template<typename T>
        T& get()
        {
            auto type = std::type_index(typeid(T));

            std::shared_ptr<void>* cached = nullptr;
            ServiceProvider* cacheOwner = nullptr;

            auto& instMap = instances_;
            if (root->instances_.count(type))
            {
                cached = &root->instances_[type];
                cacheOwner = root;
            }
            else if (instances_.count(type))
            {
                cached = &instances_[type];
                cacheOwner = this;
            }

            if (cached) return *std::static_pointer_cast<T>(*cached);

            FactoryEntry* fit = findFactory(type);
            if (!fit) throw std::runtime_error("Factory not registered");

            std::shared_ptr<void> instance = fit->factory(*this);

            switch (fit->lifetime)
            {
            case ServiceLifetime::Singleton:
                root->instances_[type] = instance;
                break;
            case ServiceLifetime::Scoped:
                instances_[type] = instance;
                break;
            case ServiceLifetime::Transient:
                break;
            }

            return *std::static_pointer_cast<T>(instance);
        }

        template<typename T>
        std::shared_ptr<T> getTransient()
        {
            FactoryEntry* fit = findFactory(std::type_index(typeid(T)));
            if (!fit) throw std::runtime_error("Factory not registered");
            return std::static_pointer_cast<T>(fit->factory(*this));
        }

        ServiceProvider createScope() { return ServiceProvider(this); }

    private:
        struct FactoryEntry
        {
            std::function<std::shared_ptr<void>(ServiceProvider&)> factory;
            ServiceLifetime lifetime;
        };

        std::unordered_map<std::type_index, FactoryEntry> factories_;
        std::unordered_map<std::type_index, std::shared_ptr<void>> instances_;
        ServiceProvider* parent = nullptr;
        ServiceProvider* root = nullptr;

        template<typename Interface>
        void registerFactory(std::function<std::shared_ptr<Interface>(ServiceProvider&)> factory,
            ServiceLifetime lifetime)
        {
            factories_[std::type_index(typeid(Interface))] = {
                [factory](ServiceProvider& sp) -> std::shared_ptr<void> { return factory(sp); },
                lifetime
            };
        }

        FactoryEntry* findFactory(const std::type_index& type)
        {
            auto it = factories_.find(type);
            if (it != factories_.end()) return &it->second;
            if (parent) return parent->findFactory(type);
            return nullptr;
        }

        template<typename Tuple, std::size_t N = 0>
        void validateDependenciesLifetime(ServiceLifetime selfLifetime)
        {
            if constexpr (N < std::tuple_size_v<Tuple>)
            {
                using Dep = std::tuple_element_t<N, Tuple>;
                ServiceLifetime depLifetime = ServiceLifetime::Transient;

                auto it = factories_.find(std::type_index(typeid(Dep)));
                if (it != factories_.end()) depLifetime = it->second.lifetime;
                else if (parent) depLifetime = parent->getLifetime<Dep>();

                if (static_cast<int>(depLifetime) < static_cast<int>(selfLifetime))
                    throw std::runtime_error(
                        "Invalid lifetime: shorter-lived dependency injected into longer-lived service"
                    );

                validateDependenciesLifetime<Tuple, N + 1>(selfLifetime);
            }
        }

        template<typename T>
        ServiceLifetime getLifetime() const
        {
            auto it = factories_.find(std::type_index(typeid(T)));
            if (it != factories_.end()) return it->second.lifetime;
            if (parent) return parent->getLifetime<T>();
            return ServiceLifetime::Transient;
        }

        template<typename Dep, typename Impl = Dep>
        static auto getDependency(ServiceProvider& sp)
        {
            ServiceLifetime depLifetime = sp.getLifetime<Dep>();
            if (depLifetime == ServiceLifetime::Transient)
            {
                return sp.getTransient<Dep>();
            }
            else
            {
                return std::shared_ptr<Dep>(&sp.get<Dep>(), [](Dep*) {});
            }
        }
    };
}
