#pragma once

#include "commonTool.h"
#include "FaceMatchingSingle.h"
#include "FaceMatchingSimilarityTree.h"

//hardcode the face bounding box
//orig MO
//const int ORG_ROW_FROM = 99;
//const int ORG_ROW_TO = 187;
//const int ORG_COL_FROM = 56;
//const int ORG_COL_TO = 144;

//Dirk first adjust
//const int ORG_ROW_FROM = 90;
//const int ORG_ROW_TO = 210;
//const int ORG_COL_FROM = 56;
//const int ORG_COL_TO = 144;

//Audrey
//Crop original image to a fixed size
const int ORG_ROW_FROM = 80;
const int ORG_ROW_TO = 210;
const int ORG_COL_FROM = 50;
const int ORG_COL_TO = 150;

const int _1x1_ = 0;
const int _2x1_ = 10;
const int _2x2_ = 20;
const int _4x1_ = 30;
const int _4x2_ = 40;

// Audrey
// used to match for different parts of image in different levels
const int numberOfLevel = 4;
const int numberOfRowInController[numberOfLevel] = {1, 2, 2, 4};
const int numberOfColInController[numberOfLevel] = {1, 1, 2, 2};


const double SIMILARITY_THRESHOLD = 0.5;

class FaceMatchingMultiple {
public:
    //    FaceMatchingMultiple(std::vector<cv::Mat> imgs, QString dirPath);
    FaceMatchingMultiple(std::vector<cv::Mat> imgs, std::vector<int> labels, QMutex& openCVMutex);
    ~FaceMatchingMultiple(void);
    void train();
    //	void save(QString dirPath);
    //    void load(QString dirPath);
    //Audrey
    //Useless here
    //	void faceMatching(cv::Mat target1, cv::Mat target2, double &similarity, double &confidence);
    std::vector<cv::Mat> images;
    std::vector<int> labels;
    std::vector<FaceMatchingSingle> faceMatchingControllers;
    void computeSimilarityAndConfidence(QList<double> similarityForAllController, double &similarity, double &confidence);
    std::vector<double> getNormalizedSimilarity(cv::Mat& target1, cv::Mat& target2);
    std::vector<double> getAllNormalizedSimilarity(cv::Mat& target1, cv::Mat& target2);

    //Audrey
    std::vector<double> similarityForAllControllers;

    cv::Mat cropImg(cv::Mat img);
    //Audrey
    QString dirPath;
};

