#ifndef BORDERMAINWINDOW_H
#define BORDERMAINWINDOW_H

#include <iostream>
#include <string>

#include <QDir>
#include <QStringList>
#include <QThread>
#include <QTimer>

#include <opencv/cv.h>
#include <opencv2/contrib/contrib.hpp> //Audrey

#include "FaceMatchingManager.h"
#include "FaceSynthesisManager.h"
#include "commonTool.h"
#include "person.h"
#include "result.h"
#include "IntDouble.h"

bool intDouble_less(IntDouble const& a, IntDouble const& b);

class QBorderMainWindow : public QObject {
    Q_OBJECT

public:
    std::vector<cv::Mat> wantedPersons;
    std::vector<cv::Mat> wantedPersonsSynthesized;
    std::vector<QString> wantedPersonFileNames;
    std::vector< std::vector< cv::Point2d > > wantedPersonsLandmarkPointsList;

    int matchedIndex;
    int partialMatchedIndex;

    cv::Mat inputPerson;
    QString inputPersonFileName;
    //Audrey
    cv::Mat inputPersonSynthesized;
    bool inputPersonSynthesizedReady;
    bool isAbleToSynthesizeInputPerson;

    std::vector< cv::Point2d > inputPersonLandmarkPoints;

    std::vector<IntDouble> faceVerificationResult;
    std::vector<Person> persons;
    Result result;

    std::vector<IntDouble> partialFaceVerificationResult;
    std::vector<IntDouble> sortedPartialFaceVerificationResult;

    std::vector<IntDouble> sortedFaceVerificationResult;
    std::vector<std::vector<double> > faceVerificationResultRaw;


    Parameter parameter;

    QBorderMainWindow(QObject *parent = 0);

    QMutex faceAlignMutex;
    //opencv can not be called in parallel :(
    QMutex openCVMutex;

    FaceMatchingManager faceMatchingManager;
    FaceSynthesisManager faceSynthesisManager;

public slots:
    void findFace(int isSketch, QString selectedFile);
signals:
    void faceFound(Result result);

private slots:
    void trainFaceSynthesizeCompleted();


private:
//    void updateUI();
    void loadWantedPersons();
    QStringList getWantedPersonQStringListFromFile(QString filePath);
    void performFaceVerification(int isSketch);
    void trainFaceSynthesize();
    void faceSynthesizeCompleted();
};

#endif // BORDERMAINWINDOW_H
