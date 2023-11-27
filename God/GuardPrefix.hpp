#pragma once

#include <string>
#include <iostream>
#include <format>
#include <stdexcept>

namespace binary {
	class GuardPrefix {
		std::string prefix;
	public:
		explicit GuardPrefix(std::string prefix) : prefix(std::move(prefix)) {}

		template <class T>
		static GuardPrefix named() { return GuardPrefix(typeid(T).name()); }

		friend auto& operator<<(std::ostream& out, const GuardPrefix& guard) {
			return out.write(guard.prefix.c_str(), guard.prefix.size() + 1);
		}

		friend auto& operator>>(std::istream& in, const GuardPrefix& guard) {
			auto prefix = guard.prefix;
			in.read(prefix.data(), prefix.size() + 1);
			if (prefix != guard.prefix)
				throw std::runtime_error(std::format("Failed to read prefix {}, got {}", guard.prefix, prefix));
			return in;
		}
	};
}
