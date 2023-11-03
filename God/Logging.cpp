#include "Loggin.h"
#include "ForkedLog.hpp"
#include "StreamLog.hpp"
#include "TextFileLog.hpp"
#include <iostream>

std::shared_ptr<Log> log() {
	static std::shared_ptr<Log> root = std::make_shared<ForkedLog>(
		std::make_shared<StreamLog>(std::cout),
		std::make_shared<TextFileLog>("gamelog.txt")
	);
	return root;
}