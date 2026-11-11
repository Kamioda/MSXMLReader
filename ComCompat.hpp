#pragma once
#include <Windows.h>
#include <oaidl.h>
#include <objbase.h>

#if defined(_MSC_VER) && __has_include(<wrl/client.h>) && __has_include(<comdef.h>)
#define MSXML_READER_HAS_MSVC_COM_SUPPORT 1
#include <wrl/client.h>
#include <comdef.h>

template<class Interface>
using ComPtr = Microsoft::WRL::ComPtr<Interface>;

using Variant = _variant_t;

template<class SourceInterface, class DestinationInterface>
HRESULT ComPtrAs(const ComPtr<SourceInterface>& source, ComPtr<DestinationInterface>& destination) noexcept {
	return source.As(&destination);
}
#else
#include <memory>
#include <new>
#include <utility>

// Fallback COM and VARIANT ownership helpers for toolchains without WRL/comsuppw.
template<class Interface>
class ComPtr {
private:
	Interface* m_ptr{};
	void release() noexcept {
		if (m_ptr) {
			m_ptr->Release();
			m_ptr = nullptr;
		}
	}

public:
	ComPtr() noexcept = default;
	ComPtr(const ComPtr& other) noexcept : m_ptr(other.m_ptr) { if (m_ptr) m_ptr->AddRef(); }
	ComPtr(ComPtr&& other) noexcept : m_ptr(std::exchange(other.m_ptr, nullptr)) {}
	~ComPtr() { release(); }

	ComPtr& operator=(const ComPtr& other) noexcept {
		if (this != std::addressof(other)) {
			Interface* next = other.m_ptr;
			if (next) next->AddRef();
			release();
			m_ptr = next;
		}
		return *this;
	}

	ComPtr& operator=(ComPtr&& other) noexcept {
		if (this != std::addressof(other)) {
			release();
			m_ptr = std::exchange(other.m_ptr, nullptr);
		}
		return *this;
	}

	Interface* operator->() const noexcept { return m_ptr; }
	explicit operator bool() const noexcept { return m_ptr != nullptr; }
	Interface** ReleaseAndGetAddressOf() noexcept { release(); return &m_ptr; }

	template<class OtherInterface>
	HRESULT As(REFIID iid, OtherInterface** result) const noexcept {
		if (!result) return E_POINTER;
		*result = nullptr;
		return m_ptr ? m_ptr->QueryInterface(iid, reinterpret_cast<void**>(result)) : E_POINTER;
	}
};

class Variant {
private:
	VARIANT m_value{};

public:
	Variant() noexcept { VariantInit(&m_value); }
	explicit Variant(bool value) noexcept : Variant() {
		V_VT(&m_value) = VT_BOOL;
		V_BOOL(&m_value) = value ? VARIANT_TRUE : VARIANT_FALSE;
	}
	explicit Variant(const wchar_t* value) : Variant() {
		V_VT(&m_value) = VT_BSTR;
		V_BSTR(&m_value) = SysAllocString(value);
		if (value && !V_BSTR(&m_value)) throw std::bad_alloc();
	}
	Variant(const Variant&) = delete;
	Variant& operator=(const Variant&) = delete;
	Variant(Variant&& other) noexcept : m_value(other.m_value) { VariantInit(&other.m_value); }
	Variant& operator=(Variant&& other) noexcept {
		if (this != std::addressof(other)) {
			VariantClear(&m_value);
			m_value = other.m_value;
			VariantInit(&other.m_value);
		}
		return *this;
	}
	~Variant() { VariantClear(&m_value); }
	operator VARIANT() const noexcept { return m_value; }
};

template<class SourceInterface, class DestinationInterface>
HRESULT ComPtrAs(const ComPtr<SourceInterface>& source, REFIID iid, ComPtr<DestinationInterface>& destination) noexcept {
	return source.As(iid, destination.ReleaseAndGetAddressOf());
}
#endif
