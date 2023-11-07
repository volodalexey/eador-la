#pragma once

#include "State.hpp"
#include "Log.hpp"

class LoggedState : public State {
	std::shared_ptr<Log> log;
	std::shared_ptr<State> origin;
public:
	explicit LoggedState(std::shared_ptr<Log> log, std::shared_ptr<State> origin)
		: log(std::move(log)), origin(std::move(origin)) {}

	std::shared_ptr<Snapshot> snapshot() const override {
		log->log("Create snapshot");
		return origin->snapshot();
	}

	void applay(std::shared_ptr<Snapshot> snapshot) override {
		log->log("Applay snapshot");
		origin->applay(snapshot);
	}
};