// block_stream_reader.cpp

#include <algorithm>
#include <cassert>
#include "block_stream_reader.h"

namespace stp
{
	std::size_t block_stream_reader::advance(std::size_t count)
	{
		auto way = std::min(count, std::size_t(m_end - m_cur));
		m_cur += way;
		return count - way;
	}

	block_stream_reader::it_t block_stream_reader::read_n(std::size_t count)
	{
		auto read_from = m_cur;

		if (m_remain == 0)
		{
			m_remain = count;
			m_buf.clear();
		}
		
		auto way = std::min(std::size_t(m_end - m_cur), m_remain);
		m_cur += way;
		m_remain -= way;

		if (m_remain > 0)
		{	// End is reached, but the task is not completed.
			// Store the data to the buffer.
			assert(m_cur == m_end);
			store_data(read_from, m_end, count);
			return nullptr;
		}
		else // m_remain is unsigned value, so it is 0 in this case
		{
			if (m_buf.empty()) // Return without copying
				return read_from;
			else
			{	// Add the remaining part to the current buffer and return
				store_data(read_from, m_cur, count);
				return m_buf.data();
			}
		}
	}

	bool block_stream_reader::read_and_match(it_t seq, std::size_t size)
	{
		if (m_remain == 0)
			m_remain = size;
		for (; m_cur != m_end && m_remain > 0; ++m_cur)
		{
			if (*m_cur != *(seq + (size - m_remain)))
			{
				m_remain = 0;
				return false;
			}
			m_remain--;
		}
		return m_remain == 0;
	}
	
	block_stream_reader::read_until_result_t block_stream_reader::read_until(it_t seq, std::size_t seq_size)
	{
		auto read_from = m_cur;
		do
		{
			// Search for the first symbol of the sequence
			if ((m_cur = std::find(m_cur, m_end, *seq)) == m_end)
				break;
			// The cursor is on the sequence now. Can read it
			if (read_and_match(seq, seq_size))
			{
				auto r = compose_data(read_from, m_cur);
				return read_until_result_t(r.first, r.second - seq_size);
			}
		} while (m_cur != m_end);
		// End is reached and nothing found.
		// Store the data and continue with the next block
		store_data(read_from, m_end, m_end - read_from);
		m_endless_storage.emplace_back();
		m_buf.swap(m_endless_storage.back());
		return read_until_result_t(nullptr, nullptr);
	}

	block_stream_reader::read_until_result_t block_stream_reader::compose_data(it_t begin, it_t end)
	{
		if (m_endless_storage.empty())
			return read_until_result_t(begin, end);

		// Return the stored data reallocated in a single sequential block
		if (!m_buf.empty())
			m_buf.clear();
		// Reserve total size for the buffer first
		std::size_t size = 0;
		for (auto&& block : m_endless_storage)
			size += block.size();
		m_buf.reserve(size);
		// Now fill the buffer
		for (auto&& block : m_endless_storage)
			m_buf.insert(m_buf.end(), block.begin(), block.end());
		// Clear intermediate buffer
		m_endless_storage.clear();
		// Insert the last part
		if (begin && end)
			m_buf.insert(m_buf.end(), begin, end);
		return read_until_result_t(m_buf.data(), m_buf.data() + m_buf.size());
	}
}
