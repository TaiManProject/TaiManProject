#pragma once

#include "commonTool.h"

class FaceMatchingSingle {
public:
    FaceMatchingSingle(int rowStart, int rowEnd, int colStart, int colEnd, std::vector<cv::Mat>& orgImages, std::vector<int>& labels, int faceMatchingControlerID, QMutex& openCVMutex);
    FaceMatchingSingle(int controllerID);
	void train();
	int getArea();
	void save(QString dirPath);
    void load(QString dirPath, int controllerID);
    //double faceMatching(cv::Mat& target1, cv::Mat& target2);
    double getDistance(cv::Mat& target1, cv::Mat& target2);
    std::vector<double> getNormalizedSimilarity(cv::Mat& target1, cv::Mat& target2);
    //double getConfidence(cv::Mat& img);

	//ID for mutil face recog voting
    int faceMatchingControlerID;

	//ROI of the image
	cv::Range rowRange;
	cv::Range colRange;

    std::vector<cv::Mat> images;
    std::vector<int> labels;
	cv::Ptr<cv::FaceRecognizer> model;

	// face recognition mat
	cv::Mat W;
	cv::Mat mean;

    QMutex* openCVMutex;

    double maxDistance;

    //Audrey
    std::vector<double> similarities;
    //Audrey
    QString dirPath;

	//for resize the image
    cv::Mat cropImg(cv::Mat img);

    //Audrey
    void writeMatToFile(cv::Mat &I, QString path);
    void readFileToMat(cv::Mat &I, QString path);
};

