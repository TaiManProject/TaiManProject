#ifndef COMMON_H
#define COMMON_H

#define _INTEGRALIMAGES_LOWER_ 0
#define _INTEGRALIMAGES_UPPER_ 1

#define _CONTROLLER_P2S_ 0
#define _CONTROLLER_S2P_ 1

#define _KDTREE_MULTI_ 0
#define _KDTREE_SINGLE_ 1

#define _KNN_BRUTEFORCE_ 0
#define _KNN_KDTREE_ 1
#define _KNN_RANDOM_ 2

#define _CPGV_SOURCE_ 0
#define _CPGV_TARGET_ 1

#define _CAN_GOOD_ 0
#define _CAN_RANDOM_ 1
#define _CAN_ADD_ 2

#define PI 3.141592
#define IMAGE_ROW 250
#define IMAGE_COL 200

#define FACE_RECOG_DB_ORG_SKETCH_FOLDER_NAME "Sketch/"
#define SKETCH_DIR "Sketch/"
#define PHOTO_DIR  "Photo/"
#define FACE_RECOG_TRAINED_FILE_NAME "FaceRecog.xml"

#define _CONTROLLER_P2S_ 0
#define _CONTROLLER_S2P_ 1

#define _LOAD_MODE_ 1
#define _TRAIN_MODE_ 2

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdio>

using namespace std;

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/contrib/contrib.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <QMutex>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QSortFilterProxyModel>

const QString dataPath = "../data/";
const QString photoPath = dataPath+"PhotoAll/";
const QString sketchPath = dataPath+"SketchAll/";
const QString videoPath = dataPath+"Video/";
const QString singleSavePath = dataPath+"faceRecognitionDir/";
const QString csvLoadPath = dataPath+"FaceRecognition/";


struct Parameter {
	bool initialized; // setup windows first time display?


	int mode; //S2P OR P2S
	bool newPatch; //use six new patch or not
	int foundKNNMode;
	int samplingRate;

	QString dataPath;
	QString inputImagePath;

	int numberOfImage;

	//int RealNumOfCan;          // select 10 candidates from the training dataset
	int numOfRandomCan;
	int numOfGoodCan;
	int SearchWidthUpper;       // 5 pixels for upper, lower, left and right search range for each window
	int SearchWidthLower;
	int patchSizeUpper;
	int patchSizeLower;
	int OverlapSizeUpper;
	int OverlapSizeLower;
	int imageRol;
	int imageCol;

	//KD-Tree
	int numberOfKDTree;
	int numberOfLeafChecked;
	int modeOfKDTree;
	bool additionDim;  // add X and Y dimensions
	int additionDimWeight; // dimension weight
};

struct patchIdentifier{
	cv::Range orgRowRange;
	cv::Range orgColRange;
	int numberOfImage;
	cv::Range rowRange; //startInclude, endExclude
	cv::Range colRange; //startInclude, endExclude
	float distance;
	int rowIndex; //for debug only
	int patchType;

};

std::string int2str(int &i);
double RMS(cv::Mat& mat1, cv::Mat& mat2);
double patchDistance(cv::Mat& src1, cv::Mat& src2);
void buildMat2Vector(cv::Mat& input, std::vector<unsigned short>& output1DPatch);
int randomInt(int from, int to); // inclusive
void checkBound(int& boundColStart, int& boundColEnd, int& boundRowStart, int& boundRowEnd, const int imageColBound, const int imageRowBound);

class CommonTool {
public:
   bool verboseFaceMatch;
   void log(QString arg);
   void showImageAndLandMarks(QString title, cv::Mat& img, std::vector<cv::Point2d>& landmarks, CvScalar color, QMutex& mutex);
   void showImageAndLandMarks(QString title, cv::Mat& img, std::vector<cv::Point2d>& landmarks1, std::vector<cv::Point2d>& landmarks2, CvScalar color1, CvScalar color2, QMutex& mutex);
   CommonTool() {
       verboseFaceMatch = true;
   }
   int loadMode; //Audrey
   void setModeTrain(); // Audrey
   void setModeLoad(); //Audrey

private:
    void putLandmarksOnImage(cv::Mat& img, const std::vector<cv::Point2d>& landmarks, const CvScalar& color);
};

extern CommonTool commonTool;



#endif 
