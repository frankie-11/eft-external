#pragma once
#include "struct.h"
#include "log.h"

#define BB_POOL_TAG 'Esk'
UCHAR PiDDBLockPtr_sig[] = "\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x4C\x8B\x8C";
UCHAR PiDDBCacheTablePtr_sig[] = "\x66\x03\xD2\x48\x8D\x0D";
//you can also put the sig within the function, but some of the sig ends up on the stack and in the .text section, and causes issues when zeroing the sig memory.

EXTERN_C PVOID ResolveRelativeAddress(
	_In_ PVOID Instruction,
	_In_ ULONG OffsetOffset,
	_In_ ULONG InstructionSize
)
{
	ULONG_PTR Instr = (ULONG_PTR)Instruction;
	LONG RipOffset = *(PLONG)(Instr + OffsetOffset);
	PVOID ResolvedAddr = (PVOID)(Instr + InstructionSize + RipOffset);

	return ResolvedAddr;
}

NTSTATUS BBSearchPattern(IN PCUCHAR pattern, IN UCHAR wildcard, IN ULONG_PTR len, IN const VOID* base, IN ULONG_PTR size, OUT PVOID* ppFound, int index = 0)
{
	ASSERT(ppFound != NULL && pattern != NULL && base != NULL);
	if (ppFound == NULL || pattern == NULL || base == NULL)
		return STATUS_ACCESS_DENIED; //STATUS_INVALID_PARAMETER;
	int cIndex = 0;
	for (ULONG_PTR i = 0; i < size - len; i++)
	{
		BOOLEAN found = TRUE;
		for (ULONG_PTR j = 0; j < len; j++)
		{
			if (pattern[j] != wildcard && pattern[j] != ((PCUCHAR)base)[i + j])
			{
				found = FALSE;
				break;
			}
		}

		if (found != FALSE && cIndex++ == index)
		{
			*ppFound = (PUCHAR)base + i;
			return STATUS_SUCCESS;
		}
	}

	return STATUS_NOT_FOUND;
}

PVOID g_KernelBase = NULL;
ULONG g_KernelSize = 0;

PVOID GetKernelBase(OUT PULONG pSize)
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG bytes = 0;
	PRTL_PROCESS_MODULES pMods = NULL;
	PVOID checkPtr = NULL;
	UNICODE_STRING routineName;

	// Already found
	if (g_KernelBase != NULL)
	{
		if (pSize)
			*pSize = g_KernelSize;
		return g_KernelBase;
	}

	RtlUnicodeStringInit(&routineName, L"NtOpenFile");

	checkPtr = MmGetSystemRoutineAddress(&routineName);
	if (checkPtr == NULL)
		return NULL;

	// Protect from UserMode AV
	status = ZwQuerySystemInformation(SystemModuleInformation, 0, bytes, &bytes);
	if (bytes == 0)
	{
		log("Invalid SystemModuleInformation size");
		return NULL;
	}

	pMods = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, BB_POOL_TAG);
	RtlZeroMemory(pMods, bytes);

	status = ZwQuerySystemInformation(SystemModuleInformation, pMods, bytes, &bytes);

	if (NT_SUCCESS(status))
	{
		PRTL_PROCESS_MODULE_INFORMATION pMod = pMods->Modules;

		for (ULONG i = 0; i < pMods->NumberOfModules; i++)
		{
			// System routine is inside module
			if (checkPtr >= pMod[i].ImageBase &&
				checkPtr < (PVOID)((PUCHAR)pMod[i].ImageBase + pMod[i].ImageSize))
			{
				g_KernelBase = pMod[i].ImageBase;
				g_KernelSize = pMod[i].ImageSize;
				if (pSize)
					*pSize = g_KernelSize;
				break;
			}
		}
	}

	if (pMods)
		ExFreePoolWithTag(pMods, BB_POOL_TAG);
	//log("g_KernelBase: %x", g_KernelBase);
	//log("g_KernelSize: %x", g_KernelSize);
	return g_KernelBase;
}

NTSTATUS BBScanSection(IN PCCHAR section, IN PCUCHAR pattern, IN UCHAR wildcard, IN ULONG_PTR len, OUT PVOID* ppFound, PVOID base = nullptr)
{
	//ASSERT(ppFound != NULL);
	if (ppFound == NULL)
		return STATUS_ACCESS_DENIED; //STATUS_INVALID_PARAMETER

	if (nullptr == base)
		base = GetKernelBase(&g_KernelSize);
	if (base == nullptr)
		return STATUS_ACCESS_DENIED; //STATUS_NOT_FOUND;

	PIMAGE_NT_HEADERS64 pHdr = RtlImageNtHeader(base);
	if (!pHdr)
		return STATUS_ACCESS_DENIED; // STATUS_INVALID_IMAGE_FORMAT;

	//PIMAGE_SECTION_HEADER pFirstSection = (PIMAGE_SECTION_HEADER)(pHdr + 1);
	PIMAGE_SECTION_HEADER pFirstSection = (PIMAGE_SECTION_HEADER)((uintptr_t)&pHdr->FileHeader + pHdr->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_FILE_HEADER));

	for (PIMAGE_SECTION_HEADER pSection = pFirstSection; pSection < pFirstSection + pHdr->FileHeader.NumberOfSections; pSection++)
	{
		//DbgPrint("section: %s\r\n", pSection->Name);
		ANSI_STRING s1, s2;
		RtlInitAnsiString(&s1, section);
		RtlInitAnsiString(&s2, (PCCHAR)pSection->Name);
		if (RtlCompareString(&s1, &s2, TRUE) == 0)
		{
			PVOID ptr = NULL;
			NTSTATUS status = BBSearchPattern(pattern, wildcard, len, (PUCHAR)base + pSection->VirtualAddress, pSection->Misc.VirtualSize, &ptr);
			if (NT_SUCCESS(status)) {
				*(PULONG64)ppFound = (ULONG_PTR)(ptr); //- (PUCHAR)base
				//DbgPrint("found\r\n");
				return status;
			}
			//we continue scanning because there can be multiple sections with the same name.
		}
	}

	return STATUS_ACCESS_DENIED; //STATUS_NOT_FOUND;
}
extern "C" bool LocatePiDDB(PERESOURCE * lock, PRTL_AVL_TABLE * table)
{
	PVOID PiDDBLockPtr = nullptr, PiDDBCacheTablePtr = nullptr;
	if (!NT_SUCCESS(BBScanSection("PAGE", PiDDBLockPtr_sig, 0, sizeof(PiDDBLockPtr_sig) - 1, reinterpret_cast<PVOID*>(&PiDDBLockPtr)))) {
		log("Unable to find PiDDBLockPtr sig.");
		return false;
	}

	if (!NT_SUCCESS(BBScanSection("PAGE", PiDDBCacheTablePtr_sig, 0, sizeof(PiDDBCacheTablePtr_sig) - 1, reinterpret_cast<PVOID*>(&PiDDBCacheTablePtr)))) {
		log("Unable to find PiDDBCacheTablePtr sig");
		return false;
	}

	PiDDBCacheTablePtr = PVOID((uintptr_t)PiDDBCacheTablePtr + 3);

	*lock = (PERESOURCE)(ResolveRelativeAddress(PiDDBLockPtr, 3, 7));
	*table = (PRTL_AVL_TABLE)(ResolveRelativeAddress(PiDDBCacheTablePtr, 3, 7));

	return true;
}





PMM_UNLOADED_DRIVER MmUnloadedDrivers;
PULONG				MmLastUnloadedDriver;

BOOLEAN bDataCompare(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bMask)
		if (*szMask == 'x' && *pData != *bMask)
			return 0;

	return (*szMask) == 0;
}

UINT64 FindPattern(UINT64 dwAddress, UINT64 dwLen, BYTE* bMask, char* szMask)
{
	for (UINT64 i = 0; i < dwLen; i++)
		if (bDataCompare((BYTE*)(dwAddress + i), bMask, szMask))
			return (UINT64)(dwAddress + i);

	return 0;
}
NTSTATUS FindMmDriverData(
	VOID
)
{
	/*
	 *	nt!MmLocateUnloadedDriver:
	 *	fffff801`51c70394 4c8b15a57e1500  mov     r10,qword ptr [nt!MmUnloadedDrivers (fffff801`51dc8240)]
	 *	fffff801`51c7039b 4c8bc9          mov     r9 ,rcx
	 */
	PVOID MmUnloadedDriversInstr = (PVOID)FindPattern((UINT64)g_KernelBase, g_KernelSize,
		(BYTE*)"\x4C\x8B\x15\x00\x00\x00\x00\x4C\x8B\xC9",
		"xxx????xxx"
	);

	/*
	 *	nt!MiRememberUnloadedDriver+0x59:
	 *	fffff801`5201a4c5 8b057ddddaff    mov     eax,dword ptr [nt!MmLastUnloadedDriver (fffff801`51dc8248)]
	 *	fffff801`5201a4cb 83f832          cmp     eax,32h
	*/
	PVOID MmLastUnloadedDriverInstr = (PVOID)FindPattern((UINT64)g_KernelBase, g_KernelSize,
		(BYTE*)"\x8B\x05\x00\x00\x00\x00\x83\xF8\x32",
		"xx????xxx"
	);

	if (MmUnloadedDriversInstr == NULL || MmLastUnloadedDriverInstr == NULL)
	{
		return STATUS_NOT_FOUND;
	}

	MmUnloadedDrivers = *(PMM_UNLOADED_DRIVER*)ResolveRelativeAddress(MmUnloadedDriversInstr, 3, 7);
	MmLastUnloadedDriver = (PULONG)ResolveRelativeAddress(MmLastUnloadedDriverInstr, 2, 6);
	/*log("MmUnloadedDrivers ModuleEnd: %x", MmUnloadedDrivers->ModuleEnd);
	log("MmUnloadedDrivers ModuleStart: %x", MmUnloadedDrivers->ModuleStart);
	log("MmUnloadedDrivers Name: %s", MmUnloadedDrivers->Name);
	log("MmUnloadedDrivers UnloadTime: %x", MmUnloadedDrivers->UnloadTime);*/

	//log("MmUnloadedDrivers Addr: %x", MmUnloadedDrivers);
	//log("MmLastUnloadedDriver Addr: %x", MmLastUnloadedDriver);
	return STATUS_SUCCESS;
}

BOOLEAN IsUnloadedDriverEntryEmpty(
	_In_ PMM_UNLOADED_DRIVER Entry
)
{
	if (Entry->Name.MaximumLength == 0 ||
		Entry->Name.Length == 0 ||
		Entry->Name.Buffer == NULL)
	{
		return TRUE;
	}

	return FALSE;
}

BOOLEAN IsMmUnloadedDriversFilled(
	VOID
)
{
	for (ULONG Index = 0; Index < MM_UNLOADED_DRIVERS_SIZE; ++Index)
	{
		PMM_UNLOADED_DRIVER Entry = &MmUnloadedDrivers[Index];
		if (IsUnloadedDriverEntryEmpty(Entry))
		{
			return FALSE;
		}
	}

	return TRUE;
}



ERESOURCE PsLoadedModuleResource;




namespace clear {

	void clearCache(UNICODE_STRING DriverName, ULONG timeDateStamp)
	{
		// first locate required variables
		PERESOURCE PiDDBLock; PRTL_AVL_TABLE PiDDBCacheTable;
		if (!LocatePiDDB(&PiDDBLock, &PiDDBCacheTable)) {
			log("ClearCache Failed");
			return;
		}

		//log("Found PiDDBLock and PiDDBCacheTable");
		//log("Found PiDDBLock %x", PiDDBLock);
		//log("Found PiDDBCacheTable %x", PiDDBCacheTable);

		// build a lookup entry

		PiDDBCacheEntry lookupEntry = { };
		lookupEntry.DriverName = DriverName;
		lookupEntry.TimeDateStamp = timeDateStamp;

		// acquire the ddb resource lock
		ExAcquireResourceExclusiveLite(PiDDBLock, TRUE);

		// search our entry in the table
		auto pFoundEntry = (PiDDBCacheEntry*)RtlLookupElementGenericTableAvl(PiDDBCacheTable, &lookupEntry);
		if (pFoundEntry == nullptr)
		{
			// release the ddb resource lock
			ExReleaseResourceLite(PiDDBLock);
			log("ClearCache Failed (Not found)");
			return;
		}

		// first, unlink from the list
		RemoveEntryList(&pFoundEntry->List);
		// then delete the element from the avl table
		RtlDeleteElementGenericTableAvl(PiDDBCacheTable, pFoundEntry);

		// release the ddb resource lock
		ExReleaseResourceLite(PiDDBLock);
		log("chache cleared");
	}




	NTSTATUS ClearUnloadedDriver(
		_In_ PUNICODE_STRING	DriverName,
		_In_ BOOLEAN			AccquireResource
	)
	{
		if (AccquireResource)
		{
			ExAcquireResourceExclusiveLite(&PsLoadedModuleResource, TRUE);
		}

		BOOLEAN Modified = FALSE;
		BOOLEAN Filled = IsMmUnloadedDriversFilled();

		for (ULONG Index = 0; Index < MM_UNLOADED_DRIVERS_SIZE; ++Index)
		{
			PMM_UNLOADED_DRIVER Entry = &MmUnloadedDrivers[Index];
			if (Modified)
			{
				//
				// Shift back all entries after modified one.
				//
				PMM_UNLOADED_DRIVER PrevEntry = &MmUnloadedDrivers[Index - 1];
				RtlCopyMemory(PrevEntry, Entry, sizeof(MM_UNLOADED_DRIVER));

				//
				// Zero last entry.
				//
				if (Index == MM_UNLOADED_DRIVERS_SIZE - 1)
				{
					RtlFillMemory(Entry, sizeof(MM_UNLOADED_DRIVER), 0);
				}
			}
			else if (RtlEqualUnicodeString(DriverName, &Entry->Name, TRUE))
			{
				//
				// Erase driver entry.
				//
				PVOID BufferPool = Entry->Name.Buffer;
				RtlFillMemory(Entry, sizeof(MM_UNLOADED_DRIVER), 0);
				ExFreePoolWithTag(BufferPool, 'TDmM');

				//
				// Because we are erasing last entry we want to set MmLastUnloadedDriver to 49
				// if list have been already filled.
				//
				*MmLastUnloadedDriver = (Filled ? MM_UNLOADED_DRIVERS_SIZE : *MmLastUnloadedDriver) - 1;
				Modified = TRUE;
			}
		}

		if (Modified)
		{
			ULONG64 PreviousTime = 0;

			//
			// Make UnloadTime look right.
			//
			for (LONG Index = MM_UNLOADED_DRIVERS_SIZE - 2; Index >= 0; --Index)
			{
				PMM_UNLOADED_DRIVER Entry = &MmUnloadedDrivers[Index];
				if (IsUnloadedDriverEntryEmpty(Entry))
				{
					continue;
				}

				if (PreviousTime != 0 && Entry->UnloadTime > PreviousTime)
				{
					//
					// Decrease by random value here maybe.
					//
					Entry->UnloadTime = PreviousTime - 100;
				}

				PreviousTime = Entry->UnloadTime;
			}

			//
			// Clear remaining entries.
			//
			ClearUnloadedDriver(DriverName, FALSE);
		}

		if (AccquireResource)
		{
			ExReleaseResourceLite(&PsLoadedModuleResource);
		}

		return Modified ? STATUS_SUCCESS : STATUS_NOT_FOUND;
	}
}