#include "cpp-httplib/httplib.h"
#include <string>

int main() {
	std::unordered_map<std::string, std::string> games;

	httplib::Server server;
	server.Get("/game/:id/snapshot", [&](const httplib::Request& request, httplib::Response& response) {
		std::cout << "Get:  " << request.path << std::endl;
		const auto &id = request.path_params.at("id");
		
		if (not games.contains(id)) {
			response.status = 404;
			return;
		}

		const auto& snapshot = games.at(id);
		response.set_content(snapshot, "application/octet-stream");
		response.status = 200;
		});

	server.Post("/game/:id/snapshot", [&](const httplib::Request& request, httplib::Response& response) {
		std::cout << "Post: " << request.path << std::endl;
		const auto& id = request.path_params.at("id");
		games[id] = request.body;
		});

	std::cout << "Start server on localhost:42" << std::endl;
	server.listen("localhost", 42);
}
