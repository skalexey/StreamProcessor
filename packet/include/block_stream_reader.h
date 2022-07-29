// block_stream_reader.h

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
		bool read_and_match(it_t seq, std::size_t size);
		it_t read_n(std::size_t count);
		using read_until_result_t = std::pair<it_t, it_t>;
		read_until_result_t read_until(it_t seq, std::size_t seq_size);
		inline void set_data(it_t data, std::size_t size) {
			m_data = data;
			m_end = data + size;
			m_cur = m_data;
		}
		inline it_t cursor() const {
			return m_cur;
		}
		inline it_t end() const {
			return m_end;
		}
		inline it_t data() const {
			return m_data;
		}
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
		using read_pred_t = std::function<bool()>;
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