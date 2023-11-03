#pragma once

#include "Log.hpp"
#include "WithVector.hpp"
#include <vector>
#include <memory>

class ForkedLog : public Log, private WithVector<std::shared_ptr<Log>> {
public:
    using WithVector::WithVector;

    void log(const std::string& message) override {
        for (const auto& log : items) {
            log->log(message);
        }
    }
};
