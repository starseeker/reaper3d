#ifndef REAPER_AI_FSM_H
#define REAPER_AI_FSM_H

#include <cstddef>
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace reaper::ai::fsm
{

class State {
	int id;
	std::size_t max_transitions;
	std::vector<std::pair<int, int>> transitions;

public:
	State(int state_id, std::size_t transition_capacity);

	int get_id() const noexcept { return id; }
	void add_transition(int input, int output_state);
	void delete_transition(int output_state);
	int get_output(int input) const;
};

class FSM {
	using StateMap = std::map<int, std::unique_ptr<State>>;

	int current_state;
	StateMap states;

public:
	explicit FSM(int initial_state)
		: current_state(initial_state)
	{
	}

	int get_current_state() const noexcept { return current_state; }
	void set_current_state(int state_id) noexcept { current_state = state_id; }

	State* get_state(int state_id) noexcept;
	const State* get_state(int state_id) const noexcept;
	State* add_state(std::unique_ptr<State> state);
	void delete_state(int state_id);
	int state_transition(int input);
};

}

#endif
