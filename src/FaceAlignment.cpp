#include "FaceAlignment.h"

FaceAlignment::FaceAlignment(std::string dataPath, QMutex& mutex) {
    commonTool.log(QString("Init Face Alignment with data path --> %1").arg(QString::fromStdString(dataPath)));
    this->dataPath = dataPath + "/faceAlignData/";
    this->mutex = &mutex;
    stasm_init(this->dataPath.c_str(), 0);
}

bool FaceAlignment::detectLandmarks(cv::Mat& img, std::vector<std::vector<cv::Point2d> >& points, std::vector<cv::Point2d>& centers) {

    float landmarks[2 * stasm_NLANDMARKS];
    mutex->lock();
    //int success =  stasm_search_single(&foundface, landmarks, (char*)img.data, img.cols, img.rows, dataPath.c_str(), dataPath.c_str());
    //if (!success) commonTool.log("Error!!!");

    int allowMultiFace = 0;
    int minFaceWidthInpercentOfWidth = 5;
    int success = stasm_open_image((char*)img.data, img.cols, img.rows, dataPath.c_str(), allowMultiFace, minFaceWidthInpercentOfWidth);
    if (!success) commonTool.log("Error!!!");

    int foundface = 1;
    bool aFaceWasFound = false;

    while (foundface) {
        success =  stasm_search_auto(&foundface, landmarks);

        if (!success) commonTool.log("Error when calling stasm_search_auto");
        //commonTool.log(QString("FoundFace --> %1").arg(foundface));
        //if (!success || foundface == 0) {
        //    return false;
        //}
        if (foundface) {
            aFaceWasFound = true;
            int numberOfPoint = stasm_NLANDMARKS;
            std::vector<cv::Point2d> landmarkList;
            cv::Point2d center = cv::Point2d(0.0, 0.0);
            for (int i = 0; i < numberOfPoint; i++) {
                cv::Point2d p = cv::Point2d(landmarks[i*2], landmarks[i*2+1]);
                landmarkList.push_back(p);
                center += p;
            }
            points.push_back(landmarkList);
            center = cv::Point2d(center.x/numberOfPoint, center.y/numberOfPoint);
            centers.push_back(center);
        }
    }
    mutex->unlock();
    return aFaceWasFound;
}

//single
bool FaceAlignment::alignStasm4(cv::Mat& inputImg, cv::Mat& alignedImg, std::vector<cv::Point2d>& landmarkPoints, cv::Point2d& centerOfLandmarks) {
       std::vector<cv::Mat> imgList;
       std::vector<std::vector<cv::Point2d> > landmarkList;
       std::vector<cv::Point2d> centersOfLandmarks;
       bool success = alignStasm4(inputImg, imgList, landmarkList, centersOfLandmarks);
       if (imgList.size() > 0) {
           alignedImg = imgList.at(0);
       }
       if (landmarkList.size() > 0) {
           landmarkPoints = landmarkList.at(0);
       }
       if (centersOfLandmarks.size() > 0) {
           centerOfLandmarks = centersOfLandmarks.at(0);
       }
       return success;
}

//multi
bool FaceAlignment::alignStasm4(cv::Mat& inputImg, std::vector<cv::Mat>& alignedImg, std::vector<std::vector<cv::Point2d> >& landmarkPoints, std::vector<cv::Point2d>& centersOfLandmarks) {

    //cv::Mat im_gray;
    //cv::cvtColor(inputImg,im_gray,CV_RGB2GRAY);

    //commonTool.log("Performing Active Shape Models with Stasm 4.0 to find features in face.");

    //cv::namedWindow( "test", CV_WINDOW_NORMAL );
    //cv::imshow( "test", inputImg );

    std::vector<std::vector<cv::Point2d> > landmarkList;
    bool success = detectLandmarks(inputImg, landmarkList, centersOfLandmarks);
    if (!success) {
        return false;
    }

    int numberOfFaces = landmarkList.size();
    //commonTool.log(QString("Found number of faces in a single frame --> %1").arg(numberOfFaces));

    for (int i=0;i<numberOfFaces;i++) {
        std::vector<cv::Point2d> points = landmarkList.at(i);

        //commonTool.log(QString("Extracted %1 landmarks.").arg(points.size()));

        //DEBUG
        commonTool.showImageAndLandMarks(QString("Before Alignment"), inputImg, points, CV_RGB(255, 0, 0), *mutex);
        //DEBUG

        cv::Mat resizedImg;
        cv::Mat rotatedImg = cv::Mat( 250, 200, inputImg.type() );
        double ratio = 50/sqrt(pow(points.at(LEFT_EYE_INDEXV4).x - points.at(RIGHT_EYE_INDEXV4).x, 2) + pow(points.at(LEFT_EYE_INDEXV4).y - points.at(RIGHT_EYE_INDEXV4).y, 2));
        cv::resize(inputImg, resizedImg, cv::Size(), ratio, ratio, CV_INTER_LINEAR );
        //resize all the point
        for (int i = 0; i < (int)points.size(); i++){
            points.at(i) = points.at(i) * ratio;
        }

        //DEBUG
//        commonTool.showImageAndLandMarks(QString("Scaling: %1").arg(ratio), resizedImg, points, CV_RGB(255, 0, 0), *mutex);
        //DEBUG

        /* for rotation */
        double degree = (atan((points.at(LEFT_EYE_INDEXV4).y - points.at(RIGHT_EYE_INDEXV4).y)/(points.at(LEFT_EYE_INDEXV4).x - points.at(RIGHT_EYE_INDEXV4).x)) * 180)/PI;
        cv::Mat rotationMat = cv::getRotationMatrix2D(points.at(NOSE_INDEXV4), degree, 1.0);
        cv::warpAffine( resizedImg, rotatedImg, rotationMat, cv::Size(resizedImg.cols, resizedImg.rows) , cv::INTER_LINEAR, cv::BORDER_REPLICATE);

        //rotation all the point
        for (int i = 0; i < (int)points.size(); i++){
            points.at(i) = rotatePoint(points.at(i), rotationMat);
        }

        //DEBUG
        commonTool.showImageAndLandMarks(QString("after rotation"), rotatedImg, points, CV_RGB(255, 0, 0), *mutex);
        //DEBUG

        //make sure the small image will not outside
        cv::copyMakeBorder(rotatedImg, rotatedImg, IMAGE_ROW, IMAGE_ROW, IMAGE_COL, IMAGE_COL, cv::BORDER_REPLICATE);
        for (int i = 0; i < (int)points.size(); i++){
            points.at(i).x = points.at(i).x + IMAGE_COL;
            points.at(i).y = points.at(i).y + IMAGE_ROW;
        }

    // Mo did this: bad way, not checking all points!
    //    cv::circle(rotatedImg, points.at(LEFT_EYE_INDEX), 5, cv::Scalar(255, 0, 0));
    //    cv::circle(rotatedImg, points.at(RIGHT_EYE_INDEX), 5, cv::Scalar(0, 0, 255));
    //    cv::circle(rotatedImg, points.at(NOSE_INDEX), 5, cv::Scalar(0, 255, 0));
    //
    //    cv::imshow("rotatedImg", rotatedImg);
    //    cv::waitKey(0);
    //

        //DEBUG
        commonTool.showImageAndLandMarks(QString("after copymakeborder"), rotatedImg, points, CV_RGB(255, 0, 0), *mutex);
        //DEBUG
        cv::Rect ROI = cv::Rect(points.at(LEFT_EYE_INDEXV4).x - 75.0, points.at(LEFT_EYE_INDEXV4).y - 125.0, IMAGE_COL, IMAGE_ROW);
        //There was a bug here previously
        cv::Point2d leftEye(points.at(LEFT_EYE_INDEXV4));
        for (int i = 0; i < (int)points.size(); i++){
            points.at(i).x = points.at(i).x - (leftEye.x - 75.0);
            points.at(i).y = points.at(i).y - (leftEye.y - 125.0);
        }
        alignedImg.push_back(rotatedImg(ROI));
        landmarkPoints.push_back(points);
        //DEBUG
        commonTool.showImageAndLandMarks(QString("After Alignment"), alignedImg[0], points, CV_RGB(255, 0, 0), *mutex);
        //DEBUG
    }

    return true;

}

//bool FaceAlignmentManager::saveImg(string path){
//	return cv::imwrite(path, alignedImg);
//}

cv::Point2d FaceAlignment::rotatePoint(cv::Point2d targetPoint, double degree, cv::Point2d minPoint){
	cv::Mat rotationMat = cv::getRotationMatrix2D(minPoint, degree, 1.0);
	
	return this->rotatePoint(targetPoint, rotationMat);
}


cv::Point2d FaceAlignment::rotatePoint(cv::Point2d targetPoint, cv::Mat rotationMat){
	
	cv::Mat cvMat = cv::Mat::zeros(3,1,CV_64F);
	cvMat.at<double>(0, 0) = targetPoint.x;
	cvMat.at<double>(1, 0) = targetPoint.y;
	cvMat.at<double>(2, 0) = 1;
	cv::Mat rotatedMat = rotationMat * cvMat;

	return cv::Point2d(rotatedMat.at<double>(0, 0), rotatedMat.at<double>(1, 0));
}

