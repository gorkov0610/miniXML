#include <iostream>
#include "..\include\miniXML\document.hpp"

using namespace miniXML;

int main(){
    std::string content = "<?xml verstion=\"1.0\" encoding=\"UTF-8\"?><note><from name=\"Person A\" /><to name=\"Person b\" /></note>";
    document d;
    d.parseFromString(content);
    d.writeToFile("fromString.xml");
    std::cout << d.rootNode().toString();
    return 0;
}