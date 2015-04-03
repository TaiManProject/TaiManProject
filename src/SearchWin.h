#ifndef SEARCHWIN_H
#define SEARCHWIN_H

#include <string>
#include <fstream>
#include <iostream>

#include <QList>
#include <time.h>
#include <opencv/cv.h>

#include <flann/flann.hpp>

#include "commonTool.h"
#include "IntegralImage.h"

class SearchWin {
public:
	SearchWin(void);
	SearchWin(Parameter parameter, int level, int indexOfIndexPatchRow, int indexOfIndexPatchCol);
	SearchWin(Parameter parameter, int level);

    void buildMatAndTree(std::vector<IntegralImage*> images);
    flann::Matrix<unsigned short> buildMat(std::vector<IntegralImage*>& images);
    void buildKDTree(flann::Matrix<unsigned short>& allPatch);
	//void clearMat();
    void foundKNNInSearchWin(std::vector<patchIdentifier>& listOfKNN, std::vector<unsigned short>& testPatchIn1D_p, int& numberOfCanToFound);
    void foundKNNInSearchWinByBruteforce(std::vector<IntegralImage*> images, std::vector<patchIdentifier>& listOfKNN, cv::Mat testPatch, int numberOfCanToFound);
    //void foundKRandom(std::vector<patchIdentifier>& listOfKNN, int numberToFound);

private:
	Parameter parameter;
    std::string id;

	int level; // for ID use only

	int searchWinExpandSize;

	int indexPatchPosRow;
	int indexPatchPosCol;
	int indexPatchSize;
	int indexPatchOverlapSize;
	int indexPatchStep;

	int dbPatchInRow;
	int dbPatchInCol;

	int imageRow;
	int imageCol;

	int searchWinRow;
	int searchWinCol; //for bound case
	int searchWinNumPatch;
	cv::Range searchWinRangeRow;
	cv::Range searchWinRangeCol;

	int allPatchNumRow;
	int allPatchNumCol;

	//cv::flann::Index kdtree;
    //why not unisgned char?
    flann::Index<flann::L2<unsigned short> >* kdtree;

	patchIdentifier rowNumber2Identifier(int rowNumber);

};

#endif
