#include <iostream>
#include "..\include\document.hpp"

using namespace miniXML;
using namespace miniXML::details;
int main(){
    document d;
    node& root = d.rootNode();
    auto prolog = std::make_unique<node>(node_type::PROCESSING_INSTRUCTION_NODE, "xml version=\"1.0\" encoding=\"UTF-8\"");
    std::cout << "prolog created\n";
    auto parent = std::make_unique<node>(node_type::ELEMENT_NODE, "parrent");
    std::cout << "parent created\n";
    auto child = std::make_unique<node>(node_type::ELEMENT_NODE, "child");
    std::cout << "child created\n";
    parent->appendAttribute("id", "p001");
    parent->appendChild(std::move(child));
    root.appendChild(std::move(prolog));
    root.appendChild(std::move(parent));

    d.writeToFile("generated.xml");
    return 0;
}