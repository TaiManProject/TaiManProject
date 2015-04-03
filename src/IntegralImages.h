#ifndef INTEGRALIMAGES_H
#define INTEGRALIMAGES_H

#include "IntegralImage.h"
#include "SearchWin.h"
#include "commonTool.h"

class IntegralImages {
public:
    IntegralImages();
    std::vector<IntegralImage*> integralImages;

    void load(QString Path, QStringList fileNameList, bool treeNeeded, Parameter parameter);
    int size();
    void clean();
	patchIdentifier rowNum2Identifier(int rowNum, int level, int indexOfIndexPatch);
    int foundKNN( std::vector<patchIdentifier>& listOfKNN, cv::Mat testPatch, int level, int numberOfCanToFound, int indexOfIndexPatch, int rangRowStart, int rangColStart);
	cv::Mat getImage(int index);
	QString getImagePath(int index);

private:
	int imageRow;
	int imageCol;

	Parameter parameter;

	int patchStepUpper;
    int patchStepLower;

	int numIndexPatchColUpper;
    int numIndexPatchRowUpper;
    int numIndexPatchColLower;
    int numIndexPatchRowLower;

    std::vector<SearchWin*> searchWinsLower;
    std::vector<SearchWin*> searchWinsUpper;

	void buildKDTree(int level);
};

#endif // INTEGRALIMAGES_H
