#include "person.h"

Person::Person()
{

}

Person::~Person()
{

}

void Person::setFull(IntDouble full) {
    this->full = full;
}

void Person::setPartial(IntDouble partial) {
    this->partial = partial;
}

IntDouble Person::getFull(){
    return this->full;
}

IntDouble Person::getParital(){
    return this->partial;
}

std::string Person::getInfo(){
    char c[200];
    std::sprintf(c, "Full similarity:\t%.2f\tConfidence:\t%.2f\nPartial similarity:\t%.2f\tConfidence:\t%.2f", this->full.d, this->full.confidence, this->partial.d, this->partial.confidence);
    std::cout << c << "###" << std::endl;
//    return c;
    return std::string(c);
}

void Person::setPhotoPath(std::string photoPath){
    this->photoPath = photoPath;
}

std::string Person::getPhotoPath() {
    return this->photoPath;
}
