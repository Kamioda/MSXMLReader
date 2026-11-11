cmake_minimum_required(VERSION 3.21)

project(MSXMLReaderSamples LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "MSXMLReader samples require Windows and MSXML.")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(add_msxmlreader_sample target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE ole32 oleaut32)
    if(MSVC)
        target_compile_options(${target} PRIVATE /EHsc)
        target_link_libraries(${target} PRIVATE comsuppw)
    endif()
endfunction()

add_msxmlreader_sample(01-default 01-default.cpp)
add_msxmlreader_sample(02-internal-dtd 02-internal-dtd.cpp)
add_msxmlreader_sample(03-external-references-only 03-external-references-only.cpp)
add_msxmlreader_sample(04-external-dtd 04-external-dtd.cpp)

if(MINGW OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU"))
    target_link_options(04-external-dtd PRIVATE -municode)
endif()

# The fourth sample opens the XML by path and resolves its sibling DTD.
configure_file(external-dtd.xml external-dtd.xml COPYONLY)
configure_file(external-dtd.dtd external-dtd.dtd COPYONLY)
