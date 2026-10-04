#pragma once

#include <string>
#include <vector>

namespace hl
{

    struct ResourceDefinition
    {
        std::string name;
        std::string type;

        struct Child
        {
            std::string name;
            std::string type;
        };

        std::vector<Child> resources;
    };

}