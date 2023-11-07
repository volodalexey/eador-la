#pragma once

#include "State.hpp"

class CurrentGameState : public State {
public:
	std::shared_ptr<Snapshot> snapshot() const override;
	void applay(std::shared_ptr<Snapshot> snapshot) override;
};