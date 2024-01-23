//
// Created by juanb on 10/1/2024.
//
#include "PlayerState.hpp"
#include "systems/PlayerSystem.hpp"
#include "directors/InputFlags.hpp"

void PlayerState::handleInput(const std::uint8_t input)
{
    m_desiredSpeed = 0.f;
    auto& player = m_entity.getComponent<Player>();
    if ((input & InputFlag::Left) && !(input & InputFlag::Right))
    {
        player.facing = Player::Facing::Left;
        m_desiredSpeed = -player.speed;
    }
    if ((input & InputFlag::Right) && !(input & InputFlag::Left))
    {
        player.facing = Player::Facing::Right;
        m_desiredSpeed = player.speed;
    }
}
