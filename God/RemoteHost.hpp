#pragma once

#include "Host.hpp"
#include "MemSnapshot.hpp"
#include "cpp-httplib/httplib.h"

class RemoteHost : public Host {
	mutable httplib::Client client;
	std::string snapshotPath;
public:
	RemoteHost(httplib::Client client, std::string snapshotPath)
		: client(std::move(client)), snapshotPath(std::move(snapshotPath)) {}

	RemoteHost(std::string host, std::string gameId)
		: RemoteHost(httplib::Client(host), "/game/" + gameId + "/snapshot") {}

	void send(std::shared_ptr<Snapshot> snapshot) override {
		if (not client.Post(snapshotPath, snapshot->bytes(), "application/octet-stream"))
			throw std::runtime_error("Failed while sending snapshot");
	}

	std::shared_ptr<Snapshot> lastSnapshot() const override {
		auto result = client.Get(snapshotPath);
		if (not result) throw std::runtime_error("Failed while downloading snapshot");
		return std::make_shared<MemSnapshot>(std::move(result.value().body));
	}
};