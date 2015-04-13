#ifndef RESULT_H
#define RESULT_H
#include "person.h"
#include <string>
#include <vector>

class Result
{
public:
    Result();
    void setSynPerson(Person synPerson);
    Person synPerson;
    std::vector<Person> persons;
    int foundMethod;
};

#endif // RESULT_H
