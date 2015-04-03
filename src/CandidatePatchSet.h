#ifndef CANDIDATEPATCHSET_H
#define CANDIDATEPATCHSET_H

#include <opencv/cv.h>
#include <QRect>
#include <QPoint>

#include "commonTool.h"
//#include "QCandidatePatchWidget.h"


#define _CANDIDATE_SOURCE_ 0
#define _CANDIDATE_TARGET_ 1


struct CandidatePatch{
    patchIdentifier identifier;
    cv::Mat sourcePatchData;
    cv::Mat targetPatchData;
    double weight;
};

class CandidatePatchSet
{
public:
    CandidatePatchSet();
    ~CandidatePatchSet();

    QList<CandidatePatch*> candidatePatchSet;
    cv::Mat getCombinedSource();
    cv::Mat getCombinedTarget();
    double RMSByPatch(cv::Mat input, int type);
    int size();
    cv::Mat getSource(int candidateIndex);
    cv::Mat getTarget(int candidateIndex);
    double getWeight(int candidateIndex);
    patchIdentifier getPatchIdentifier(int candidateIndex);
//    QCandidatePatchWidget* getCandidatePatchWidget(int candidateIndex);
    QRect getSearchWin();
    QRect* getOrgPatchRect();

private:
    bool combinedSourceReady;
    cv::Mat combinedSource;
    bool combinedTargetReady;
    cv::Mat combinedTarget;
	
};

#endif // CANDIDATEPATCHSET_H
