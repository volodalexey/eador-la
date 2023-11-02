#pragma once

#include "BaseInterface.hpp"
#include "Snapshot.hpp"
#include <string>
#include <memory>

class State : BaseInterface {
public:
	virtual std::shared_ptr<Snapshot> snapshot() const = 0;
	virtual void applay(std::shared_ptr<Snapshot> snapshot) = 0;
};