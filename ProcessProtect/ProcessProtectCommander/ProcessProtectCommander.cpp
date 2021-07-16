#include <iostream>
#include <string>

#include "Windows.h"

struct ProcessProtectArgs {
	size_t pid;
};

int main(int argc, char** argv) {

	std::cout << "start protect process commander" << std::endl;
	if (argc != 2) {
		std::cout << "invalid number of arguments" << std::endl;
		return 1;
	}

	auto handle = CreateFileA("\\\\.\\ProcessProtect", GENERIC_ALL,
		0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == handle) {
		std::cout << "open ProcessProtect handle failed" << std::endl;
		return 1;
	}
	ProcessProtectArgs args = { static_cast<size_t>(std::stoi(argv[1])) };
	
	DWORD returned_bytes = 0;
	if (!DeviceIoControl(handle, 0x22CCDC, &args, sizeof args,
		nullptr, 0, &returned_bytes, nullptr)) {
		std::cout << "device ioctl failed" << std::endl;
		CloseHandle(handle);
		return 1;
	}

	std::cout << "device ioctl succeeded" << std::endl;

	CloseHandle(handle);
	return 0;
}

