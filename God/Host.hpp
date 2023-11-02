#pragma once

#include "BaseInterface.hpp"
#include "Snapshot.hpp"
#include <string>
#include <memory>

class Host : public BaseInterface {
public:
	virtual void send(std::shared_ptr<Snapshot> snapshot) = 0;
	virtual std::shared_ptr<Snapshot> lastSnapshot() const = 0;
};