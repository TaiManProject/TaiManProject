#include "UsingQP.h"
#include "dlib/optimization.h"

MRFQP::MRFQP(Parameter parameter):Synthesis(parameter)
{
    lowerBound = 0;  // lower bound for each alpha is 0
    upperBound = 1;  // upper bound for each alpha is 1
    iteration = 10;  // temporarily set the iteration number to 40
    Dsigma = 1.7;    // the weight for different layer
}

MRFQP::MRFQP()
{

}

int MRFQP::initialize(cv::Mat &alpha, const std::vector<struct Element> &imagePatch, const int &NumHeight, const int &NumWidth)
{
    // this function is used to initialize alphaUpper and alphaLower
    // the weight is defined like this, suppose the the candidate patch has the distance x1,x2, ...
    // total = x1+x2+...., then the weight is define as 1-x1/total, and then average.
    // alpha        :    the weight we want to initialize
    // imagePatch   :    the struct used to store the information about image patch
    // NumHeight    :    the number of patches in height
    // NumWidth     :    the number of patches in width
    for(int i = 0; i < NumHeight; i++)
        for(int j = 0; j < NumWidth; j++)
        {
            int tempLocation = i*NumWidth +j;
            int pStep = tempLocation*NumOfCan;
            int count = NumOfCan - 1;
            const struct Element &tempElement = imagePatch.at(tempLocation);
            double total = 0;
            for(int kk = 0; kk < NumOfCan; kk++)
                total += tempElement.PatchData[kk].distance;
            if(total != 0)
                for(int kk = 0; kk < NumOfCan; kk++)
                {
                    alpha(cv::Range(pStep+kk,pStep+kk+1), cv::Range(0,1)).setTo(cv::Scalar((total - tempElement.PatchData[kk].distance)/total/count));
                }
            else
                for(int kk = 0; kk < NumOfCan; kk++)
                {
                    alpha(cv::Range(pStep+kk, pStep+kk+1), cv::Range(0,1)).setTo(cv::Scalar(1.0/NumOfCan));
                }
        }
    return 1;
}

int inline MRFQP::getNeighbor(const int &i, const int &j,const int &NumHeight, const int &NumWidth, std::vector<int>& Neighbor)
{
    // this function is used to find the neighbor of current point i, j.
    // the order of storage: first row, then col
    Neighbor.push_back(i);
    Neighbor.push_back(j);
    Neighbor.push_back(-1);
    if(i != 0)
    {
        // must have an upper neighbor
        Neighbor.push_back(i-1);
        Neighbor.push_back(j);
        Neighbor.push_back(0);
    }
    if(i != NumHeight -1)
    {
        // must have a lower neighbor
        Neighbor.push_back(i+1);
        Neighbor.push_back(j);
        Neighbor.push_back(1);
    }
    if(j != 0)
    {
        // must have a left neighbor
        Neighbor.push_back(i);
        Neighbor.push_back(j-1);
        Neighbor.push_back(2);
    }
    if(j != NumWidth - 1)
    {
        // must have a right neighbor
        Neighbor.push_back(i);
        Neighbor.push_back(j+1);
        Neighbor.push_back(3);
    }
    return 1;
}

int inline MRFQP::getNeighbor(const int &i, const int &j,const int &NumHeight, const int &NumWidth, std::vector<int>& Neighbor, const int &px, const int &py)
{
    // this function is used to find the neighbor of current point i, j excluding for (px,py)
    // for this ,the neighbor do not contain the point itself
    // the order of storage: first row, then col
    if(i != 0)
    {
        // must have an upper neighbor
        if((i-1) != px && j != py)
        {
            Neighbor.push_back(i-1);
            Neighbor.push_back(j);
            Neighbor.push_back(0);
        }
    }
    if(i != NumHeight -1)
    {
        // must have a lower neighbor
        if((i+1) != px && j != py)
        {
            Neighbor.push_back(i+1);
            Neighbor.push_back(j);
            Neighbor.push_back(1);
        }

    }
    if(j != 0)
    {
        // must have a left neighbor
        if(i != px && (j-1) != py)
        {
            Neighbor.push_back(i);
            Neighbor.push_back(j-1);
            Neighbor.push_back(2);
        }

    }
    if(j != NumWidth - 1)
    {
        // must have a right neighbor
        if(i != px && (j+1) != py)
        {
            Neighbor.push_back(i);
            Neighbor.push_back(j+1);
            Neighbor.push_back(3);
        }
    }
    return 1;

}

int MRFQP::getParameterUpper(cv::Mat &C, cv::Mat &H, cv::Mat &A, cv::Mat &b, const int &xi, const int &xj, std::vector<int> &Neighbor)
{
    // this function is used to compute the parameter of each minimization problem int the upper layer,
    // the parameters to be optimized are only the one in the center
    getNeighbor(xi, xj,NumHeiUpper,NumWidUpper,Neighbor);
    size_t numNeighbor = Neighbor.size()/3;
    int matSize = NumOfCan;                      // the  number of weight to be optimized

    C = cv::Mat::zeros(matSize, matSize, CV_32FC1);  // quadratic term
    H = cv::Mat::zeros(matSize, 1, CV_32FC1);        // linear term
    A = cv::Mat::ones(1, matSize, CV_32FC1);  // constraint paramter
    b = cv::Mat::ones(1, 1, CV_32FC1);  // constraint const

    /************************************************************************************/
    // the following code is used to put the data term into Qaa which is A here
    /***********************************************************************************/

    // first put the data term into A
    struct Element &tempElementCenter = ImagePatchUpper.at(xi*NumWidUpper+xj);
    tempElementCenter.quTerm.copyTo(C);

    // put the smooth term into A
    if(tempElementCenter.quSketchUp.empty() != true)
        C += tempElementCenter.quSketchUp;
    if(tempElementCenter.quSketchLow.empty() != true)
        C += tempElementCenter.quSketchLow;
    if(tempElementCenter.quSketchLeft.empty() != true)
        C += tempElementCenter.quSketchLeft;
    if(tempElementCenter.quSketchRight.empty() != true)
        C += tempElementCenter.quSketchRight;
    /**************************************************************************************/
    // end of computing Qaa
    /**************************************************************************************/

    /**************************************************************************************/
    // the following code is used to compute the linear term H
    /**************************************************************************************/
    H -= tempElementCenter.leaTerm;
    for(int i = 1; i < numNeighbor; i++)
    {
        int px , py, mark;
        px = Neighbor[3*i];
        py = Neighbor[3*i+1];
        mark = Neighbor[3*i+2];
        int currentLocation = px*NumWidUpper+py;
        struct Element &tempElementNeighbor = ImagePatchUpper.at(currentLocation);
        cv::Mat tempAlpha = alphaUpper(cv::Range(currentLocation*NumOfCan, (currentLocation+1)*NumOfCan),cv::Range(0,1));  // useful Wb
        cv::Mat midtemp;
        cv::Mat tempT;
        switch (mark)
        {
        case 0:
            {
                // this neighbor is upper neighbor
                quaSmoothUpperDown.at(currentLocation).copyTo(tempT);
                midtemp = tempT.t();
                break;
            }
        case 1:
            {
                // this neighbor is lower  neighbor
                quaSmoothUpperDown.at((px-1)*NumWidUpper+py).copyTo(midtemp);
                break;
            }
        case 2:
            {
                // this neighbor is left neighbor
                quaSmoothUpperRight.at(px*(NumWidUpper-1)+py).copyTo(tempT);
                midtemp = tempT.t();
                break;
            }
        case 3:
            {
                // this neighbor is a right neighbor
                quaSmoothUpperRight.at(px*(NumWidUpper-1)+py-1).copyTo(midtemp);
                break;
            }
        }
        H += midtemp*tempAlpha;
    }
    /*****************************************************************************/
    // end of computing H
    /*****************************************************************************/
    return 1;
}




int MRFQP::getParameterLower(cv::Mat &C, cv::Mat &H, cv::Mat &A, cv::Mat &b, const int &xi, const int &xj, std::vector<int> &Neighbor)
{
    // this function is used to compute the parameter of each minimization problem int the lower layer,
    // the parameters to be optimized are only the one in the center
    getNeighbor(xi, xj,NumHeiLower,NumWidLower,Neighbor);
    size_t numNeighbor = Neighbor.size()/3;
    int matSize = NumOfCan;                      // the  number of weight to be optimized

    C = cv::Mat::zeros(matSize, matSize, CV_32FC1);  // quadratic term
    H = cv::Mat::zeros(matSize, 1, CV_32FC1);        // linear term
    A = cv::Mat::ones(1, matSize, CV_32FC1);  // constraint paramter
    b = cv::Mat::ones(1, 1, CV_32FC1);  // constraint const

    /************************************************************************************/
    // the following code is used to put the data term into Qaa which is A here
    /***********************************************************************************/

    // compute the data term of Qaa
    struct Element &tempElementCenter = ImagePatchLower.at(xi*NumWidLower+xj);
    tempElementCenter.quTerm.copyTo(C);
    if(tempElementCenter.quSketchUp.empty() != true)
        C += tempElementCenter.quSketchUp;
    if(tempElementCenter.quSketchLow.empty() != true)
        C += tempElementCenter.quSketchLow;
    if(tempElementCenter.quSketchRight.empty() != true)
        C += tempElementCenter.quSketchRight;
    if(tempElementCenter.quSketchLeft.empty() != true)
        C += tempElementCenter.quSketchLeft;

    // compute the smooth term of different layer
    cv::Mat tempLowerMiddle, tempLower;
    int cols = patchSizeLower*patchSizeLower;
    tempLower.create(NumOfCan, cols,CV_32FC1);
    for(int mm = 0; mm < NumOfCan; mm++)
    {
        const struct Patch &tempPatch = tempElementCenter.PatchData.at(mm);
        tempPatch.SketchPatchData32F.copyTo(tempLowerMiddle);
        cv::Mat newTemp = tempLowerMiddle.reshape(1,1);
        newTemp.copyTo(tempLower(cv::Range(mm, mm+1), cv::Range(0, cols)));
    }
    C += beta*tempLower *tempLower.t();

    /*********************************************************************************/
    // end of computing Qaa
    /*********************************************************************************/

    /*********************************************************************************/
    // the following code is used to compute the linear term
    /*********************************************************************************/
    H -= tempElementCenter.leaTerm;

    for(int i = 1; i < (int)numNeighbor; i++) {
        int px , py, mark;
        px = Neighbor[3*i];
        py = Neighbor[3*i+1];
        mark = Neighbor[3*i+2];
        int currentLocation = px*NumWidLower+py;
        cv::Mat tempAlpha = alphaLower(cv::Range(currentLocation*NumOfCan, (currentLocation+1)*NumOfCan), cv::Range(0,1));  // useful Wb
        cv::Mat midtemp;
        cv::Mat tempT;
        switch (mark)
        {
        case 0:
            {
                // this neighbor is upper neighbor
                quaSmoothLowerDown.at(currentLocation).copyTo(tempT);
                midtemp = tempT.t();
                break;
            }
        case 1:
            {
                // this neighbor is lower  neighbor
                quaSmoothLowerDown.at((px-1)*NumWidLower+py).copyTo(midtemp);
                break;
            }
        case 2:
            {
                // this neighbor is left neighbor
                quaSmoothLowerRight.at(px*(NumWidLower-1)+py).copyTo(tempT);
                midtemp = tempT.t();
                break;
            }
        case 3:
            {
                // this neighbor is a right neighbor
                quaSmoothLowerRight.at(px*(NumWidLower-1)+py-1).copyTo(midtemp);
                break;
            }
        }
        H += midtemp*tempAlpha;
    }

    // compute the smooth term of different layer
    int smoothCols = patchSizeLower*patchSizeLower;
    cv::Mat smoothDif, tempSmoothDif;
    smoothDif.create(NumOfCan, smoothCols, CV_32FC1);
    for(int mm = 0; mm < NumOfCan; mm++)
    {
        const struct Patch &tempSketch = tempElementCenter.PatchData.at(mm);
        tempSketch.SketchPatchData32F.copyTo(tempSmoothDif);
        cv::Mat newTempSmoothDif = tempSmoothDif.reshape(1,1);
        newTempSmoothDif.copyTo(smoothDif(cv::Range(mm, mm+1), cv::Range(0, smoothCols)));
    }
    int rangRA = xi*OverlapSizeLower, rangRB = xi*OverlapSizeLower+patchSizeLower;
    int rangCA = xj*OverlapSizeLower, rangCB = xj*OverlapSizeLower+patchSizeLower;
    cv::Mat tempUpperSketch;
    resultUpper(cv::Range(rangRA, rangRB), cv::Range(rangCA, rangCB)).copyTo(tempUpperSketch);
    cv::Mat newTempUpperSketch = tempUpperSketch.reshape(1,1);

    H = H - (beta * smoothDif * newTempUpperSketch.t());
    return 1;
}



inline double computerDifference(const cv::Mat &tempPatch, const cv::Mat &tempSketch, const int &ranga1, const int &rangb1, const int &rangc1, const int &rangd1, const int &ranga2, const int &rangb2, const int &rangc2, const int &rangd2)
{
    // this function is used to compute the difference of the overlapping part
    // ranga1, rangb1, rangc1, rangd1 are used to mark the range of the first mat
    // ranga2, rangb2, rangc2, rangd2 are used to make the range of the second mat
    double result = 0;
    cv::Mat difference = tempPatch(cv::Range(ranga1,rangb1), cv::Range(rangc1,rangd1))- tempSketch(cv::Range(ranga2,rangb2),cv::Range(rangc2,rangd2));
    cv::Mat dist;
    cv::pow(difference, 2.0, dist);
    cv::Scalar x;
    x = sum(dist);
    if(x.channels == 1)
        result += x.val[0];
    else
        result += x.val[0] + x.val[1] + x.val[2];
    return result;
}


int MRFQP::findPath(const cv::Mat &srcPatch, const cv::Mat &distPatch, std::vector<int> &path, const int &type, std::vector<double> &cost, const int &PatchSize, const int &OverlapSize)
{
//    qDebug() << "findPath begin";
    // this function is used to find the cutting path
    // the patch starts from the last element of srcPatch should keep, which means start from "-1" of distPatch

    int PatchStep = PatchSize - OverlapSize;

    // tempCost is used to record all the possible costs
    // the selection width is OverlapSize + 1
    float **tempCost = NULL;
    tempCost = new float *[PatchSize];
    for(int i = 0; i < PatchSize; i++){
        //cout << "i:  " << i << endl;
        tempCost[i] = new float[OverlapSize+1];
    }
//    qDebug() << "tempCost";

    // PatchCandidate is used to stored all the candidate paths
    int **PathCandidate = NULL;
    PathCandidate = new int *[OverlapSize + 1];
    for(int i = 0; i <= OverlapSize; i++){
        PathCandidate[i] = new int[PatchSize];
    }
//    qDebug() << "pathCandidate";

    // first compute the element wise distance of two matrix
    cv::Mat tempDist = srcPatch - distPatch;
    pow(tempDist, 2.0, tempDist);
    std::vector<cv::Mat> vecMat;
    cv::split(tempDist, vecMat);
    cv::Mat distMat;
    distMat = vecMat.at(0);
    for(int i = 1; i < (int)vecMat.size(); i++){
        distMat += vecMat.at(i);
    }

    if(type == _USINGQP_LEFT_)
    {
//        qDebug() << "_USINGQP_LEFT_)";
        // in this situation, the patches are left and right neighbors

        // compute the cost of the first row
        for(int i = 0; i <= OverlapSize; i++)
            tempCost[0][i] = distMat.at<float>(0,i);

        // compute the cost of the rest rows
        for(int i = 1; i < PatchSize; i++)
            for(int j = 0; j <= OverlapSize; j++)
            {
                // used to record the minimum value

                // SearchRange is used to compute the search range of next node
                // usually, it only search left, middle and right three node
                int SearchRange = -1;

                // startIndex is used to record the begin node used to search
                int startIndex = -1;

                if(j == 0)
                {
                    // the first node, no left node
                    startIndex = 0;
                }
                else
                {
                    // the other node are from the left node
                    startIndex = j - 1;
                }

                if(j == 0 || j == OverlapSize)
                {
                    // these two nodes are at the edge and they only need to search two nodes
                    SearchRange = 2;
                }
                else
                {
                    SearchRange = 3;
                }

                float currentCost = distMat.at<float>(i,j);
                float minValue = 1e32;
                int Mark = -1;

                for(int k = 0; k < SearchRange; k++)
                {
                    // used to compute the cost of the next row
                    int index = startIndex + k;
                    if(minValue > tempCost[i-1][index])
                    {
                        minValue = tempCost[i-1][index];
                        Mark = index;
                    }
                }
                PathCandidate[j][i] = Mark;
                tempCost[i][j] = minValue + currentCost;
            }
    }
    else
    {
        // in this situation, the patches are up and down neighbors;

        // compute the cost of the first row
        for(int i = 0; i <= OverlapSize; i++)
            tempCost[0][i] = distMat.at<float>(i,0);

        // compute the cost of the rest rows
        for(int i = 1; i < PatchSize; i++)
            for(int j = 0; j <= OverlapSize; j++)
            {
                // used to record the minimum value
                float minValue = 1e32;
                int Mark = -1;

                // SearchRange is used to compute the search range of next node
                // usually, it only search left, middle and right three node
                int SearchRange;

                // startIndex is used to record the begin node used to search
                int startIndex;

                if(j == 0)
                {
                    // the first node, no up node
                    startIndex = 0;
                }
                else
                {
                    // the other node are from the up node
                    startIndex = j - 1;
                }

                if(startIndex == 0 || startIndex == OverlapSize - 1)

                {
                    // these two nodes are at the edge and they only need to search two nodes
                    SearchRange = 2;
                }
                else
                {
                    SearchRange = 3;
                }
                float currentCost = distMat.at<float>(j,i);
                for(int k = 0; k < SearchRange; k++)
                {
                    // used to compute the cost of the next column
                    int index = startIndex + k;
                    if(minValue > tempCost[i-1][index])
                    {
                        minValue = tempCost[i-1][index];
                        Mark = index;
                    }
                }
                PathCandidate[j][i] = Mark;
                tempCost[i][j] = minValue + minValue;
            }
    }
    //cout << "ckpoint A" << endl;
//    qDebug() << "ckpoint A";
    float MinCost = 1e32;
    int MinIndex = 0;
    //cout << "ckpoint B" << endl;
//    qDebug() << "ckpoint B";
    // find the minimum cost path
    for(int i = 0; i <= OverlapSize; i++)
    {
        //cout << "ckpoint C" << endl;
        float pathCost = tempCost[PatchSize - 1][i];
        if(MinCost > tempCost[PatchSize - 1][i])
        {
            MinIndex = i;
            MinCost = pathCost;
        }

        //record the index for the minimum path
        path[PatchSize - 1] = MinIndex - 1;

        // record the cost for the current node of the path
        if(type == _USINGQP_LEFT_)
            cost[PatchSize - 1] = distMat.at<float>(PatchSize - 1, MinIndex);
        else
            cost[PatchSize - 1] = distMat.at<float>(MinIndex, PatchSize - 1);
    }
//    qDebug() << "ckpoint C";
    for(int i = PatchSize - 2; i >=0; i--)
    {
        //cout << "ckpoint D i: " << i << "PatchSize: " << PatchSize << endl;
//        qDebug() << "ckpoint D i: " << i << "PatchSize: " << PatchSize;
        // record the path
        path[i] = PathCandidate[MinIndex][i+1] - 1;
        //cout << "path[i]" << path[i] << endl;
//        qDebug() << "path[i]" << path[i];
        MinIndex = path[i] + 1;

        if(type == _USINGQP_LEFT_){

            cost[i] = distMat.at<float>(i, MinIndex);
            //cout << "i: " << i << "  MinIndex: " << MinIndex << endl;
//            qDebug() << "i: " << i << "  MinIndex: " << MinIndex;
        }
        else{
            cost[i] = distMat.at<float>(MinIndex, i);
            //cout << "i: " << i << "  MinIndex: " << MinIndex << endl;
        }
    }
//    qDebug() << "ckpoint D1";
    // release memory
    for(int i = 0; i < PatchSize; i++){
        //cout << "ckpoing D1";
        delete[] tempCost[i];
    }
//    cout << "ckpoing D2";
    delete[] tempCost;
//    cout << "ckpoing D3";
    tempCost = NULL;

    //cout << "ckpoint E" << endl;

    for(int i = 0; i <= OverlapSize; i++)
        delete[] PathCandidate[i];
    delete[] PathCandidate;
    PathCandidate = NULL;

    return 0;
}

int MRFQP::CutEdge(cv::Mat &FinalResult, std::vector<cv::Mat> &FinalCandidate, const int &rows, const int &cols, const int &PatchSize, const int &overlapSize, const int &NumHei, const int &NumWid)
{
    // this function is used to merge all the candidate patches into the final results

    // initialize the final results
    //cout << "row: " << rows << "  col:" << cols << endl;
    FinalResult = cv::Mat::ones(rows, cols, CV_32FC1);
    int PatchStep = PatchSize - overlapSize;

//    qDebug() << "CutEdge";
    // pre-compute the ranges of all patches
    //cout << "Check the Range" << endl;
    //cout << "PatchSize: " << PatchSize << "  PatchStep: " << PatchStep << "   overlapSize: " << overlapSize << endl;

    cv::Range leftRow(0, PatchSize), leftCol(PatchStep - 1, PatchSize);
    cv::Range rightRow(0, PatchSize), rightCol(0, overlapSize + 1);
    cv::Range upRow(PatchStep - 1, PatchSize), upCol(0, PatchSize);
    cv::Range downRow(0, overlapSize + 1), downCol(0, PatchSize);

    //cout << "after created the range" << endl;
    //cout << "NumHei: " << NumHei << "  NumWid: " << NumWid << endl;
    for(int i = 0; i < NumHei; i++)
    {

        for(int j = 0; j < NumWid; j++)
        {


            //cout << "i: " << i << "  j:" << j << endl;

            int position = i * NumWid + j;
            cv::Mat &tempCurPatch = FinalCandidate.at(position);

            if(i == 0 && j == 0)
            {
//                qDebug() << "first patch";
                // this is the first patch, directly put it into the final results
                tempCurPatch.copyTo(FinalResult(cv::Range(0, PatchSize), cv::Range(0, PatchSize)));
            }
            else if(i == 0)
            {
//                qDebug() << "i == 0";
                /*********** these patches only have overlapping parts with the left patch *************/

                // get the position of the left patch
                int rowStart = i * PatchStep;
                int rowEnd = rowStart + PatchSize;
                int colStart = (j - 1) * PatchStep;
                int colEnd = colStart + PatchSize;

                //cout << "rowStart: " << rowStart << "  rowEnd: " << rowEnd << "  colStart:" << colStart <<"  colEnd: " << colEnd << endl;

                cv::Mat leftPatch;
                FinalResult(cv::Range(rowStart, rowEnd), cv::Range(colStart, colEnd)).copyTo(leftPatch);
                //cout << "check pt1" << endl;
//                qDebug() << "check pt1";
                cv::Mat leftOverlapPatch;
                leftPatch(leftRow, leftCol).copyTo(leftOverlapPatch);
                //cout << "check pt2" << endl;
//                qDebug() << "check pt2";
                // overlapping part of the right patch
                cv::Mat rightOverlapPatch;
                tempCurPatch(rightRow, rightCol).copyTo(rightOverlapPatch);
                //cout << "check pt3" << endl;
//                qDebug() << "check pt3";
                // find the cutting path, it is the right margin of the left patch
                // which means the cutPath is actually the last element of the left patch
                std::vector<int> cutPath(PatchSize);
                std::vector<double> cost(PatchSize);
                //cout << "check pt4" << endl;
//                qDebug() << "check pt4";
                findPath(leftOverlapPatch, rightOverlapPatch, cutPath, _USINGQP_LEFT_, cost, PatchSize, overlapSize);
                //cout << "check pt5" << endl;
//                qDebug() << "check pt5";
                for(int kk = 0; kk < PatchSize; kk++){
                    tempCurPatch(cv::Range(kk, kk+1), cv::Range(cutPath[kk]+1, PatchSize)).copyTo(FinalResult(cv::Range(rowStart + kk, rowStart + kk + 1), cv::Range(j*PatchStep+cutPath[kk] + 1, j*PatchStep+PatchSize)));
                    //cout << "kk: " << kk << endl;
                }// for testing
                //cout << "check pt5" << endl;
//                qDebug() << "check pt6";
                cv::Mat temp;
                FinalResult(cv::Range(i*PatchStep, i*PatchStep+PatchSize), cv::Range(j*PatchStep, j*PatchStep+PatchSize)).copyTo(temp);
            }
            else if(j == 0)
            {
//                qDebug() << "j == 0";
                /*********** these patches only have overlapping parts with the up patch *************/

                // get the position of the left patch
                int rowStart = (i - 1)  * PatchStep;
                int rowEnd = rowStart + PatchSize;
                int colStart = j * PatchStep;
                int colEnd = colStart + PatchSize;

                cv::Mat upPatch;
                FinalResult(cv::Range(rowStart, rowEnd), cv::Range(colStart, colEnd)).copyTo(upPatch);

                cv::Mat upOverlapPatch;
                upPatch(upRow, upCol).copyTo(upOverlapPatch);

                // overlapping part of the down patch
                cv::Mat downOverlapPatch;
                tempCurPatch(downRow, downCol).copyTo(downOverlapPatch);

                // find the cutting path, it is the up margin of the left patch
                // which means the cutPath is actually the last element of the up patch
                std::vector<int> cutPath(PatchSize);
                std::vector<double> cost(PatchSize);
                findPath(upOverlapPatch, downOverlapPatch, cutPath, _USINGQP_DOWN_, cost, PatchSize, overlapSize);
                for(int kk = 0; kk < PatchSize; kk++)
                    tempCurPatch(cv::Range(cutPath[kk] + 1, PatchSize), cv::Range(kk, kk+1)).copyTo(FinalResult(cv::Range(i*PatchStep+cutPath[kk]+1, i*PatchStep+PatchSize), cv::Range(j*PatchStep+kk, j*PatchStep+kk+1)));
            }
            else
            {
//                qDebug() << "else";

                /************  these patches have overlapping parts with the up patches and left patches  **********/

                // left part
                int leftRowStart = i * PatchStep;
                int leftRowEnd = leftRowStart + PatchSize;
                int leftColStart = (j - 1)*PatchStep;
                int leftColEnd = leftColStart + PatchSize;

                cv::Mat leftPatch;
                FinalResult(cv::Range(leftRowStart, leftRowEnd), cv::Range(leftColStart, leftColEnd)).copyTo(leftPatch);
                cv::Mat leftOverlapPatch;
                leftPatch(leftRow, leftCol).copyTo(leftOverlapPatch);

                cv::Mat rightOverlapPatch;
                tempCurPatch(rightRow, rightCol).copyTo(rightOverlapPatch);

                std::vector<int> leftCutPath(PatchSize);
                std::vector<double> leftCost(PatchSize);
                findPath(leftOverlapPatch, rightOverlapPatch, leftCutPath, _USINGQP_LEFT_, leftCost, PatchSize, overlapSize);

                // up part
                int upRowStart = (i - 1) * PatchStep;
                int upRowEnd = upRowStart + PatchSize;
                int upColStart = j * PatchStep;
                int upColEnd = upColStart + PatchSize;

                cv::Mat upPatch;
                FinalResult(cv::Range(upRowStart, upRowEnd), cv::Range(upColStart, upColEnd)).copyTo(upPatch);
                cv::Mat upOverlapPatch;
                upPatch(upRow, upCol).copyTo(upOverlapPatch);

                cv::Mat downOverlapPatch;
                tempCurPatch(downRow, downCol).copyTo(downOverlapPatch);

                std::vector<int> upCutPath(PatchSize);
                std::vector<double> upCost(PatchSize);
                findPath(upOverlapPatch, downOverlapPatch, upCutPath, _USINGQP_DOWN_, upCost, PatchSize, overlapSize);

                // find the cross point of the two patch
                int indexLeft = -1, indexUp = -1;
                int tempSign = -1;

                for(int mm = 0; mm < PatchSize; mm++)
                {
                    // for the cross point, left's row equals to up's column
                    // up's row equals to left's column
                    int cutPointLeft = leftCutPath[mm] + 1;
                    int cutPointUp = upCutPath[cutPointLeft] +1 ;
                    if(cutPointUp == mm)
                    {
                        // the two line are intersect in the same point
                        indexLeft = mm;
                        indexUp = cutPointLeft;
                        break;
                    }
                    else if (abs(cutPointUp - mm) == 1)
                    {
                        // the two line are near to each other at this point
                        tempSign = mm;
                    }
                }

                if(indexLeft == -1 || indexUp == -1)
                {
                    // the two line are not intersect at the same point
                    indexLeft = tempSign;
                    indexUp = leftCutPath[tempSign] + 1;
                }

                // there are four situations for cutting the edges
                // we suppose there are four pieces of cut edges example like this:
                //           |
                //           |
                //           | segment 1
                // ---------- -------
                // segment2  | segment3
                //           |
                //           |    segment4
                // the four cases are: 1+4, 2+3, 1+2, 3+4, next we compute the cost of these four situations respectively
                std::vector<double> allCost(4,0);

                // 1 + 4
                for(int nn = 0; nn < PatchSize; nn++)
                    allCost[0] += leftCost[nn];

                // 2 + 3
                for(int nn = 0; nn < PatchSize; nn++)
                    allCost[1] += upCost[nn];

                // 1 + 2
                for(int nn = 0; nn <= indexLeft; nn++)
                    allCost[2] += leftCost[nn];
                for(int nn = 0; nn <= indexUp; nn++)
                    allCost[2] += upCost[nn];

                // 3+4
                for(int nn = indexLeft; nn < PatchSize; nn++)
                    allCost[3] += leftCost[nn];
                for(int nn = indexUp; nn < PatchSize; nn++)
                    allCost[3] += upCost[nn];

                double minCost = 1e200;
                int cutCase = -1;
                for(int kk = 0; kk < 4; kk++)
                {
                    if(minCost >= allCost[kk])
                    {
                        minCost = allCost[kk];
                        cutCase = kk;
                    }
                }

                switch(cutCase)
                {
                case 0:
                    {
                        // the same with i == 0;
                        for(int kk = 0; kk < PatchSize; kk++)
                            tempCurPatch(cv::Range(kk, kk+1), cv::Range(leftCutPath[kk]+1, PatchSize)).copyTo(FinalResult(cv::Range(leftRowStart + kk, leftRowStart + kk + 1), cv::Range(j*PatchStep+leftCutPath[kk] + 1, j*PatchStep+PatchSize)));
                        // for testing
                        cv::Mat temp;
                        FinalResult(cv::Range(i*PatchStep, i*PatchStep+PatchSize), cv::Range(j*PatchStep, j*PatchStep+PatchSize)).copyTo(temp);
                    }
                case 1:
                    {
                        // the same with j == 0
                        for(int kk = 0; kk < PatchSize; kk++)
                            tempCurPatch(cv::Range(upCutPath[kk] + 1, PatchSize), cv::Range(kk, kk+1)).copyTo(FinalResult(cv::Range(i*PatchStep+upCutPath[kk]+1, i*PatchStep+PatchSize), cv::Range(j*PatchStep+kk, j*PatchStep+kk+1)));
                        // for testing
                        cv::Mat temp;
                        FinalResult(cv::Range(i*PatchStep, i*PatchStep+PatchSize), cv::Range(j*PatchStep, j*PatchStep+PatchSize)).copyTo(temp);
                    }
                case 2:
                    {

                        // fist copy the right up part
                        for(int kk = 0; kk <= indexLeft; kk++)
                            tempCurPatch(cv::Range(kk,kk+1), cv::Range(leftCutPath[kk]+1, PatchSize)).copyTo(FinalResult(cv::Range(leftRowStart + kk, leftRowStart + kk + 1), cv::Range(j*PatchStep+leftCutPath[kk] + 1, j*PatchStep+PatchSize)));

                        // copy the left down part
                        for(int kk = 0; kk <= indexUp; kk++)
                            tempCurPatch(cv::Range(upCutPath[kk] + 1, PatchSize), cv::Range(kk, kk+1)).copyTo(FinalResult(cv::Range(i*PatchStep+upCutPath[kk]+1, i*PatchStep+PatchSize), cv::Range(j*PatchStep+kk, j*PatchStep+kk+1)));

                        // copy the rest part
                        tempCurPatch(cv::Range(indexLeft, PatchSize), cv::Range(indexUp, PatchSize)).copyTo(FinalResult(cv::Range(i*PatchStep+indexLeft, i*PatchStep+PatchSize), cv::Range(j*PatchStep+indexUp, j*PatchStep+PatchSize)));

                        // for testing
                        cv::Mat temp;
                        FinalResult(cv::Range(i*PatchStep, i*PatchStep+PatchSize), cv::Range(j*PatchStep, j*PatchStep+PatchSize)).copyTo(temp);
                    }
                case 3:
                    {


                        // copy the original finalResult part to a temp Mat
                        cv::Mat temp;
                        FinalResult(cv::Range(i*PatchStep, i*PatchStep+PatchSize), cv::Range(j*PatchStep, j*PatchStep+PatchSize)).copyTo(temp);

                        // copy the whole new patch to the finalResult
                        tempCurPatch(cv::Range(0, PatchSize), cv::Range(0, PatchSize)).copyTo(FinalResult(cv::Range(i*PatchStep, i*PatchStep+PatchSize), cv::Range(j*PatchStep, j*PatchStep+PatchSize)));

                        // copy the left down part back to the finalResult
                        for(int kk = indexLeft+1; kk<PatchSize; kk++)
                        {
                            if(leftCutPath[kk] != -1){
                                temp(cv::Range(kk,kk+1), cv::Range(0, leftCutPath[kk]+1)).copyTo(FinalResult(cv::Range(i*PatchStep+kk, i*PatchStep+kk+1), cv::Range(j*PatchStep, j*PatchStep+leftCutPath[kk]+1)));
                            }
                        }
                        // copy the right up part back to the finaResult
                        for(int kk = indexUp+1; kk < PatchSize; kk++)
                        {
                            if(upCutPath[kk] != -1)
                            {
                                temp(cv::Range(0, upCutPath[kk]+1), cv::Range(kk,kk+1)).copyTo(FinalResult(cv::Range(i*PatchStep, i*PatchStep+upCutPath[kk]+1), cv::Range(j*PatchStep, j*PatchStep+1)));
                            }
                        }

                        // copy the rest part back to the finalResult
                        temp(cv::Range(0, indexLeft+1), cv::Range(0,indexUp+1)).copyTo(FinalResult(cv::Range(i*PatchStep, i*PatchStep+indexLeft+1), cv::Range(j*PatchStep, j*PatchStep+indexUp+1)));
/*
                        if(i == NumHei - 1)
                        {
                            int ccc = 1;
                        }
                        // copy the original finalResult part to a temp Mat
                        Mat temp;
                        FinalResult(Range(i*PatchStep, i*PatchStep+PatchSize), Range(j*PatchStep, j*PatchStep+PatchSize)).copyTo(temp);

                        // copy the whole new patch to the finalResult
                        tempCurPatch(Range(0, PatchSize), Range(0, PatchSize)).copyTo(FinalResult(Range(i*PatchStep, i*PatchStep+PatchSize), Range(j*PatchStep, j*PatchStep+PatchSize)));

                        // copy the left down part back to the finalResult
                        for(int kk = indexLeft; kk<PatchSize; kk++)
                        {
                            if(leftCutPath[kk] != -1)// OpenCV will return error if upCutPath[kk] == 1
                            {
                                temp(Range(kk,kk+1), Range(0, leftCutPath[kk]+1)).copyTo(FinalResult(Range(i*PatchStep+kk, i*PatchStep+kk+1), Range(j*PatchStep, j*PatchStep+leftCutPath[kk]+1)));
                            }
                        }

                        // copy the right up part back to the finaResult
                        for(int kk = indexUp; kk < PatchSize; kk++)
                        {
                            if(upCutPath[kk] != -1)   // OpenCV will return error if upCutPath[kk] == 1
                            {
                                temp(Range(0, upCutPath[kk]+1), Range(kk,kk+1)).copyTo(FinalResult(Range(i*PatchStep, i*PatchStep+upCutPath[kk]+1), Range(j*PatchStep, j*PatchStep+1)));

                            }
                        }

                        // copy the rest part back to the finalResult
                        if(indexLeft > 0 && indexUp > 0)   // OpenCV will return error if indexLeft or indexUp == 1
                        {
                            temp(Range(0, indexLeft), Range(0,indexUp)).copyTo(FinalResult(Range(i*PatchStep, i*PatchStep+indexLeft), Range(j*PatchStep, j*PatchStep+indexUp)));
                        }

*/
                    }
                }
            }
        }
    }
//    qDebug() << "CutEdge over";
    return 0;
}




int inline Mat2Dlib(dlib::matrix<double> &dist, const cv::Mat& src)
{
    // this function is used to convert a mat to a dlib matrix
    dist.set_size(src.rows, src.cols);
    for(int i = 0; i < src.rows; i++){
        for(int j = 0; j < src.cols; j++){
            dist(i,j) = src.at<float>(i,j);
		}
	}
    return 1;
}

int inline Dlib2Mat(const dlib::matrix<double> &src, cv::Mat &dist)
{
    // this function is used to convert a dlib matrix to a mat
    dist.create(src.nr(), src.nc(), CV_32FC1);
    for(int i = 0; i < src.nr(); i++){
        for(int j = 0; j < src.nc(); j++){
			float temp = (float) src(i, j);
            dist.at<float>(i,j) = temp;
		}
	}
    return 1;
}


int MRFQP::energyMinimize()
{
//    qDebug() << "energyMinimize()";
    double eps = 1e-10;
    int max_iter = 100;
    alphaUpper = cv::Mat::zeros(NumHeiUpper*NumWidUpper*NumOfCan, 1, CV_32FC1);
    alphaLower = cv::Mat::zeros(NumHeiLower*NumWidLower*NumOfCan, 1, CV_32FC1);
    initialize(alphaUpper, ImagePatchUpper, NumHeiUpper, NumWidUpper);
    initialize(alphaLower, ImagePatchLower, NumHeiLower, NumWidLower);
    /*****************************************************************/
    // deal with the upper layer
    int NumReserve = 3*NumOfCan;            // the number needed to reserve for the neighbor of each patch
                                            // each neighbor need 3 part of space to store
    //Timer timer;
    //timer.start();
//    qDebug() << "timer start()";

    for(int ite = 0; ite < iteration; ite++)
    {
        //cout<<"iteration:  "<<ite<<endl;
        for(int i = 0; i < NumHeiUpper; i++)
        {
            for(int j = 0; j < NumWidUpper; j++)
            {
                if( (i+j)%2 == 0)
                {

                    cv::Mat C, H, A, b;
                    std::vector<int> Neighbor;
                    Neighbor.reserve(NumReserve);   // every neighbor marked by 3 element, first is x coordinate
                                                    // second is y coordinate, the third is the mark of up, down, left, right
                                                    // up: 0, down: 1, left: 2, right: 3
                    getParameterUpper(C, H, A, b, i, j, Neighbor);
                    dlib::matrix<double> matrix_C, matrix_H, matrix_Alpha;
                    Mat2Dlib(matrix_C, C);
                    Mat2Dlib(matrix_H, H);
                    matrix_H = matrix_H*(-1);
                    matrix_Alpha.set_size(NumOfCan,1);
                    dlib::set_all_elements(matrix_Alpha, 1.0/NumOfCan);
                    dlib::solve_qp_using_smo(matrix_C, matrix_H, matrix_Alpha, eps, max_iter);
                    cv::Mat tempAlpha;
                    Dlib2Mat(matrix_Alpha, tempAlpha);

                    int pStart = (i*NumWidUpper+j)*NumOfCan;  // the start position of the current patch in the weight vector
                    tempAlpha.copyTo(alphaUpper(cv::Range(pStart, pStart+NumOfCan),cv::Range(0,1)));
                }
            }
        }

//        qDebug() << "checkpoint 1";
        for(int i = 0; i < NumHeiUpper; i++)
        {
            for(int j = 0; j < NumWidUpper; j++)
            {
                if( (i+j)%2 == 1)
                {
                    cv::Mat C, H, A, b;
                    std::vector<int> Neighbor;
                    Neighbor.reserve(NumReserve);   // every neighbor marked by 3 element, first is x coordinate
                                                    // second is y coordinate, the third is the mark of up, down, left, right
                                                    // up: 0, down: 1, left: 2, right: 3
                    getParameterUpper(C, H, A, b, i, j, Neighbor);

                    dlib::matrix<double> matrix_C, matrix_H, matrix_Alpha;
                    Mat2Dlib(matrix_C, C);
                    Mat2Dlib(matrix_H, H);
                    matrix_H = matrix_H*(-1);
                    matrix_Alpha.set_size(NumOfCan,1);
                    dlib::set_all_elements(matrix_Alpha, 1.0/NumOfCan);
                    dlib::solve_qp_using_smo(matrix_C, matrix_H,matrix_Alpha, eps, max_iter);
                    cv::Mat tempAlpha;
                    Dlib2Mat(matrix_Alpha, tempAlpha);

                    int pStart = (i*NumWidUpper+j)*NumOfCan;    // start position
                    tempAlpha.copyTo(alphaUpper(cv::Range(pStart, pStart+NumOfCan), cv::Range(0,1)));

                }
            }
        }

    }

//    qDebug() << "timer end()";
    //timer.end();
    //cout<<"time used for one iteration of the upper layer " << endl;
    /********************************************************************/

    /*******************************************************************/
    // the following code is used to compute the upper layer result using
    // the linear combination of the sketch patch
    /*******************************************************************/

    //fstream fwriteUpper;
    //fwriteUpper.open(fileNameUpper.c_str(), fstream::out);
    //fwriteUpper<<NumHeiUpper<<endl;
    //fwriteUpper<<NumWidUpper<<endl;


    std::vector<cv::Mat> sketchCandidate(NumHeiUpper*NumWidUpper);
    for(int i = 0; i < NumHeiUpper; i++)
    {
        for(int j = 0; j < NumWidUpper; j++)
        {
            int currentPosition = i*NumWidUpper+j;
            const struct Element &tempPatch = ImagePatchUpper.at(currentPosition);
            sketchCandidate.at(currentPosition)= cv::Mat::zeros(patchSizeUpper, patchSizeUpper, CV_32FC1);
            double weight = 0;
            for(int mm = 0; mm < NumOfCan; mm++)
            {
                int currentStep = currentPosition*NumOfCan+mm;
                weight = alphaUpper.at<float>(currentStep,0);
                //fwriteUpper<<weight<<" ";
                sketchCandidate.at(currentPosition) += weight*tempPatch.PatchData.at(mm).SketchPatchData32F;
            }
            //fwriteUpper<<endl;
        }
    }

    //fwriteUpper.close();

    // get the final result of
    //resultUpper = Mat::ones(ImageRows, ImageCols, CV_64FC1);
    //cout << "b4 1st CutEdge" << endl;
//    qDebug() << "CutEdge";
    CutEdge(resultUpper, sketchCandidate, ImageRows, ImageCols, patchSizeUpper, OverlapSizeUpper, NumHeiUpper, NumWidUpper );
    //cout << "after 1st CutEdge" << endl;
    /*******************************************************************/
    //timer.start();

//    qDebug() << "timer2 start";

    // get the weight for the lower layer
    //cout << "iteration: " << iteration << "NumHeiLower: " << NumHeiLower << "  NumWidLower:" << NumWidLower << endl;
    for(int ite = 0; ite < iteration; ite++)
    {
        //cout<<"iteration:  "<<ite<<endl;
        for(int i = 0; i < NumHeiLower; i++)
        {
            for(int j = 0; j < NumWidLower; j++)
            {
                //cout << "i(outer): " << i << "  j(inner): " << j << endl;
                if((i+j)%2 == 0)
                {
                    cv::Mat C, H, A, b;
                    std::vector<int> Neighbor;
                    Neighbor.reserve(NumReserve);	// every neighbor marked by 3 element, first is x coordinate
                                            // second is y coordinate, the third is the mark of up, down, left, right
                                            // up: 0, down: 1, left: 2, right: 3
                    getParameterLower(C, H, A, b, i, j, Neighbor);

                    dlib::matrix<double> matrix_C, matrix_H, matrix_Alpha;
                    Mat2Dlib(matrix_C, C);
                    Mat2Dlib(matrix_H, H);
                    matrix_H = matrix_H*(-1);
                    matrix_Alpha.set_size(NumOfCan,1);
                    dlib::set_all_elements(matrix_Alpha, 1.0/NumOfCan);
                    dlib::solve_qp_using_smo(matrix_C, matrix_H,matrix_Alpha, eps, max_iter);
                    cv::Mat tempAlpha;
                    Dlib2Mat(matrix_Alpha, tempAlpha);

                    int pStart = (i*NumWidLower+j)*NumOfCan;
                    tempAlpha.copyTo(alphaLower(cv::Range(pStart, pStart+NumOfCan), cv::Range(0,1)));
                }
            }
        }
        for(int i = 0; i < NumHeiLower; i++)
        {
            for(int j = 0; j < NumWidLower; j++)
            {
                if((i+j)%2 == 1)
                {
                    cv::Mat C, H, A, b;
                    std::vector<int> Neighbor;
                    Neighbor.reserve(NumReserve);   // every neighbor marked by 3 element, first is x coordinate
                                            // second is y coordinate, the third is the mark of up, down, left, right
                                            // up: 0, down: 1, left: 2, right: 3
                    getParameterLower(C, H, A, b, i, j, Neighbor);

                    dlib::matrix<double> matrix_C, matrix_H, matrix_Alpha;
                    Mat2Dlib(matrix_C, C);
                    Mat2Dlib(matrix_H, H);
                    matrix_H = matrix_H*(-1);
                    matrix_Alpha.set_size(NumOfCan,1);
                    dlib::set_all_elements(matrix_Alpha, 1.0/NumOfCan);
                    dlib::solve_qp_using_smo(matrix_C, matrix_H,matrix_Alpha, eps, max_iter);
                    cv::Mat tempAlpha;
                    Dlib2Mat(matrix_Alpha, tempAlpha);

                    int pStart = (i*NumWidLower+j)*NumOfCan;
                    tempAlpha.copyTo(alphaLower(cv::Range(pStart, pStart+NumOfCan), cv::Range(0,1)));

                }
            }
        }
    }
//    qDebug() << "timer2 end";
    //timer.end();
    //cout << "time used for one iteration of the lower layer " <<endl;
    /****************************************************************/
    // get the final result

    /*******************************************************************/
    // the following code is used to compute the upper layer result using
    // the linear combination of the sketch patch
    /*******************************************************************/

    //fstream fwriteLower;
    //fwriteLower.open(fileNameLower.c_str(), fstream::out);
    //fwriteLower<<NumHeiLower<<endl;
    //fwriteLower<<NumWidLower<<endl;

    std::vector<cv::Mat> sketchCandidateLower(NumHeiLower*NumWidLower);
    for(int i = 0; i < NumHeiLower; i++)
    {
        for(int j = 0; j < NumWidLower; j++)
        {

            int currentPosition = i*NumWidLower+j;
            const struct Element &tempPatch = ImagePatchLower.at(currentPosition);
            sketchCandidateLower.at(currentPosition)= cv::Mat::zeros(patchSizeLower, patchSizeLower, CV_32FC1);
            double weight = 0;
            for(int mm = 0; mm < NumOfCan; mm++)
            {
                int currentStep = currentPosition*NumOfCan + mm;
                weight = alphaLower.at<float>(currentStep,0);
                //fwriteLower<<weight<<" ";
                sketchCandidateLower.at(currentPosition) += weight*tempPatch.PatchData.at(mm).SketchPatchData32F;
            }
        //fwriteLower<<endl;
        }
    }
    //fwriteLower.close();
        // get the final result of
    //FinalResult = Mat::ones(ImageRows, ImageCols, CV_64FC1);
    //cout << "b4 CutEdge" << endl;
    CutEdge(FinalResult, sketchCandidateLower, ImageRows, ImageCols, patchSizeLower, OverlapSizeLower, NumHeiLower, NumWidLower);
    /****************************************************************/

    return 0;
}

cv::Mat MRFQP::getResultWeight(){
    return alphaLower;
}
