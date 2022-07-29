// Receiver.cpp

#include <algorithm>
#include "Receiver.h"

namespace stp
{
	void Receiver::SetOnReceive(ICallback* cb)
	{
		m_on_receive = cb;
	}

	void Receiver::Receive(it_t data, std::size_t size)
	{
		m_parser.set_data(data, size);
		while (auto&& packet = m_parser.parse())
		{
			if (m_on_receive)
			{
				if (packet->is_bin())
					m_on_receive->BinaryPacket(packet->data(), packet->size());
				else
					m_on_receive->TextPacket(packet->data(), packet->size());
			}
		}
	}
}