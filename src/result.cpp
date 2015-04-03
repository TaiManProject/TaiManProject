#include "result.h"

Result::Result()
{
    this->synPhoto = "";
    this->persons = std::vector<Person>();
}

void Result::setSynPhoto(std::string synPhoto) {
    this->synPhoto = synPhoto;
}
