// packet_parser.cpp

#include <string>
#include <bit>
#include "packet_parser.h"

namespace
{
	using namespace stp;

	const char bin_packet_header = 0x24;
	const char bin_packet_size_width = 4;
	const char* text_packet_end_seq = "\r\n\r\n";
	const std::size_t text_packet_end_seq_size = strlen(text_packet_end_seq);

	std::size_t parse_packet_size(packet_parser::it_t first, packet_parser::it_t last)
	{
		return std::byteswap(*reinterpret_cast<const unsigned short*>(first));
	}
}

namespace stp
{
	packet::ptr_t packet_parser::parse()
	{
		if (m_reader.data() == m_reader.end())
			return nullptr;

		packet::ptr_t ret(nullptr);

		// Work with reader

		if (m_task == parse_task::NONE)
		{
			m_reader.clear_buffers();
			m_task = parse_task::PACKET_HEADER;
		}

		if (auto packet = do_parse_task())
			return std::move(packet);
		else
			return nullptr;
	}

	packet::ptr_t packet_parser::do_parse_task(parse_task task)
	{
		if (task != parse_task::NONE)
			m_task = task;

		switch (m_task)
		{
		case parse_task::PACKET_HEADER:
		{
			data_t header = 0;
			if (m_reader.cursor() != m_reader.end())
			{
				if (*m_reader.cursor() == bin_packet_header)
				{
					m_reader.advance();
					return do_parse_task(parse_task::BIN_PACKET_SIZE);
				}
				else
					return do_parse_task(parse_task::TEXT_PACKET_DATA);
			}
			break;
		}
		case parse_task::BIN_PACKET_SIZE:
			if (auto&& data = m_reader.read_n(bin_packet_size_width))
			{
				m_packet_size = parse_packet_size(data, data + bin_packet_size_width);
				return do_parse_task(parse_task::BIN_PACKET_DATA);
			}
			break;
		case parse_task::TEXT_PACKET_DATA:
		{
			auto r = m_reader.read_until(text_packet_end_seq, text_packet_end_seq_size);
			if (r.first != nullptr)
			{
				m_task = parse_task::NONE;
				return std::make_unique<packet>(r.first, r.second, false);
			}
			break;
		}
		case parse_task::BIN_PACKET_DATA:
			if (auto&& data = m_reader.read_n(m_packet_size))
			{
				m_task = parse_task::NONE;
				return std::move(std::make_unique<packet>(data, data + m_packet_size, true));
			}
			break;
		default:
			break;
		}

		return nullptr;
	}
}