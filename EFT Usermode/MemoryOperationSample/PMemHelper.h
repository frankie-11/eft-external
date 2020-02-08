#include <Windows.h>
#include <stdint.h>
#include <iostream>
#include <time.h>
#include <Psapi.h>
#include <functional>
#include <tlhelp32.h>
#include "xorstr.hpp"
#include "subauth.h"


class PMem
{
public:
	PMem();
	~PMem();

	void	initialize();
	void	deinitialize();

	SOCKET	connectsystem();
	void	disconnect(SOCKET connection);

	uint64_t get_process_base_address(SOCKET connection, uint32_t process_id);
	uint64_t get_process_peb(SOCKET connection, uint32_t process_id);

};



class MemoryIO;
class MemoryIO {
public:
	MemoryIO(PMem& physmem_api, SOCKET connection);
	uint32_t readbuffer(uint64_t address, PVOID buffer, size_t size);
	uint64_t ReadChain(uint64_t base, const std::vector<uint64_t>& offsets);
	std::string GetUnicodeString(uint64_t addr, int stringLength);
	template <typename T>
	T read(uint64_t address);
	uint64_t get_process_base() const { return cached_process_base_address; };
	uint64_t get_process_peb() const { return cached_peb; };
	uint64_t find_process_by_id(const std::wstring& name);

private:
	uint64_t cached_process_base_address = 0, cached_dwPID = 0, cached_peb = 0;
	SOCKET cached_connection;
	PMem& physmem_api;
};

template<typename T>
inline T MemoryIO::read(uint64_t address)
{
	T buffer{ };
	readbuffer(address, &buffer, sizeof(T));
	return buffer;
}



extern MemoryIO* memio;