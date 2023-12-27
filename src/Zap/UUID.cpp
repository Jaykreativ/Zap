#include "Zap/UUID.h"
namespace Zap {
	UUID::UUID() {
		m_uuid = 1;// TODO random number generator
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