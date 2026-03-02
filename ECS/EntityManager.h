#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H
#pragma once

#include "EcsTypes.h"

#include <cassert>
#include <queue>
#include <vector>

namespace VkEngine
{
    class EntityManager
    {
    private:
        std::vector<ComponentMask> m_Signatures;
        std::queue<EntityID>       m_FreeIDs;
        uint32_t                   m_ActiveCount = 0u;

    public:
        EntityManager();

        EntityID      CreateEntity();
        void          DestroyEntity( EntityID entity );

        void          SetSignature( EntityID entity, const ComponentMask& mask );
        ComponentMask GetSignature( EntityID entity ) const;

        uint32_t      GetActiveCount() const;
    };
}

#endif
