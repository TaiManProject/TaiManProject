#ifndef PERSON_H
#define PERSON_H
#include <string>
#include "IntDouble.h"

class Person
{
public:
    Person();
    ~Person();
    void setFull(IntDouble);
    void setPartial(IntDouble);
    void setPhotoPath(std::string photoPath);
    IntDouble getFull();
    IntDouble getParital();
    string getInfo();
    std::string getPhotoPath();
private:
    IntDouble full;
    IntDouble partial;
    std::string photoPath;
    std::string info;
};

#endif // PERSON_H
