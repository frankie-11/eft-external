#pragma once
#include "stdint.h"
#include "subauth.h"

constexpr auto packet_magic = 0x12345568;
constexpr auto server_ip = 0x7F000001; // 127.0.0.1
constexpr auto server_port = 28125;

enum class PacketType
{
	packet_copy_memory,
	packet_get_base_address,
	packet_get_peb,
	packet_completed
};

struct PacketCopyMemory
{
	uint32_t dest_process_id;
	uint64_t dest_address;

	uint32_t src_process_id;
	uint64_t src_address;

	uint32_t size;
};

struct PacketGetBaseAddress
{
	uint32_t process_id;
};

struct PacketGetBasePeb
{
	uint32_t process_id;
};

struct PackedCompleted
{
	uint64_t result;
};

struct PacketHeader
{
	uint32_t   magic;
	PacketType type;
};

struct Packet
{
	PacketHeader header;
	union
	{
		PacketCopyMemory	 copy_memory;
		PacketGetBaseAddress get_base_address;
		PacketGetBasePeb get_base_peb;
		PackedCompleted		 completed;
	} data;
};
