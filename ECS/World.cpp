#include "World.h"

namespace VkEngine
{
    EntityID World::CreateEntity()
    {
        const EntityID entity = m_EntityManager.CreateEntity();
        m_EntityManager.SetSignature( entity, ComponentMask{} );
        return entity;
    }

    void World::DestroyEntity( EntityID entity )
    {
        auto it = m_EntityArchetype.find( entity );

        if( it != m_EntityArchetype.end() )
        {
            it->second->RemoveEntity( entity );
            m_EntityArchetype.erase( it );
        }

        m_EntityManager.DestroyEntity( entity );
    }

    ArchetypeID World::MaskToID( const ComponentMask& mask ) const
    {
        return mask.to_ullong();
    }

    Archetype* World::GetOrCreateArchetype( const ComponentMask& mask )
    {
        const ArchetypeID id = MaskToID( mask );

        auto it = m_Archetypes.find( id );
        if( it != m_Archetypes.end() )
            return it->second.get();

        auto       archetype = std::make_unique<Archetype>( mask );
        Archetype* raw       = archetype.get();

        m_Archetypes[id] = std::move( archetype );

        return raw;
    }

    void World::EnsureArraysRegistered( Archetype* archetype, const ComponentMask& mask )
    {
        for( ComponentTypeID id = 0u; id < MAX_COMPONENTS; ++id )
        {
            if( !mask.test( id ) )
                continue;

            if( archetype->HasArray( id ) )
                continue;

            auto it = m_ArrayRegistrars.find( id );
            if( it != m_ArrayRegistrars.end() )
                it->second( archetype );
        }
    }

    std::vector<Archetype*> World::QueryArchetypes( const ComponentMask& required_mask )
    {
        std::vector<Archetype*> result;
        result.reserve( m_Archetypes.size() );

        for( auto& [id, archetype] : m_Archetypes )
        {
            if( ( archetype->GetMask() & required_mask ) == required_mask )
                result.push_back( archetype.get() );
        }

        return result;
    }

    void World::Update( float dt )
    {
        for( auto& system : m_Systems )
            system->Update( dt );
    }
}
