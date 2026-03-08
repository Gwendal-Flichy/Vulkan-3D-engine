#ifndef ARCHETYPE_H
#define ARCHETYPE_H
#pragma once

#include "EcsTypes.h"

#include <cassert>
#include <memory>
#include <unordered_map>
#include <vector>

namespace VkEngine
{
    class IComponentArray
    {
    public:
        virtual ~IComponentArray() = default;

        virtual void   CopyTo( size_t index, IComponentArray* destination ) = 0;
        virtual void   RemoveAt( size_t index )                             = 0;
        virtual size_t GetSize() const                                      = 0;
    };


    template<typename T>
    class ComponentArray : public IComponentArray
    {
    private:
        std::vector<T> m_Data;

    public:
        void Insert( T component )
        {
            m_Data.push_back( std::move( component ) );
        }

        T& GetAt( size_t index )
        {
            assert( index < m_Data.size() && "ComponentArray index out of range" );
            return m_Data[index];
        }

        const T& GetAt( size_t index ) const
        {
            assert( index < m_Data.size() && "ComponentArray index out of range" );
            return m_Data[index];
        }

        void CopyTo( size_t index, IComponentArray* destination ) override
        {
            assert( index < m_Data.size() && "ComponentArray index out of range" );

            static_cast<ComponentArray<T>*>( destination )->Insert( m_Data[index] );
        }

        void RemoveAt( size_t index ) override
        {
            assert( index < m_Data.size() && "ComponentArray index out of range" );

            if( index != m_Data.size() - 1u )
                m_Data[index] = std::move( m_Data.back() );

            m_Data.pop_back();
        }

        size_t GetSize() const override { return m_Data.size(); }
    };


    class Archetype
    {
    private:
        ComponentMask m_Mask;

        std::unordered_map<EntityID, size_t>                                  m_EntityToIndex;
        std::vector<EntityID>                                                  m_Entities;
        std::unordered_map<ComponentTypeID, std::unique_ptr<IComponentArray>>  m_Arrays;

    public:
        explicit Archetype( const ComponentMask& mask );

        template<typename T>
        void RegisterArray()
        {
            const ComponentTypeID type_id = ComponentTypeRegistry::GetID<T>();

            assert( m_Mask.test( type_id ) && "Component type not in archetype mask" );
            assert( m_Arrays.find( type_id ) == m_Arrays.end() && "Array already registered" );

            m_Arrays[type_id] = std::make_unique<ComponentArray<T>>();
        }

        void RegisterEntity( EntityID entity )
        {
            assert( !HasEntity( entity ) && "Entity already in archetype" );
            m_EntityToIndex[entity] = m_Entities.size();
            m_Entities.push_back( entity );
        }

        template<typename T>
        void InsertComponent( EntityID entity, T component )
        {
            const ComponentTypeID type_id = ComponentTypeRegistry::GetID<T>();

            assert( m_Arrays.count( type_id ) && "Array not registered for this type" );
            assert( HasEntity( entity ) && "Entity not registered in archetype" );

            static_cast<ComponentArray<T>*>( m_Arrays[type_id].get() )->Insert( std::move( component ) );
        }

        template<typename T>
        T& GetComponent( EntityID entity )
        {
            return static_cast<ComponentArray<T>*>(
                       m_Arrays.at( ComponentTypeRegistry::GetID<T>() ).get()
                   )->GetAt( GetEntityIndex( entity ) );
        }

        template<typename T>
        const T& GetComponent( EntityID entity ) const
        {
            return static_cast<const ComponentArray<T>*>(
                       m_Arrays.at( ComponentTypeRegistry::GetID<T>() ).get()
                   )->GetAt( GetEntityIndex( entity ) );
        }

        void MoveEntityTo( EntityID entity, Archetype* destination );
        void RemoveEntity( EntityID entity );

        bool   HasEntity( EntityID entity ) const;
        size_t GetEntityIndex( EntityID entity ) const;
        size_t GetEntityCount() const;

        ComponentMask                GetMask()     const;
        const std::vector<EntityID>& GetEntities() const;

        bool             HasArray( ComponentTypeID type_id ) const;
        IComponentArray* GetArray( ComponentTypeID type_id );
    };
}

#endif
