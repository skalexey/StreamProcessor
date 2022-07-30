// packet_parser.h

// This class is aimed to retrieve packets from a stream of data blocks.
// Packets are returned in the form of stp::packet class.
// Work with data stream is deligated to m_reader of block_stream_reader class.

#pragma once
#include <cstddef>
#include "block_stream_reader.h"
#include "packet.h"

namespace stp
{
	class packet_parser
	{
	public:
		using data_t = char;
		using it_t = const data_t*;
		// Public interface
		packet::ptr_t parse();
		inline void set_data(it_t data, std::size_t size) {
			m_reader.set_data(data, size);
		}

	protected:
		enum parse_task  : int
		{
			NONE,
			PACKET_HEADER,
			BIN_PACKET_SIZE,
			TEXT_PACKET_DATA,
			BIN_PACKET_DATA,
			COUNT
		};

		packet::ptr_t do_parse_task(parse_task task = parse_task::NONE);

	private:
		block_stream_reader m_reader;
		parse_task m_task = parse_task::NONE;
		std::size_t m_packet_size = 0;
	};
}
