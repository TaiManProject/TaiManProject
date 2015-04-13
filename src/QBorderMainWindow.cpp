#include "QBorderMainWindow.h"
#include "FaceMatchingManager.h"
#include "FaceAlignment.h"

QBorderMainWindow::QBorderMainWindow(QObject *parent):QObject(parent) {
    commonTool.log(QString("QBorderMainWindow::QBorderMainWindow"));
    //    this->parameter = parameter;
    //init parameter
    this->parameter.initialized = false;
    this->parameter.mode = _CONTROLLER_S2P_;
    this->parameter.dataPath = dataPath;
    this->parameter.numberOfKDTree = 4;
    this->parameter.modeOfKDTree = _KDTREE_SINGLE_;
    this->parameter.numberOfLeafChecked = 1024;
    this->parameter.additionDim = true;
    this->parameter.additionDimWeight = 99;
    this->parameter.foundKNNMode = _KNN_KDTREE_;
    this->parameter.samplingRate = 2;
    this->parameter.newPatch = false;
    this->parameter.numOfGoodCan = 10;
    this->parameter.numOfRandomCan = 0;
    this->parameter.SearchWidthUpper = 5;       // 5 pixels for upper, lower, left and right search range for each window
    this->parameter.SearchWidthLower = 8;
    this->parameter.patchSizeUpper = 20;
    this->parameter.patchSizeLower = 10;
    this->parameter.OverlapSizeUpper = 5;
    this->parameter.OverlapSizeLower = 5;
    this->parameter.imageCol = 200;
    this->parameter.imageRol = 250;

    //Audrey
    //In multiple face matching mode
    faceMatchingManager.enableMultiLevel = true;
    commonTool.setModeLoad();
    //        commonTool.setModeTrain();

    //training for face verification
    //add suspects to training data?
    if (commonTool.loadMode == _TRAIN_MODE_) {
        //Audrey
        //1.1 Train face recognition
        commonTool.setModeTrain();
        std::vector<cv::Mat> additionalImages;
        QString faceRecogDBPath = dataPath + QString::fromStdString(FaceMatchingManager::trainingDirectory);
        faceMatchingManager.train(faceRecogDBPath, additionalImages, openCVMutex);
    } else if (commonTool.loadMode == _LOAD_MODE_) {
        //Audrey
        //1.2 Load trained model and related parameters
        commonTool.setModeLoad();
        std::vector<cv::Mat> additionalImages;
        faceMatchingManager.train(csvLoadPath, additionalImages, openCVMutex);
    }


    //Audrey
    //2. Train face synthesis
    trainFaceSynthesize();

    // Audrey
    loadWantedPersons();

    //    ui.setupUi(this);
    //    connect(ui.inputPersonLabel, &ClickableQLabel::clicked, this, &QBorderMainWindow::onClickedInputPersonLabel);

    connect(&faceSynthesisManager, &FaceSynthesisManager::FSTrainingDone, this, &QBorderMainWindow::trainFaceSynthesizeCompleted);
    connect(&faceSynthesisManager, &FaceSynthesisManager::FSDone, this, &QBorderMainWindow::faceSynthesizeCompleted);

    matchedIndex = -1;
    partialMatchedIndex = -1;
    inputPersonSynthesizedReady = false;
    isAbleToSynthesizeInputPerson = false;

    //    loadWantedPersons();
    //    updateUI();
}



QStringList QBorderMainWindow::getWantedPersonQStringListFromFile(QString filePath) {
    QStringList wantedPersonList;
    commonTool.log(QString("Importing wanted persons from file %1").arg(filePath));
    QFile wantedPersonFile(filePath);
    if (wantedPersonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!wantedPersonFile.atEnd()) {
            QString name = wantedPersonFile.readLine().trimmed();
            wantedPersonList.append(name);
        }
        wantedPersonFile.close();
    }
    commonTool.log(QString("Imported %1 wanted persons.").arg(wantedPersonList.size()));
    return wantedPersonList;
}

void QBorderMainWindow::loadWantedPersons() {
    QStringList wantedPersonList = getWantedPersonQStringListFromFile(parameter.dataPath + "completeAR/wanted.txt");
    QString wantedPersonFolder = parameter.dataPath + "completeAR/";
    FaceAlignment fa(parameter.dataPath.toStdString(), faceAlignMutex);
    for(int i = 0; i < wantedPersonList.size(); i++) {
        cv::Mat grayWantedPerson;
        {
            cv::Mat wantedPerson = cv::imread((wantedPersonFolder+wantedPersonList.at(i)).toStdString());
            cv::cvtColor(wantedPerson, grayWantedPerson,CV_RGB2GRAY);
        }
        cv::Mat alignedFace;
        std::vector<cv::Point2d> landmarkPoints;
        cv::Point2d centerOfLandmarks; //not used
        fa.alignStasm4(grayWantedPerson, alignedFace, landmarkPoints, centerOfLandmarks);

        if (alignedFace.empty()) {
            commonTool.log("aligned face is empty ...");
            if (grayWantedPerson.cols == 200 && grayWantedPerson.rows == 250){
                alignedFace = grayWantedPerson;
            } else {
                commonTool.log("Error. Cannot align face");
                continue;
            }
        }
        wantedPersonsLandmarkPointsList.push_back(landmarkPoints);
        wantedPersons.push_back(alignedFace);
        wantedPersonFileNames.push_back(wantedPersonList.at(i));
        cv::Mat emptySketch;
        wantedPersonsSynthesized.push_back(emptySketch);
    }
    commonTool.log(QString::number(wantedPersonsSynthesized.size()));

    //

    QStringList wantedPersonSketchList = getWantedPersonQStringListFromFile(parameter.dataPath + "completeAR/wantedSketch.txt");

    for(int i = 0; i < wantedPersonSketchList.size(); i++) {
        cv::Mat grayWantedPerson;
        {
            cv::Mat wantedPerson = cv::imread((wantedPersonFolder+wantedPersonSketchList.at(i)).toStdString());
            cv::cvtColor(wantedPerson, grayWantedPerson,CV_RGB2GRAY);

        }
        cv::Mat alignedSketch;
        std::vector<cv::Point2d> landmarkPoints;
        cv::Point2d centerOfLandmarks; //not used
        fa.alignStasm4(grayWantedPerson, alignedSketch, landmarkPoints, centerOfLandmarks);

        if (alignedSketch.empty()) {
            commonTool.log("aligned face is empty ...");
            if (grayWantedPerson.cols == 200 && grayWantedPerson.rows == 250){
                alignedSketch = grayWantedPerson;
            } else {
                commonTool.log("Error. Cannot align face");
                continue;
            }
        }

        wantedPersonsLandmarkPointsList.push_back(landmarkPoints);
        wantedPersons.push_back(alignedSketch);
        wantedPersonFileNames.push_back(wantedPersonSketchList.at(i));
        wantedPersonsSynthesized.push_back(alignedSketch);
    }
}

//void QBorderMainWindow::onClickedInputPersonLabel(ClickableQLabel* label) {

//    /*QString selectedFile = QFileDialog::getOpenFileName(this,
//                                                        tr("Select the image file"), dataPath+"completeAR/",
//                                                        tr("Image Files (*.png *.jpg *.bmp)"));
//    */
//    //Audrey
//    //Change the input image path and folder for show
//    commonTool.log("onClickedInputPersonLabel.1");
//    QString selectedFile = QFileDialog::getOpenFileName(this,
//                                                        tr("Select the input image"), dataPath+"InputImage/",
//                                                        tr("Image Files (*.png *.jpg *.bmp)"));
//    commonTool.log("onClickedInputPersonLabel.2");
//    if (selectedFile.size() != 0) {
//        cv::Mat selectedGrayImg;
//        {
//            cv::Mat selectedImg = cv::imread(selectedFile.toStdString());
//            cv::cvtColor(selectedImg, selectedGrayImg,CV_RGB2GRAY);
//            inputPersonSynthesizedReady = false; //xlzhu
//        }
////        cv::imshow("haha",selectedGrayImg);
//        commonTool.log("onClickedInputPersonLabel.3");
//        cv::Mat alignedFace;
//        std::vector<cv::Point2d> landmarkPoints;
//        FaceAlignment fa(parameter.dataPath.toStdString(), faceAlignMutex);
//        cv::Point2d centerOfLandmarks; //not used
//        fa.alignStasm4(selectedGrayImg, alignedFace, landmarkPoints, centerOfLandmarks);
//        if (alignedFace.empty()){
//            commonTool.log("Error importing input person ...");
//        }
//        inputPerson = alignedFace;
//        QFileInfo f(selectedFile);
//        inputPersonFileName = f.fileName();
//        inputPersonLandmarkPoints = landmarkPoints;
//        commonTool.log("QBorderMainWindow::face Alignment Completed!");
//        if (inputPersonFileName.contains("Mp", Qt::CaseSensitive) && isAbleToSynthesizeInputPerson) {
//            faceSynthesisManager.synthesize(inputPerson);
//            inputPersonSynthesized = faceSynthesisManager.FaceSynthesisManager::getSysthesizedImg();
//            inputPersonSynthesizedReady = true;
//            commonTool.log("QBorderMainWindow::Input Person Synthesized!");
//        }

//        performFaceVerification();
//    }
//}

//void QBorderMainWindow::updateUI() {
//    commonTool.log("updateUI.begin");

//    //update the matched person
//    commonTool.log(QString("update the matched person. size: %1").arg(wantedPersons.size()));
//    if(matchedIndex!=-1) {
//        cv::imwrite("../data/person.jpg",wantedPersons.at(matchedIndex));
//        persons.at(matchedIndex).setPhotoPath("../data/person.jpg");

//    } else if (partialMatchedIndex!=-1) {
//        //        cv::imwrite("/home/niesh/faceSketchSynthesize/data/person.jpg",wantedPersons.at(partialMatchedIndex));
//        cv::imwrite("../data/person.jpg",wantedPersons.at(partialMatchedIndex));
//    } else {
//        //        cv::imwrite("/home/niesh/faceSketchSynthesize/data/person.jpg",cv::Mat());
//        cv::imwrite("../data/person.jpg",cv::Mat());
//    }
//    commonTool.log("updateUI.end");
//}

void QBorderMainWindow::trainFaceSynthesize() {
    commonTool.log("QBorderMainWindow::trainFaceSynthesis()");
    QString photoPath = parameter.dataPath + PHOTO_DIR;
    QString sketchPath = parameter.dataPath + SKETCH_DIR;
    faceSynthesisManager.setParameter(parameter);
    faceSynthesisManager.setPath(photoPath, sketchPath);
    faceSynthesisManager.setDirectionSketchToPhoto(); // Audrey TODO HardCode!
    faceSynthesisManager.train();
}

void QBorderMainWindow::faceSynthesizeCompleted() {
    int ii=0;
    //    commonTool.log("22222233355"+QString::number(wantedPersonsSynthesized.size()));
    for (int i=0;i<(int)wantedPersonsSynthesized.size();i++) {
        if (!wantedPersonsSynthesized.at(i).empty()) {
            cv::Mat img = faceSynthesisManager.synthesizedImgs.at(ii++);
            wantedPersonsSynthesized.at(i) = img;
        }
    }
    //    commonTool.log("22222233355"+QString::number(wantedPersonsSynthesized.size()));
    commonTool.log("QBorderMainWindow::faceSynthesizeCompleted() --- Done!!!");

    //    performFaceVerification();
}



void QBorderMainWindow::trainFaceSynthesizeCompleted() {
    //push back sketches
    std::vector<cv::Mat> sketches;
    //    commonTool.log("222222333444"+QString::number(wantedPersonsSynthesized.size()));
    for (int i=0;i<(int)wantedPersonsSynthesized.size();i++) {
        if (!wantedPersonsSynthesized.at(i).empty()) {
            sketches.push_back(wantedPersonsSynthesized.at(i));
        }
    }
    //Audrey
    //Start to synthesize sketches
    faceSynthesisManager.synthesize(sketches);
    isAbleToSynthesizeInputPerson = true;
    commonTool.log("QBorderMainWindow::trainFaceSynthesizeCompleted() --- DONE!" + QString::number(faceSynthesisManager.synthesizedImgs.size()));
}


void QBorderMainWindow::performFaceVerification(int isSketch) {
    //verify each input face with the wantedPersons.
    commonTool.log("Performing face verification");
    //    commonTool.log("222222333"+QString::number(wantedPersonsSynthesized.size()));
    matchedIndex = -1;
    partialMatchedIndex = -1;
    if (!inputPerson.empty()) {
        faceVerificationResult.clear();
        partialFaceVerificationResult.clear();
        sortedPartialFaceVerificationResult.clear();
        sortedFaceVerificationResult.clear();
        faceVerificationResultRaw.clear();
        persons.clear();
        for (int i=0;i<(int)wantedPersons.size();i++) {
            std::vector<double>  similarities;
            //            commonTool.log("222222333"+QString::number(wantedPersonsSynthesized.size()));
            /*
            if (!wantedPersonsSynthesized.at(i).empty()) {
                similarities = faceMatchingManager.getNormalizedSimilarity(wantedPersonsSynthesized.at(i), inputPerson);
                commonTool.log(QString("%1 -> %2*").arg(i).arg(similarities.at(0)));
            } else {
                similarities = faceMatchingManager.getNormalizedSimilarity(wantedPersons.at(i), inputPerson);
                commonTool.log(QString("%1 -> %2").arg(i).arg(similarities.at(0)));
            }
            */

            //Audrey
            //commonTool.log("QBorderMainWindow::faceInputSynthesizeCompleted()");
            //synthesize the input face now
            if (isSketch == 1 && inputPersonSynthesizedReady) {
                if (!wantedPersonsSynthesized.at(i).empty()) {
                    //                    commonTool.log("1111111");
                    similarities = faceMatchingManager.getNormalizedSimilarity(wantedPersonsSynthesized.at(i), inputPersonSynthesized);
                    //                    commonTool.log(QString("%1 -> %2*").arg(i).arg(similarities.at(0)));
                } else {
                    //                    commonTool.log("2222222"+QString::number(wantedPersonsSynthesized.size()) + "EE" + QString::number(i));
                    similarities = faceMatchingManager.getNormalizedSimilarity(wantedPersons.at(i), inputPersonSynthesized);
                    //                    commonTool.log(QString("%1 -> %2").arg(i).arg(similarities.at(0)));
                }
            } else if (!wantedPersonsSynthesized.at(i).empty()) {
                commonTool.log("3333333");
                similarities = faceMatchingManager.getNormalizedSimilarity(wantedPersonsSynthesized.at(i), inputPerson);
                commonTool.log(QString("%1 -> %2").arg(i).arg(similarities.at(0)));
            } else {
                commonTool.log("4444444");
                similarities = faceMatchingManager.getNormalizedSimilarity(wantedPersons.at(i), inputPerson);
                commonTool.log(QString("%1 -> %2").arg(i).arg(similarities.at(0)));
            }
            commonTool.log(QString("numberOfSimilarityScores = %1").arg(similarities.size()));

            Person person;
            IntDouble full;
            full.i = i;
            full.confidence = 100.0;
            full.d = similarities.at(0);
            faceVerificationResult.push_back(full);
            faceVerificationResultRaw.push_back(similarities);
            person.setFull(full);
            //partial score ...
            //
            commonTool.log("clac partial score.");
            IntDouble partial;
            partial.i = i;
            partial.confidence = 0.0;
            partial.d = 0.0;
            int c=0;
            if (similarities.at(0) > faceMatchingManager.alarmThreshold) {
                c++;
                partial.confidence += 100.0;
                partial.d += similarities.at(0);
            }
            if (c==0) {
                if (similarities.at(1) > faceMatchingManager.alarmThreshold) {
                    c++;
                    partial.confidence += 50.0;
                    partial.d += similarities.at(1);
                }
                if (similarities.at(2) > faceMatchingManager.alarmThreshold) {
                    c++;
                    partial.confidence += 50.0;
                    partial.d += similarities.at(2);
                } if (c==0) {
                    if (similarities.at(3) > faceMatchingManager.alarmThreshold) {
                        c++;
                        partial.confidence += 25.0;
                        partial.d += similarities.at(3);
                    }
                    if (similarities.at(4) > faceMatchingManager.alarmThreshold) {
                        c++;
                        partial.confidence += 25.0;
                        partial.d += similarities.at(4);
                    }
                    if (similarities.at(5) > faceMatchingManager.alarmThreshold) {
                        c++;
                        partial.confidence += 25.0;
                        partial.d += similarities.at(5);
                    }
                    if (similarities.at(6) > faceMatchingManager.alarmThreshold) {
                        c++;
                        partial.confidence += 25.0;
                        partial.d += similarities.at(6);
                    }
                    if (c==0) {
                        if (similarities.at(7) > faceMatchingManager.alarmThreshold) {
                            c++;
                            partial.confidence += 100.0/8.0;
                            partial.d += similarities.at(7);
                        }
                        if (similarities.at(8) > faceMatchingManager.alarmThreshold) {
                            c++;
                            partial.confidence += 100.0/8.0;
                            partial.d += similarities.at(8);
                        }
                        if (similarities.at(9) > faceMatchingManager.alarmThreshold) {
                            c++;
                            partial.confidence += 100.0/8.0;
                            partial.d += similarities.at(9);
                        }
                        if (similarities.at(10) > faceMatchingManager.alarmThreshold) {
                            c++;
                            partial.confidence += 100.0/8.0;
                            partial.d += similarities.at(10);
                        }
                        if (similarities.at(11) > faceMatchingManager.alarmThreshold) {
                            c++;
                            partial.confidence += 100.0/8.0;
                            partial.d += similarities.at(11);
                        }
                        if (similarities.at(12) > faceMatchingManager.alarmThreshold) {
                            c++;
                            partial.confidence += 100.0/8.0;
                            partial.d += similarities.at(12);
                        }
                        if (similarities.at(13) > faceMatchingManager.alarmThreshold) {
                            c++;
                            partial.confidence += 100.0/8.0;
                            partial.d += similarities.at(13);
                        }
                        if (similarities.at(14) > faceMatchingManager.alarmThreshold) {
                            c++;
                            partial.confidence += 100.0/8.0;
                            partial.d += similarities.at(14);
                        }
                        if (c==0) {
                            //no more levels
                        } else {
                            partial.d /= c;
                        }
                    } else {
                        partial.d /= c;
                    }
                } else {
                    partial.d /= c;
                }
            }
            commonTool.log(QString("partial %1.").arg(partial.d));
            partialFaceVerificationResult.push_back(partial);
            person.setPartial(partial);
            person.setSimilarities(similarities);
            persons.push_back(person);
            //
        }

        //sort
        commonTool.log("sort 1");
        sortedFaceVerificationResult = faceVerificationResult;
        for(int i = 0; i < sortedFaceVerificationResult.size();i++){
            sortedFaceVerificationResult[i].print();
        }
        std::sort(sortedFaceVerificationResult.begin(), sortedFaceVerificationResult.end(), intDouble_less);
        for(int i = 0; i < sortedFaceVerificationResult.size();i++){
            sortedFaceVerificationResult[i].print();
        }
        commonTool.log(QString("sort 2: partialFaceVerificationResult.size(): %1").arg(partialFaceVerificationResult.size()));
        for(size_t i = 0; i < partialFaceVerificationResult.size();i++){
            partialFaceVerificationResult.at(i).print();
        }
        sortedPartialFaceVerificationResult = partialFaceVerificationResult;
        std::sort(sortedPartialFaceVerificationResult.begin(), sortedPartialFaceVerificationResult.end(), intDouble_less);
        commonTool.log(QString("sort 2 over: partialFaceVerificationResult.size(): %1").arg(partialFaceVerificationResult.size()));
        for(std::vector<IntDouble>::iterator it = sortedPartialFaceVerificationResult.begin(); it != sortedPartialFaceVerificationResult.end(); ++it){
            it->print();
        }
        commonTool.log("sort end");
        //get the highest Score! (necessary?)
        matchedIndex = sortedFaceVerificationResult.front().i;
        if (faceVerificationResult.at(matchedIndex).d < faceMatchingManager.alarmThreshold)
            matchedIndex = -1;

        partialMatchedIndex = sortedPartialFaceVerificationResult.front().i;
        if (partialFaceVerificationResult.at(partialMatchedIndex).d < faceMatchingManager.alarmThreshold)
            partialMatchedIndex = -1;

        commonTool.log(QString("matchedIndex --> %1").arg(matchedIndex));
        commonTool.log(QString("partialMatchedIndex --> %1").arg(partialMatchedIndex));

        if(matchedIndex != -1){
            result.foundMethod = 1;
            char numstr[21]; // enough to hold all numbers up to 64-bits
            for (int i = 0; i < numOfPerson; i++) {
                sprintf(numstr, "%d", i);
                string name = "../data/person";
                name = name + numstr + ".jpg";
                int index = sortedFaceVerificationResult.at(i).i;
                cv::imwrite(name, wantedPersons.at(index));
                persons.at(index).setLandmarks(wantedPersonsLandmarkPointsList.at(index));
                persons.at(index).setPhotoPath(name);
                result.persons.push_back(persons.at(index));
            }
        } else if(partialMatchedIndex != -1) {
            result.foundMethod = 2;
            char numstr[21]; // enough to hold all numbers up to 64-bits
            for (int i = 0; i < numOfPerson; i++) {
                sprintf(numstr, "%d", i);
                string name = "../data/person";
                name = name + numstr + ".jpg";
                int index = sortedPartialFaceVerificationResult.at(i).i;
                cv::imwrite(name, wantedPersons.at(index));
                persons.at(index).setPhotoPath(name);
                persons.at(index).setLandmarks(wantedPersonsLandmarkPointsList.at(index));
                result.persons.push_back(persons.at(index));
            }
        }
    }
}

bool intDouble_less(IntDouble const& a, IntDouble const& b) {
    return a.d > b.d;
}

void QBorderMainWindow::findFace(int isSketch, QString selectedFile) {
    qDebug() << "findFace begin\n" << selectedFile;
    if (selectedFile.size() != 0) {
        cv::Mat selectedGrayImg;
        {
            cv::Mat selectedImg = cv::imread(selectedFile.toStdString());
            cv::cvtColor(selectedImg, selectedGrayImg,CV_RGB2GRAY);
            inputPersonSynthesizedReady = false; //xlzhu
        }
        commonTool.log("onClickedInputPersonLabel.3");
        cv::Mat alignedFace;
        std::vector<cv::Point2d> landmarkPoints;
        FaceAlignment fa(parameter.dataPath.toStdString(), faceAlignMutex);
        cv::Point2d centerOfLandmarks; //not used
        fa.alignStasm4(selectedGrayImg, alignedFace, landmarkPoints, centerOfLandmarks);
        if (alignedFace.empty()){
            commonTool.log("Error importing input person ...");
        }
        inputPerson = alignedFace;
        cv::imwrite("../data/inputPerson.jpg",inputPerson);
        QFileInfo f(selectedFile);
        inputPersonFileName = f.fileName();
        inputPersonLandmarkPoints = landmarkPoints;
        commonTool.log("QBorderMainWindow::face Alignment Completed!");
        Person synPerson;
        synPerson.setLandmarks(inputPersonLandmarkPoints);
        if(isSketch == 1){
            if (isAbleToSynthesizeInputPerson) {
                faceSynthesisManager.synthesize(inputPerson);
                inputPersonSynthesized = faceSynthesisManager.FaceSynthesisManager::getSysthesizedImg();
                cv::imwrite("../data/inputPersonSynthesized.jpg",inputPersonSynthesized);
                synPerson.setPhotoPath("../data/inputPersonSynthesized.jpg");
                inputPersonSynthesizedReady = true;
                commonTool.log("QBorderMainWindow::Input Person Synthesized!");
            }
        } else {
            synPerson.setPhotoPath("../data/inputPerson.jpg");
        }
        result = Result();
        result.setSynPerson(synPerson);
        performFaceVerification(isSketch);
        emit faceFound(result);
    }
}
