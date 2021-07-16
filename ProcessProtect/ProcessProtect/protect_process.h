#pragma once
#include <ntifs.h>

struct ProcessProtectArgs {
	size_t pid;
};

struct ProtectProcessEntry {
	ProcessProtectArgs args;
	SINGLE_LIST_ENTRY next;
};

extern SINGLE_LIST_ENTRY* g_protected_processes;

NTSTATUS protect_process(const ProcessProtectArgs& args);

NTSTATUS register_protectors();