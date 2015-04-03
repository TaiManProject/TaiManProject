#include "commonTool.h"


using namespace std;


string int2str(int &i){
	string s;
	stringstream ss(s);
	ss << i;

	return ss.str();
}

double RMS(cv::Mat& mat1, cv::Mat& mat2){ //change to reference?

	if (mat1.rows != mat2.rows){
		return -1.0;
	}
	if (mat1.cols != mat2.cols){
		return -1.0;
	}
	mat1.convertTo(mat1, CV_32F);
	mat2.convertTo(mat2, CV_32F);

	int numberOfElement = mat1.rows * mat1.cols;
	double result = 0;
	
    cv::Mat temp;
    temp = mat1 - mat2;

    cv::Mat dst;
	//dst.create(mat1.rows, mat1.cols, CV_32FC1);
    cv::pow(temp, 2.0, dst);

    cv::Scalar tsum;
    tsum = sum(dst);
    result = tsum.val[0] + tsum.val[1] + tsum.val[2];

	result /= numberOfElement;
	result = cv::sqrt(result);
	return result;

}

double patchDistance(cv::Mat& src1, cv::Mat& src2) {
	cv::Mat src1_32F;
	cv::Mat src2_32F;

	src1.convertTo(src1_32F, CV_32F);
	src2.convertTo(src2_32F, CV_32F);
    double result = 0;
    
	cv::Mat temp;
    temp = src1_32F - src2_32F;

    cv::Mat dst;
    pow(temp, 2.0, dst);

    cv::Scalar tsum;
    tsum = sum(dst);
    result = tsum.val[0] + tsum.val[1] + tsum.val[2];
	
	return result;
}

void buildMat2Vector(cv::Mat& input, vector<unsigned short>& output1DPatch) {
	for (int i = 0; i < input.rows; i++){
		for (int j = 0; j < input.cols; j++){
			unsigned short temp;
			temp = (unsigned short)input.at<unsigned char>(i, j);
            output1DPatch.push_back(temp);
		}
	}
}

int randomInt(int from, int to) {
	int random = rand();
	return (random % (to-from + 1)) + from;
}

void checkBound(int& boundColStart, int& boundColEnd, int& boundRowStart, int& boundRowEnd, const int imageColBound, const int imageRowBound){
	if (boundColStart < 0){
		boundColStart = 0;
	}
	if (boundColEnd > imageColBound){
		boundColEnd = imageColBound;
	}
	if (boundRowStart < 0){
		boundRowStart = 0;
	}
	if (boundRowEnd > imageRowBound){
		boundRowEnd = imageRowBound;
	}
}

void CommonTool::putLandmarksOnImage(cv::Mat& img, const std::vector<cv::Point2d>& landmarks, const CvScalar& color) {
    int radius = 1;
    int thickness = 2;
    int connectivity = 4;

    for (int i=0;i<(int)landmarks.size();i++) {
        CvPoint pt = cvPoint(landmarks.at(i).x,landmarks.at(i).y);
        cv::circle(img, pt, radius, color, thickness, connectivity);
    }
}

void CommonTool::showImageAndLandMarks(QString title, cv::Mat& img, std::vector<cv::Point2d>& landmarks1, std::vector<cv::Point2d>& landmarks2, CvScalar color1, CvScalar color2, QMutex& mutex) {
    mutex.lock();
    if (this->verboseFaceMatch) {
        if (!img.empty()) {
            //there is an actual image
            cv::Mat localImg = img.clone();
            putLandmarksOnImage(localImg, landmarks1, color1);
            putLandmarksOnImage(localImg, landmarks2, color2);
            cv::imshow(title.toStdString(), localImg);
        }
    }
    mutex.unlock();
}

void CommonTool::showImageAndLandMarks(QString title, cv::Mat& img, std::vector<cv::Point2d>& landmarks, CvScalar color, QMutex& mutex) {
    mutex.lock();
    if (this->verboseFaceMatch) {
        if (!img.empty()) {
            //there is an actual image
            cv::Mat localImg = img.clone();
            putLandmarksOnImage(localImg, landmarks, color);
            cv::imshow(title.toStdString(), localImg );
        } else {
            //show a black img
            cv::Mat black = cv::Mat::zeros(cv::Size(200, 250), CV_8UC1); // a black image
            cv::imshow( qPrintable(title), black);
        }
    }
    mutex.unlock();
}

void CommonTool::log(QString arg) {
     if (this->verboseFaceMatch) {
         qDebug() << arg;
     }
 }

CommonTool commonTool;

//Audrey
void CommonTool::setModeLoad(){
    loadMode = _LOAD_MODE_;
}
void CommonTool::setModeTrain(){
    loadMode = _TRAIN_MODE_;
}
