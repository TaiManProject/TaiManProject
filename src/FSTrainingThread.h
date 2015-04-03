#pragma once

#include <QThread>
#include <QStringList>
#include "commonTool.h"
#include "IntegralImages.h"

//Training for the face synthesize
class FSTrainingThread : public QThread {
    Q_OBJECT
public:
    FSTrainingThread(void);
    void setThread( 	QString photoPath_p,
                        QString sketchPath_p,
                        QStringList checkedPhotoList_p,
                        QStringList checkedSketchList_p,
                        bool photoKDTree_p,
                        bool sketchKDTree_p,
                        Parameter parameter_p,
                        IntegralImages* photoImages_p,
                        IntegralImages* sketchImages_p);

    IntegralImages* getPhotoImages();
    IntegralImages* getSketchImages();

signals:
    void trainingDone();


private:
    //input
    QString photoPath;
    QString sketchPath;
    QStringList checkedPhotoList;
    QStringList checkedSketchList;
    bool photoKDTree;
    bool sketchKDTree;
    Parameter parameter;

    //output
    IntegralImages* photoImages;
    IntegralImages* sketchImages;

    void run();
};

