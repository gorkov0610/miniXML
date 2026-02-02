#include <iostream>
#include "..\include\document.hpp"

using namespace miniXML;

int main(){
    document d("file.xml");

    std::string content = d.rootNode().toString(2);
    std::cout << content;
    return 0;
}