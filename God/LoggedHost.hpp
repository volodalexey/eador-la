#pragma once

#include "Host.hpp"
#include "Log.hpp"

class LoggedHost : public Host {
	std::shared_ptr<Log> log;
	std::shared_ptr<Host> origin;
public:
	explicit LoggedHost(std::shared_ptr<Log> log, std::shared_ptr<Host> origin) : log(std::move(log)), origin(std::move(origin)) {}

	void send(std::shared_ptr<Snapshot> snapshot) override {
		log->log("Send snapshot");
		origin->send(std::move(snapshot));
	}

	std::shared_ptr<Snapshot> lastSnapshot() const override {
		log->log("Get last snapshot");
		return origin->lastSnapshot();
	}
};