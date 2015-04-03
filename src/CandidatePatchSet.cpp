#include "CandidatePatchSet.h"
#include <fstream>

CandidatePatchSet::CandidatePatchSet(){
    combinedSourceReady = false;
    combinedTargetReady = false;
}

CandidatePatchSet::~CandidatePatchSet()
{

}

cv::Mat CandidatePatchSet::getCombinedSource(){
    if (!combinedSourceReady){
        int row = candidatePatchSet.at(0)->sourcePatchData.rows;
        int col = candidatePatchSet.at(0)->sourcePatchData.cols;
        combinedSource = cv::Mat::zeros(row, col, CV_32F);

        for (int i = 0; i < candidatePatchSet.size(); i++){
            double weight = candidatePatchSet.at(i)->weight;
            cv::Mat candidatePatchSource = candidatePatchSet.at(i)->sourcePatchData;
            candidatePatchSource.convertTo(candidatePatchSource, CV_32F);
            combinedSource += candidatePatchSource * weight;
        }
        combinedSourceReady = true;
        combinedTarget.convertTo(combinedTarget, CV_8U);
    }
    return combinedSource;
}

cv::Mat CandidatePatchSet::getCombinedTarget(){
    if (!combinedTargetReady){
        int row = candidatePatchSet.at(0)->targetPatchData.rows;
        int col = candidatePatchSet.at(0)->targetPatchData.cols;
        combinedTarget = cv::Mat::zeros(row, col, CV_32F);

        for (int i = 0; i < candidatePatchSet.size(); i++){
            double weight = candidatePatchSet.at(i)->weight;
            cv::Mat candidatePatchTarget = candidatePatchSet.at(i)->targetPatchData;
            candidatePatchTarget.convertTo(candidatePatchTarget, CV_32F);
            combinedTarget += candidatePatchTarget * weight;
        }
        combinedTargetReady = true;
        combinedTarget.convertTo(combinedTarget, CV_8U);
    }
    return combinedTarget;
}

double CandidatePatchSet::RMSByPatch(cv::Mat input, int type){
    cv::Mat mat;
    if (type == _CANDIDATE_SOURCE_) {
        mat = getCombinedSource();
    } else if (type == _CANDIDATE_TARGET_){
        mat = getCombinedTarget();
    }
    return RMS(input, mat);
}

int CandidatePatchSet::size(){
    return candidatePatchSet.size();
}

cv::Mat CandidatePatchSet::getSource(int candidateIndex){
    return candidatePatchSet.at(candidateIndex)->sourcePatchData;
}
cv::Mat CandidatePatchSet::getTarget(int candidateIndex){
    return candidatePatchSet.at(candidateIndex)->targetPatchData;
}

double CandidatePatchSet::getWeight(int candidateIndex){
    return candidatePatchSet.at(candidateIndex)->weight;
}

patchIdentifier CandidatePatchSet::getPatchIdentifier(int candidateIndex){
    return candidatePatchSet.at(candidateIndex)->identifier;

}

QRect CandidatePatchSet::getSearchWin(){
    QRect searchWin;
    for (int i = 0; i < candidatePatchSet.size(); i++){
        int colStart = candidatePatchSet.at(i)->identifier.colRange.start;
        int colEnd = candidatePatchSet.at(i)->identifier.colRange.end;
        int rowStart = candidatePatchSet.at(i)->identifier.rowRange.start;
        int rowEnd = candidatePatchSet.at(i)->identifier.rowRange.end;

        QPoint topLeft(colStart, rowStart);
        QPoint bottomRight(colEnd, rowEnd);

        QRect patch(topLeft, bottomRight);

        searchWin = searchWin.united(patch);
    }
    return searchWin;
}

QRect* CandidatePatchSet::getOrgPatchRect(){
    QRect* orgPatchRect = new QRect(0, 0, 0, 0);

    if (size() <= 0){
        return orgPatchRect;
    }
    int colStart = getPatchIdentifier(0).orgColRange.start;
    int colEnd = getPatchIdentifier(0).orgColRange.end;
    int rowStart = getPatchIdentifier(0).orgRowRange.start;
    int rowEnd = getPatchIdentifier(0).orgRowRange.end;

    orgPatchRect->setTop(rowStart);
    orgPatchRect->setBottom(rowEnd);
    orgPatchRect->setLeft(colStart);
    orgPatchRect->setRight(colEnd);

    return orgPatchRect;
}
