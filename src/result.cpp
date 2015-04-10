#include "result.h"

Result::Result()
{
    this->synPhoto = "";
    this->persons = std::vector<Person>();
    this->foundMethod = 0;
}

void Result::setSynPhoto(std::string synPhoto) {
    this->synPhoto = synPhoto;
}
