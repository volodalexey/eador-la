#include "GlobalCrutches.h"
#include "Logging.h"
#include "Host.hpp"
#include "LoggedHost.hpp"
#include "RemoteHost.hpp"
#include "State.hpp"
#include "LoggedState.hpp"
#include "CurrentGameState.hpp"

namespace {
	std::shared_ptr<Host> host;
	std::shared_ptr<State> state;

	struct SecondMain {
		SecondMain() {
			log()->log("Just Fun!");
			host = std::make_shared<LoggedHost>(log(), std::make_shared<RemoteHost>("localhost:42"));
			state = std::make_shared<LoggedState>(log(), std::make_shared<CurrentGameState>());
		}
	} lifeHack;
}

void onBeforeStep() {
	log()->log("Start step");
}

void onAfterStep() {
	log()->log("End step");
}
