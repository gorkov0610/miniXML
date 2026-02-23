#include <iostream>
#include "..\include\miniXML\document.hpp"

using namespace miniXML;
using namespace miniXML::details;
int main(){
    document d;
    //get the root
    auto& root = d.rootNode();
    //create the prolog
    auto prolog = std::make_unique<processingInstructionNode>("xml version=\"1.0\" encoding=\"UTF-8\"");
    std::cout << "prolog created\n";
    //create the parent
    auto parent = std::make_unique<elementNode>("parent");
    std::cout << "parent created\n";
    //create the child
    auto child = std::make_unique<node>(node_type::ELEMENT_NODE, "child");
    std::cout << "child created\n";
    //append an attribute to parrent
    parent->appendAttribute("id", "p001");
    //append all elements accordingly
    parent->appendChild(std::move(child));
    root.appendChild(std::move(prolog));
    root.appendChild(std::move(parent));

    //write the generated document to a file
    d.writeToFile("generated.xml");
    return 0;
}