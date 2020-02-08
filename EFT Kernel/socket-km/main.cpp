#include "clean.hpp"
#include "log.h"


extern void NTAPI initiliaze_sys(void*);

extern "C" NTSTATUS DriverEntry(
	PDRIVER_OBJECT  driver_object,
	PUNICODE_STRING registry_path
)
{
	KeEnterGuardedRegion();

	UNREFERENCED_PARAMETER(driver_object);
	UNREFERENCED_PARAMETER(registry_path);

	UNICODE_STRING driver_int = RTL_CONSTANT_STRING(L"iqvw64e.sys");//iqvw64e.sys 0x5284EAC3 (timeDateStamp)

	clear::clearCache(driver_int, 0x5284EAC3);

	FindMmDriverData();


	PWORK_QUEUE_ITEM WorkItem = (PWORK_QUEUE_ITEM)ExAllocatePool(NonPagedPool, sizeof(WORK_QUEUE_ITEM));

	ExInitializeWorkItem(WorkItem, initiliaze_sys, WorkItem);

	ExQueueWorkItem(WorkItem, DelayedWorkQueue);

	if (clear::ClearUnloadedDriver(&driver_int, true) == STATUS_SUCCESS)
	{
		log("dr int cleared");
	}
	else {
		log("dr int not found");
	}

	KeLeaveGuardedRegion();

	return STATUS_SUCCESS;
}