#include "System.h"
#include "World.h"

namespace VkEngine
{
    void System::Update( float dt )
    {
        if( !m_Active )
            return;

        assert( m_World != nullptr && "System has no World reference" );

        std::vector<Archetype*> matched = m_World->QueryArchetypes( m_RequiredMask );
        OnUpdate( dt, matched );
    }

    void System::SetRequiredMask( const ComponentMask& mask )
    {
        m_RequiredMask = mask;
    }

    void System::SetWorld( World* world )
    {
        m_World = world;
    }

    void System::SetActive( bool active )
    {
        m_Active = active;
    }

    const ComponentMask& System::GetRequiredMask() const { return m_RequiredMask; }
    World*               System::GetWorld()        const { return m_World;        }
    bool                 System::IsActive()        const { return m_Active;       }

    bool System::MatchesMask( const ComponentMask& entity_mask ) const
    {
        return ( entity_mask & m_RequiredMask ) == m_RequiredMask;
    }
}
