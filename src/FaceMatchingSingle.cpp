#include "FaceMatchingSingle.h"
#include <cstring>

FaceMatchingSingle::FaceMatchingSingle(int rowStart, int rowEnd, int colStart, int colEnd, std::vector<cv::Mat>& orgImages, std::vector<int>& labels, int faceMatchingControlerID, QMutex& openCVMutex) {

    this->faceMatchingControlerID = faceMatchingControlerID;
    this->rowRange.start = rowStart;
    this->rowRange.end = rowEnd;
    this->colRange.start = colStart;
    this->colRange.end = colEnd;
    this->openCVMutex = &openCVMutex;
    openCVMutex.lock();
    commonTool.log(QString("FaceMatchingSingle::FaceMatchingSingle. (rowStart, rowEnd, colStart, colEnd) = (%1, %2, %3, %4)").arg(rowStart).arg(rowEnd).arg(colStart).arg(colEnd));

    for (int i = 0; i < (int)orgImages.size(); i++) {
       cv::Mat croppedImg = this->cropImg(orgImages.at(i));
       //Audrey
       //No image flashing show, to save running time for some seconds
       //cv::namedWindow( "faceMatchingTraining", CV_WINDOW_NORMAL ); //crash here ...
       //       cv::imshow( "faceMatchingTraining", croppedImg );
       //
       this->images.push_back(croppedImg);
       //commonTool.log(QString("%1 %2").arg(croppedImg.cols).arg(croppedImg.rows));
       //commonTool.log(QString("%1").arg(croppedImg.type()));
    }

//    cv::destroyWindow("faceMatchingTraining");

    this->labels = labels;
    this->train();

    //Compute max distance for later use
    if (true) {
        maxDistance = 0;
        //commonTool.log("start getting max distance ...");
        // Audrey
        // Reconstruct faces and compute distance between randomly two images then
        //NIESH
        //Calculate cv::subspaceProject to many times. Could optimized here.
        for (int i=0;i<(int)images.size();i++) {
            cv::Mat projection1 = cv::subspaceProject(W, mean, images.at(i).reshape(1, 1));
            for (int j=i+1;j<(int)images.size();j++) {
                //commonTool.log(QString("%1 %2").arg(i).arg(j));
                cv::Mat projection2 = cv::subspaceProject(W, mean, images.at(j).reshape(1, 1));
                double distance = cv::norm(projection1, projection2);
                if (distance > maxDistance)
                    maxDistance = distance;
            }
        }
        //commonTool.log(QString("Stop getting max distance ... %1").arg(maxDistance));
    } else {
        //hardcode (not good ...) only OK for single level ...
        maxDistance = 10000;
    }

//    commonTool.log(QString("maxDistance ---> %1").arg(maxDistance));
    //Audrey
    //Save the trained model
    this->save(singleSavePath);
    openCVMutex.unlock();
}

FaceMatchingSingle::FaceMatchingSingle(int controllerID) {
    this->faceMatchingControlerID = controllerID;
}

void FaceMatchingSingle::train() {
    commonTool.log("FaceMatchingSingle::train().");
//    commonTool.log(QString("number of image in training DB --> %1").arg(images.size()));
    //Audrey
    //Comment next line
    //images.size();
    this->model = cv::createFisherFaceRecognizer();
    //this->model = cv::createFisherFaceRecognizer(100);
	this->model->train(images, labels);
    this->W = model->getMat("eigenvectors");
    this->mean = model->getMat("mean");
}

int FaceMatchingSingle::getArea() {
	return rowRange.size() * colRange.size();
}

void FaceMatchingSingle::save(QString dirPath) {
    QString dirPath1 = singleSavePath + QString::number(this->faceMatchingControlerID) + "_model.yaml";
//    commonTool.log(dirPath1);
    this->model->save(dirPath1.toStdString());

    //Audrey
//  Save all parameters to file
    QString dirPath2 = singleSavePath + QString::number(this->faceMatchingControlerID) + "_model_para";
//    commonTool.log(dirPath2);
    char tempc[100];
    std::string dpstring = dirPath2.toStdString();
    dpstring.copy(tempc,dpstring.size(),0);
    *(tempc + dpstring.size())=0;
//    commonTool.log(dirPath2);
    FILE* file1 = std::fopen(tempc,"w");
//    commonTool.log(dirPath2);
    fprintf(file1,"%lf\n", maxDistance); //Save maxDistance
    fprintf(file1, "%d\n", similarities.size());  //Save similarities
    for (int i=0; i<similarities.size(); ++i) {
        fprintf(file1, "%lf ", similarities.at(i));
    }
    fprintf(file1, "%d %d %d %d\n", rowRange.start, rowRange.end, colRange.start, colRange.end); //Save four paras
    fprintf(file1, "%d\n", labels.size()); //Save labels
    for (int i=0; i<labels.size(); ++i) {
        fprintf(file1, "%d ", labels.at(i));
    }
    fprintf(file1, "\n");
    fclose(file1);
}

void FaceMatchingSingle::load(QString dirPath, int controllerID) {
    //Audrey
    this->faceMatchingControlerID = controllerID;

    //    Load model
    this->model = cv::createFisherFaceRecognizer();
    QString dirPath1 = singleSavePath + QString::number(this->faceMatchingControlerID) + "_model.yaml";
    this->model->load(dirPath1.toStdString());
    this->W = model->getMat("eigenvectors");
    this->mean = model->getMat("mean");

//    commonTool.log("FaceMatchingManager::trainMulti.  1");

    QString dirPath2 = singleSavePath + QString::number(this->faceMatchingControlerID) + "_model_para";

//    Load maxDistance
    char tempc[100];
    double maxDistanceTemp;
    std::string dpstring = dirPath2.toStdString();
    dpstring.copy(tempc,dpstring.size(),0);
    *(tempc + dpstring.size())=0;  //Convert string to char
    FILE* file2 = std::fopen(tempc,"r");
    fscanf(file2,"%lf", &maxDistanceTemp);
    this->maxDistance = maxDistanceTemp;

//    Load similarities vector
    int size;
    double similaritiesTemp;
    fscanf(file2, "%d", &size);
    for (int i=0; i<size; ++i) {
        fscanf(file2, "%lf", &similaritiesTemp);
        this->similarities.push_back(similaritiesTemp);
    }

//    Load the four coordinate values, which will be used to call controllerID
    int rs, re, cs, ce;
    fscanf(file2, "%d %d %d %d", &rs, &re, &cs, &ce);
    this->rowRange.start = rs;
    this->rowRange.end = re;
    this->colRange.start = cs;
    this->colRange.end = ce;

//    Load labels
    int labelsTemp;
    fscanf(file2, "%d", &size);
    for (int i=0; i<size; ++i) {
        fscanf(file2, "%d", &labelsTemp);
        this->labels.push_back(labelsTemp);
    }
    fclose(file2);

//    commonTool.log("FaceRec data all loaded successfully!");
}

double FaceMatchingSingle::getDistance(cv::Mat& target1, cv::Mat& target2) {
    cv::Mat sample1 = cropImg(target1);
    cv::Mat sample2 = cropImg(target2);
    if (sample1.type() == CV_8UC3) {
        commonTool.log("error, why color?");
        cv::cvtColor(sample1, sample1, CV_BGR2GRAY);
    }
    if (sample2.type() == CV_8UC3) {
        commonTool.log("error, why color ?");
        cv::cvtColor(sample2, sample2, CV_BGR2GRAY);
    }
    if (sample1.type() != sample2.type()) {
        commonTool.log("error, diffrent image type of sample");
    }
    cv::Mat proj1 = cv::subspaceProject(W, mean, sample1.reshape(1, 1));
    cv::Mat proj2 = cv::subspaceProject(W, mean, sample2.reshape(1, 1));
    return cv::norm(proj1, proj2);
}

std::vector<double> FaceMatchingSingle::getNormalizedSimilarity(cv::Mat& target1, cv::Mat& target2) {
    cv::Mat sample1 = cropImg(target1);
    cv::Mat sample2 = cropImg(target2);
    if (sample1.type() == CV_8UC3) {
        cv::cvtColor(sample1, sample1, CV_BGR2GRAY);
    }
    if (sample2.type() == CV_8UC3) {
        cv::cvtColor(sample2, sample2, CV_BGR2GRAY);
    }
    cv::Mat proj1 = cv::subspaceProject(W, mean, sample1.reshape(1, 1));
    cv::Mat proj2 = cv::subspaceProject(W, mean, sample2.reshape(1, 1));
    double distance = cv::norm(proj1, proj2);
    double similarity = ((maxDistance - distance) / maxDistance) * 100;

    if (similarity < 0){
        similarity = 0.0;
    }
    if (similarity > 100.0){
        similarity = 100.0;
    }
    std::vector<double> similarities;
    similarities.push_back(similarity);
    this->similarities.push_back(similarity);
    //Audrey
    //All other calling functions call this similarities values
    return similarities;
}

cv::Mat FaceMatchingSingle::cropImg(cv::Mat img) {
    return img(rowRange, colRange).clone(); //is it necessary to clone here?
}
