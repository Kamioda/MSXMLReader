#pragma once
#include <Windows.h>
#include <memory>
#include <string>

inline std::string GetErrorMessage(const unsigned long& errorCode) {
	auto deleter = [](char* buffer) { if (buffer) LocalFree(buffer); };
	LPSTR messageBuffer = nullptr;
	const DWORD length = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&messageBuffer),
		0,
		nullptr);
	std::unique_ptr<char, decltype(deleter)> messageBufferOwner(messageBuffer, deleter);

	std::string message;
	if (length && messageBufferOwner) {
		message.assign(messageBufferOwner.get(), length);
		while (!message.empty() && (message.back() == '\r' || message.back() == '\n' || message.back() == ' ')) {
			message.pop_back();
		}
	}
	else {
		message = "Unknown Windows error";
	}
	return "Error " + std::to_string(errorCode) + ": " + message;
}

inline std::string GetErrorMessage() {
	return GetErrorMessage(GetLastError());
}
