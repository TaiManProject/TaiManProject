#include "Synthesis.h"

Synthesis::Synthesis()
{

}

Synthesis::Synthesis(Parameter parameter)
{
	param = parameter;
	//RealNumOfCan = parameter.RealNumOfCan;          // select 10 candidates from the training dataset
	SearchWidthUpper = parameter.SearchWidthUpper;       // 5 pixels for upper, lower, left and right search range for each window
	SearchWidthLower = parameter.SearchWidthLower;
	patchSizeUpper = parameter.patchSizeUpper;
	patchSizeLower = parameter.patchSizeLower;
	OverlapSizeUpper = parameter.OverlapSizeUpper;
	OverlapSizeLower = parameter.OverlapSizeLower;
	newPatch = parameter.newPatch;

	if (newPatch == true){
		NumOfCan = param.numOfGoodCan + param.numOfRandomCan + 6;              // number of candidates finally used for synthesizing the sketches
	} else{
		NumOfCan = param.numOfGoodCan + param.numOfRandomCan;			// Sketch to Photo no need the 6 stand patch
	}

    dataSigma = 0.5;
    smoothSigma = 1;
    alpha = 0.25;         // the weight for smooth term of same layer
    beta = 0.08;          // the weight for smooth term of different layer
}

inline int Synthesis::GenerateNewPatches(std::vector<Patch> &newPatches, const int &patchSize)
{
    // this function is used to create some new patches that does not in the
    // training data set

	double step = 1.0/(patchSize-1);

    // for the first patch, all black
    cv::Mat additionalSketch1, additionalPhoto1;
	additionalSketch1 = cv::Mat::zeros(patchSize, patchSize, CV_8UC1);
    additionalPhoto1 = cv::Mat::zeros(patchSize, patchSize, CV_8UC1);

    struct Patch additional1;
    additional1.PhotoPatchData8U = additionalPhoto1;
    additional1.SketchPatchData8U = additionalSketch1;

    // for the second patch
    cv::Mat additionalSketch2, additionalPhoto2;
    additionalSketch2 = cv::Mat::zeros(patchSize, patchSize, CV_8UC1);
    additionalPhoto2 = cv::Mat::zeros(patchSize, patchSize, CV_8UC1);

    for(int i = 0; i < patchSize; i++)
    {
		int value = (1 - (step * i)) * 255; 
        additionalPhoto2(cv::Range(i, i+1), cv::Range(0, patchSize))  = value;
        additionalSketch2(cv::Range(i, i+1), cv::Range(0, patchSize)) = value;
    }

    struct Patch additional2;
    additional2.PhotoPatchData8U = additionalPhoto2;
    additional2.SketchPatchData8U = additionalSketch2;

    // for the third patch
    cv::Mat additionalSketch3, additionalPhoto3;
    additionalSketch3 = cv::Mat::zeros(patchSize, patchSize, CV_8UC1);
    additionalPhoto3 = cv::Mat::zeros(patchSize, patchSize, CV_8UC1);

    for(int i = 0; i < patchSize; i++)
    {
		int value = (step * i) * 255; 
        additionalPhoto3(cv::Range(i, i+1), cv::Range(0, patchSize))  = value;
        additionalSketch3(cv::Range(i, i+1), cv::Range(0, patchSize)) = value;
    }

    struct Patch additional3;
    additional3.PhotoPatchData8U = additionalPhoto3;
    additional3.SketchPatchData8U = additionalSketch3;

    

    // for the forth patch
    cv::Mat additionalSketch4, additionalPhoto4;
    additionalSketch4 = cv::Mat::zeros(patchSize, patchSize, CV_8UC1);
    additionalPhoto4 = cv::Mat::zeros(patchSize, patchSize, CV_8UC1);

    for(int i = 0; i < patchSize; i++)
    {
		int value = (1 - (step * i)) * 255; 
        additionalPhoto4(cv::Range(0, patchSize), cv::Range(i, i+1))  = value;
        additionalSketch4(cv::Range(0, patchSize), cv::Range(i, i+1)) = value;
    }

    struct Patch additional4;
    additional4.PhotoPatchData8U = additionalPhoto4;
    additional4.SketchPatchData8U = additionalSketch4;

    

    // for the fifth patch
    cv::Mat additionalSketch5, additionalPhoto5;
    additionalSketch5 = cv::Mat::zeros(patchSize, patchSize, CV_8UC1);
    additionalPhoto5 = cv::Mat::zeros(patchSize, patchSize, CV_8UC1);

    for(int i = 0; i < patchSize; i++)
    {
		int value = (step * i) * 255; 
        additionalPhoto5(cv::Range(0, patchSize), cv::Range(i, i+1))  = value;
        additionalSketch5(cv::Range(0, patchSize), cv::Range(i, i+1)) = value;
    }

    struct Patch additional5;
    additional5.PhotoPatchData8U = additionalPhoto5;
    additional5.SketchPatchData8U = additionalSketch5;

    


    cv::Mat additionalSketchW, additionalPhotoW;
    additionalSketchW = cv::Mat::ones(patchSize, patchSize, CV_8UC1);
    additionalPhotoW = cv::Mat::ones(patchSize, patchSize, CV_8UC1);

	additionalSketchW *= 255;
	additionalPhotoW *= 255;

    struct Patch additionalW;
    additionalW.PhotoPatchData8U = additionalPhotoW;
    additionalW.SketchPatchData8U = additionalSketchW;

    //double step = -1.0/patchSize;

    //// for the first patch, all black
    //cv::Mat additionalSketch1, additionalPhoto1;
    //additionalSketch1 = cv::Mat::ones(patchSize, patchSize, CV_32FC1);
    //additionalPhoto1 = cv::Mat::ones(patchSize, patchSize, CV_32FC1);
    //struct Patch additional1;
    //additional1.PhotoPatchData = -1 * additionalPhoto1;
    //additional1.SketchPatchData = -1 * additionalSketch1;

    //


    //// for the second patch
    //cv::Mat additionalSketch2, additionalPhoto2;
    //additionalSketch2 = cv::Mat::zeros(patchSize, patchSize, CV_32FC1);
    //additionalPhoto2 = cv::Mat::zeros(patchSize, patchSize, CV_32FC1);

    //for(int i = 0; i < patchSize; i++)
    //{
    //    additionalPhoto2(cv::Range(i, i+1), cv::Range(0, patchSize))  = -1 - i*step;
    //    additionalSketch2(cv::Range(i, i+1), cv::Range(0, patchSize)) = -1 - i*step;
    //}


    //struct Patch additional2;
    //additional2.PhotoPatchData = additionalPhoto2;
    //additional2.SketchPatchData = additionalSketch2;

    //

    //// for the third patch
    //cv::Mat additionalSketch3, additionalPhoto3;
    //additionalSketch3 = cv::Mat::zeros(patchSize, patchSize, CV_32FC1);
    //additionalPhoto3 = cv::Mat::zeros(patchSize, patchSize, CV_32FC1);

    //for(int i = 0; i < patchSize; i++)
    //{
    //    additionalPhoto3(cv::Range(i, i+1), cv::Range(0, patchSize))  =  i*step;
    //    additionalSketch3(cv::Range(i, i+1), cv::Range(0, patchSize)) =  i*step;
    //}

    //struct Patch additional3;
    //additional3.PhotoPatchData = additionalPhoto3;
    //additional3.SketchPatchData = additionalSketch3;

    //

    //// for the forth patch
    //cv::Mat additionalSketch4, additionalPhoto4;
    //additionalSketch4 = cv::Mat::zeros(patchSize, patchSize, CV_32FC1);
    //additionalPhoto4 = cv::Mat::zeros(patchSize, patchSize, CV_32FC1);

    //for(int i = 0; i < patchSize; i++)
    //{
    //    additionalPhoto4(cv::Range(0, patchSize), cv::Range(i, i+1))  = -1 - i*step;
    //    additionalSketch4(cv::Range(0, patchSize), cv::Range(i, i+1)) = -1 - i*step;
    //}

    //struct Patch additional4;
    //additional4.PhotoPatchData = additionalPhoto4;
    //additional4.SketchPatchData = additionalSketch4;

    //

    //// for the fifth patch
    //cv::Mat additionalSketch5, additionalPhoto5;
    //additionalSketch5 = cv::Mat::zeros(patchSize, patchSize, CV_32FC1);
    //additionalPhoto5 = cv::Mat::zeros(patchSize, patchSize, CV_32FC1);

    //for(int i = 0; i < patchSize; i++)
    //{
    //    additionalPhoto5(cv::Range(0, patchSize), cv::Range(i, i+1))  = i*step;
    //    additionalSketch5(cv::Range(0, patchSize), cv::Range(i, i+1)) = i*step;
    //}

    //struct Patch additional5;
    //additional5.PhotoPatchData = additionalPhoto5;
    //additional5.SketchPatchData = additionalSketch5;

    //


    //cv::Mat additionalSketchW, additionalPhotoW;
    //additionalSketchW = cv::Mat::ones(patchSize, patchSize, CV_32FC1);
    //additionalPhotoW = cv::Mat::ones(patchSize, patchSize, CV_32FC1);
    //struct Patch additionalW;
    //additionalW.PhotoPatchData = additionalPhotoW;
    //additionalW.SketchPatchData = additionalSketchW;

    

    // we add another 4 additional patches for which is half dark and half zeros


	newPatches.push_back(additional1);
	newPatches.push_back(additional2);
	newPatches.push_back(additional3);
	newPatches.push_back(additional4);
	newPatches.push_back(additional5);
	newPatches.push_back(additionalW);
    return 0;
}
/*
inline double Synthesis::distance(const cv::Mat &src1, const cv::Mat &src2)
{
    //  this function is used to compute the distance of two mat src1 and src2 
    // this function can be adaptive to any distance
    // at this moment, we just use Euclidean distance of two patch
    double result = 0;


    cv::Mat temp;
    temp = src1 - src2;



    cv::Mat dst;
    pow(temp, 2.0, dst);

    cv::Scalar tsum;
    tsum = sum(dst);
    result = tsum.val[0] + tsum.val[1] + tsum.val[2];

    //cout << "src1: " << endl << src1 << endl;

    //cout << "src2: " << endl << src2 << endl;


    //  NCC
    Scalar src1Avg, src2Avg;
    double arc1AvgDouble, arc2AvgDouble;

    src1Avg = mean(src1);
    arc1AvgDouble = src1Avg.val[0] + src1Avg.val[1] + src1Avg.val[2];
    Mat src1SubAvg = src1 - arc1AvgDouble;


    src2Avg = mean(src2);
    arc2AvgDouble = src2Avg.val[0] + src2Avg.val[1] + src2Avg.val[2];
    Mat src2SubAvg = src2 - arc2AvgDouble;

    //cout << "arc1AvgDouble: " << arc1AvgDouble << " arc2AvgDouble: " << arc2AvgDouble << endl;

    Mat src1MulSrc2;
    multiply(src1SubAvg, src2SubAvg, src1MulSrc2);

    //cout << "src1SubAvg" << endl << src1SubAvg << endl;
    //cout << "src2SubAvg" << endl << src2SubAvg << endl;
    //cout << "src1MulSrc2" << endl << src1MulSrc2 << endl;

    Scalar src1MulSrc2Sum;
    double molecular;
    src1MulSrc2Sum = sum(src1MulSrc2);
    molecular = src1MulSrc2Sum.val[0] + src1MulSrc2Sum.val[1] + src1MulSrc2Sum.val[2];

    //cout << "molecular: " << molecular << endl;

    Mat src1SubAvgPow2, src2SubAvgPow2;
    cv::pow(src1SubAvg, 2.0, src1SubAvgPow2);
    cv::pow(src2SubAvg, 2.0, src2SubAvgPow2);

    Scalar src1SubAvgPow2Sum, src2SubAvgPow2Sum;
    double src1SubAvgPow2SumDouble, src2SubAvgPow2SumDouble;
    src1SubAvgPow2Sum = sum(src1SubAvgPow2);
    src2SubAvgPow2Sum = sum(src2SubAvgPow2);
    src1SubAvgPow2SumDouble = src1SubAvgPow2Sum.val[0] + src1SubAvgPow2Sum.val[1] + src1SubAvgPow2Sum.val[2];
    src2SubAvgPow2SumDouble = src2SubAvgPow2Sum.val[0] + src2SubAvgPow2Sum.val[1] + src2SubAvgPow2Sum.val[2];

    double denominator = sqrt(src1SubAvgPow2SumDouble * src2SubAvgPow2SumDouble);

    //cout << "denominator: " << denominator << endl;

    if (denominator == 0){
        return 0;
    }

    result = molecular / denominator;
    result = result * -1.0;
    result = result + 1.0;

    //cout << "Resutl: " << result << endl;
    

    return result;
}
*/

inline int Synthesis::patchCopy(struct Patch &src, struct Patch &dist)
{
    // copy patch src to patch dist
    dist.distance = src.distance;
    src.PhotoPatchData8U.copyTo(dist.PhotoPatchData8U);
    src.SketchPatchData8U.copyTo(dist.SketchPatchData8U);
	src.PhotoPatchData32F.copyTo(dist.PhotoPatchData32F);
    src.SketchPatchData32F.copyTo(dist.SketchPatchData32F);
    return 0;
}

/*
inline int Synthesis::CopyPatch(vector<struct Element> &ImagePatch, const int &row, const int &col, const int &Height, const int &Width,const int &disparity, const int &type, cv::Mat &result, const cv::Range &rowRange, const cv::Range &colRange)
{
    // this function is used to get the right sketch patch used to compute the smooth term
    // ImagePatch  : the data structure to store all the element
    // row, col    : the row and column of the current patch
    // Height      : the height of the patch image
    // Width       : the width of the patch image
    // disparity   : the disparity of current wanted patch
    // type        : the relation of current wanted patch to the current sketch patch to be calculated
    // result      : store the sketch patch
    switch(type)
    {
        case UPPER:
            {
                ImagePatch.at((row-1)*Width+col).PatchData.at(disparity).SketchPatchData(rowRange, colRange).copyTo(result);
                break;
            }
        case LOWER:
            {
                ImagePatch.at((row+1)*Width+col).PatchData.at(disparity).SketchPatchData(rowRange,colRange).copyTo(result);
                break;
            }
        case LEFT:
            {
                ImagePatch.at(row*Width+ col-1).PatchData.at(disparity).SketchPatchData(rowRange, colRange).copyTo(result);
                break;
            }
        case RIGHT:
            {
                ImagePatch.at(row*Width+col+1).PatchData.at(disparity).SketchPatchData(rowRange, colRange).copyTo(result);
                break;
            }
    }
        return 1;
}
*/

int inline Synthesis::ComputeLeftRight(const cv::Mat &src1, const cv::Mat &src2, const int &i , const int &j, std::vector<cv::Mat> &quaSmooth, const int &Width, struct Element &left, struct Element &right)
{
    // this function is used to compute the cross-term and square term of smooth term
    // src1 src2   :  left and right overlap patch
    // left right  :  left and right part of square term
    cv::Mat temp;
    temp = src1*src2.t();
    quaSmooth.at(i*Width+j) = temp*(-1)*alpha;
    temp = src1*src1.t();
    left.quSketchRight = temp*alpha;
    temp = src2*src2.t();
    right.quSketchLeft = temp*alpha;
    return 0;
}

int inline Synthesis::ComputeUpDown(const cv::Mat &src1, const cv::Mat &src2, const int &i , const int &j, std::vector<cv::Mat> &quaSmooth, const int &Width, struct Element &up, struct Element &down)
{
    // this function is used to compute the cross-term and square term of smooth term
    // src1 src2   :  up and down overlap patch
    // up down     :  up and down part of square term
    cv::Mat temp;
    temp = src1*src2.t();
    quaSmooth.at(i*Width+j) = temp*(-1*alpha);
    temp = src1*src1.t();
    up.quSketchLow = temp*alpha;
    temp = src2*src2.t();
    down.quSketchUp = temp*alpha;
    return 0;
}
int Synthesis::ComputeSmooth(std::vector<struct Element>&ImagePatch, const int Height, const int &Width, const int &PatchSize, const int &OverlapSize, std::vector<cv::Mat> &quaSmoothRight, std::vector<cv::Mat> &quaSmoothDown)
{
    // this function is used to compute the contribution to the parameter by the smooth term
    // the to be computed terms within this function are quaSmooth and quSketch
    // we use two loop to satisfy this

    // compute the overlap range
    int middle = PatchSize-OverlapSize;

    cv::Range lowRow(middle,PatchSize), lowCol(0,PatchSize);        // range for a patch's lower overlapping part
    cv::Range upRow(0, OverlapSize), upCol(0, PatchSize);           // range for a patch's upper overlapping part
    cv::Range rightRow(0, PatchSize), rightCol(middle, PatchSize);  // range for a patch's right overlapping part
    cv::Range leftRow(0, PatchSize), leftCol(0, OverlapSize);       // range for a patch's left overlapping part

    int cols = OverlapSize*PatchSize;  // three channels
    /***********************************************************************************/
    // left to right
    for(int i = 0; i < Height; i++)
        for(int j = 0; j < Width - 1; j++)
        {
            // compute the left right smooth term
            struct Element &tempQue1 = ImagePatch.at(i*Width+j);      // left patch
            struct Element &tempQue2 = ImagePatch.at(i*Width+j+1);    // right patch
            std::vector<struct Patch> &tempPatch1 = tempQue1.PatchData;    // left patch's candidate
            std::vector<struct Patch> &tempPatch2 = tempQue2.PatchData;    // right patch's candidate

            cv::Mat temp1, temp2;
            temp1.create(NumOfCan, cols, CV_32FC1);
            temp2.create(NumOfCan, cols, CV_32FC1);
            cv::Mat temp3, temp4;
            for(int kk = 0; kk < NumOfCan; kk++)
            {
                tempPatch1.at(kk).SketchPatchData32F(rightRow, rightCol).copyTo(temp3);    // copy the kkth overlapping sktch part to temp3
                tempPatch2.at(kk).SketchPatchData32F(leftRow, leftCol).copyTo(temp4);
                cv::Mat newTemp3 = temp3.reshape(1,1);
                newTemp3.copyTo(temp1(cv::Range(kk,kk+1), cv::Range(0,cols)));
                cv::Mat newTemp4 = temp4.reshape(1,1);
                newTemp4.copyTo(temp2(cv::Range(kk,kk+1), cv::Range(0, cols)));
            }
            ComputeLeftRight(temp1, temp2, i, j, quaSmoothRight, Width-1, tempQue1, tempQue2);
        }

    /*******************************************************************************************/
    // up to down
    for(int i = 0; i < Height - 1; i++)
        for(int j = 0; j < Width; j++)
        {
            struct Element &tempQue1 = ImagePatch.at(i*Width+j);
            struct Element &tempQue2 = ImagePatch.at((i+1)*Width+j);
            std::vector<struct Patch> &tempPatch1 = tempQue1.PatchData;
            std::vector<struct Patch> &tempPatch2 = tempQue2.PatchData;


            cv::Mat temp1, temp2;
            temp1.create(NumOfCan, cols, CV_32FC1);
            temp2.create(NumOfCan, cols, CV_32FC1);
            cv::Mat temp3, temp4;
            for(int kk = 0; kk < NumOfCan; kk++)
            {
                tempPatch1.at(kk).SketchPatchData32F(lowRow, lowCol).copyTo(temp3);
                tempPatch2.at(kk).SketchPatchData32F(upRow, upCol).copyTo(temp4);
                cv::Mat newTemp3 = temp3.reshape(1,1);
                newTemp3.copyTo(temp1(cv::Range(kk,kk+1), cv::Range(0,cols)));
                cv::Mat newTemp4 = temp4.reshape(1,1);
                newTemp4.copyTo(temp2(cv::Range(kk,kk+1), cv::Range(0, cols)));
            }
            ComputeUpDown(temp1, temp2, i, j, quaSmoothDown, Width, tempQue1, tempQue2);
        }
    return 1;



}

int Synthesis::ComputeQuaTerm(std::vector<struct Element>& ImagePatch, const int Height, const int &Width, const int PatchSize)
{
    // this function is used to compute the quadratic term contributed by the data term
    int cols = PatchSize*PatchSize;
    for(int i = 0; i < Height; i++)
        for(int j = 0; j < Width; j++)
        {
            struct Element &patchData = ImagePatch.at(i*Width+j);
            std::vector<struct Patch> &tempPatch = patchData.PatchData;

            cv::Mat temp1, temp2;
            temp1.create(NumOfCan, cols, CV_32FC1);
            cv::Mat tempTest(1,cols,CV_32FC1);

            patchData.TestData.copyTo(temp2);
            cv::Mat newTemp2 =temp2.reshape(1,1);
            newTemp2.copyTo(tempTest(cv::Range(0,1), cv::Range(0,cols)));

            for(int kk = 0; kk < NumOfCan; kk++)
            {
                tempPatch.at(kk).PhotoPatchData32F.copyTo(temp2);
                newTemp2 = temp2.reshape(1,1);
                newTemp2.copyTo(temp1(cv::Range(kk,kk+1), cv::Range(0, cols)));
            }
            patchData.quTerm = temp1*(temp1.t());
            patchData.leaTerm = temp1*(tempTest.t());
        }
    return 1;

    /*
    // this function is used to compute the quadratic term contributed by the data term
    int cols = PatchSize*PatchSize;
    for(int i = 0; i < Height; i++)
        for(int j = 0; j < Width; j++)
        {
            struct Element &patchData = ImagePatch.at(i*Width+j);
            vector<struct Patch> &tempPatch = patchData.PatchData;

            Mat temp1, temp2;
            temp1.create(NumOfCan, cols, CV_64FC1);
            Mat tempTest(1,cols,CV_64FC1);

            patchData.TestData.copyTo(temp2);
            Mat newTemp2 =temp2.reshape(1,1);
            newTemp2.copyTo(tempTest(Range(0,1), Range(0,cols)));

            for(int kk = 0; kk < NumOfCan; kk++)
            {
                tempPatch.at(kk).PhotoPatchData.copyTo(temp2);
                newTemp2 = temp2.reshape(1,1);
                newTemp2.copyTo(temp1(Range(kk,kk+1), Range(0, cols)));
            }


            //Mat tempTranpose = temp1.t();




            Mat testMul;
            temp1(Range(1,3),Range(1,2)).copyTo(testMul);





            fstream temptest;
            temptest.open("temp2.txt",ios_base::out);
            for(int i = 0; i < testMul.rows; i++)
            {
                for(int j = 0; j < testMul.cols; j++)
                {
                    temptest<<testMul.at<double>(i,j)<<" ";
                }
                temptest<<endl;
            }
            temptest<<"************" << endl;

            Mat temp_Mul = testMul.t();
            for(int i = 0; i < temp_Mul.rows; i++)
            {
                for(int j = 0; j < temp_Mul.cols; j++)
                {
                    temptest<<temp_Mul.at<double>(i,j)<<" ";
                }
                temptest<<endl;
            }



            Mat tempResult = testMul * temp_Mul;

            fstream temptestMul;
            temptestMul.open("temp1.txt",ios_base::out);
            for(int i = 0; i < tempResult.rows; i++)
            {
                for(int j = 0; j < tempResult.cols; j++)
                {
                    temptestMul<<tempResult.at<double>(i,j)<<" ";
                }
                temptestMul<<endl;
            }


            Mat tempTranpose;
            Mat tempTranpose1 = temp1.t();
            tempTranpose1.copyTo(tempTranpose);

            //patchData.quTerm = temp1*temp1.t();
           // Mat tempMatrix = temp1*tempTranpose;

            patchData.quTerm.create(temp1.rows, temp1.rows, CV_64FC1);
            //patchData.quTerm= temp1*tempTranpose;
            multiply(temp1, tempTranpose, patchData.quTerm);
            patchData.leaTerm = temp1*tempTest.t();
        }
    return 1;
*/

}






int Synthesis::FindPatches(IntegralImages* trainingSource,
                           IntegralImages* trainingTarget,
                           IntegralImage testImage,
                           const int level,
                           const int &patchSize,
                           const int &NumHei,
                           const int &NumWid,
                           std::vector<struct Element> &ImagePatch,
                           const int &OverlapSize,
                           const int &SearchWidth
						   )
{

    /**********  function used to find the nearest patch within a search range of the current patch **********/
    // TrainingPhotos   : the current training photo to be dealt with
    // imageIn          : Mat used to store the integral image
    // TrainingSketches : the current training sketch to be dealt with
    // TestImage        : the to be synthesized photo
    // tempImage
    // searchRange      : the width of the search from current window
    // patchSize        : the size of each patch
    // NumHei           : the number of patches along the height
    // NumWid           : the number of patches along the width
    // Candidate        : used to store the candidates of all the patch

    Patch currentPatch;   // the patch current dealt with
   

    int RangUp;                  // the real search of the up part of the window
    int RangLow;                 // the real search of the low  part of the window
    int RangLeft;                // the real search of the left part of the window
    int RangRight;               // the real search of the right part of the window

	int searchWinSize; //if random patch needed
	if (level == _INTEGRALIMAGES_LOWER_){
        searchWinSize = param.SearchWidthLower;  // the lower width extended for the search window = 8
	} else if (level == _INTEGRALIMAGES_UPPER_){
        searchWinSize = param.SearchWidthUpper; // 5 pixels
	}

    //Mat photoIntegral;
    //if (level == _INTEGRALIMAGES_LOWER_){
    //    photoIntegral = trainingPhoto.image_Integral_10;
    //} else {
    //    photoIntegral = trainingPhoto.image_Integral_20;
    //}

    /******    this part is used to generate some new patches  *****/
    

    /************************    end    ****************************/


    for(int i = 0; i < NumHei; i++){
		
        for(int j = 0; j < NumWid; j++){

            //start = clock();
            /******** for every patch of the tested photo *********/
            // j : the No. of column
            // i : the No. of row
            /*******************  test *************/

            int PatchStep = patchSize - OverlapSize;
            int rangRowStart = i*PatchStep;
            int rangRowEnd = rangRowStart + patchSize;    // about row

            int rangColStart = j*PatchStep;
            int rangColEnd = rangColStart + patchSize;    // about column

            Element &testImageCurrentElement = ImagePatch.at(i*NumWid+j);   // the change of tempElement will change ImagePatch


            testImage.image(cv::Range(rangRowStart, rangRowEnd), cv::Range(rangColStart, rangColEnd)).copyTo(testImageCurrentElement.TestData);
			
            std::vector<patchIdentifier> listOfKNN; //stort all can
            std::vector<patchIdentifier> listOfGoodCan; //stort good can


			//found good candidates
            trainingSource->foundKNN(listOfGoodCan, testImageCurrentElement.TestData, level, param.numOfGoodCan, i * NumWid + j, rangRowStart, rangColStart);
            for (int i = 0; i < (int)listOfGoodCan.size(); i++){
                patchIdentifier identifier = listOfGoodCan.at(i);
				identifier.patchType = _CAN_GOOD_;

                listOfKNN.push_back(identifier);
			}
			

			//found radom candidates
			if (param.numOfRandomCan > 0){
				int currentSearchWinRowStart = rangRowStart - searchWinSize;
				int currentSearchWinRowEnd = rangRowEnd + searchWinSize;
				int currentSearchWinColStart = rangColStart - searchWinSize;
				int currentSearchWinColEnd = rangColEnd + searchWinSize;

				checkBound(currentSearchWinRowStart, currentSearchWinRowEnd, currentSearchWinColStart, currentSearchWinColEnd, param.imageRol, param.imageCol);
				
				for ( int i = 0; i < param.numOfRandomCan; i++){
					patchIdentifier identifier;
					identifier.numberOfImage = randomInt(0, trainingSource->size() - 1);
					identifier.colRange.start = randomInt(currentSearchWinColStart, currentSearchWinColEnd - patchSize - 1);
					identifier.rowRange.start = randomInt(currentSearchWinRowStart, currentSearchWinRowEnd - patchSize - 1);
					identifier.colRange.end = identifier.colRange.start + patchSize;
					identifier.rowRange.end = identifier.rowRange.start + patchSize;
					identifier.patchType = _CAN_RANDOM_;
					cv::Mat patch;
					(trainingSource->integralImages.at(identifier.numberOfImage)->image)(identifier.rowRange, identifier.colRange).copyTo(patch);
					identifier.distance = patchDistance(patch, testImageCurrentElement.TestData);

                    listOfKNN.push_back(identifier);
				}
			}
			
			// copy the raw data to data type Patch
            for ( int k = 0; k < listOfKNN.size(); k ++){
				Patch tempPatch;

                tempPatch.identifier = listOfKNN.at(k);
				tempPatch.identifier.orgRowRange.start = rangRowStart;
				tempPatch.identifier.orgRowRange.end = rangRowEnd;

				tempPatch.identifier.orgColRange.start = rangColStart;
				tempPatch.identifier.orgColRange.end = rangColEnd;

				(trainingSource->integralImages.at(tempPatch.identifier.numberOfImage)->image)(tempPatch.identifier.rowRange, tempPatch.identifier.colRange).copyTo(tempPatch.PhotoPatchData8U);
				(trainingTarget->integralImages.at(tempPatch.identifier.numberOfImage)->image)(tempPatch.identifier.rowRange, tempPatch.identifier.colRange).copyTo(tempPatch.SketchPatchData8U);
				tempPatch.distance = currentPatch.distance;

				testImageCurrentElement.PatchData.push_back(tempPatch);
			}


			



            /***********  this part is used to put the additional patch into the candidate  *********/
			
            if(newPatch == true){

                std::vector<Patch> tempNewPatch;
				GenerateNewPatches(tempNewPatch, patchSize);

                //cout << "Photo to Sketch Mode on, now create new patch" << endl;
                for(int kk = 0; kk < tempNewPatch.size(); kk++){
                    //cout << "kk: " << kk << endl;
                    // compute the distance between the additional patch with the test patch
					

                    tempNewPatch.at(kk).distance = patchDistance(tempNewPatch.at(kk).PhotoPatchData8U, testImageCurrentElement.TestData);
					
					patchIdentifier identifier;
					identifier.patchType = _CAN_ADD_;
					identifier.distance = tempNewPatch.at(kk).distance;
					identifier.orgColRange = cv::Range(rangColStart, rangColEnd);
					identifier.orgRowRange = cv::Range(rangRowStart, rangRowEnd);

					tempNewPatch.at(kk).identifier = identifier;

                    //cout << "distance of additional patch: " << tempNewPatch.at(kk).distance << endl;

                    // put it in the queue
                    testImageCurrentElement.PatchData.insert(testImageCurrentElement.PatchData.begin(), tempNewPatch.at(kk));
                }
				
            }

            /***************************                end            ******************************/
            // get the real search range for the current patch
            //RangLeft  = (rangColStart >= SearchWidth ? SearchWidth: rangColStart);
            //RangRight = ((testImage.getImageCol() - rangColEnd) >= SearchWidth ? SearchWidth : testImage.getImageCol() - rangColEnd); // about cols

            //RangUp    = (rangRowStart >= SearchWidth ? SearchWidth: rangRowStart);
            //RangLow   = ((testImage.getImageRow() - rangRowEnd) >= SearchWidth ? SearchWidth : testImage.getImageRow() - rangRowEnd); // about cols


            // for the current patch, search the Range
            //struct Patch tempPatch;      // store the patch in the rang that has the minimum distance from the test patch
			
            /***********************   try to use convolution to acceleration **************************/
			/*
            Mat kernel(tempElement.TestData);
            Mat tempSrc;         // store the src image used to do the convolution
            Mat tempIn;          // store the integral image
            double testInPatch;  // store the integral patch for the test image

            // get the src image
            trainingPhoto.image(Range(rangRowStart-RangUp, rangRowEnd+RangLow), Range(rangColStart - RangLeft, rangColEnd + RangRight)).copyTo(tempSrc);
            //cout << "imageIn: " << rangRowStart - RangUp << " " << rangRowEnd + RangLow << " " << rangColStart - RangLeft << " " << rangColEnd + RangRight << " " << endl;
            //cout << "imageIn.dims: " << imageIn.dims << endl;
            // get the integral image of the training photos
            photoIntegral(Range(rangRowStart - RangUp,rangRowStart + RangLow), Range(rangColStart - RangLeft, rangColStart+RangRight)).copyTo(tempIn);

            // get the integral image of the test photos
            //cout << "check pt3 " << endl;
            testInPatch = testImage.image_Integral.at<double>(rangRowEnd, rangColEnd)
                        + testImage.image_Integral.at<double>(rangRowStart, rangColStart)
                        - testImage.image_Integral.at<double>(rangRowEnd, rangColStart)
                        - testImage.image_Integral.at<double>(rangRowStart, rangColEnd);


            Mat filterResult;
            Mat tempResult;
            Mat patchDistance;             // used to store the distance of two patch


            Point tempPoint(-1,-1);


            filter2D(tempSrc,tempResult, -1, kernel, tempPoint, 0, BORDER_DEFAULT);
            tempResult(Range(patchSize/2, tempResult.rows-patchSize/2), Range(patchSize/2, tempResult.cols-patchSize/2)).copyTo(filterResult);

            //cout << "tempSrc" << endl;
            //cout << tempSrc << endl;
            //cout << "filterResultMat"<< endl;
            //cout << filterResult << endl;


            Mat tempTestIn = Mat::ones(tempIn.rows,tempIn.cols,CV_64F);
            tempTestIn = tempTestIn * testInPatch;
            patchDistance = tempIn - 2*filterResult+tempTestIn;

            //cout << "patchDistance" << endl;
            //cout << patchDistance << endl;

            Point minLocation;   // used to find the minimum value's position
            Point maxLocation;   // the location of the maximum value's position
            double minValue = 999999999.99;
            double maxValue;
            minMaxLoc(patchDistance, &minValue, &maxValue, &minLocation, &maxLocation);

			*/
            /*
            //for every pixel in the search range
            double distanceSmallest = 999999999.99;
            int iSmallest = 0;
            int jSmallest = 0;

            //cout << "SearchWidth: " << SearchWidth << endl;
            for ( int i = 0; i < tempSrc.rows - patchSize; i++){
                for ( int j = 0; j < tempSrc.cols - patchSize; j++){
                    Mat dbImagePatch;
                    tempSrc(Range(i, i + patchSize), Range(j, j + patchSize)).copyTo(dbImagePatch);
                    double tempResultForDistanceFunction = distance(dbImagePatch, tempElement.TestData);
                    //cout << "tempSrc: " << tempSrc.rows << " " << tempSrc.cols << endl;
                    //cout << "distance: " << i << " " << j << " " << tempResultForDistanceFunction << endl;

                    //to see which is smaller
                    if (distanceSmallest > tempResultForDistanceFunction){
                        distanceSmallest = tempResultForDistanceFunction;
                        iSmallest = i;
                        jSmallest = j;
                    }
                }
            }
            //cout << "Smallest distance: " << distanceSmallest << " i:" << iSmallest << " j:" << jSmallest << endl;
            //cout << "       old method: " << minValue <<" y: " << minLocation.y << " x: " << minLocation.x << endl;
            //cout << endl;
            */

			/*
            if(minValue<0)
            {
                if(abs(minValue)<0.00001)
                    minValue = 0;
                else
                {
                    cout << "i: " << i << " j: " << j << endl;
                    cout << "wrong, distance cannot be negative" << endl;
                    return -1;
                }
            }
			*/
			

            //tempPatch.distance = sqrt(minValue/2)/dataSigma;


            //int rangaRow = rangRowStart - RangUp + minLocation.y;
            //int rangbRow = rangaRow + patchSize;
            //int rangaCol = rangColStart - RangLeft + minLocation.x;
            //int rangbCol = rangaCol + patchSize;



            //trainingPhoto.image(Range(rangaRow, rangbRow), Range(rangaCol, rangbCol)).copyTo(tempPatch.PhotoPatchData);
            //trainingSketch.image(Range(rangaRow, rangbRow), Range(rangaCol, rangbCol)).copyTo(tempPatch.SketchPatchData);

            /***********************                end                       **************************/

            /*******************   Using list to store the number of candidate  *************/
            /*
			std::list<struct Patch> &tempList = testImageCurrentElement.createPhotoPatchData;
            if(tempList.empty())
            {
                tempList.push_front(tempPatch);
            }
            else
            {
                std::list<struct Patch>::iterator iy = tempList.begin();

                while(iy != tempList.end() && iy->distance > tempPatch.distance)
                    iy++;
                if(iy == tempList.end())
                    tempList.push_back(tempPatch);
                else
                    tempList.insert(iy,tempPatch);
            }
            // if(tempList.size() == NumOfCan + 1)
            //	tempList.pop_front();

            // we need to find NumOfCan - 1 samples from the training database
            if(tempList.size() == RealNumOfCan + 1)
                tempList.pop_front();

            // copy the list data to the vector
            if(NumPhotos == No + 1)
                testImageCurrentElement.PatchData.insert(testImageCurrentElement.PatchData.begin(), tempList.begin(),tempList.end());

			*/

       }
    }

	for ( int i = 0; i < ImagePatch.size(); i++){
		for ( int j = 0; j < ImagePatch.at(i).PatchData.size(); j++){
			ImagePatch.at(i).PatchData.at(j).PhotoPatchData8U.convertTo(ImagePatch.at(i).PatchData.at(j).PhotoPatchData32F, CV_32FC1);
			ImagePatch.at(i).PatchData.at(j).SketchPatchData8U.convertTo(ImagePatch.at(i).PatchData.at(j).SketchPatchData32F, CV_32FC1);
		}
	}
    //MO //cout<<"total time used "<< timer.getTimeInString() <<endl;
    return 0;
}




int Synthesis::FindKNN(IntegralImages* sourceImages, IntegralImages* targetImages, IntegralImage testImage)
{
    //cout << "start FindKNN" << endl;
    /*****************  this function is used to find the k nearest neighbors for each patch  *****************/
    // TrainingPhotos   : all the names of training photos
    // TrainingSketches : all the names of training sketches
    // PhotoName        : the name of the test photos

    ImageRows = testImage.getImageRow();    // store the rows of the image
    ImageCols = testImage.getImageCol();    // store the cols of the image

	

/*
    int NumPhotos = TrainingPhotos.size();
    int NumSketches = TrainingSketches.size();
    if(NumPhotos != NumSketches)
        perror("the number of training photos and training sketches are not the same");

    // load the test image
    Mat TestImage = imread(PhotoName, 1);
    //cout<<PhotoName<<endl;
    ImageRows = TestImage.rows;    // store the rows of the image
    ImageCols = TestImage.cols;    // store the cols of the image

    //cout<<"ImageRows: "<<ImageRows;
    //cout<<"ImageCols: "<<ImageCols;
    cvtColor(TestImage, TestImage, CV_RGB2GRAY, 0);

    //cout<<"ImageRows: "<<ImageRows;
    //cout<<"ImageCols: "<<ImageCols;

    TestImage.convertTo(TestImage,CV_64FC1);
    TestImage = 1.0*TestImage/255.0;

    // the following code is used to compute the integral image for the test image
    Mat powImage;
    //vector<Mat> tempSplit;
    //Mat sumImage;
    Mat testInImage;      // used to store the integral image

    pow(TestImage, 2.0, powImage);
    //split(powImage, tempSplit);
    //sumImage = tempSplit[0] + tempSplit[1] + tempSplit[2];
    integral(powImage, testInImage);
*/
    // 1. compute the number of patches for each row and column
    // if there will be some pixels which are left, we will just leave them unprocessed
    int PatchStepUpper = patchSizeUpper - OverlapSizeUpper;
    int PatchStepLower = patchSizeLower - OverlapSizeLower;

    //cout << "testImage.getImageCol() " << testImage.getImageCol() << endl;

    NumWidUpper = (testImage.getImageCol() - patchSizeUpper)/PatchStepUpper + 1;
    NumHeiUpper = (testImage.getImageRow() - patchSizeUpper)/PatchStepUpper + 1;
    NumWidLower = (testImage.getImageCol()- patchSizeLower)/PatchStepLower + 1;
    NumHeiLower = (testImage.getImageRow()- patchSizeLower)/PatchStepLower + 1;

    std::vector<struct Element> tempImagePatchUpper(NumHeiUpper*NumWidUpper);
    std::vector<struct Element> tempImagePatchLower(NumHeiLower*NumWidLower);
    ImagePatchUpper = tempImagePatchUpper;
    ImagePatchLower = tempImagePatchLower;

    std::vector<cv::Mat> tempUpperRight(NumHeiUpper*(NumWidUpper-1));
    std::vector<cv::Mat> tempUpperDown((NumHeiUpper-1)*NumWidUpper);
    std::vector<cv::Mat> tempLowerRight(NumHeiLower*(NumWidLower-1));
    std::vector<cv::Mat> tempLowerDown((NumHeiLower-1)*NumWidLower);

    quaSmoothUpperRight = tempUpperRight;
    quaSmoothUpperDown = tempUpperDown;
    quaSmoothLowerRight = tempLowerRight;
    quaSmoothLowerDown = tempLowerDown;



    // 2. find the k nearest neighbors for each patch

	FindPatches(sourceImages, targetImages, testImage, _INTEGRALIMAGES_UPPER_, patchSizeUpper, NumHeiUpper, NumWidUpper, ImagePatchUpper, OverlapSizeUpper, SearchWidthUpper);
	FindPatches(sourceImages, targetImages, testImage, _INTEGRALIMAGES_LOWER_, patchSizeLower, NumHeiLower, NumWidLower, ImagePatchLower, OverlapSizeLower, SearchWidthLower);



    //Timer timer;
    //timer.start();
    //for(int k = 0; k < photoImages.size(); k++)
    //{
        //prb->setValue(k);
        /*********** for every photo and sketch in the training set ************/
        //cout << "Training photo number: " << k << endl;
        // get the current training photo
        //Mat tempPhotos = imread(TrainingPhotos[k],1);
        //FileStorage Upper(TrainingPhotoIn20.at(k).c_str(),FileStorage::READ);
        //FileStorage Lower(TrainingPhotoIn10.at(k).c_str(),FileStorage::READ);

        //cout << "TrainingPhotoIn20.at(k): "<<TrainingPhotoIn20.at(k) << endl;
        //Mat UpperIn, LowerIn;
        //Upper["final20"]>>UpperIn;
        //Lower["final10"]>>LowerIn;
        //cvtColor(tempPhotos, tempPhotos, CV_RGB2GRAY, 0);
        //tempPhotos.convertTo(tempPhotos,CV_64FC1);
        //tempPhotos = 1.0*tempPhotos/255.0;

        //get the current training sketch
        //cout << "  " << TrainingSketches[k] << endl;
        //Mat tempSketches = imread(TrainingSketches[k], 1);
        //cvtColor(tempSketches, tempSketches, CV_RGB2GRAY, 0);
        //tempSketches.convertTo(tempSketches,CV_64FC1);
        //tempSketches = 1.0*tempSketches/255.0;

        //cout << "Find Patches Start" << endl;
        // deal with the upper layer of the current training patch
        //FindPatches(*photoImages.integralImages.at(k),
        //            *sketchImages.integralImages.at(k),
        //            testImage,
        //            20,
        //            patchSizeUpper, NumHeiUpper, NumWidUpper, ImagePatchUpper, OverlapSizeUpper,k, photoImages.size(), SearchWidthUpper);

        // deal with the lower layer of the current training patch
        //FindPatches(*photoImages.integralImages.at(k),
        //           *sketchImages.integralImages.at(k),
        //            testImage,
        //            10,
        //            patchSizeLower, NumHeiLower, NumWidLower, ImagePatchLower, OverlapSizeLower,k, photoImages.size(), SearchWidthLower);

        //cout<<"Find Patches End"<<endl;
    //}
    //timer.end();
    //MO //cout << "&&&***********************************&&&&" << endl;
    //MO //cout << "the total time used to deal with on image is " << timer.getTimeInString() << endl;

    //cout << "ComputerQuaTerm Upper" << endl;
    ComputeQuaTerm(ImagePatchUpper, NumHeiUpper, NumWidUpper, patchSizeUpper);
    //cout << "ComputerQuaTerm Lower" << endl;
    ComputeQuaTerm(ImagePatchLower, NumHeiLower, NumWidLower, patchSizeLower);

    //cout << "ComputeSmooth Upper" << endl;
    ComputeSmooth(ImagePatchUpper, NumHeiUpper, NumWidUpper, patchSizeUpper, OverlapSizeUpper, quaSmoothUpperRight, quaSmoothUpperDown);
    //cout << "ComputeSmooth Lower" << endl;
    ComputeSmooth(ImagePatchLower, NumHeiLower, NumWidLower, patchSizeLower, OverlapSizeLower, quaSmoothLowerRight, quaSmoothLowerDown);
    return 0;
}

int Synthesis::getPatchSize(){
    return patchSizeLower;
}

int Synthesis::getPatchOverlapSize(){
    return OverlapSizeLower;
}
