#include "FaceSynthesisManager.h"


FaceSynthesisManager::FaceSynthesisManager(void) {
    pathChecked = false;
    sysnthesisDirection = DIRECTION_NOT_SET;

    photoImages = new IntegralImages();
    sketchImages = new IntegralImages();

    synthesizedImgReady = false;
    Sketch = NULL;

    QObject::connect(&trainingThread, &FSTrainingThread::trainingDone, this, &FaceSynthesisManager::FSTrainingThreadDoneSlot);
}

QString FaceSynthesisManager::checkTraningData(QStringList photoList, QStringList sketchList){
    QString errorString = "";

    if (photoList.size() != sketchList.size()){
        return "The number of photo images and sketch images are not the same.\n  Number of photo images: " + QString::number(photoList.size()) + "\n" +
                "  Number of sketch images: " + QString::number(sketchList.size()) + "\n";
    }

    if (photoList.size() == 0 || sketchList.size() == 0){
        return "The training data is empty";
    }

    for (int i = 0; i < photoList.size(); ++i){
        if (photoList.at(i).compare(sketchList.at(i)) != 0){

            return "File name of photo images and sketch images not match. \n  at " +
                    QString::number(i) + " photo image: " + photoList.at(i) + " sketch image: " + sketchList.at(i);
        }
    }
    return errorString;
}

QString FaceSynthesisManager::setPath(QString photoPath_p, QString sketchPath_p){
    pathChecked = false;
    QString errorString = "";

    QDir photoDir (photoPath_p);
    QDir sketchDir (sketchPath_p);

    if (!photoDir.exists()){
        errorString += "Data Path not contain Photo directory\n";
        return errorString;
    }
    if (!sketchDir.exists()){
        errorString += "Data Path not contain Sketch directory\n";
        return errorString;
    }

    QStringList photoList_Full = photoDir.entryList();
    QStringList sketchList_Full = sketchDir.entryList();

    QStringList photoList = photoList_Full.filter(".jpg");
    QStringList sketchList = sketchList_Full.filter(".jpg");

    errorString += checkTraningData(photoList, sketchList);

    if (errorString.isEmpty()){

        photoPath = photoPath_p;
        sketchPath = sketchPath_p;

        checkedPhotoList = photoList;
        checkedSketchList = sketchList;

        pathChecked = true;
    }
    return errorString;
}

void FaceSynthesisManager::setDirectionPhotoToSketch(){
    sysnthesisDirection = DIRECTION_PHOTO_TO_SKETCH;
}
void FaceSynthesisManager::setDirectionSketchToPhoto(){
    sysnthesisDirection = DIRECTION_SKETCH_TO_PHOTO;
}

cv::Mat FaceSynthesisManager::getSysthesizedImg(){
    return synthesizedImg;
}

void FaceSynthesisManager::setParameter(Parameter parameter_p){
    parameter = parameter_p;
}

void FaceSynthesisManager::train() {
    if (!pathChecked){
        commonTool.log("Please check the data path\n");
        return;
    }
    if (sysnthesisDirection == DIRECTION_NOT_SET){
        commonTool.log("Please check the setting. Direction of Synthesis is not set.\n");
        return;
    }

    bool photoKDTree = false;
    bool sketchKDTree = false;

    if (sysnthesisDirection == DIRECTION_PHOTO_TO_SKETCH){
        commonTool.log("Face synthesize direction: photo -> sketch.");
        photoKDTree = true;
    } else if (sysnthesisDirection == DIRECTION_SKETCH_TO_PHOTO){
        commonTool.log("Face synthesize direction: sketch -> photo.");
        sketchKDTree = true;
    }
    trainingThread.setThread(photoPath,sketchPath,checkedPhotoList,checkedSketchList,photoKDTree,sketchKDTree,parameter,photoImages,sketchImages);
    commonTool.log("before trainingThread.start()");
    trainingThread.start();
    commonTool.log("after trainingThread.start()");
}

void FaceSynthesisManager::FSTrainingThreadDoneSlot(){
    trainedImage = photoImages->size();

    commonTool.log(QString("FSTrainingThreadDoneSlot()"));
    emit FSTrainingDone();
}

int FaceSynthesisManager::getNumberOfTrainedImage(){

    return trainedImage;
}

QString FaceSynthesisManager::checkTrained(){
    if (photoImages->size() != sketchImages->size()){
        return "The number of trained photo and sketch are different";
    }

    if (photoImages->size() == 0 || sketchImages->size() == 0){
        return "No training data is empty";
    }
    return "";
}

void FaceSynthesisManager::run() {
    synthesizedImgs.clear();
    for (int i=0;i<(int)inputImgs.size();i++) {
        synthesize(inputImgs.at(i));
        synthesizedImgs.push_back(synthesizedImg);
    }
    emit FSDone();
}

void FaceSynthesisManager::synthesize(std::vector<cv::Mat> inputImgs) {
    commonTool.log(QString("synthesize(std::vector<cv::Mat> inputImgs)"));
    this->inputImgs = inputImgs;
    this->start();
}

void FaceSynthesisManager::synthesize(cv::Mat inputImg) {
    commonTool.log(QString("synthesize(cv::Mat inputImg)"));

    synthesizedImgReady = false;

    testImage.load(inputImg);

    QString errorString;
    errorString += checkTrained();

    if (!errorString.isEmpty()){
        commonTool.log(errorString);
        return;
    }

    //clean the previour round
    if (Sketch != NULL){
        delete Sketch;
    }
    for ( int i = 0; i < outputCandidatePatchSet.size(); i++){ // size is zero for the first round
        CandidatePatchSet* currentCandidatePatchSet = outputCandidatePatchSet.at(i);  //for each patch
        for ( int j = 0; j < currentCandidatePatchSet->candidatePatchSet.size(); j++){
            CandidatePatch* currentCandidatePatch = currentCandidatePatchSet->candidatePatchSet.at(j);  //for each can patch set
            delete currentCandidatePatch;
        }
        currentCandidatePatchSet->candidatePatchSet.clear();
        delete currentCandidatePatchSet;
    }
    outputCandidatePatchSet.clear();


    //Start to load the image
    Sketch = new MRFQP(parameter);
    if (parameter.mode == _CONTROLLER_P2S_){
        Sketch->FindKNN(photoImages, sketchImages, testImage);
    } else if (parameter.mode == _CONTROLLER_S2P_){
        Sketch->FindKNN(sketchImages, photoImages, testImage);
    }

    Sketch->energyMinimize();


    Sketch->FinalResult.convertTo(Sketch->FinalResult, CV_8UC1);

    synthesizedImg = Sketch->FinalResult.clone();

    int totalNumberOfPatch = Sketch->NumHeiLower * Sketch->NumWidLower;

    commonTool.log(QString("Number of patches: %1.").arg(totalNumberOfPatch));

    for (int i = 0; i < totalNumberOfPatch; i++){

        CandidatePatchSet *patchSet = new CandidatePatchSet;

        for (int j = 0; j < Sketch->NumOfCan; j++){

            CandidatePatch* candidatePatch = new CandidatePatch;
            candidatePatch->identifier = Sketch->ImagePatchLower.at(i).PatchData.at(j).identifier;
            Sketch->ImagePatchLower.at(i).PatchData.at(j).PhotoPatchData8U.copyTo(candidatePatch->sourcePatchData);
            Sketch->ImagePatchLower.at(i).PatchData.at(j).SketchPatchData8U.copyTo(candidatePatch->targetPatchData);

            candidatePatch->weight = Sketch->getResultWeight().at<float>(i*Sketch->NumOfCan + j);

            patchSet->candidatePatchSet.append(candidatePatch);
        }

        //sorting
        for ( int i = 0; i < patchSet->candidatePatchSet.size() - 1; i++){
            for ( int j = 0; j < patchSet->candidatePatchSet.size() - 1; j++){
                if ( patchSet->candidatePatchSet.at(j)->weight < patchSet->candidatePatchSet.at(j+1)->weight){
                    patchSet->candidatePatchSet.swap(j, j+1);
                }
            }
        }
        outputCandidatePatchSet.append(patchSet);
    }

    synthesizedImgReady = true;
}

void FaceSynthesisManager::clean()
{
    checkedPhotoList.clear();
    checkedSketchList.clear();

    pathChecked = false;

    photoImages->clean();
    sketchImages->clean();
}

cv::Mat FaceSynthesisManager::getPatchFromInputImage(QRect rect){
    cv::Mat patch = testImage.getPatchFromImage(rect);
    return patch;
}

cv::Mat FaceSynthesisManager::getCanPatchOrgImg(int type, int index){
    cv::Mat output;
    if ((type == _CPGV_SOURCE_ && sysnthesisDirection == DIRECTION_PHOTO_TO_SKETCH) || (type == _CPGV_TARGET_ && sysnthesisDirection == DIRECTION_SKETCH_TO_PHOTO)){
        output = photoImages->getImage(index);
    } else {
        output = sketchImages->getImage(index);
    }
    return output;
}

QString FaceSynthesisManager::getCanPatchOrgImgPath(int type, int index){
    QString output;
    if ((type == _CPGV_SOURCE_ && sysnthesisDirection == DIRECTION_PHOTO_TO_SKETCH) || (type == _CPGV_TARGET_ && sysnthesisDirection == DIRECTION_SKETCH_TO_PHOTO)){
        output = photoImages->getImagePath(index);
    } else{
        output = sketchImages->getImagePath(index);
    }
    return output;
}

CandidatePatchSet* FaceSynthesisManager::getCandidatePatchSet(QPointF patchCoordinate){
    return outputCandidatePatchSet.at(Sketch->NumWidLower * patchCoordinate.ry() + patchCoordinate.rx());
}
