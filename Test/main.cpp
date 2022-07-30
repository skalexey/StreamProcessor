// main.cpp : Defines the entry point for the application.
//

#include <fstream>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <random>
#include <cassert>
#include <bit>
#include <array>
#include <chrono>
#ifdef __cpp_lib_filesystem
	#include <filesystem>
#endif
#include "Receiver.h"

using block_t = std::vector<char>;

std::vector<std::string> packets_data = {
	"Packet 1",
	"Packet 2\x24 with inner header byte",
	"This is packet 3",
	"Test packet 4"
};

std::vector<std::string> text_blocks = {
	"Text packet 1\r\n\r\n",
	"And this is the text packet 2\r\n\r\n",
	"Packet 3\r\n\r\n",
	"Packet 4\r\n\r\n",
	"PACK 5\r\n\r\n",
	"Packet6: BLOCK 1",
	" Block 2 ",
	"Block 3\r\n\r\n",
	"Packet 7\r\n\r\n",
	"Packet 8\r\n\r",
	"jsdfjiofsd\r\n\r\n",
	"Packet 9\r\n\r\n",
	"Packet 10\r\n\r",
	"\nPacket 11\r",
	"\n contains this: \n\r\n",
	"some data",
	"\r\n\r\n",
	"Packet 12\r\n",
	"\r\nPacket 13\r",
	"\n\r\n",
	"Packet 14\r\n",
	"\r",
	"\n",
	"Packet 15\r\n\r\n",
	"Packet 16\r\n\r\nPacket 17\r\n\r\nPacket 18\r\n\r\n", // 24
	"\r\nPacket 19\r\n\r\nPacket 20",
	"\r\n\r\nPacket 21\r",
	"\n\r\nPacket 22",
	"\r\n\r\n"
};

std::list<block_t> DecomposeData(const block_t& data)
{
	std::list<block_t> ret;
	std::mt19937 gen32;
	auto it = data.begin();
	while (it != data.end())
	{
		std::size_t remaining_size = data.end() - it;
		auto division_point = std::min(std::max(std::size_t(1), gen32() % remaining_size), remaining_size);
		ret.emplace_back(it, it + division_point);
		it += division_point;
	}
	return ret;
}

block_t MakeTextPacket(const std::string& data)
{
	block_t ret;
	ret.resize(data.size() + 4);
	std::string end_seq = "\r\n\r\n";
	for (int i = 0; i < end_seq.size(); i++)
		ret[data.size() + i] = end_seq[i];
	for (int i = 0; i < data.size(); i++)
		ret[i] = data[i];
	return ret;
}

const int bin_pack_size_width = 4;

block_t MakeBinPacketSize(unsigned short size)
{
	auto size_little_endian = std::byteswap(size);
	auto data = reinterpret_cast<unsigned char*>(&size_little_endian);
	block_t ret(data, data + 4);
	return ret;
}

block_t MakeBinPacket(const char* data, std::size_t size)
{
	assert(size <= USHRT_MAX); // The packet size is only 4 byte length
	block_t ret;
	ret.resize(size + 5);
	ret[0] = 0x24;
	auto sz = MakeBinPacketSize(size);
	for (int i = 0; i < sz.size(); i++)
		ret[i + 1] = sz[i];
	for (int i = 0; i < size; i++)
		ret[i + 5] = data[i];
	return ret;
}

block_t PackData(const char* data, std::size_t size)
{
	block_t ret;
	ret.reserve(std::size_t(size * 1.1f));
	std::size_t remaining = size;
	while (remaining > 0)
	{
		auto packet_size = std::max(std::size_t(1), std::min(std::size_t(USHRT_MAX), remaining));
		auto packet = MakeBinPacket(data + size - remaining, packet_size);
		ret.insert(ret.end(), packet.begin(), packet.end());
		remaining -= packet_size;
	}
	return ret;
}

std::vector<block_t> PackDataIntoArray(const char* data, std::size_t size)
{
	std::vector<block_t> ret;
	std::size_t remaining = size;
	while (remaining > 0)
	{
		auto packet_size = std::max(std::size_t(1), std::min(std::size_t(USHRT_MAX), remaining));
		ret.emplace_back(MakeBinPacket(data + size - remaining, packet_size));
		remaining -= packet_size;
	}
	return ret;
}

// Callback for tests with text packets from text_blocks array
class ManualDataCallback : public ICallback
{
	void BinaryPacket(const char* data, std::size_t size) override {
		std::string s(data, size);
		std::cout << "Binary packet with size " << size << " received:\n'" << s << "'\n\n";
	}

	void TextPacket(const char* data, std::size_t size) override {
		std::string s(data, size);
		std::cout << "Text packet with size " << size << " received:\n'" << s << "'\n\n";
	}
};

// Callback for tests with binary and text packets from packets_data
class RandomlyDividedDataCallback : public ICallback
{
	int current_packet_index = 0;

	void BinaryPacket(const char* data, std::size_t size) override {
		std::string s(data, size);
		std::cout << "Binary packet with size " << size << " received:\n'" << s << "'\n\n";
		assert(s == packets_data[current_packet_index++]);
	}

	void TextPacket(const char* data, std::size_t size) override {
		std::string s(data, size);
		std::cout << "Text packet with size " << size << " received:\n'" << s << "'\n\n";
		assert(s == packets_data[current_packet_index++]);
	}
};

void SendBlock(stp::Receiver& r, const std::string& s, bool log_data = true)
{
	std::cout << "	Send block with size " << s.size() << "\n";
	if (log_data)
		std::cout << ": '" << s << "'\n";
	r.Receive(s.data(), s.size());
}

void SendBlock(stp::Receiver& r, const char* data, std::size_t size, bool log_data = true)
{
	std::cout << "	Send block with size " << size << "\n";
	if (log_data)
		std::cout << ": '" << std::string(data, size) << "'\n";
	r.Receive(data, size);
}

void TextPacketsTest()
{
	using namespace stp;
	std::cout << "Text Packets Test\n";
	Receiver r;
	ManualDataCallback c;
	r.SetOnReceive(&c);

	for (int i = 0; i < text_blocks.size(); i++)
		SendBlock(r, text_blocks[i].data());
}

block_t CollectPacketsData(bool text = false)
{
	block_t all_in_one;
	for (int i = 0; i < packets_data.size(); i++)
	{
		auto packet = text ?
			MakeTextPacket(packets_data[i])
			: MakeBinPacket(packets_data[i].data(), packets_data[i].size())
		;
		all_in_one.insert(all_in_one.end(), packet.begin(), packet.end());
	}
	return all_in_one;
}

void TextPacketsTest2()
{
	using namespace stp;
	std::cout << " === Text packets random block decomposition test === \n";
	Receiver r;
	RandomlyDividedDataCallback c;
	r.SetOnReceive(&c);
	auto blocks = DecomposeData(CollectPacketsData(true));
	for (auto&& block : blocks)
		SendBlock(r, block.data(), block.size());

	std::cout << " === TEST FINISHED === \n";
}

void BinPacketsTest1()
{
	using namespace stp;
	std::cout << " === Bin Packets Test === \n";
	Receiver r;
	RandomlyDividedDataCallback c;
	r.SetOnReceive(&c);

	for (int i = 0; i < packets_data.size(); i++)
	{
		auto packet = MakeBinPacket(packets_data[i].data(), packets_data[i].size());
		SendBlock(r, packet.data(), packet.size());
	}
}

void BinPacketsTest2()
{
	using namespace stp;
	std::cout << " === Bin packets Random block decomposition test ===\n";
	Receiver r;
	RandomlyDividedDataCallback c;
	r.SetOnReceive(&c);

	auto blocks = DecomposeData(CollectPacketsData(false));
	for (auto&& block : blocks)
		SendBlock(r, block.data(), block.size());

	std::cout << " === TEST FINISHED === \n";
}

inline block_t file_contents(const std::string& fpath)
{
	std::ifstream f(fpath);
	return block_t((std::istreambuf_iterator<char>(f)),
		(std::istreambuf_iterator<char>()));
}

class FileTestCallback : public ICallback
{
	void BinaryPacket(const char* data, std::size_t size) override {
		std::string s(data, size);
		std::string fname = "test_copy.jpg";
		std::cout << "Binary packet with size " << size << " received. Put it in the file '" << fname << "'\n";
		std::ofstream fout(fname, std::ios::binary | std::ios::app);
		fout.write(data, size);
	}

	void TextPacket(const char* data, std::size_t size) override {
	}
};

void FileTest()
{
	using namespace stp;
	std::cout << " === File test ===";
	std::string fname = "test.jpg";
	auto file = std::ifstream(fname, std::ios::binary);
	std::cout << "	Load file '" << fname << "' from the run directory\n";
	if (!file.is_open())
	{
		std::cout << "Error: Can't open file '" << fname << "'\n";
#ifdef __cpp_lib_filesystem
		std::cout << "The file is searched by the path '" << (std::filesystem::current_path() / fname).string() << "'\n";
#endif
		return;
	}

	std::string fname_out = "test_copy.jpg";
	std::ofstream fout(fname_out, std::ios::binary | std::ios::trunc);
	fout.close();

	block_t data((std::istreambuf_iterator<char>(file)),
		(std::istreambuf_iterator<char>()));
	auto blocks = DecomposeData(PackData(data.data(), data.size()));

	auto begin = std::chrono::steady_clock::now();
	Receiver r;
	FileTestCallback c;
	r.SetOnReceive(&c);
	for (auto it = blocks.begin(); it != blocks.end(); ++it)
		SendBlock(r, it->data(), it->size(), false);
	auto end = std::chrono::steady_clock::now();
	auto dur = end - begin;
	std::cout << "The test test took " << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << "ms\n";
}

int main(int argc, char* argv[])
{
	using namespace stp;
	std::cout << "=== Test ===\n";
	TextPacketsTest();
	TextPacketsTest2();
	BinPacketsTest2();
	FileTest();
	return 0;
}
