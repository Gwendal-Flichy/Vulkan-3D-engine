#ifndef ECS_TYPES_H
#define ECS_TYPES_H
#pragma once

#include <bitset>
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace VkEngine
{
    using EntityID        = uint32_t;
    using ComponentTypeID = uint32_t;
    using ArchetypeID     = uint64_t;

    static constexpr EntityID   INVALID_ENTITY = UINT32_MAX;
    static constexpr uint32_t   MAX_COMPONENTS = 64u;
    static constexpr uint32_t   MAX_ENTITIES   = 10000u;

    using ComponentMask = std::bitset<MAX_COMPONENTS>;

    class ComponentTypeRegistry
    {
    private:
        static ComponentTypeID s_NextID;

    public:
        template<typename T>
        static ComponentTypeID GetID()
        {
            static const ComponentTypeID id = s_NextID++;
            return id;
        }
    };
}

#endif
