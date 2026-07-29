#include "ai/fsm.h"

#include <algorithm>
#include <stdexcept>

namespace reaper::ai::fsm
{

State::State(int state_id, std::size_t transition_capacity)
	: id(state_id),
	  max_transitions(std::max<std::size_t>(transition_capacity, 1))
{
	transitions.reserve(max_transitions);
}

void State::add_transition(int input, int output_state)
{
	const auto existing = std::find_if(
		transitions.begin(),
		transitions.end(),
		[input](const auto& transition) {
			return transition.first == input;
		});
	if (existing != transitions.end()) {
		existing->second = output_state;
		return;
	}

	if (transitions.size() >= max_transitions)
		throw std::length_error("FSM state transition capacity exceeded");
	transitions.emplace_back(input, output_state);
}

void State::delete_transition(int output_state)
{
	transitions.erase(
		std::remove_if(
			transitions.begin(),
			transitions.end(),
			[output_state](const auto& transition) {
				return transition.second == output_state;
			}),
		transitions.end());
}

int State::get_output(int input) const
{
	const auto transition = std::find_if(
		transitions.begin(),
		transitions.end(),
		[input](const auto& candidate) {
			return candidate.first == input;
		});
	return transition == transitions.end() ? id : transition->second;
}

State* FSM::get_state(int state_id) noexcept
{
	const auto state = states.find(state_id);
	return state == states.end() ? nullptr : state->second.get();
}

const State* FSM::get_state(int state_id) const noexcept
{
	const auto state = states.find(state_id);
	return state == states.end() ? nullptr : state->second.get();
}

State* FSM::add_state(std::unique_ptr<State> state)
{
	if (!state)
		throw std::invalid_argument("Cannot add a null FSM state");

	const int state_id = state->get_id();
	auto [position, inserted] = states.emplace(state_id, std::move(state));
	return position->second.get();
}

void FSM::delete_state(int state_id)
{
	states.erase(state_id);
	if (current_state == state_id)
		current_state = 0;
}

int FSM::state_transition(int input)
{
	if (State* state = get_state(current_state))
		current_state = state->get_output(input);
	else
		current_state = 0;
	return current_state;
}

}
