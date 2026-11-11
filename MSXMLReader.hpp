#pragma once
#include "WinError.hpp"
#include "BString.hpp"
#include "ComCompat.hpp"
#include <msxml.h>
#pragma push_macro("DOMDocument")
#pragma push_macro("CLSID_DOMDocument")
#include <msxml2.h>
#pragma pop_macro("CLSID_DOMDocument")
#pragma pop_macro("DOMDocument")
#include <algorithm>
#include <cmath>
#include <concepts>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <cwctype>

namespace msxml_detail {
	// CLSID_DOMDocument from msxml.h. msxml2.h aliases that name to a different class.
	inline constexpr CLSID legacyDomDocumentClass{
		0x2933BF90, 0x7B36, 0x11D2, { 0xB2, 0x0E, 0x00, 0xC0, 0x4F, 0x98, 0x3E, 0x60 }
	};
#if !defined(MSXML_READER_HAS_MSVC_COM_SUPPORT)
	inline constexpr IID iidXmlDomDocument{
		0x2933BF81, 0x7B36, 0x11D2, { 0xB2, 0x0E, 0x00, 0xC0, 0x4F, 0x98, 0x3E, 0x60 }
	};
	inline constexpr IID iidXmlDomDocument2{
		0x2933BF95, 0x7B36, 0x11D2, { 0xB2, 0x0E, 0x00, 0xC0, 0x4F, 0x98, 0x3E, 0x60 }
	};
#endif

	template<class String>
	String ReplaceEscapedNewlines(String value) {
		using Char = typename String::value_type;
		const String sentinel{ Char('e'), Char('m'), Char('p'), Char('t'), Char('y') };
		if (value == sentinel) return {};
		const String escaped{ Char('\\'), Char('n') };
		const String newline{ Char('\n') };
		for (auto pos = value.find(escaped); pos != String::npos; pos = value.find(escaped, pos + newline.size())) {
			value.replace(pos, escaped.size(), newline);
		}
		return value;
	}

	inline std::wstring LF(std::wstring value) { return ReplaceEscapedNewlines(std::move(value)); }

	inline std::string WStringToString(const std::wstring& value) {
		const int size = WideCharToMultiByte(CP_OEMCP, 0, value.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (!size) throw std::runtime_error(GetErrorMessage(GetLastError()));
		std::string result(static_cast<std::size_t>(size), '\0');
		WideCharToMultiByte(CP_OEMCP, 0, value.c_str(), -1, result.data(), size, nullptr, nullptr);
		result.pop_back();
		return result;
	}

	template<typename Number>
	Number ParseNumber(const std::wstring& text) {
		std::size_t parsed = 0;
		auto hasOnlyTrailingWhitespace = [&] {
		return std::ranges::all_of(text.begin() + static_cast<std::ptrdiff_t>(parsed), text.end(),
			[](wchar_t ch) { return std::iswspace(ch) != 0; });
		};
		if constexpr (std::signed_integral<Number>) {
			const long long value = std::stoll(text, &parsed);
			if (!hasOnlyTrailingWhitespace()) throw std::invalid_argument("Invalid trailing characters in XML integer");
			if (value < (std::numeric_limits<Number>::min)()
				|| value > (std::numeric_limits<Number>::max)())
				throw std::out_of_range("XML integer is outside the requested type's range");
			return static_cast<Number>(value);
		} 
		else if constexpr (std::unsigned_integral<Number>) {
			if (text.empty() || text.contains(L'-'))
				throw std::invalid_argument("XML unsigned integer cannot be negative");
			const unsigned long long value = std::stoull(text, &parsed);
			if (!hasOnlyTrailingWhitespace()) throw std::invalid_argument("Invalid trailing characters in XML integer");
			if (value > (std::numeric_limits<Number>::max)())
				throw std::out_of_range("XML integer is outside the requested type's range");
			return static_cast<Number>(value);
		} 
		else {
			const long double value = std::stold(text, &parsed);
			if (!hasOnlyTrailingWhitespace()) throw std::invalid_argument("Invalid trailing characters in XML number");
			if (!std::isfinite(value) || value < (std::numeric_limits<Number>::lowest)()
				|| value > (std::numeric_limits<Number>::max)())
				throw std::out_of_range("XML floating-point value is outside the requested type's range");
			const Number converted = static_cast<Number>(value);
			if (value != 0 && converted == 0)
				throw std::out_of_range("XML floating-point value underflows the requested type");
			return converted;
		}
	}

	inline std::wstring StringToWString(const std::string& value) {
		const int size = MultiByteToWideChar(CP_OEMCP, 0, value.c_str(), -1, nullptr, 0);
		if (!size) throw std::runtime_error(GetErrorMessage(GetLastError()));
		std::wstring result(static_cast<std::size_t>(size), L'\0');
		MultiByteToWideChar(CP_OEMCP, 0, value.c_str(), -1, result.data(), size);
		return result;
	}

	template<typename CharType>
	concept WideChar = std::same_as<CharType, std::wstring>;

	template<typename CharType>
	concept MultiByteChar = std::same_as<CharType, std::string>;
}

class XmlNode;
class XmlNodeList;

class XmlNamedNodeMap {
private:
	ComPtr<IXMLDOMNamedNodeMap> m_namedNodeMap;
public:
	XmlNamedNodeMap() = delete;
	XmlNamedNodeMap(const ComPtr<IXMLDOMNamedNodeMap>& namedNodeMap) : m_namedNodeMap(namedNodeMap) {
		if (!namedNodeMap) throw std::invalid_argument("XML named node map cannot be null");
	}
	XmlNamedNodeMap(const XmlNamedNodeMap&) = delete;
	XmlNamedNodeMap(XmlNamedNodeMap&&) = default;
	XmlNamedNodeMap& operator=(const XmlNamedNodeMap&) = delete;
	XmlNamedNodeMap& operator=(XmlNamedNodeMap&&) = default;
	XmlNode operator[](const std::wstring& name) const;
	XmlNode operator[](const std::string& name) const;
};

class XmlNode {
private:
	ComPtr<IXMLDOMNode> m_node;
public:
	XmlNode() = delete;
	XmlNode(const ComPtr<IXMLDOMNode>& node) : m_node(node) {
		if (!node) throw std::invalid_argument("XML node cannot be null");
	}
	XmlNode(const XmlNode&) = delete;
	XmlNode(XmlNode&&) = default;
	XmlNode& operator=(const XmlNode&) = delete;
	XmlNode& operator=(XmlNode&&) = default;
	std::wstring text() const {
		if (!this->m_node) throw std::logic_error("XML node is empty after move");
		BString bstr{};
		if (const HRESULT hr = this->m_node->get_text(&bstr); FAILED(hr))
			throw std::runtime_error("Failed to get XML node text");
		std::wstring str = msxml_detail::LF(bstr() ? std::wstring(bstr()) : std::wstring{});
		str.resize(std::wcslen(str.c_str()));
		return str;
	}
	template<msxml_detail::WideChar CharType>
	inline CharType get() const {
		return this->text();
	}
	template<msxml_detail::MultiByteChar CharType>
	inline CharType get() const {
		return msxml_detail::WStringToString(this->text());
	}
	template<std::signed_integral I>
	inline I get() const {
		return msxml_detail::ParseNumber<I>(this->text());
	}
	template<std::unsigned_integral I>
	inline I get() const {
		return msxml_detail::ParseNumber<I>(this->text());
	}
	template<std::floating_point F>
	inline F get() const {
		return msxml_detail::ParseNumber<F>(this->text());
	}
	XmlNodeList child() const;
	inline XmlNamedNodeMap attribute() {
		if (!this->m_node) throw std::logic_error("XML node is empty after move");
		ComPtr<IXMLDOMNamedNodeMap> namedNodeMap{};
		if (const HRESULT hr = this->m_node->get_attributes(namedNodeMap.ReleaseAndGetAddressOf()); FAILED(hr) || hr == S_FALSE)
			throw std::runtime_error("Failed to get XML attributes");
		return XmlNamedNodeMap(namedNodeMap);
	}
};

class XmlNodeList {
private:
	ComPtr<IXMLDOMNodeList> m_nodeList;
public:
	XmlNodeList() = delete;
	XmlNodeList(const ComPtr<IXMLDOMDocument>& document, const std::wstring& xpath)
		: m_nodeList() {
		if (!document) throw std::invalid_argument("XML document cannot be null");
		if (xpath.empty()) throw std::invalid_argument("XML Path cannot be empty");
		const BString path(xpath);
		if (const HRESULT hr = document->selectNodes(path(), this->m_nodeList.ReleaseAndGetAddressOf()); FAILED(hr))
			throw std::invalid_argument("Invalid XML Path: " + msxml_detail::WStringToString(xpath));
	}
	XmlNodeList(const ComPtr<IXMLDOMNodeList>& nodeList) : m_nodeList(nodeList) {
		if (!nodeList) throw std::invalid_argument("XML node list cannot be null");
	}
	XmlNodeList(const XmlNodeList&) = delete;
	XmlNodeList(XmlNodeList&&) = default;
	XmlNodeList& operator=(const XmlNodeList&) = delete;
	XmlNodeList& operator=(XmlNodeList&&) = default;
	inline XmlNode operator[](const std::size_t& index) const {
		if (!m_nodeList) throw std::logic_error("XML node list is empty after move");
		if (index > static_cast<std::size_t>((std::numeric_limits<long>::max)()))
			throw std::out_of_range("XML node index out of range");
		ComPtr<IXMLDOMNode> node{};
		if (const HRESULT hr = this->m_nodeList->get_item(static_cast<long>(index), node.ReleaseAndGetAddressOf()); FAILED(hr))
			throw std::runtime_error("Failed to get XML node");
		if (!node) throw std::out_of_range("XML node index out of range");
		return XmlNode(node);
	}
	inline long length() const {
		if (!m_nodeList) throw std::logic_error("XML node list is empty after move");
		long length{};
		if (const HRESULT hr = this->m_nodeList->get_length(&length); FAILED(hr))
			throw std::runtime_error("Failed to get XML node list length");
		return length;
	}
};

inline XmlNode XmlNamedNodeMap::operator[](const std::wstring& name) const {
	if (!this->m_namedNodeMap) throw std::logic_error("XML named node map is empty after move");
	const BString bstr(name);
	ComPtr<IXMLDOMNode> node{};
	if (const HRESULT hr = this->m_namedNodeMap->getNamedItem(bstr(), node.ReleaseAndGetAddressOf()); FAILED(hr))
		throw std::runtime_error("Failed to get XML attribute");
	if (!node) throw std::out_of_range("XML attribute not found: " + msxml_detail::WStringToString(name));
	return XmlNode(node);
}

inline XmlNode XmlNamedNodeMap::operator[](const std::string& name) const {
	return (*this)[msxml_detail::StringToWString(name)];
}

inline XmlNodeList XmlNode::child() const {
	if (!this->m_node) throw std::logic_error("XML node is empty after move");
	ComPtr<IXMLDOMNodeList> child{};
	if (const HRESULT hr = this->m_node->get_childNodes(child.ReleaseAndGetAddressOf()); FAILED(hr))
		throw std::runtime_error("Failed to get XML child nodes");
	return XmlNodeList(child);
}

class MSXMLReader {
private:
	ComPtr<IXMLDOMDocument> m_document;
	template<class LoadFunction>
		requires std::invocable<LoadFunction&, const ComPtr<IXMLDOMDocument>&>
			&& std::same_as<std::invoke_result_t<LoadFunction&, const ComPtr<IXMLDOMDocument>&>, VARIANT_BOOL>
	MSXMLReader(LoadFunction&& loadFunc)
		: m_document() {
#if defined(MSXML_READER_HAS_MSVC_COM_SUPPORT)
		if (const HRESULT hr = CoCreateInstance(msxml_detail::legacyDomDocumentClass, nullptr, CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(this->m_document.ReleaseAndGetAddressOf())); FAILED(hr))
#else
		if (const HRESULT hr = CoCreateInstance(msxml_detail::legacyDomDocumentClass, nullptr, CLSCTX_INPROC_SERVER,
			msxml_detail::iidXmlDomDocument, reinterpret_cast<void**>(this->m_document.ReleaseAndGetAddressOf())); FAILED(hr))
#endif
			throw std::runtime_error("Failed to create XML DOM Document");
#if !defined MSXML_READER_USE_EXTERNAL_REFERENCE
		if (const HRESULT hr = this->m_document->put_resolveExternals(VARIANT_FALSE); FAILED(hr))
			throw std::runtime_error("Failed to disable external XML references");
#endif
#if !defined MSXML_READER_USE_DTD
		ComPtr<IXMLDOMDocument2> documentProperties;
#if defined(MSXML_READER_HAS_MSVC_COM_SUPPORT)
		if (const HRESULT hr = ComPtrAs(this->m_document, documentProperties); FAILED(hr))
#else
		if (const HRESULT hr = ComPtrAs(this->m_document, msxml_detail::iidXmlDomDocument2, documentProperties); FAILED(hr))
#endif
			throw std::runtime_error("Failed to access XML DOM Document properties");
		const BString prohibitDTD(L"ProhibitDTD");
		if (const HRESULT hr = documentProperties->setProperty(prohibitDTD(), Variant(true)); FAILED(hr))
			throw std::runtime_error("Failed to prohibit XML DTDs");
#endif
		VARIANT_BOOL loadResult = loadFunc(this->m_document);
		if (loadResult != VARIANT_TRUE) throw std::runtime_error("Failed to load XML");
	}
public:
	MSXMLReader(const std::wstring& xml) 
		: MSXMLReader([&](const ComPtr<IXMLDOMDocument>& document) {
				BString bstr(xml);
				VARIANT_BOOL loadResult{};
				if (const HRESULT hr = document->loadXML(bstr(), &loadResult); FAILED(hr))
					throw std::runtime_error("Failed to load XML");
				return loadResult;
			}
		) {}
	static MSXMLReader FromFile(const std::filesystem::path& xmlFilePath) {
		return MSXMLReader([&](const ComPtr<IXMLDOMDocument>& document) {
				VARIANT_BOOL loadResult{};
				if (const HRESULT hr = document->put_async(VARIANT_FALSE); FAILED(hr))
					throw std::runtime_error("Failed to set XML DOM Document to synchronous mode");
				if (const HRESULT hr = document->load(Variant(xmlFilePath.wstring().c_str()), &loadResult); FAILED(hr))
					throw std::runtime_error("Failed to load XML file");
				return loadResult;
			}
		);
	}
	XmlNodeList Load(const std::string& xpath) const {
		return XmlNodeList(this->m_document, msxml_detail::StringToWString(xpath));
	}
	XmlNodeList Load(const std::wstring& xpath) const {
		return XmlNodeList(this->m_document, xpath);
	}
private:
	std::pair<std::string, XmlNodeList> inLoad(const std::string& xpath) const {
		return { xpath, XmlNodeList(this->m_document, msxml_detail::StringToWString(xpath)) };
	}
	std::pair<std::wstring, XmlNodeList> inLoad(const std::wstring& xpath) const {
		return { xpath, XmlNodeList(this->m_document, xpath) };
	}
public:
	template<class ...Args>
	std::unordered_map<std::string, XmlNodeList> Load(const std::string& xpath, Args... args) const {
		std::unordered_map<std::string, XmlNodeList> nodes;
		nodes.reserve(1 + sizeof...(Args));
		nodes.emplace(inLoad(xpath));
		(nodes.emplace(inLoad(std::forward<Args>(args))), ...);
		return nodes;
	}
	template<class ...Args>
	std::unordered_map<std::wstring, XmlNodeList> Load(const std::wstring& xpath, Args... args) const {
		std::unordered_map<std::wstring, XmlNodeList> nodes;
		nodes.reserve(1 + sizeof...(Args));
		nodes.emplace(inLoad(xpath));
		(nodes.emplace(inLoad(std::forward<Args>(args))), ...);
		return nodes;
	}
};
