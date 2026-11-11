#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include <utility>

class BString {
private:
	BSTR m_bstr;
public:
	BString() : m_bstr(nullptr) {}
	BString(const std::wstring& str) 
		: m_bstr(SysAllocString(str.c_str())) {
		if (!m_bstr) throw std::bad_alloc();
	}
	BString(const BString&) = delete;
	BString(BString&& other) noexcept
		: m_bstr(std::exchange(other.m_bstr, nullptr)) {}
	~BString() {
		if (m_bstr) SysFreeString(m_bstr);
	}
	BString& operator=(const BString&) = delete;
	BString& operator=(BString&& other) noexcept {
		if (this != std::addressof(other)) {
			SysFreeString(m_bstr);
			m_bstr = std::exchange(other.m_bstr, nullptr);
		}
		return *this;
	}
	BSTR* operator &() {
		SysFreeString(m_bstr);
		m_bstr = nullptr;
		return &m_bstr;
	}
	const BSTR& operator()() const noexcept {
		return m_bstr;
	}
	std::size_t length() const {
		return SysStringLen(m_bstr);
	}
};
