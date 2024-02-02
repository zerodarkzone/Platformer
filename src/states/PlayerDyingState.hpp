//
// Created by juanb on 31/1/2024.
//

#ifndef PLAYERDYINGSTATE_HPP
#define PLAYERDYINGSTATE_HPP

#include "PlayerState.hpp"

class PlayerDyingState : public PlayerState
{
public:
    void handleInput(std::uint8_t input) override;

    void fixedUpdate(float dt) override;

    void onEnter() override;

    void onExit() override;

    ~PlayerDyingState() override = default;

    PlayerDyingState(const FSM::StateID id, const cro::Entity entity, cro::MessageBus* mb) : PlayerState(id, entity, mb) {}

private:
    static constexpr float totalReSpawnTime = 5.f;
    float m_reSpawnTime = 0.f;
};

#endif //PLAYERDYINGSTATE_HPP
