#ifndef WORLD_H
#define WORLD_H
#pragma once

#include "Archetype.h"
#include "EcsTypes.h"
#include "EntityManager.h"
#include "System.h"

#include <cassert>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace VkEngine
{
    class World
    {
    private:
        using ArrayRegistrar = std::function<void( Archetype* )>;

        EntityManager m_EntityManager;

        std::unordered_map<ArchetypeID, std::unique_ptr<Archetype>> m_Archetypes;
        std::unordered_map<EntityID, Archetype*>                    m_EntityArchetype;
        std::vector<std::unique_ptr<System>>                        m_Systems;

        std::unordered_map<ComponentTypeID, ArrayRegistrar> m_ArrayRegistrars;

        ArchetypeID MaskToID( const ComponentMask& mask ) const;
        Archetype*  GetOrCreateArchetype( const ComponentMask& mask );
        void        EnsureArraysRegistered( Archetype* archetype, const ComponentMask& mask );

    public:
        World()  = default;
        ~World() = default;

        EntityID CreateEntity();
        void     DestroyEntity( EntityID entity );

        template<typename T>
        void AddComponent( EntityID entity, T component = T{} )
        {
            const ComponentTypeID type_id  = ComponentTypeRegistry::GetID<T>();
            const ComponentMask   old_mask = m_EntityManager.GetSignature( entity );

            assert( !old_mask.test( type_id ) && "Entity already has this component" );

            if( m_ArrayRegistrars.find( type_id ) == m_ArrayRegistrars.end() )
            {
                m_ArrayRegistrars[type_id] = []( Archetype* arch )
                {
                    arch->RegisterArray<T>();
                };
            }

            ComponentMask new_mask = old_mask;
            new_mask.set( type_id );

            Archetype* new_archetype = GetOrCreateArchetype( new_mask );
            EnsureArraysRegistered( new_archetype, new_mask );

            Archetype* old_archetype = nullptr;
            auto       it            = m_EntityArchetype.find( entity );

            if( it != m_EntityArchetype.end() )
                old_archetype = it->second;

            if( old_archetype != nullptr )
                old_archetype->MoveEntityTo( entity, new_archetype );
            else
                new_archetype->RegisterEntity( entity );

            new_archetype->InsertComponent<T>( entity, std::move( component ) );

            m_EntityArchetype[entity] = new_archetype;
            m_EntityManager.SetSignature( entity, new_mask );
        }

        template<typename T>
        void RemoveComponent( EntityID entity )
        {
            const ComponentTypeID type_id  = ComponentTypeRegistry::GetID<T>();
            const ComponentMask   old_mask = m_EntityManager.GetSignature( entity );

            assert( old_mask.test( type_id ) && "Entity does not have this component" );

            ComponentMask new_mask = old_mask;
            new_mask.reset( type_id );

            Archetype* old_archetype = m_EntityArchetype.at( entity );
            Archetype* new_archetype = GetOrCreateArchetype( new_mask );
            EnsureArraysRegistered( new_archetype, new_mask );

            old_archetype->MoveEntityTo( entity, new_archetype );

            m_EntityArchetype[entity] = new_archetype;
            m_EntityManager.SetSignature( entity, new_mask );
        }

        template<typename T>
        T& GetComponent( EntityID entity )
        {
            assert( m_EntityArchetype.count( entity ) && "Entity has no archetype" );
            return m_EntityArchetype.at( entity )->GetComponent<T>( entity );
        }

        template<typename T>
        const T& GetComponent( EntityID entity ) const
        {
            assert( m_EntityArchetype.count( entity ) && "Entity has no archetype" );
            return m_EntityArchetype.at( entity )->GetComponent<T>( entity );
        }

        template<typename T>
        bool HasComponent( EntityID entity ) const
        {
            return m_EntityManager.GetSignature( entity ).test(
                       ComponentTypeRegistry::GetID<T>() );
        }

        template<typename... Ts>
        ComponentMask BuildMask() const
        {
            ComponentMask mask;
            ( mask.set( ComponentTypeRegistry::GetID<Ts>() ), ... );
            return mask;
        }

        template<typename T, typename... Args>
        T* RegisterSystem( Args&&... args )
        {
            static_assert( std::is_base_of<System, T>::value, "T must inherit from System" );

            auto  system   = std::make_unique<T>( std::forward<Args>( args )... );
            T*    observer = system.get();
            observer->SetWorld( this );

            m_Systems.push_back( std::move( system ) );
            return observer;
        }

        void                    Update( float dt );
        std::vector<Archetype*> QueryArchetypes( const ComponentMask& required_mask );
    };
}

#endif
