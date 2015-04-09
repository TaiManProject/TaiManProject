#ifndef PERSON_H
#define PERSON_H
#include <string>
#include <vector>
#include <opencv/cv.h>
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
    void setSimilarities(std::vector<double> similarities);
    std::vector<double> getSimilarities();
    void setLandmarks(std::vector<cv::Point2d> landmarks);
    std::vector<cv::Point2d> getLandmarks();

private:
    IntDouble full;
    IntDouble partial;
    std::string photoPath;
    std::string info;
    std::vector<double>  similarities;
    std::vector<cv::Point2d> landmarks;
};

#endif // PERSON_H
