![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)
# miniXML - The minimalist XML parser  
miniXML is a small, header-only XML parser and writer, written in modern C++.  
It provides a simple DOM-like tree structure for parsing, inspecting, modifying and serializing XML documents.   
       
## Contents: 
- [Status](#status)      
- [Features](#features)
- [Limitations](#limitations)
- [Installation](#installation)
- [Basic usage](#basic-usage)
- [Ownership](#ownership)
- [Requirements](#requirements)
- [Design](#design)
- [License](#license)

## Status

miniXML is a stable, minimal XML parser intended for small and medium-sized XML processing tasks.
The project focuses on simplicity, readability and a small API surface.

## Features  
- DOM-style XML tree representation
- Parsing XML from files or strings
- Writing XML back to files or strings
- Element, text, comment and processing instruction nodes
- Attribute support
- XML namespace support
- Simple tree navigation and modification
- Header-only, no dependencies

## Limitations
miniXML is intentionally minimal. It doesn't support:
- DTD or Schema validations
- Entity expansion
- Advanced encodings (currently only UTF-8)

## Installation
### 1. Single include (header-only)  
Simply copy the `miniXML` headers in your project, and include `document.hpp`.
```cpp
#include "include/document.hpp"
```
### 2. Using CMake  
If you want to manage dependencies via CMake, the library provides an **INTERFACE** target.
```
cmake_minimum_required(VERSION 3.15)
project(MyProject)

add_executable(MyExecutable main.cpp)

# Add miniXML
add_subdirectory(path/to/miniXML)

# Link the interface target
target_link_libraries(MyExecutable PRIVATE miniXML)

set_target_properties(
    MyExecutable PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)
```      
- `path/to/miniXML` should point to the folder containing `miniXML` and its `CMakeLists.txt`.   
- The `miniXML` folder should be the root folder containing `CMakeLists.txt` and `include/`.
- This makes the project aware of the headers without modifying them.   
- The library is header-only, so no compilation is required beyond including the headers.
## Basic usage
```cpp
#include "include/document.hpp"
#include <iostream>

int main(){
    miniXML::document d("file.xml");

    auto& root = d.rootNode();
    auto* prolog = root.findChild<miniXML::processingInstructionNode>("xml version=\"1.0\" encoding=\"UTF-8\"");

    std::cout << prolog->toString();
    return 0;
}
```
Compile this and run it and you should get `<xml version='1.0' encoding='UTF-8'>`, or the prolog of the document.  
```cpp
#include "include/document.hpp"

int main(){
    miniXML::document d;

    //get the root
    auto& root = d.rootNode();
    //create the prolog
    auto prolog = std::make_unique<miniXML::processingInstructionNode>("xml version=\"1.0\" encoding=\"UTF-8\"");
    //create the parent tag
    auto parent = std::make_unique<miniXML::elementNode>("parent");
    //create the text
    auto child = std::make_unique<miniXML::textNode>("Hello World!");
    //append an attribute and the text to parent
    parent->appendAttribute("foo", "bar");
    parent->appendChild(std::move(child));
    //append the 2 elements to root
    root.appendChild(std::move(prolog));
    root.appendChild(std::move(parent));

    //write the document to a file
    d.writeToFile("generated.xml");
    return 0;
}
```
This is how you would generate a file using the library.  
```cpp
#include "include/document.hpp"
#include <iostream>

int main(){
    miniXML::document d("file.xml");

    //get the root
    auto& root = d.rootNode();
    auto* note = root.findChild<miniXML::elementNode>("note");
    auto* from = note ? note->findChild<miniXML::elementNode>("from") : nullptr;
    if(from){
        std::cout << "Node found" << std::endl;
        auto attributes = from->getAttributes(); 
        if(!attributes.empty()){
            std::cout << "The node has attributes:" << std::endl;
            for(const auto& a : attributes){
                std::cout << a.qualifiedName << "='" << a.value << "'\n";
            }
        }
    }else{
        std::cout << "Node not found" << std::endl;
    }
    return 0;
}
```
**Note:** for this example you need the `file.xml` in the `testing` folder.   
This should inform if a certain node is present in the file and if it has attributes, it will print them.

## Ownership

Nodes are owned through `std::unique_ptr`.

Ownership is transferred when using `appendChild()`:

```cpp
auto child = std::make_unique<miniXML::textNode>("Hello");
parent->appendChild(std::move(child));
```

A node may only belong to a single parent.
## Requirements
- C++17 or newer
- Standard library only
## Design
- DOM-style tree implementation
- Ownership handled through `std::unique_ptr`
- Parent links are stored as non-owning pointers
- XML namespaces are resolved during parsing
- Parsing and writing logic are separated
- Polymorphic node hierarchy based on virtual dispatch
- Works with *GCC*, *Clang* and *MSVC*
## License
miniXML is released under the MIT License. See `LICENSE` for more details.