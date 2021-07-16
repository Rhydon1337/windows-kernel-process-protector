#include <ntifs.h>

#include "device_handlers.h"
#include "protect_process.h"
#include "consts.h"

#define LDRP_VALID_SECTION 0x20

typedef struct _KLDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	PVOID ExceptionTable;
	UINT32 ExceptionTableSize;
	PVOID GpValue;
	struct _NON_PAGED_DEBUG_INFO* NonPagedDebugInfo;
	PVOID ImageBase;
	PVOID EntryPoint;
	UINT32 SizeOfImage;
	UNICODE_STRING FullImageName;
	UNICODE_STRING BaseImageName;
	UINT32 Flags;
	UINT16 LoadCount;

	union
	{
		UINT16 SignatureLevel : 4;
		UINT16 SignatureType : 3;
		UINT16 Unused : 9;
		UINT16 EntireField;
	} u;

	PVOID SectionPointer;
	UINT32 CheckSum;
	UINT32 CoverageSectionSize;
	PVOID CoverageSection;
	PVOID LoadedImports;
	PVOID Spare;
	UINT32 SizeOfImageNotRounded;
	UINT32 TimeDateStamp;
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	DbgPrint("Hello from driver entry\n");

	UNICODE_STRING  nt_name;
	UNICODE_STRING  win32_name;
	PDEVICE_OBJECT  device_object = nullptr;

	// Set the ValidSection bit to bypass altitude limits
	// and allow for callbacks to be registered in an unsigned
	// driver / manually mapped data.
	PKLDR_DATA_TABLE_ENTRY DriverSection = (PKLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;
	DriverSection->Flags |= LDRP_VALID_SECTION;
	
	RtlInitUnicodeString(&nt_name, NT_DEVICE_NAME);
	NTSTATUS nt_status = IoCreateDevice(DriverObject, 0, &nt_name,
		FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN,
		TRUE, &device_object);

	if (!NT_SUCCESS(nt_status)) {
		DbgPrint("Couldn't create the device object\n");
		return nt_status;
	}

	DriverObject->MajorFunction[IRP_MJ_CREATE] = device_create_close;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = device_create_close;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = device_ioctl;

	RtlInitUnicodeString(&win32_name, DOS_DEVICE_NAME);

	nt_status = IoCreateSymbolicLink(&win32_name, &nt_name);
	if (!NT_SUCCESS(nt_status)) {
		DbgPrint("Couldn't create symbolic link\n");
		IoDeleteDevice(device_object);
		return nt_status;
	}

	return register_protectors();
}