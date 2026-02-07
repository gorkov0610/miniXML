#include <iostream>
#include "..\include\miniXML\document.hpp"

using namespace miniXML;

int main(){
    document d("file.xml");

    //serialize with 2 spaces indentation
    std::string content = d.rootNode().toString(2);
    std::cout << content;
    return 0;
}