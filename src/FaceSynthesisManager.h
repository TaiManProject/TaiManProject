#pragma once

#include "FSTrainingThread.h"
#include "CandidatePatchSet.h"
#include "IntegralImages.h"
#include "Synthesis.h"
#include "UsingQP.h"

#include <string>
#include <QDir>

#define DIRECTION_NOT_SET 0
#define DIRECTION_PHOTO_TO_SKETCH 1
#define DIRECTION_SKETCH_TO_PHOTO 2

class FaceSynthesisManager: public QThread {
    Q_OBJECT
public:
    FaceSynthesisManager(void);

    void train();
    int getNumberOfTrainedImage();
    void synthesize(cv::Mat inputImg);
    void synthesize(std::vector<cv::Mat> inputImg);

    QString setPath(QString photoPath, QString sketchPath);
    void setDirectionPhotoToSketch();
    void setDirectionSketchToPhoto();
    void setParameter(Parameter parameter_p);
    bool saveKDTree();
    void clean();

    //method after synthesized
    cv::Mat getSysthesizedImg();

    std::vector<cv::Mat> synthesizedImgs;

    CandidatePatchSet* getCandidatePatchSet(QPointF patchCoordinate);
    cv::Mat getPatchFromInputImage(QRect rect);
    cv::Mat getCanPatchOrgImg(int type, int index);
    QString getCanPatchOrgImgPath(int type, int index);

private:
    QString photoPath;
    QString sketchPath;

    QStringList checkedPhotoList;
    QStringList checkedSketchList;

    int sysnthesisDirection;
    Parameter parameter;

    bool pathChecked;

    int trainedImage;

    IntegralImages* photoImages;
    IntegralImages* sketchImages;

    IntegralImage testImage;

    MRFQP *Sketch;

    bool synthesizedImgReady;
    cv::Mat synthesizedImg;


    QList<CandidatePatchSet*> outputCandidatePatchSet;

    FSTrainingThread trainingThread;

    QString checkTraningData(QStringList photoList, QStringList sketchList);
    QString checkTrained();

    std::vector<cv::Mat> inputImgs;
    void run();

signals:
    void FSTrainingDone();
    void FSDone();

public slots:
    void FSTrainingThreadDoneSlot();

};
