#define MSXML_READER_USE_EXTERNAL_REFERENCE
#include "CoInitializer.hpp"
#include "MSXMLReader.hpp"
#include <iostream>

int main() {
    try {
        CoInitializer com;
        try {
            const MSXMLReader reader(L"<!DOCTYPE root [<!ELEMENT root EMPTY>]><root/>");
        } catch (const std::runtime_error& error) {
            std::cout << "DTD rejected: " << error.what() << '\n';
            return 0;
        }
        std::cerr << "Expected the DTD to be rejected\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
    }
    return 1;
}
