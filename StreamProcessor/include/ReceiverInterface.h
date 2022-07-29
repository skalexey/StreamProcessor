// ReceiverInterface.h

#pragma once
#include <cstddef>

struct ReceiverInterface
{
	virtual void Receive(const char* data, std::size_t size) = 0;
};
