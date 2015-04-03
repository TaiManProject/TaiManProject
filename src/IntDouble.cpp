#include "IntDouble.h"

IntDouble::IntDouble()
{

}

IntDouble::~IntDouble()
{

}

void IntDouble::print() {
    commonTool.log(QString("i = %1, d = %2, confidence = %3").arg(this->i).arg(this->d).arg(this->confidence));
}
