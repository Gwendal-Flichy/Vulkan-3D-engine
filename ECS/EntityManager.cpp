#include "EntityManager.h"

namespace VkEngine
{
    EntityManager::EntityManager()
    {
        m_Signatures.resize( MAX_ENTITIES );

        for( EntityID id = 0u; id < MAX_ENTITIES; ++id )
            m_FreeIDs.push( id );
    }

    EntityID EntityManager::CreateEntity()
    {
        assert( m_ActiveCount < MAX_ENTITIES && "Max entity count reached" );

        const EntityID id = m_FreeIDs.front();
        m_FreeIDs.pop();
        ++m_ActiveCount;

        return id;
    }

    void EntityManager::DestroyEntity( EntityID entity )
    {
        assert( entity < MAX_ENTITIES && "EntityID out of range" );

        m_Signatures[entity].reset();
        m_FreeIDs.push( entity );
        --m_ActiveCount;
    }

    void EntityManager::SetSignature( EntityID entity, const ComponentMask& mask )
    {
        assert( entity < MAX_ENTITIES && "EntityID out of range" );
        m_Signatures[entity] = mask;
    }

    ComponentMask EntityManager::GetSignature( EntityID entity ) const
    {
        assert( entity < MAX_ENTITIES && "EntityID out of range" );
        return m_Signatures[entity];
    }

    uint32_t EntityManager::GetActiveCount() const
    {
        return m_ActiveCount;
    }
}
