#ifndef USINGQP_H
#define USINGQP_H

#include "Synthesis.h"



#define _USINGQP_UPPER 1
#define _USINGQP_LOWER 2

#define _USINGQP_LEFT_ 0
#define _USINGQP_DOWN_ 1

class MRFQP : public Synthesis
{
public:
    cv::Mat FinalResult;          // used to store the final result
    MRFQP();
    MRFQP(Parameter paramter);
    int energyMinimize();    // get the alpha that minimize the alpha
    cv::Mat getResultWeight();  // get the weight of candidate Patch

private:
    cv::Mat alphaUpper;      // used to store all the weight of each patch in the upper layer
    cv::Mat alphaLower;      // used to store all the weight of each patch in the lower layer
    double lowerBound, upperBound;  // the lower bound and upper bound of each alpha
    int iteration;                  // used to mark the maximum number of iterations
    double Dsigma;                  // the weight for different layer
    cv::Mat resultUpper;                 // used to store the result of upper layer;

    int initialize(cv::Mat &alpha, const std::vector<struct Element> &imagePatch, const int &NumHeight, const int &NumWidth);
                                    // initialize the value of alpha based on the distance
    int getParameterUpper(cv::Mat &C, cv::Mat &H, cv::Mat &A, cv::Mat &b, const int &xi, const int &xj, std::vector<int> &Neighbor);
                                    // use to get ready for minimize each patch in the upper layer
    int getParameterLower(cv::Mat &C, cv::Mat &H, cv::Mat &A, cv::Mat &b, const int &xi, const int &xj, std::vector<int> &Neighbor);
                                    // use to get ready for minimize each patch in the lower layer
    int inline getNeighbor(const int &i, const int &j,const int &NumHeight, const int &NumWidth, std::vector<int>&Neighbor);
                                    // this function is used to find the neighbor of the point i, j
    int inline getNeighbor(const int &i, const int &j,const int &NumHeight, const int &NumWidth, std::vector<int>&Neighbor, const int &px, const int &py);

    int CutEdge(cv::Mat &FinalResult, std::vector<cv::Mat> &FinalCandidate, const int &rows, const int &cols, const int &PatchSize, const int &overlapSize, const int &NumHei, const int &NumWid);
    int findPath(const cv::Mat &srcPatch, const cv::Mat &distPatch, std::vector<int> &path, const int &type, std::vector<double> &cost, const int &PatchSize, const int &OverlapPart);
};

#endif // USINGQP_H
