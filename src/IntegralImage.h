#ifndef INTEGRALIMAGE_H
#define INTEGRALIMAGE_H

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <QString>
#include <QRect>

class IntegralImage {
public:
    IntegralImage();
    IntegralImage(QString imagePath);

    bool load(QString imagePath);
	bool load(cv::Mat img);

    QString printMat();
    int getIntegralImageRow() const;
    int getIntegralImageCol() const;
    int getImageRow() const;
    int getImageCol() const;

	cv::Mat getPatchFromImage(QRect rect);

    cv::Mat image;
    QString filePath;

};

#endif // INTEGRALIMAGE_H
