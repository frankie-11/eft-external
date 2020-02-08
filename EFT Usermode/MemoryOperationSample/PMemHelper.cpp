#include "PMemHelper.h"

#include "server_shared.h"
#include "eftstructs.h"
#include <codecvt>
#pragma comment(lib, "Ws2_32")
#pragma warning(disable : 4996)
static bool send_packet(
	const SOCKET	connection,
	const Packet& packet,
	uint64_t& out_result)
{
	Packet completion_packet{ };

	if (send(connection, (const char*)&packet, sizeof(Packet), 0) == SOCKET_ERROR)
		return false;

	const auto result = recv(connection, (char*)&completion_packet, sizeof(Packet), 0);
	if (result < sizeof(PacketHeader) ||
		completion_packet.header.magic != packet_magic ||
		completion_packet.header.type != PacketType::packet_completed)
		return false;

	out_result = completion_packet.data.completed.result;
	return true;
}

static uint32_t copy_memory(
	const SOCKET	connection,
	const uint32_t	src_process_id,
	const uintptr_t src_address,
	const uint32_t	dest_process_id,
	const uintptr_t	dest_address,
	const size_t	size)
{
	Packet packet{ };

	packet.header.magic = packet_magic;
	packet.header.type = PacketType::packet_copy_memory;

	auto& data = packet.data.copy_memory;
	data.src_process_id = src_process_id;
	data.src_address = uint64_t(src_address);
	data.dest_process_id = dest_process_id;
	data.dest_address = uint64_t(dest_address);
	data.size = uint64_t(size);

	uint64_t result = 0;
	if (send_packet(connection, packet, result))
		return uint32_t(result);

	return 0;
}

PMem::PMem()
{
	this->initialize();
}

PMem::~PMem()
{
	//this->deinitialize();
}

void PMem::initialize()
{
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 2), &wsa_data);
}

void PMem::deinitialize()
{
	WSACleanup();
}

SOCKET PMem::connectsystem()
{
	SOCKADDR_IN address{ };

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(server_ip);
	address.sin_port = htons(server_port);

	const auto connection = socket(AF_INET, SOCK_STREAM, 0);
	if (connection == INVALID_SOCKET)
		return INVALID_SOCKET;

	if (connect(connection, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR)
	{
		closesocket(connection);
		return INVALID_SOCKET;
	}

	return connection;
}

void PMem::disconnect(const SOCKET connection)
{
	closesocket(connection);
}



uint64_t PMem::get_process_base_address(const SOCKET connection, const uint32_t process_id)
{
	Packet packet{ };

	packet.header.magic = packet_magic;
	packet.header.type = PacketType::packet_get_base_address;

	auto& data = packet.data.get_base_address;
	data.process_id = process_id;

	uint64_t result = 0;
	if (send_packet(connection, packet, result))
		return result;

	return 0;
}

uint64_t PMem::get_process_peb(SOCKET connection, uint32_t process_id)
{
	Packet packet{ };

	packet.header.magic = packet_magic;
	packet.header.type = PacketType::packet_get_peb;

	auto& data = packet.data.get_base_peb;
	data.process_id = process_id;

	uint64_t result = 0;
	if (send_packet(connection, packet, result))
		return result;

	return 0;
}


MemoryIO::MemoryIO(PMem& physmem_api, SOCKET connection) :physmem_api(physmem_api)
{
	cached_connection = connection;
	if (connection == INVALID_SOCKET)
	{
		printf("connection failed");
		return;
	}

	cached_dwPID = find_process_by_id(_xor_(L"EscapeFromTarkov.exe").operator const wchar_t *());
	cached_peb = physmem_api.get_process_peb(cached_connection, cached_dwPID);

	//uint64_t gom = this->read<uint64_t>(cached_peb + 0x15181E8);
	//cached_peb = this->get_module_address(string.operator const wchar_t* ());
	//catched_twnd = FindWindow(_xor_(L"UnityWndClass"), 0);
	//debugging

//	printf(_xor_("baseaddress : 0x%X\n"), cached_process_base_address);
	//printf(_xor_("PEB : 0x%X\n"), cached_peb);
	//printf(_xor_("gom : 0x%X\n"), gom);
	//printf(_xor_("peb : 0x%X\n"), cached_peb);
}


uint32_t MemoryIO::readbuffer(uint64_t address, PVOID buffer, size_t size)
{
	if (address == 0)
		return false;
	return copy_memory(cached_connection, cached_dwPID, address, GetCurrentProcessId(), uintptr_t(buffer), size);
}

uint64_t MemoryIO::ReadChain(uint64_t base, const std::vector<uint64_t>& offsets) {
	uint64_t result = read<uint64_t>(base + offsets.at(0));
	for (int i = 1; i < offsets.size(); i++) {
		result = read<uint64_t>(result + offsets.at(i));
	}
	return result;
}


std::string MemoryIO::GetUnicodeString(uint64_t addr, int stringLength)
{
	char16_t wcharTemp[64] = { '\0' };
	readbuffer(addr, wcharTemp, stringLength * 2);
	std::string u8_conv = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(wcharTemp);
	return u8_conv;
}


uint64_t MemoryIO::find_process_by_id(const std::wstring& name)
{
	const auto snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE) {
		return 0;
	}

	PROCESSENTRY32 proc_entry{};
	proc_entry.dwSize = sizeof proc_entry;

	auto found_process = false;
	if (!!Process32First(snap, &proc_entry)) {
		do {
			if (name == proc_entry.szExeFile) {
				found_process = true;
				break;
			}
		} while (!!Process32Next(snap, &proc_entry));
	}

	CloseHandle(snap);
	return found_process
		? proc_entry.th32ProcessID
		: 0;
}

