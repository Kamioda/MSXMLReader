#define MSXML_READER_USE_DTD
#include "CoInitializer.hpp"
#include "MSXMLReader.hpp"
#include <iostream>

int main() {
    try {
        CoInitializer com;
        const MSXMLReader reader(LR"(
<!DOCTYPE root [
    <!ELEMENT root (#PCDATA)>
    <!ENTITY label "Internal DTD">
]>
<root>&label;</root>)");
        std::wcout << reader.Load(L"/root")[0].text() << L'\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
