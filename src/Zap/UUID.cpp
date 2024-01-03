#include "Zap/UUID.h"
#include <random>

namespace Zap {
	UUID::UUID() {
		std::random_device rd;
		std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
		m_uuid = dist(rd);
	}

	UUID::UUID(uint64_t id)
		: m_uuid(id)
	{}

	UUID::operator uint64_t() const {
		return m_uuid;
	}

	void UUID::operator=(uint64_t id) {
		m_uuid = id;
	}
}