#pragma once

#include "FaceMatchingMultiple.h"
#include "commonTool.h"


//supports both single and multi level matching!
class FaceMatchingManager {
public:
    FaceMatchingManager();

    //threshold for visualization of alarm result
    double alarmThreshold;

    bool enableMultiLevel;

    static const std::string trainingDirectory;
    static const std::string trainingFileName;

    void train(QString faceRecognitionDir, std::vector<cv::Mat>& additionImages, QMutex& openCVMutex);
    void trainMulti(QString faceRecognitionDir,  QMutex& openCVMutex);
    void trainSingle(QString faceRecognitionDir, std::vector<cv::Mat>& additionImages, QMutex& openCVMutex);

    //void faceMatchMulti(cv::Mat target1, cv::Mat target2, double &similarity, double &confidence);
    double getDistance(cv::Mat target1, cv::Mat target2);
    std::vector<double> getNormalizedSimilarity(cv::Mat target1, cv::Mat target2);

    FaceMatchingMultiple* controllerMulti;
    FaceMatchingSingle* controllerSingle;

    bool readCSV(const std::string& filename, std::vector<std::string>& imagesPath, std::vector<int>& labels);

    cv::Mat cropImg(cv::Mat img);

};

