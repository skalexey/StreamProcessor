// packet.h

#pragma once

#include <memory>
#include <cstddef>

namespace stp
{
	struct packet
	{
	public:
		using ptr_t = std::unique_ptr<packet>;
		using data_t = const char;
		using it_t = data_t*;
		// Constructors
		packet(it_t data_begin, it_t data_end, bool is_bin)
			: m_data(data_begin), m_end(data_end), m_is_bin(is_bin) {}

		// Binary packet property
		inline bool is_bin() const {
			return m_is_bin;
		}
		// Data access
		inline it_t data() const {
			return m_data;
		}
		inline std::size_t size() const {
			return (m_data && m_end) ? (m_end - m_data) : 0;
		}

	protected:
		it_t m_data = nullptr;
		it_t m_end = nullptr;
		bool m_is_bin = false;
	};
}