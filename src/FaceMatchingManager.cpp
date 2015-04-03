#include "FaceMatchingManager.h"
#include "FaceMatchingSingle.h"

const std::string FaceMatchingManager::trainingDirectory = "FaceRecognition/";
const std::string FaceMatchingManager::trainingFileName = "FaceRecognitionPhoto.csv";

FaceMatchingManager::FaceMatchingManager() {
    alarmThreshold = 79.9;
}

bool FaceMatchingManager::readCSV(const std::string& filename, std::vector<std::string>& imagesPath, std::vector<int>& labels) {
    std::ifstream file(filename.c_str(), std::ifstream::in);

    commonTool.log(QString("----------------------> Reading CSV file: %1.").arg(filename.c_str()));
    //commonTool.log(QString("using seperator ---> %1").arg(separator));

    if (!file) {
        std::string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
        return false;  // Audrey TODO
        // Add a error exception
    }

    std::string line, path, classlabel;
    while (std::getline(file, line)) {
        //
        //commonTool.log(QString("%1").arg(QString::fromStdString(line)));
        //

        std::stringstream liness(line);
        //std::getline(liness, path, separator);
        std::getline(liness, path, ';');
        std::getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty()) {
            imagesPath.push_back(path);
            int l = atoi(classlabel.c_str());
            labels.push_back(l);
            //
            //commonTool.log(QString("%1 <> %2").arg(QString::fromStdString(line)).arg(l));
            //
        }
    }
    return true;
}

void FaceMatchingManager::train(QString faceRecognitionDir, std::vector<cv::Mat>& additionalImages, QMutex& openCVMutex) {
    commonTool.log(QString("FaceMatchingManager::train(%1)").arg( faceRecognitionDir));
    if (enableMultiLevel) {
        trainMulti(faceRecognitionDir, openCVMutex);
    } else {
        trainSingle(faceRecognitionDir, additionalImages, openCVMutex);
    }
}

//TODO: incooperate the additional images!
void FaceMatchingManager::trainMulti(QString faceRecognitionDir, QMutex& openCVMutex) {
    commonTool.log(QString("FaceMatchingManager::trainMulti(%1)").arg( faceRecognitionDir));
    QString csvPath = faceRecognitionDir + QString::fromStdString(trainingFileName);

    std::vector<std::string> imagesPath;
    std::vector<int> labels;

    readCSV(csvPath.toStdString(), imagesPath, labels);
    for (int i = 0; i < (int)imagesPath.size(); i++){
        imagesPath[i] = faceRecognitionDir.toStdString() + imagesPath[i];
        //commonTool.log(QString::fromStdString(imagesPath[i]));
    }

    std::vector<cv::Mat> images;
    for (int i = 0; i < (int)imagesPath.size(); i++) {
        cv::Mat img = cv::imread(imagesPath.at(i), 0);
        if (img.empty()){
            commonTool.log("Error empty image!!!!!!!!!");
            exit(1);
        }
        // Audrey TODO
        // No push back if empty
        images.push_back(img);
    }

    commonTool.log("Images loaded!");

    if (commonTool.loadMode == _TRAIN_MODE_) {
        commonTool.log("FaceMatchingManager::trainMulti.");

    } else if (commonTool.loadMode == _LOAD_MODE_) {
        commonTool.log("FaceMatchingManager::loadMulti.");
    }
    controllerMulti = new FaceMatchingMultiple(images, labels, openCVMutex);
}

void FaceMatchingManager::trainSingle(QString faceRecognitionDir, std::vector<cv::Mat>& additionalImages, QMutex& openCVMutex) {

    commonTool.log("FaceMatchingManager::trainSingle.");

    QString csvPath = faceRecognitionDir  + QString::fromStdString(trainingFileName);

    std::vector<std::string> imagesPath;
    std::vector<int> labels;

    bool justSuspects = false;

    std::vector<cv::Mat> images;

    if (!justSuspects) {
        readCSV(csvPath.toStdString(), imagesPath, labels);
        for (int i = 0; i < (int)imagesPath.size(); i++){
            imagesPath[i] = faceRecognitionDir.toStdString() + imagesPath[i];
        }
        for (int i = 0; i < (int)imagesPath.size(); i++) {
            cv::Mat img = cv::imread(imagesPath.at(i), 0);
            if (img.empty())
                commonTool.log("error empty image!!!!!!!!!");
            images.push_back(img);
        }
    }

    for (int i=0;i<(int)additionalImages.size();i++) {
        images.push_back(additionalImages.at(i));
        if (justSuspects) {
            labels.push_back(i);
        } else {
            labels.push_back(labels.back()+1+i);
        }


    }

    //for (int i=0;i<labels.size();i++) {
    //    commonTool.log(QString("%1").arg(labels.at(i)));
    //}

    controllerSingle = new FaceMatchingSingle(ORG_ROW_FROM, ORG_ROW_TO, ORG_COL_FROM, ORG_COL_TO, images, labels, 0, openCVMutex);
}

//void FaceMatchingManager::faceMatchMulti(cv::Mat target1, cv::Mat target2, double &similarity, double &confidence){
//    controllerMulti->faceMatching(target1, target2, similarity, confidence);
//}

//void FaceMatchingManager::faceMatchSingle(cv::Mat target1, cv::Mat target2, double &similarity){
//    similarity = controllerSingle->faceMatching(target1, target2);
//}

double FaceMatchingManager::getDistance(cv::Mat target1, cv::Mat target2) {
    return controllerSingle->getDistance(target1, target2);
}

std::vector<double> FaceMatchingManager::getNormalizedSimilarity(cv::Mat target1, cv::Mat target2) {
    if (enableMultiLevel) {
        return controllerMulti->getNormalizedSimilarity(target1, target2);
    } else {
        return controllerSingle->getNormalizedSimilarity(target1, target2);
    }
}

cv::Mat FaceMatchingManager::cropImg(cv::Mat img) {
    if (enableMultiLevel) {
        return controllerMulti->cropImg(img);
    } else {
        return controllerSingle->cropImg(img);
    }
}


//double FaceMatchingManager::getConfidence(cv::Mat& img) {
//    return controllerSingle->getConfidence(img);
//}

