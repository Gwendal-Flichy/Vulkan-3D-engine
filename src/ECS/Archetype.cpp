#include "Archetype.h"

namespace VkEngine
{
    Archetype::Archetype( const ComponentMask& mask )
        : m_Mask( mask )
    {
    }

    void Archetype::MoveEntityTo( EntityID entity, Archetype* destination )
    {
        assert( HasEntity( entity ) && "Entity not found in archetype" );
        assert( destination != nullptr && "Destination archetype is null" );

        const size_t index = m_EntityToIndex.at( entity );

        destination->m_EntityToIndex[entity] = destination->m_Entities.size();
        destination->m_Entities.push_back( entity );

        for( auto& [type_id, array] : m_Arrays )
        {
            if( destination->HasArray( type_id ) )
                array->CopyTo( index, destination->GetArray( type_id ) );
        }

        RemoveEntity( entity );
    }

    void Archetype::RemoveEntity( EntityID entity )
    {
        assert( HasEntity( entity ) && "Entity not found in archetype" );

        const size_t   index       = m_EntityToIndex.at( entity );
        const size_t   last_index  = m_Entities.size() - 1u;
        const EntityID last_entity = m_Entities[last_index];

        for( auto& [type_id, array] : m_Arrays )
            array->RemoveAt( index );

        if( index != last_index )
        {
            m_Entities[index]            = last_entity;
            m_EntityToIndex[last_entity] = index;
        }

        m_Entities.pop_back();
        m_EntityToIndex.erase( entity );
    }

    bool Archetype::HasEntity( EntityID entity ) const
    {
        return m_EntityToIndex.count( entity ) > 0u;
    }

    size_t Archetype::GetEntityIndex( EntityID entity ) const
    {
        assert( HasEntity( entity ) && "Entity not found in archetype" );
        return m_EntityToIndex.at( entity );
    }

    size_t Archetype::GetEntityCount() const
    {
        return m_Entities.size();
    }

    ComponentMask Archetype::GetMask() const
    {
        return m_Mask;
    }

    const std::vector<EntityID>& Archetype::GetEntities() const
    {
        return m_Entities;
    }

    bool Archetype::HasArray( ComponentTypeID type_id ) const
    {
        return m_Arrays.count( type_id ) > 0u;
    }

    IComponentArray* Archetype::GetArray( ComponentTypeID type_id )
    {
        auto it = m_Arrays.find( type_id );
        return ( it != m_Arrays.end() ) ? it->second.get() : nullptr;
    }
}
