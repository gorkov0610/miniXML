# miniXML - The minimalist XML parser  
miniXML is a small, header-only XML parser and writer, written in modern C++.  
It provides a simple DOM-like tree structure for parsing, inspecting, modifying and serializing XML documents.   
       
## Contents:       
- [Features](#features)
- [Limitations](#limitations)
- [Installation](#installation)
- [Basic usage](#basic-usage)
- [Requirements](#requirements)
- [Design](#design)
- [License](#license)

## Features  
- DOM-style XML tree representation
- Parsing from files or trees
- Writing XML back to files or strings
- Element, text, comment and processing instruction nodes
- Attribute support
- Simple tree navigation and modification
- Header-only, no dependencies

## Limitations
miniXML is intentionally minimal. It doesn't support:
- XML namespaces
- DTD or Schema validations
- Entity expansion
- Advanced encodings (currently only UTF-8)

## Installation
Simply copy the `miniXML` headers in your project, and include `document.hpp`.
```cpp
#include "include/document.hpp"
```
## Basic usage
```cpp
#include "include/document.hpp"
#include <iostream>

int main(){
    miniXML::document d("file.xml");

    auto& root = d.rootNode();
    auto prolog = root.findChild(miniXML::details::node_type::PROCESSING_INSTRUCTION_NODE);

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
    auto prolog = std::make_unique<miniXML::node>(miniXML::details::node_type::PROCESSING_INSTRUCTION_NODE, "xml version=\"1.0\" encoding=\"UTF-8\"");
    //create the parent tag
    auto parent = std::make_unique<miniXML::node>(miniXML::details::node_type::ELEMENT_NODE, "parent");
    //create the text
    auto child = std::make_unique<miniXML::node>(miniXML::details::node_type::TEXT_NODE, "Hello World!");
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
    auto note = root.findChild(miniXML::details::node_type::ELEMENT_NODE);
    auto from = note->findChild("from");
    if(from){
        std::cout << "Node found" << std::endl;
        auto attributes = from->getAttributes(); 
        if(!attributes.empty()){
            std::cout << "The node has attributes:" << std::endl;
            for(const auto& a : attributes){
                std::cout << a.first << "=\'" << a.second << "\'" << std::endl;
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
## Requirements
- C++17 or newer
- Standard library only
## Design
- `document` owns the XML content and parsing logic
- `node` represents a single XML node in the tree
- Internal parsing types are kept in `miniXML::details`
- Memory is managed using `std::unique_ptr`
- Parsing and writing logic is separated in `parser.hpp`
## License
miniXML is released under the MIT License. See `LICENSE` for more details.