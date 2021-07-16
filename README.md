# windows-kernel-process-protector
Protect a process from code injection, termination and hooking.

Using Object Manager callbacks mechanism in order to protect the process.

While developing the driver I encountered an error when using ObRegisterCallbacks.

Error STATUS_ACCESS_DENIED -> The callback routines do not reside in a signed kernel binary image.

In order to bypass it and not forcing all the user to disable integrity checks for the entire system,
I patched the flag inside the kernel.
When the loader loads the driver it first validates its signature (unless testing mode is on) and then mark the
driver in inside struct that this driver is not signed (yes also in testing mode).

Therefore, all we need is to patch this flag:

```C++
PKLDR_DATA_TABLE_ENTRY DriverSection = (PKLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;
DriverSection->Flags |= LDRP_VALID_SECTION;
```

## Usage

sc create ProcessProtect binPath={ProcessProtectDriverFullPath.sys} type=kernel

sc start ProcessProtect

ProcessProtectCommander.exe {pid}

DONE!!!
