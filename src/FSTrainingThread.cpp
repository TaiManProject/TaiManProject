#include "FSTrainingThread.h"


FSTrainingThread::FSTrainingThread(void)
{
}

void FSTrainingThread::setThread(QString photoPath_p, QString sketchPath_p, QStringList checkedPhotoList_p,QStringList checkedSketchList_p, bool photoKDTree_p, bool sketchKDTree_p, Parameter parameter_p, IntegralImages* photoImages_p, IntegralImages* sketchImages_p){
	photoPath = photoPath_p;
	sketchPath = sketchPath_p;
	checkedPhotoList = checkedPhotoList_p;
	checkedSketchList = checkedSketchList_p;
	photoKDTree = photoKDTree_p;
	sketchKDTree = sketchKDTree_p;
	parameter = parameter_p;
	photoImages = photoImages_p;
	sketchImages = sketchImages_p;
    commonTool.log("FSTrainingThread::setThread.");
}


void FSTrainingThread::run(){
    commonTool.log("Starting own thread for building face synthesize data.");
	photoImages->load(photoPath, checkedPhotoList, photoKDTree, parameter );
	sketchImages->load(sketchPath, checkedSketchList, sketchKDTree, parameter );
	emit trainingDone();
}

IntegralImages* FSTrainingThread::getPhotoImages(){
	return photoImages;
}

IntegralImages* FSTrainingThread::getSketchImages(){
	return sketchImages;
}
