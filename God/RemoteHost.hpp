#pragma once

#include "Host.hpp"
#include "MemSnapshot.hpp"
#include "cpp-httplib/httplib.h"

class Path : public std::string {
public:
	using std::string::basic_string;
};

class RemoteHost : public Host {
	mutable httplib::Client client;
	Path snapshotPath;
public:
	RemoteHost(httplib::Client client, Path snapshotPath)
		: client(std::move(client)), snapshotPath(std::move(snapshotPath)) {}

	RemoteHost(httplib::Client client, std::string gameId)
		: RemoteHost(std::move(client), Path("/game/" + gameId + "/snapshot")) {}

	RemoteHost(std::string host, std::string gameId)
		: RemoteHost(httplib::Client(host), std::move(gameId)) {}

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