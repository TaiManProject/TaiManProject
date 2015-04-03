#include "IntegralImages.h"


IntegralImages::IntegralImages(){
    imageRow = 250;
    imageCol = 200;
    //kdtreePatchSizeLower = 10;
    //kdtreePatchSizeUpper = 20;

    //This vector should build firstm since it will run clean first
    //searchWinsLower = new std::vector<SearchWin*>();
    //searchWinsUpper = new std::vector<SearchWin*>();
}
/*
IntegralImages::IntegralImages(QString path, QStringList fileNameList, QString delimiter, bool treeNeeded, int patchSizeLower_p, int patchSizeUpper_p){
    imageRow = 250;
    imageCol = 200;
    kdtreePatchSizeLower = patchSizeLower_p;
    kdtreePatchSizeUpper = patchSizeUpper_p;
    load(path, fileNameList, delimiter, treeNeeded);
}


void IntegralImages::load(QString path, QStringList fileNameList, QString delimiter, bool treeNeeded, int imageRow_p, int imageCol_p, int patchSizeLower_p, int patchSizeUpper_p){
    imageRow = imageRow_p;
    imageCol = imageCol_p;
    kdtreePatchSizeLower = patchSizeLower_p;
    kdtreePatchSizeUpper = patchSizeUpper_p;
    load(path, fileNameList, delimiter, treeNeeded);
}*/

void IntegralImages::load(QString path, QStringList fileNameList, bool treeNeeded, Parameter parameter_p){

    clean(); // clean the last round

    parameter = parameter_p;

    //compute the parameter related variables
    patchStepUpper = parameter.patchSizeUpper - parameter.OverlapSizeUpper;
    numIndexPatchColUpper = (imageCol - parameter.patchSizeUpper)/patchStepUpper + 1;
    numIndexPatchRowUpper = (imageRow - parameter.patchSizeUpper)/patchStepUpper + 1;

    patchStepLower = parameter.patchSizeLower - parameter.OverlapSizeLower;
    numIndexPatchColLower = (imageCol - parameter.patchSizeLower)/patchStepLower + 1;
    numIndexPatchRowLower = (imageRow - parameter.patchSizeLower)/patchStepLower + 1;

    //Audrey
    //Load images that needed to be synthesized later
    commonTool.log(QString("Loading %1 images from path %2.").arg(fileNameList.size()).arg(qPrintable(path)));
    for (int i = 0; i < fileNameList.size(); i++) {
        QString imagePath;
        imagePath = path + fileNameList.at(i);
        commonTool.log(QString("Loading %1.").arg(imagePath));
        IntegralImage* const image = new IntegralImage(imagePath);
        integralImages.push_back(image);
    }

    //Audrey TODO
    //Image need to be 200*250, same size with the first loaded image(hardcode!),  see the checkTrainingData().
    imageRow = integralImages[0]->getImageRow();
    imageCol = integralImages[0]->getImageCol();

    if (treeNeeded == true){
        commonTool.log("Building upper face synthesize tree.");
        buildKDTree(_INTEGRALIMAGES_UPPER_);
        commonTool.log("Building lower face synthesize tree.");
        buildKDTree(_INTEGRALIMAGES_LOWER_);
    }
}

int IntegralImages::size(){
    return integralImages.size();
}

void IntegralImages::clean(){
    for (size_t i = 0 ; i < integralImages.size(); i++){
        delete integralImages.at(i);
    }
    integralImages.clear();

    for (size_t i = 0; i < searchWinsLower.size(); i++){
        delete searchWinsLower.at(i);
    }
    searchWinsLower.clear();

    for (size_t i = 0; i < searchWinsUpper.size(); i++){
        delete searchWinsUpper.at(i);
    }
    searchWinsUpper.clear();
}


/*
cv::Mat vectorToMat(vector<cv::Mat> inputVector, int col){
    cv::Mat outputMat;
    outputMat.create(inputVector.size(), col, CV_32F);
    for ( int i = 0; i < inputVector.size(); i++){
        cv::Mat patch; //copy the patch since isContinue
        cv::Mat patchLower;
        cv::Mat patch1D;

        inputVector.at(i).copyTo(patch);
        patch1D = patch.reshape(1, 1);

        int temp1 = patch1D.type();
        int temp2 = outputMat.type();

        patch1D.copyTo(outputMat(cv::Range(i, i + 1), cv::Range(0, col)));

    }
    return outputMat;
}
*/

void IntegralImages::buildKDTree(int level){
//    int kdtreePatchSize;
    int numIndexPatchCol;
    int numIndexPatchRow;
//    int patchStep;
//    int patchSize;
//    int searchWidth;
    std::vector<SearchWin*>* searchWins;
    //cout << "Start build KD Tree in InteralImages" << endl;

    //Audrey
    //Check the building level value 0? 1?
    if (level == _INTEGRALIMAGES_LOWER_) {
//        patchSize = parameter.patchSizeLower;           // 10
        numIndexPatchCol = numIndexPatchColLower;
        numIndexPatchRow = numIndexPatchRowLower;
//        patchStep = patchStepLower;                     // 10 - 5 = 5
//        searchWidth = parameter.SearchWidthLower;       // 8
        searchWins = &searchWinsLower;
    } else {
//        patchSize = parameter.patchSizeUpper;
        numIndexPatchCol = numIndexPatchColUpper;
        numIndexPatchRow = numIndexPatchRowUpper;
//        patchStep = patchStepUpper;
//        searchWidth = parameter.SearchWidthUpper;
        searchWins = &searchWinsUpper;
    }

    //Audrey
    //Build single KDTree, defalut KNN mode is _KNN_KDTREE_.
    if (parameter.foundKNNMode == _KNN_KDTREE_){
        if (parameter.modeOfKDTree == _KDTREE_MULTI_){
            // for each index patch
            for ( int i = 0; i < numIndexPatchRow; i++ ){
                for ( int j = 0; j < numIndexPatchCol; j++){
                    SearchWin* tempSearchWin = new SearchWin(parameter, level, i, j);
                    tempSearchWin->buildMatAndTree(integralImages);
                    searchWins->push_back(tempSearchWin);
                }
            }
        } else if(parameter.modeOfKDTree == _KDTREE_SINGLE_){
            //Default choice
            SearchWin* tempSearchWin = new SearchWin(parameter, level);
            tempSearchWin->buildMatAndTree(integralImages);
            searchWins->push_back(tempSearchWin);
        }
    } else if (parameter.foundKNNMode == _KNN_BRUTEFORCE_){
        for ( int i = 0; i < numIndexPatchRow; i++ ){
            for ( int j = 0; j < numIndexPatchCol; j++){
                SearchWin* tempSearchWin = new SearchWin(parameter, level, i, j);
                searchWins->push_back(tempSearchWin);
            }
        }
    } else if (parameter.foundKNNMode == _KNN_RANDOM_){
        //TODO: may need do some setting
    }
}

int IntegralImages::foundKNN(std::vector<patchIdentifier>& listOfKNN, cv::Mat testPatch, int level, int numberOfCanToFound, int indexOfIndexPatch, int rangRowStart, int rangColStart){


    //std::vector<int> index(numberOfCanToFound);
    //std::vector<float> dist(numberOfCanToFound);
    std::vector<SearchWin*>* searchWins;
    //vector<unsigned short>* testPatchIn1D = new vector<unsigned short>();
    std::vector<unsigned short> testPatchIn1D;

    //change cv::Mat to 1D vector
    buildMat2Vector(testPatch, testPatchIn1D);


    if (level == _INTEGRALIMAGES_LOWER_){
        searchWins = &searchWinsLower;
    } else if (level == _INTEGRALIMAGES_UPPER_){
        searchWins = &searchWinsUpper;
    }

    if (parameter.foundKNNMode == _KNN_KDTREE_){
        //for KDTree Mode
        //check need to add X and Y dim or not
        if (parameter.additionDim){
            testPatchIn1D.insert(testPatchIn1D.begin(), rangRowStart * parameter.additionDimWeight); //put y at first
            testPatchIn1D.insert(testPatchIn1D.begin(), rangColStart * parameter.additionDimWeight); //than put x at first, at the end x than y
        }

        if (parameter.modeOfKDTree == _KDTREE_MULTI_) {
            searchWins->at(indexOfIndexPatch)->foundKNNInSearchWin(listOfKNN, testPatchIn1D, numberOfCanToFound);
        } else if (parameter.modeOfKDTree == _KDTREE_SINGLE_) {
            searchWins->at(0)->foundKNNInSearchWin(listOfKNN, testPatchIn1D, numberOfCanToFound);
        }
    } else if (parameter.foundKNNMode == _KNN_BRUTEFORCE_) {
        searchWins->at(indexOfIndexPatch)->foundKNNInSearchWinByBruteforce(integralImages, listOfKNN, testPatch, numberOfCanToFound);
    }

    return 0;
}

cv::Mat IntegralImages::getImage(int index){
    return integralImages.at(index)->image;
}

QString IntegralImages::getImagePath(int index){
    return integralImages.at(index)->filePath;
}

