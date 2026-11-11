#pragma once
#include "WinError.hpp"
#include "ComCompat.hpp"
#include <stdexcept>

class CoInitializer {
public:
    CoInitializer(LPVOID pvReserved = nullptr, DWORD dwCoInit = COINIT_APARTMENTTHREADED) {
        HRESULT hr = CoInitializeEx(pvReserved, dwCoInit);
		if (FAILED(hr)) throw std::runtime_error("Failed to initialize COM: " + GetErrorMessage(hr));
    }
	CoInitializer(const CoInitializer&) = delete;
    CoInitializer(CoInitializer&&) = delete;
    ~CoInitializer() {
        CoUninitialize();
    }
	CoInitializer& operator=(const CoInitializer&) = delete;
	CoInitializer& operator=(CoInitializer&&) = delete;
};

