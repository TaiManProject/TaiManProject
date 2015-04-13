#include "person.h"

Person::Person()
{
    this->full = IntDouble();
    this->partial = IntDouble();
    this->photoPath = "";
    this->landmarks = std::vector<cv::Point2d>();
    this->similarities = std::vector<double>();
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
//    std::cout << c << "###" << std::endl;
//    return c;
    return std::string(c);
}

void Person::setPhotoPath(std::string photoPath){
    this->photoPath = photoPath;
}

std::string Person::getPhotoPath() {
    return this->photoPath;
}

void Person::setSimilarities(std::vector<double> similarities) {
    this->similarities = similarities;
}

std::vector<double> Person::getSimilarities() {
    return this->similarities;
}

void Person::setLandmarks(std::vector<cv::Point2d> landmarks) {
    this->landmarks = landmarks;
}

std::vector<cv::Point2d> Person::getLandmarks() {
    return this->landmarks;
}
