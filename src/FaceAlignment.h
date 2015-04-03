#pragma once

#include "commonTool.h"
#include "stasm4/stasm_lib.h"

#include <sstream>
#include <string>
//stasm3
const int LEFT_EYE_INDEX = 31;
const int RIGHT_EYE_INDEX = 36;
const int NOSE_INDEX = 67;
//stasm4
const int LEFT_EYE_INDEXV4 = 38;
const int RIGHT_EYE_INDEXV4 = 39;
const int NOSE_INDEXV4 = 52;


class FaceAlignment {
public:
    FaceAlignment(std::string path, QMutex& mutex);
    bool alignStasm4(cv::Mat& inputImg, cv::Mat& alignedImg, std::vector<cv::Point2d>& landmarkPoints, cv::Point2d& centerOfLandmarks);
    bool alignStasm4(cv::Mat& inputImg, std::vector<cv::Mat>& alignedImg, std::vector<std::vector<cv::Point2d> >& landmarkPoints, std::vector<cv::Point2d>& centersOfLandmarks);
    bool detectLandmarks(cv::Mat& img, std::vector<std::vector<cv::Point2d> >& landmarkPoints, std::vector<cv::Point2d>& centers);

private:
    std::string path1;
    std::string path2;
    std::string dataPath;
	cv::Point2d rotatePoint(cv::Point2d targetPoint, double degree, cv::Point2d minPoint);
	cv::Point2d rotatePoint(cv::Point2d targetPoint, cv::Mat rotationMat);
    QMutex* mutex;
};
