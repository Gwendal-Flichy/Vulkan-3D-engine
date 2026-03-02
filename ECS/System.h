#ifndef SYSTEM_H
#define SYSTEM_H
#pragma once

#include "EcsTypes.h"

#include <cassert>
#include <vector>

namespace VkEngine
{
    class World;
    class Archetype;

    class System
    {
    private:
        ComponentMask m_RequiredMask;
        World*        m_World  = nullptr;
        bool          m_Active = true;

    public:
        System()          = default;
        virtual ~System() = default;

        void Update( float dt );

        void SetRequiredMask( const ComponentMask& mask );
        void SetWorld( World* world );
        void SetActive( bool active );

        const ComponentMask& GetRequiredMask() const;
        World*               GetWorld()        const;
        bool                 IsActive()        const;

        bool MatchesMask( const ComponentMask& entity_mask ) const;

    protected:
        virtual void OnUpdate( float dt, const std::vector<Archetype*>& archetypes ) = 0;
    };
}

#endif
