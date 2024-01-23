//
// Created by juanb on 20/12/2023.
//

#ifndef PHYSICS_TEST_ACTORS_HPP
#define PHYSICS_TEST_ACTORS_HPP


#include <cstdint>

enum class ActorID : std::uint8_t
{
    None = 0,
    Player,
    Enemy,
    Bullet,
    Floor,
    Wall,
    Map,
    Background,
    Background2,
    Background3,
    Count
};

struct ActorInfo final
{
    ActorID id = ActorID::None;
};

#endif //PHYSICS_TEST_ACTORS_HPP
