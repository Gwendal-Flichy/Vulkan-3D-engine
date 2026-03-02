#include "World.h"

#include <cassert>
#include <cstdio>
#include <string>

using namespace VkEngine;

struct Position { float x = 0.f, y = 0.f, z = 0.f; };
struct Velocity  { float dx = 0.f, dy = 0.f, dz = 0.f; };
struct Health    { int   value = 100; };

static int s_Passed = 0;
static int s_Failed = 0;

#define TEST( name, expr )                          \
    do {                                            \
        if( expr ) {                                \
            printf( "[PASS] %s\n", name );          \
            ++s_Passed;                             \
        } else {                                    \
            printf( "[FAIL] %s\n", name );          \
            ++s_Failed;                             \
        }                                           \
    } while( false )


static void Test_CreateDestroy()
{
    World world;

    EntityID e = world.CreateEntity();
    TEST( "CreateEntity returns valid ID",   e != INVALID_ENTITY );
    TEST( "ActiveCount after create is 1",   world.QueryArchetypes( ComponentMask{} ).size() == 0u );

    world.DestroyEntity( e );

    EntityID e2 = world.CreateEntity();
    TEST( "Recycled ID after destroy", e2 != INVALID_ENTITY );

    world.DestroyEntity( e2 );
}

static void Test_AddGetComponent()
{
    World world;

    EntityID e = world.CreateEntity();
    world.AddComponent<Position>( e, { 1.f, 2.f, 3.f } );

    TEST( "HasComponent after add",          world.HasComponent<Position>( e ) );
    TEST( "GetComponent returns correct x",  world.GetComponent<Position>( e ).x == 1.f );
    TEST( "GetComponent returns correct y",  world.GetComponent<Position>( e ).y == 2.f );
    TEST( "GetComponent returns correct z",  world.GetComponent<Position>( e ).z == 3.f );

    world.DestroyEntity( e );
}

static void Test_MultipleComponents()
{
    World world;

    EntityID e = world.CreateEntity();
    world.AddComponent<Position>( e, { 5.f, 0.f, 0.f } );
    world.AddComponent<Velocity>( e, { 1.f, 2.f, 3.f } );
    world.AddComponent<Health>(   e, { 80 }              );

    TEST( "Has Position",                    world.HasComponent<Position>( e ) );
    TEST( "Has Velocity",                    world.HasComponent<Velocity>( e ) );
    TEST( "Has Health",                      world.HasComponent<Health>( e ) );
    TEST( "Position preserved after multi",  world.GetComponent<Position>( e ).x == 5.f );
    TEST( "Velocity preserved after multi",  world.GetComponent<Velocity>( e ).dx == 1.f );
    TEST( "Health preserved after multi",    world.GetComponent<Health>( e ).value == 80 );

    world.DestroyEntity( e );
}

static void Test_RemoveComponent()
{
    World world;

    EntityID e = world.CreateEntity();
    world.AddComponent<Position>( e, { 3.f, 0.f, 0.f } );
    world.AddComponent<Velocity>( e, { 1.f, 0.f, 0.f } );

    world.RemoveComponent<Velocity>( e );

    TEST( "HasComponent false after remove", !world.HasComponent<Velocity>( e ) );
    TEST( "Position preserved after remove", world.GetComponent<Position>( e ).x == 3.f );

    world.DestroyEntity( e );
}

static void Test_ArchetypeSharing()
{
    World world;

    EntityID e1 = world.CreateEntity();
    EntityID e2 = world.CreateEntity();

    world.AddComponent<Position>( e1, { 1.f, 0.f, 0.f } );
    world.AddComponent<Position>( e2, { 2.f, 0.f, 0.f } );

    auto archetypes = world.QueryArchetypes( world.BuildMask<Position>() );

    TEST( "Same archetype for same mask",    archetypes.size() == 1u );
    TEST( "Archetype holds 2 entities",      archetypes[0]->GetEntityCount() == 2u );
    TEST( "e1 position correct",             world.GetComponent<Position>( e1 ).x == 1.f );
    TEST( "e2 position correct",             world.GetComponent<Position>( e2 ).x == 2.f );

    world.DestroyEntity( e1 );
    world.DestroyEntity( e2 );
}

static void Test_SwapPopIntegrity()
{
    World world;

    EntityID e1 = world.CreateEntity();
    EntityID e2 = world.CreateEntity();
    EntityID e3 = world.CreateEntity();

    world.AddComponent<Position>( e1, { 1.f, 0.f, 0.f } );
    world.AddComponent<Position>( e2, { 2.f, 0.f, 0.f } );
    world.AddComponent<Position>( e3, { 3.f, 0.f, 0.f } );

    world.DestroyEntity( e2 );

    TEST( "e1 position intact after middle destroy", world.GetComponent<Position>( e1 ).x == 1.f );
    TEST( "e3 position intact after middle destroy", world.GetComponent<Position>( e3 ).x == 3.f );

    world.DestroyEntity( e1 );
    world.DestroyEntity( e3 );
}

static void Test_QueryArchetypes()
{
    World world;

    EntityID e1 = world.CreateEntity();
    EntityID e2 = world.CreateEntity();

    world.AddComponent<Position>( e1, {} );
    world.AddComponent<Velocity>( e1, {} );

    world.AddComponent<Position>( e2, {} );

    auto pos_vel = world.QueryArchetypes( world.BuildMask<Position, Velocity>() );
    auto pos_only = world.QueryArchetypes( world.BuildMask<Position>() );

    TEST( "Query Position+Velocity finds 1 archetype", pos_vel.size() == 1u );
    TEST( "Query Position finds 2 archetypes",         pos_only.size() == 2u );

    world.DestroyEntity( e1 );
    world.DestroyEntity( e2 );
}

static void Test_System()
{
    class MovementSystem : public System
    {
    public:
        int UpdateCallCount = 0;

        void OnUpdate( float dt, const std::vector<Archetype*>& archetypes ) override
        {
            for( Archetype* arch : archetypes )
            {
                const std::vector<EntityID>& entities = arch->GetEntities();

                for( EntityID entity : entities )
                {
                    Position& pos = arch->GetComponent<Position>( entity );
                    Velocity& vel = arch->GetComponent<Velocity>( entity );

                    pos.x += vel.dx * dt;
                    pos.y += vel.dy * dt;
                    pos.z += vel.dz * dt;
                }
            }
            ++UpdateCallCount;
        }
    };

    World world;

    auto* sys = world.RegisterSystem<MovementSystem>();
    sys->SetRequiredMask( world.BuildMask<Position, Velocity>() );

    EntityID e = world.CreateEntity();
    world.AddComponent<Position>( e, { 0.f, 0.f, 0.f } );
    world.AddComponent<Velocity>( e, { 1.f, 0.f, 0.f } );

    world.Update( 1.f );

    TEST( "System OnUpdate called once",      sys->UpdateCallCount == 1 );
    TEST( "System moved entity position",     world.GetComponent<Position>( e ).x == 1.f );

    world.Update( 1.f );

    TEST( "System accumulated position",      world.GetComponent<Position>( e ).x == 2.f );

    world.DestroyEntity( e );
}

static void Test_MoveEntityToIntegrity()
{
    World world;

    EntityID e = world.CreateEntity();
    world.AddComponent<Position>( e, { 7.f, 8.f, 9.f } );
    world.AddComponent<Velocity>( e, { 1.f, 2.f, 3.f } );

    world.RemoveComponent<Velocity>( e );

    TEST( "Position.x intact after archetype move", world.GetComponent<Position>( e ).x == 7.f );
    TEST( "Position.y intact after archetype move", world.GetComponent<Position>( e ).y == 8.f );
    TEST( "Position.z intact after archetype move", world.GetComponent<Position>( e ).z == 9.f );

    world.DestroyEntity( e );
}

int main()
{
    printf( "=== ECS Tests ===\n\n" );

    Test_CreateDestroy();
    Test_AddGetComponent();
    Test_MultipleComponents();
    Test_RemoveComponent();
    Test_ArchetypeSharing();
    Test_SwapPopIntegrity();
    Test_QueryArchetypes();
    Test_System();
    Test_MoveEntityToIntegrity();

    printf( "\n=== Results: %d passed, %d failed ===\n", s_Passed, s_Failed );

    return ( s_Failed == 0 ) ? 0 : 1;
}
