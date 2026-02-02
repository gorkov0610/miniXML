#include "..\include\document.hpp"


using namespace miniXML;
int main(){
    document d("file.xml");
    d.writeToFile("parsed.xml");
    return 0;
}
