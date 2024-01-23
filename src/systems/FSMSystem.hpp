//
// Created by juanb on 11/1/2024.
//

#ifndef PHYSICS_TEST_FINITESTATEMACHINESYSTEM_HPP
#define PHYSICS_TEST_FINITESTATEMACHINESYSTEM_HPP

#include <stack>
#include <cstdint>
#include <map>
#include <functional>
#include <crogine/ecs/System.hpp>

namespace FSM
{
    using StateID = std::int32_t;

    enum State: StateID
    {
        None,
        Count
    };
}

class BaseState
{
public:
    virtual void handleInput(std::uint8_t input) = 0;

    virtual void update(float dt) = 0;

    virtual void fixedUpdate(float dt) = 0;

    virtual void onEnter() = 0;

    virtual void onExit() = 0;

    [[nodiscard]] virtual FSM::StateID getStateID() const { return m_id; }

    virtual ~BaseState() = default;

    explicit BaseState(FSM::StateID id, cro::Entity entity) : m_id(id), m_entity(entity) {}

    BaseState() = delete;

protected:
    FSM::StateID m_id = FSM::State::None;
    cro::Entity m_entity;
};

class FiniteStateMachine
{
public:
    FiniteStateMachine() = default;

    FiniteStateMachine(const FiniteStateMachine&) = delete;

    const FiniteStateMachine& operator=(const FiniteStateMachine&) = delete;

    FiniteStateMachine(FiniteStateMachine&&) noexcept = default;

    FiniteStateMachine& operator=(FiniteStateMachine&&) = default;

    template<typename T, typename... Args>
    void registerState(FSM::StateID id, Args&&... args)
    {
        static_assert(std::is_base_of<BaseState, T>::value, "Must derive from State class");
        m_factories[id] = [id, args=std::make_tuple<Args...>(std::forward<Args>(args)...)]() {
            return std::apply([id](auto&&... args_) { return std::make_unique<T>(id, args_...); },
                              std::move(args));
        };
    }

    void pushState(FSM::StateID id);

    std::unique_ptr<BaseState> popState();

    void clearStates();

    void changeState(FSM::StateID id);

    [[nodiscard]] BaseState* getCurrentState() const { return isEmpty() ? nullptr : m_states.top().get(); }

    [[nodiscard]] FSM::StateID getCurrentStateID() const
    {
        return getCurrentState() ? getCurrentState()->getStateID() : static_cast<FSM::StateID>(FSM::State::None);
    }

    [[nodiscard]] FSM::StateID getPrevStateID() const { return m_prevState; }
    [[nodiscard]] bool isEmpty() const { return m_states.empty(); }
    [[nodiscard]] std::size_t getSize() const { return m_states.size(); }
    [[nodiscard]] bool hasState(const FSM::StateID id) const { return m_factories.contains(id); }

private:
    std::stack<std::unique_ptr<BaseState>> m_states;
    std::map<FSM::StateID, std::function<std::unique_ptr<BaseState>(void)>> m_factories;
    FSM::StateID m_prevState = FSM::State::None;
};


class FiniteStateMachineSystem final : public cro::System
{
public:
    explicit FiniteStateMachineSystem(cro::MessageBus& mb);

    void handleMessage(const cro::Message&) override;

    void process(float dt) override;

    void fixedUpdate(float dt);
};


#endif //PHYSICS_TEST_FINITESTATEMACHINESYSTEM_HPP
