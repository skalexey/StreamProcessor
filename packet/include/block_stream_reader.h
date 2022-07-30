// block_stream_reader.h

// This class is used to read chunks in a sequence of data blocks.
// New block is passed using set_data() method.
// When the end of a block is reached before any task is performed
// the remaining byte count to process is stored into m_remain variable. 
// Tasks which return data store the current block into the internal 
// buffer in this case, then compose it with new blocks and return it in the end.
// The 
#pragma once
#include <cstddef>
#include <utility>
#include <vector>
#include <list>
#include <functional>

namespace stp
{
	class block_stream_reader
	{
	public:
		using data_t = char;
		using it_t = const data_t*;

		// Public interface
		// Move cursor forward by <count> bytes.
		// If the end is reached, the remaining byte count is returned.
		std::size_t advance(std::size_t count = 1);

		// Try to read <size> bytes and compare with data in seq.
		// Returns false if any byte different from the sequence, 
		// or the end is reached before the all <size> bytes are read.
		// Automatically continues reading with the same arguments
		// until the task is done.
		bool read_and_match(it_t seq, std::size_t size);

		// Try to read <count> bytes and return the data.
		// Returns nullptr if the end of a block is reached
		// before the data is read.
		// Automatically continues reading with the same arguments
		// until the task is done and returns the data in the end.
		it_t read_n(std::size_t count);

		using read_until_result_t = std::pair<it_t, it_t>;
		// Try to read data until the given sequence <seq> is reached.
		// Returns nullptr if the end of a block is reached
		// before the data is read.
		// Automatically continues reading with the same arguments
		// until the task is done and returns the data in the end.
		read_until_result_t read_until(it_t seq, std::size_t seq_size);

		// Receives a new data block.
		inline void set_data(it_t data, std::size_t size) {
			m_data = data;
			m_end = data + size;
			m_cur = m_data;
		}

		// Get current position of any read task.
		inline it_t cursor() const {
			return m_cur;
		}
		// Return ponter to end of the passed data block.
		inline it_t end() const {
			return m_end;
		}
		// Return ponter to begin of the passed data block.
		inline it_t data() const {
			return m_data;
		}
		// Reset all pointers of a passed block, and clear stored data.
		inline void reset() {
			m_data = m_end = m_cur = nullptr;
			m_remain = 0;
			clear_buffers();
		}
		inline void clear_buffers() {
			m_buf.clear();
			m_endless_storage.clear();
		}
	protected:
		// Store the data and don't allow to use it again
		using buf_t = std::vector<data_t>;
		void inline store_data(it_t begin, it_t end, std::size_t buf_cap) {
			if (m_buf.empty())
				m_buf.reserve(buf_cap);
			m_buf.insert(m_buf.end(), begin, end);
			m_cur = end;
		}
		read_until_result_t compose_data(it_t begin, it_t end);

	private:
		std::size_t m_remain = 0;
		it_t m_cur = nullptr;
		it_t m_data = nullptr;
		it_t m_end = nullptr;
		buf_t m_buf;
		std::list<buf_t> m_endless_storage;
	};
}