#ifndef FACESDKWAPPER_H
#define FACESDKWAPPER_H

#include "common.h"
#include "face_detection.h"
#include "face_identification.h"
#include "face_alignment.h"
#include <QMutex>


#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#define SEETA_PROVIDER "SeetaFacer"
#define VIFACE_PROVIDER "VIFacer"

typedef struct FacePoint
{
    FacePoint()
    {
        x = 0;
        y = 0;
    }
    int x;
    int y;
}FacePoint;

typedef struct FaceInfo {

  FaceInfo()
  {
    x = 0;
    y = 0;
    width = 0;
    height = 0;
    score = 0;
    rool = 0 ;
    pitch = 0;
    yaw = 0;
  }

  //人脸范围
  int x;
  int y;
  int width;
  int height;

  float score; //得分,0--100,越高说明头像可能性越大
  float rool;//人脸平面顺时针旋转程度
  float pitch; //低头程度
  float yaw;  //侧脸
  float fuzzy; //模糊度
  //5点坐标
  FacePoint points[5];

} FaceInfo;


typedef struct ImageFileAndFeature
{
    ImageFileAndFeature(QString file, std::vector<float> &feature)
    {
        this->file = file;
        this->feature = feature;
    }

    QString file;
    std::vector<float> feature;
}ImageFileAndFeature;

class FaceSdkWapper
{
public:
    FaceSdkWapper();

    ~FaceSdkWapper();

    int initWithGPU(int gpu,  SString sdk_provider);

    void release();

    std::list<FaceInfo> faceDetectAndAlign(cv::Mat &img);

    std::vector<float> &faceExtractFeature(cv::Mat &img, FaceInfo & faceInfo);



    float calSimilarity(std::vector<float> &feature1, std::vector<float> & feature2);

    void compareTopN(std::vector<float> &feature, std::list<ImageFileAndFeature>& faces, int n, vector<ImageFileAndFeature> &topFaces, vector<float> &topScore, float minScore);

    static QMutex initLock;

private:

    int gpu;

    SString sdk_provider;
    //seeta

    seeta::FaceDetection * seeta_faceDetection;

    seeta::FaceAlignment * seeta_faceAlignMent;

    seeta::FaceIdentification * seeta_faceIdentification;

    std::vector<float> seeta_face_features;
    std::vector<float> seeta_face_features_zero;

    //VIface


};

#endif // FACESDKWAPPER_H
