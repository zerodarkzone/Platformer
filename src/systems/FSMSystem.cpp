//
// Created by juanb on 11/1/2024.
//

#include "FSMSystem.hpp"

void FiniteStateMachine::pushState(FSM::State_t id, cro::Entity& entity)
{
	if (m_factories.count(id) == 0)
	{
		return;
	}
	if (!m_states.empty())
	{
		m_prevState = getCurrentStateID();
	}
	auto state = m_factories[id]();
	state->onEnter(entity);
	m_states.emplace(std::move(state));
}

std::unique_ptr<BaseState> FiniteStateMachine::popState(cro::Entity& entity)
{
	if (m_states.empty())
	{
		return nullptr;
	}
	m_prevState = getCurrentStateID();
	auto state = std::move(m_states.top());
	m_states.pop();
	state->onExit(entity);

	return state;
}

void FiniteStateMachine::clearStates()
{
	while (!m_states.empty())
	{
		m_states.pop();
	}
}

void FiniteStateMachine::changeState(FSM::State_t id, cro::Entity& entity)
{
	popState(entity);
	pushState(id, entity);
}

FiniteStateMachineSystem::FiniteStateMachineSystem(cro::MessageBus& mb) : cro::System(mb, typeid(FiniteStateMachineSystem))
{
	requireComponent<FiniteStateMachine>();
}

void FiniteStateMachineSystem::handleMessage(const cro::Message& msg)
{
	System::handleMessage(msg);
}

void FiniteStateMachineSystem::process(float dt)
{
	auto& entities = getEntities();
	for (auto& entity : entities)
	{
		auto& fsm = entity.getComponent<FiniteStateMachine>();
		if (fsm.getCurrentState() != nullptr)
		{
			fsm.getCurrentState()->update(entity, dt);
		}
	}
}

void FiniteStateMachineSystem::fixedUpdate(float dt)
{
	auto& entities = getEntities();
	for (auto& entity : entities)
	{
		auto& fsm = entity.getComponent<FiniteStateMachine>();
		if (fsm.getCurrentState() != nullptr)
		{
			fsm.getCurrentState()->fixedUpdate(entity, dt);
		}
	}
}