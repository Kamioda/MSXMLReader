#define MSXML_READER_USE_EXTERNAL_REFERENCE
#define MSXML_READER_USE_DTD
#include "CoInitializer.hpp"
#include "MSXMLReader.hpp"
#include <iostream>

int wmain(int argc, wchar_t* argv[]) {
    if (argc != 2) {
        std::wcerr << L"Usage: 04-external-dtd.exe <path-to-external-dtd.xml>\n";
        return 2;
    }
    try {
        CoInitializer com;
        const auto reader = MSXMLReader::FromFile(argv[1]);
        auto root = reader.Load(L"/root")[0];
        std::wcout << root.attribute()[L"source"].text() << L'\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
