#pragma once

#include <unordered_map>
#include <stdint.h>

namespace Zap {
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t id);

		operator uint64_t() const;

		void operator=(uint64_t id);

	private:
		uint64_t m_uuid = 0;
	};
}

namespace std {
	template<>
	struct hash<Zap::UUID> {
		size_t operator()(const Zap::UUID& key) const {
			return hash<uint64_t>()(key);
		}
	};
}

