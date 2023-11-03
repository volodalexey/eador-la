#include "GlobalCrutches.h"
#include "Logging.h"
#include <iostream>

namespace {
	struct SecondMain {
		SecondMain() {
			log()->log("Just Fun!");
		}
	} lifeHack;
}

void onBeforeStep() {
	log()->log("Start step");
}

void onAfterStep() {
	log()->log("End step");
}
