//
// Created by juanb on 11/1/2024.
//

#ifndef PHYSICS_TEST_FINITESTATEMACHINESYSTEM_HPP
#define PHYSICS_TEST_FINITESTATEMACHINESYSTEM_HPP

#include <stack>
#include <memory>
#include <cstdint>
#include <map>
#include <functional>
#include <crogine/ecs/System.hpp>

namespace FSM
{
	typedef std::uint8_t State_t;
	enum State: State_t
	{
		None,
		Count
	};
}

namespace cro
{
	class Entity;
}

class BaseState
{
public:
	virtual void handleInput(cro::Entity& entity, std::uint8_t input) = 0;
	virtual void update(cro::Entity& entity, float dt) = 0;
	virtual void fixedUpdate(cro::Entity& entity, float dt) = 0;
	virtual void onEnter(cro::Entity& entity) = 0;
	virtual void onExit(cro::Entity& entity) = 0;
	[[nodiscard]] virtual FSM::State_t getStateID() const { return m_id; }

	virtual ~BaseState() = default;
	explicit BaseState(FSM::State_t id) : m_id(id) {}
	BaseState() = delete;
protected:
	float m_desiredSpeed = 0.f;
	FSM::State_t m_id = FSM::State::None;
};

class FiniteStateMachine
{
public:
	FiniteStateMachine() = default;
	FiniteStateMachine(const FiniteStateMachine&) = delete;
	const FiniteStateMachine& operator=(const FiniteStateMachine&) = delete;
	FiniteStateMachine(FiniteStateMachine&&)  noexcept = default;
	FiniteStateMachine& operator=(FiniteStateMachine&&) = default;

	template <typename T, typename... Args>
	void registerState(FSM::State_t id, Args&&... args)
	{
		static_assert(std::is_base_of<BaseState, T>::value, "Must derive from State class");
		m_factories[id] = [&args...]()
		{
			return std::make_unique<T>(std::forward<Args>(args)...);
		};
	}
	void pushState(FSM::State_t id, cro::Entity& entity);
	std::unique_ptr<BaseState> popState(cro::Entity& entity);
	void clearStates();
	void changeState(FSM::State_t id, cro::Entity& entity);
	[[nodiscard]] BaseState* getCurrentState() const { return isEmpty() ? nullptr : m_states.top().get(); }
	[[nodiscard]] FSM::State_t getCurrentStateID() const { return getCurrentState() == nullptr ? static_cast<FSM::State_t>(FSM::State::None) : getCurrentState()->getStateID(); }
	[[nodiscard]] FSM::State_t getPrevStateID() const { return m_prevState; }
	[[nodiscard]] bool isEmpty() const { return m_states.empty(); }
	[[nodiscard]] std::size_t getSize() const { return m_states.size(); }
	[[nodiscard]] bool hasState(FSM::State_t id) const { return m_factories.count(id) > 0; }
private:
	std::stack<std::unique_ptr<BaseState>> m_states;
	std::map<FSM::State_t, std::function<std::unique_ptr<BaseState>(void)>> m_factories;
	FSM::State_t m_prevState = FSM::State::None;
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
