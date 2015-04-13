#include "result.h"

Result::Result()
{
    this->synPerson = Person();
    this->persons = std::vector<Person>();
    this->foundMethod = 0;
}

void Result::setSynPerson(Person synPerson) {
    this->synPerson = synPerson;
}
