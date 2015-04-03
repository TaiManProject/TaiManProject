#include "SearchWin.h"

SearchWin::SearchWin(Parameter para, int level_p, int indexOfIndexPatchRow_p, int indexOfIndexPatchCol_p){
    parameter = para;
    indexPatchPosRow = indexOfIndexPatchRow_p;
    indexPatchPosCol = indexOfIndexPatchCol_p;
    imageCol = para.imageCol;
    imageRow = para.imageRol;
    level = level_p;

    if (level == _INTEGRALIMAGES_LOWER_){
        indexPatchOverlapSize = para.OverlapSizeLower;          // 5
        searchWinExpandSize = para.SearchWidthLower;
        indexPatchSize = para.patchSizeLower;
    } else{
        indexPatchOverlapSize = para.OverlapSizeUpper;
        searchWinExpandSize = para.SearchWidthUpper;
        indexPatchSize = para.patchSizeUpper;
    }

    indexPatchStep = indexPatchSize - indexPatchOverlapSize;

    int indexPatchColStart = indexPatchPosCol * indexPatchStep;
    int indexPatchColEnd = indexPatchColStart + indexPatchSize;
    int indexPatchRowStart = indexPatchPosRow * indexPatchStep;
    int indexPatchRowEnd = indexPatchRowStart + indexPatchSize;

    int tempSearchWinColStart = indexPatchColStart - searchWinExpandSize;
    int tempSearchWinColEnd = indexPatchColEnd + searchWinExpandSize;
    int tempSearchWinRowStart = indexPatchRowStart - searchWinExpandSize;
    int tempSearchWinRowEnd = indexPatchRowEnd + searchWinExpandSize;

    //checking bound
    if (tempSearchWinColStart < 0){
        tempSearchWinColStart = 0;
    }
    if (tempSearchWinColEnd > imageCol){
        tempSearchWinColEnd = imageCol;
    }
    if (tempSearchWinRowStart < 0){
        tempSearchWinRowStart = 0;
    }
    if (tempSearchWinRowEnd > imageRow){
        tempSearchWinRowEnd = imageRow;
    }

    searchWinRangeCol.start = tempSearchWinColStart;
    searchWinRangeCol.end= tempSearchWinColEnd;

    searchWinRangeRow.start = tempSearchWinRowStart;
    searchWinRangeRow.end = tempSearchWinRowEnd;

    searchWinCol = searchWinRangeCol.end - searchWinRangeCol.start;
    searchWinRow = searchWinRangeRow.end - searchWinRangeRow.start;


    dbPatchInRow = (searchWinCol - indexPatchSize) / parameter.samplingRate + 1;
    dbPatchInCol = (searchWinRow - indexPatchSize) / parameter.samplingRate + 1;

    searchWinNumPatch = dbPatchInRow * dbPatchInCol;

    id = int2str(level) + "_" + int2str(indexPatchPosCol) + "_" + int2str(indexPatchPosRow);
}

SearchWin::SearchWin(Parameter para, int level_p){
    parameter = para;

    imageCol = para.imageCol;
    imageRow = para.imageRol;
    level = level_p;

    if (level == _INTEGRALIMAGES_LOWER_){
        indexPatchOverlapSize = para.OverlapSizeLower;      // 5
        searchWinExpandSize = para.SearchWidthLower;        // 8
        indexPatchSize = para.patchSizeLower;               // 10
    } else{
        indexPatchOverlapSize = para.OverlapSizeUpper;      // 5
        searchWinExpandSize = para.SearchWidthUpper;        // 5
        indexPatchSize = para.patchSizeUpper;               // 20
    }

    searchWinRangeCol.start = 0;
    searchWinRangeCol.end= para.imageCol;                   // 200

    searchWinRangeRow.start = 0;
    searchWinRangeRow.end = para.imageRol;                  // 250

    searchWinCol = searchWinRangeCol.end - searchWinRangeCol.start;                 // 200 - 0 = 200
    searchWinRow = searchWinRangeRow.end - searchWinRangeRow.start;                 // 250 - 0 = 250
    // UPPER  | LOWER
    dbPatchInRow = (searchWinCol - indexPatchSize) / parameter.samplingRate + 1;    // 91     | 96
    dbPatchInCol = (searchWinRow - indexPatchSize) / parameter.samplingRate + 1;    // 116    | 121
    //x_______|x_____
    searchWinNumPatch = dbPatchInRow * dbPatchInCol;                                // 10556    11616

    id = int2str(level) + "_" + int2str(indexPatchPosCol) + "_" + int2str(indexPatchPosRow);

}

void SearchWin::buildMatAndTree(std::vector<IntegralImage*> images){
    flann::Matrix<unsigned short> patches = buildMat(images);
    buildKDTree(patches);
    //allPatch.convertTo(allPatch, CV_8U);
}

// Build all patches into a 2D matrix, first 2 columns store x and y of top-left coordinates of each patch(column)
flann::Matrix<unsigned short> SearchWin::buildMat(std::vector<IntegralImage*>& images){
    allPatchNumRow = searchWinNumPatch * images.size();
    allPatchNumCol = indexPatchSize * indexPatchSize;

    if (parameter.additionDim){ //add X and Y dimensions
        allPatchNumCol = allPatchNumCol + 2;
    }

    flann::Matrix<unsigned short> allPatch(new unsigned short[allPatchNumRow * allPatchNumCol], allPatchNumRow, allPatchNumCol);
    int count = 0;
    int dbPatchRowCount = 0;
    int dbPatchColCount = 0;
    for ( int imageIndex = 0; imageIndex < (int)images.size(); imageIndex++){
        dbPatchRowCount = 0;
        for ( int currentPatchRowStart = searchWinRangeRow.start; currentPatchRowStart + indexPatchSize <= searchWinRangeRow.end; currentPatchRowStart += parameter.samplingRate){
            dbPatchRowCount++;
            dbPatchColCount = 0;
            for ( int currentPatchColStart = searchWinRangeCol.start; currentPatchColStart + indexPatchSize <= searchWinRangeCol.end; currentPatchColStart += parameter.samplingRate){
                dbPatchColCount++;

                cv::Mat tempPatch;
                cv::Mat tempPatch1D;

                int currentPatchRowEnd = currentPatchRowStart + indexPatchSize;
                int currentPatchColEnd = currentPatchColStart + indexPatchSize;

                cv::Range patchRangeRow(currentPatchRowStart, currentPatchRowEnd);
                cv::Range patchRangeCol(currentPatchColStart, currentPatchColEnd);

                images.at(imageIndex)->image(patchRangeRow, patchRangeCol).copyTo(tempPatch);
                tempPatch1D = tempPatch.reshape(1, 1);

                int additionDim = 0;
                if (parameter.additionDim){
                    additionDim = 2;

                    allPatch.ptr()[count * allPatchNumCol + 0] = currentPatchColStart * parameter.additionDimWeight;
                    allPatch.ptr()[count * allPatchNumCol + 1] = currentPatchRowStart * parameter.additionDimWeight;

                }

                //float* temp = tempPatch1D.ptr<float>(0);

                unsigned char* temp = tempPatch1D.ptr<unsigned char>(0);

                for ( int i = 0; i < tempPatch1D.cols; i++ ){
                    unsigned char tempOneElement = (unsigned char) temp[i];

                    allPatch.ptr()[count * allPatchNumCol + i + additionDim] = tempOneElement;
                }

                //tempPatch1D.copyTo(allPatch(cv::Range(count, count + 1), cv::Range(0, allPatchNumCol)));

                count++;
            }
        }
    }

    dbPatchInCol = dbPatchRowCount;
    dbPatchInRow = dbPatchColCount;
    return allPatch;
}

//Audrey
//Construct an randomized kd-tree index using 4 kd-trees
void SearchWin::buildKDTree(flann::Matrix<unsigned short>& allPatch) {
    kdtree = new flann::Index<flann::L2<unsigned short> >(allPatch, flann::KDTreeIndexParams(4));
    kdtree->buildIndex();
    //load("E:/ITFFaceMatchingProject/data/test/kdtree.txt");
}

patchIdentifier SearchWin::rowNumber2Identifier(int rowNumber){

    patchIdentifier patch;

    patch.rowIndex = rowNumber;
    patch.numberOfImage = rowNumber / searchWinNumPatch;

    int remaining = rowNumber % searchWinNumPatch;

    int rowStartInSearchWin = (remaining / dbPatchInRow) * parameter.samplingRate;
    int colStartInSearchWin = (remaining % dbPatchInRow) * parameter.samplingRate;

    patch.rowRange.start = rowStartInSearchWin + searchWinRangeRow.start;
    patch.rowRange.end = patch.rowRange.start + indexPatchSize;

    patch.colRange.start = colStartInSearchWin + searchWinRangeCol.start;
    patch.colRange.end = patch.colRange.start + indexPatchSize;

    return patch;

}

void SearchWin::foundKNNInSearchWin(std::vector<patchIdentifier>& listOfKNN, std::vector<unsigned short>& testPatchIn1D_p, int& numberOfCanToFound){

    std::vector<unsigned short> testPatchIn1D(testPatchIn1D_p.begin(), testPatchIn1D_p.end());

    std::vector<std::vector<int> > indices;
    std::vector<std::vector<float> > dists;

    flann::Matrix<unsigned short> query(new unsigned short[testPatchIn1D.size()], 1, testPatchIn1D.size());

    for ( int i = 0; i < (int)testPatchIn1D.size(); i++){
        query.ptr()[i] = (unsigned short)testPatchIn1D.at(i);
    }

    flann::SearchParams param;
    param.checks = parameter.numberOfLeafChecked;
    param.cores = 8;

    kdtree->knnSearch(query, indices, dists, numberOfCanToFound, param);

    for ( int i = 0 ; i < numberOfCanToFound; i++){
        patchIdentifier patch;
        patch = rowNumber2Identifier(indices.at(0).at(i));
        patch.distance = dists.at(0).at(i);

        listOfKNN.push_back(patch);
    }

}

void SearchWin::foundKNNInSearchWinByBruteforce(std::vector<IntegralImage*> images, std::vector<patchIdentifier>& listOfKNN, cv::Mat testPatch, int numberOfCanToFound){

    for ( int imageIndex = 0; imageIndex < (int)images.size(); imageIndex++){

        double imageMinDis = 9999999999.99;
        int imageMinRow = -1;
        int imageMinCol = -1;

        for ( int currentPatchRowStart = searchWinRangeRow.start; currentPatchRowStart + indexPatchSize <= searchWinRangeRow.end; currentPatchRowStart += parameter.samplingRate){
            for ( int currentPatchColStart = searchWinRangeCol.start; currentPatchColStart + indexPatchSize <= searchWinRangeCol.end; currentPatchColStart += parameter.samplingRate){

                cv::Mat tempPatch;

                int currentPatchRowEnd = currentPatchRowStart + indexPatchSize;
                int currentPatchColEnd = currentPatchColStart + indexPatchSize;

                cv::Range patchRangeRow(currentPatchRowStart, currentPatchRowEnd);
                cv::Range patchRangeCol(currentPatchColStart, currentPatchColEnd);

                images.at(imageIndex)->image(patchRangeRow, patchRangeCol).copyTo(tempPatch);
                double localDistance = patchDistance(tempPatch, testPatch);

                if (localDistance < imageMinDis){
                    imageMinDis = localDistance;
                    imageMinRow = currentPatchRowStart;
                    imageMinCol = currentPatchColStart;
                }


            }
        }


        patchIdentifier imageMin;
        imageMin.distance = imageMinDis;
        imageMin.colRange.start = imageMinCol;
        imageMin.colRange.end = imageMinCol + indexPatchSize;
        imageMin.rowRange.start = imageMinRow;
        imageMin.rowRange.end = imageMinRow + indexPatchSize;
        imageMin.numberOfImage = imageIndex;

        listOfKNN.push_back(imageMin);
    }
    //sort the list
    for ( int i = 0; i < (int)listOfKNN.size() - 1; i++){
        for ( int j = 0; j < (int)listOfKNN.size() - 1; j++){
            if ( listOfKNN.at(j).distance > listOfKNN.at(j + 1).distance){
                //listOfKNN->swap(j, j+1);
                std::swap(listOfKNN[j], listOfKNN[j+1]);
            }
        }
    }

    // only keep the top K
    while ((int)listOfKNN.size() > numberOfCanToFound){
        //listOfKNN->removeLast();
        listOfKNN.pop_back();
    }

}
