#ifndef SYNTHESIS_H
#define SYNTHESIS_H

#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <list>
#include <iostream>
#include <fstream>

#include "IntegralImage.h"
#include "IntegralImages.h"
#include "commonTool.h"

#define UPPER 1
#define LOWER 2
#define LEFT  3
#define RIGHT 4

struct Patch{
	patchIdentifier identifier;
    cv::Mat PhotoPatchData8U;
    cv::Mat SketchPatchData8U;
	cv::Mat PhotoPatchData32F;
	cv::Mat SketchPatchData32F;
    double distance;
};

struct Element{
    std::vector<struct Patch> PatchData;      // all the candidate patches
    std::list<struct Patch> createPhotoPatchData;    // temporarily store the candidate patch

    cv::Mat quTerm;            // used to store the contribution to the quadratic term which is (Rp^T)*(Rp)
    cv::Mat leaTerm;           // used to store the contribution to the leaner term which is Rp*Ip
    cv::Mat quSketchUp;        // used to store the contribution to the quadratic term of the sketch which is Spi*Spi
    cv::Mat quSketchLow;
    cv::Mat quSketchLeft;
    cv::Mat quSketchRight;

    cv::Mat TestData;                        // TestData is used to store the test patch
};



class Synthesis{
public:
    std::vector<Element> ImagePatchUpper;  // used to store all the candidates for the upper patch, row first
    std::vector<Element> ImagePatchLower;  // used to store all the candidates for the lower patch, row first

    std::vector<cv::Mat> quaSmoothUpperRight;  // used to store the contribution to the quadratic term by the smooth term which is Spi*Sqi for upper layer
    std::vector<cv::Mat> quaSmoothUpperDown;
    std::vector<cv::Mat> quaSmoothLowerRight; // used to store the contribution to the quadratic term by the smooth term which is Spi*Sqi for lower layer
    std::vector<cv::Mat> quaSmoothLowerDown;

	Parameter param;
    bool newPatch;

    int NumOfCan;             // the number of candidates : good + randon + new Patch
    int SearchWidthUpper;     // the width extended for the search window
    int SearchWidthLower;
    int patchSizeUpper;       // the size of the upper patch
    int patchSizeLower;       // the size of the lower patch
    int NumWidLower;          // the number of width for the upper part in the image
    int NumHeiLower;          // the number of height for the upper part
    int NumWidUpper;          // the number of width for the lower part
    int NumHeiUpper;		  // the number of height for the lower part
    int OverlapSizeUpper;     // the size of the overlap part for the upper part
    int OverlapSizeLower;     // the size of the overlap part for the lower part
    int ImageRows;            // the height of the image
    int ImageCols;            // the width of the image
    float alpha;    // the weight for smooth term of the same layer
    float beta;      // the weight for smooth term of different layer

    double dataSigma, smoothSigma;  // sigma for data term and smooth term

    Synthesis(Parameter parameter);
    Synthesis();

    int FindKNN(IntegralImages *photoImages, IntegralImages *sketchImages, IntegralImage testImage);
    int getPatchSize();
    int getPatchOverlapSize();
private:
    int GenerateNewPatches(std::vector<Patch> & newPatches, const int &patchSize);
    //int FindPatches(const Mat &TrainingPhotos, const Mat &imageIn, const Mat &TrainingSketches, const Mat &TestImage, const Mat &testInImage,const int & patchSize, const int &NumHei, const int &NumWid, vector<struct Element> &ImagePatch, const int &OverlapSize, const int &No, const int &NumPhotos, const int & SearchWidth);

	int FindPatches(IntegralImages* trainingSource, IntegralImages* trainingTarget, IntegralImage testImage,
							   const int level,
							   const int &patchSize,
							   const int &NumHei,
							   const int &NumWid,
                               std::vector<struct Element> &ImagePatch,
							   const int &OverlapSize,
							   const int &SearchWidth);


    int patchCopy(struct Patch &src, struct Patch &dist);
    //double distance(const cv::Mat &src1, const cv::Mat &src2);

    int ComputeSmooth(std::vector<struct Element>&ImagePatch, const int Height, const int &Width, const int &PatchSize, const int &OverlapSize, std::vector<cv::Mat>& quaSmoothRight, std::vector<cv::Mat> &quaSmoothDown);
    int ComputeQuaTerm(std::vector<struct Element>&ImagePatch, const int Height, const int &Widht, const int PatchSize);
    int inline ComputeLeftRight(const cv::Mat &src1, const cv::Mat &src2, const int &i , const int &j, std::vector<cv::Mat> &quaSmooth, const int &Width, struct Element &left, struct Element &right);
    int inline ComputeUpDown(const cv::Mat &src1, const cv::Mat &src2, const int &i , const int &j, std::vector<cv::Mat> &quaSmooth, const int &Width, struct Element &left, struct Element &right);
    //inline int CopyPatch(vector<struct Element> &ImagePatch, const int &row, const int &col, const int &Height, const int &Width, const int &disparity, const int &type, cv::Mat &result, const cv::Range &rowRange, const cv::Range &colRange);

};

#endif // SYNTHESIS_H
