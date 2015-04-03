QT += core
QT -= gui
QT += network

TEMPLATE = app
TARGET = faceMatch
INCLUDEPATH += .

#CONFIG += console
CONFIG -=app_bundle

LIBS += "/home/niesh/faceSketchSynthesize/stasm4.1.0/build/libstasm.a"
LIBS += `pkg-config opencv --libs`
INCLUDEPATH += "/usr/local/include" "/usr/local/include/opencv"
INCLUDEPATH += "/home/niesh/faceSketchSynthesize/hugin-2013.0.0/src/foreign" "/home/niesh/faceSketchSynthesize/hugin-2013.0.0/src/foreign/flann"
INCLUDEPATH += stasm4

# Input
HEADERS += \
    CandidatePatchSet.h \
    commonTool.h \
    FaceMatchingManager.h \
    FaceMatchingSimilarityTree.h \
    FaceSynthesisManager.h \
    FSTrainingThread.h \
    IntegralImage.h \
    IntegralImages.h \
    QBorderMainWindow.h \
    SearchWin.h \
    Synthesis.h \
    UsingQP.h \
    FaceAlignment.h \
    FaceMatchingSingle.h \
    FaceMatchingMultiple.h \
    server.h \
    result.h \
    person.h \
    IntDouble.h
SOURCES += \
    CandidatePatchSet.cpp \
    commonTool.cpp \
    FaceMatchingManager.cpp \
    FaceMatchingMultiple.cpp \
    FaceMatchingSimilarityTree.cpp \
    FaceSynthesisManager.cpp \
    FSTrainingThread.cpp \
    IntegralImage.cpp \
    IntegralImages.cpp \
    main.cpp \
    QBorderMainWindow.cpp \
    SearchWin.cpp \
    Synthesis.cpp \
    UsingQP.cpp \
    FaceAlignment.cpp \
    FaceMatchingSingle.cpp \
    server.cpp \
    result.cpp \
    person.cpp \
    IntDouble.cpp
RESOURCES +=
