#include "IntegralImage.h"

IntegralImage::IntegralImage(){
}

IntegralImage::IntegralImage(QString imagePath) {
    IntegralImage();
    load(imagePath);
}

bool IntegralImage::load(QString imagePath) {
    filePath = imagePath;
    std::string imagePathInStd = imagePath.toStdString();

    //Audrey
    //image = cv::imread(imagePathInStd,1); just to be clear.
    image = cv::imread(imagePathInStd, CV_LOAD_IMAGE_GRAYSCALE);
    if (image.data == NULL){
        return false;
    }
    //Audrey
    //cv::cvtColor(image, image, CV_RGB2GRAY, 0);

    image.convertTo(image,CV_8UC1);

    return true;
}

bool IntegralImage::load(cv::Mat img) {
    if (img.channels()!=1) {
        cv::cvtColor(img, image, CV_BGR2GRAY, 0);
        image.convertTo(image,CV_8UC1);
    } else {
        image = img;
    }

    return true;
}

int IntegralImage::getIntegralImageRow() const {
    return -1;
}

int IntegralImage::getIntegralImageCol() const {
    return -1;
}

int IntegralImage::getImageRow() const{
    return image.rows;
}
int IntegralImage::getImageCol() const{
    return image.cols;
}

QString IntegralImage::printMat(){
    
    QString imageInString;
    /*
    int cols = image_Integral_10.cols;
    int rows = image_Integral_10.rows;

    QString colsInQStr;
    QString rowsInQStr;

    colsInQStr.setNum(cols);
    rowsInQStr.setNum(rows);

    int tempCols = cols;

    imageInString = "Cols: " + colsInQStr + "   Rows: " + rowsInQStr + "\n";

    if(image_Integral_10.isContinuous())
    {
        cols = cols * rows;
        rows = 1;
    }
    for(int i = 0; i < rows; i++)
    {


        const double* image_Integral_Current = image_Integral_10.ptr<double>(i);
        for(int j = 0; j < cols; j++){

            if (j%tempCols == 0){
                QString temp;
                temp.setNum(j/tempCols);
                imageInString = imageInString + "\n" + temp ;
            }

            QString temp;
            temp.setNum(image_Integral_Current[j], 'E', 16);

            imageInString = imageInString + " " + temp;


        }

    }
    */
    return imageInString;
}

cv::Mat IntegralImage::getPatchFromImage(QRect rect){
    cv::Mat patch;
    int leftTopX= 0;
    int leftTopY = 0;
    int rightDownX = 0;
    int rightDownY = 0;

    rect.getCoords(&leftTopX, &leftTopY, &rightDownX, &rightDownY);

    patch = image(cv::Range(leftTopY, rightDownY), cv::Range(leftTopX, rightDownX));

    return patch;
}

