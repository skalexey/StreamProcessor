// Receiver.h

#pragma once

#include <list>
#include <vector>
#include <memory>
#include <optional>
#include "ReceiverInterface.h"
#include "CallbackInterface.h"
#include "packet_parser.h"

// stream processor namespace
namespace stp
{
	struct Receiver : protected ReceiverInterface
	{
		using data_t = char;
		using it_t = const data_t*;
		// Public interface
		void Receive(it_t data, std::size_t size) override;
		void SetOnReceive(ICallback* cb);

	private:
		packet_parser m_parser;
		ICallback* m_on_receive = nullptr;
	};
}