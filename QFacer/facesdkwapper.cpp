#include "facesdkwapper.h"
#include "logger.h"


QMutex FaceSdkWapper::initLock;

FaceSdkWapper::FaceSdkWapper()
{
    gpu = -1;

    seeta_faceDetection = NULL;
    seeta_faceAlignMent = NULL;
    seeta_faceIdentification = NULL;

    seeta_face_features_zero.resize(2048);
    for (int i = 0; i < 2048; i++)
    {
        seeta_face_features_zero[i] = 0.0f;
    }

    seeta_face_features.resize(2048);
}

FaceSdkWapper::~FaceSdkWapper()
{
}

int FaceSdkWapper::initWithGPU(int gpu, SString sdk_provider)
{
    initLock.lock();
    this->sdk_provider = sdk_provider;
    this->gpu = gpu;

    if(this->sdk_provider.compare(SEETA_PROVIDER) == 0)
    {
        seeta_faceDetection = new seeta::FaceDetection("model/seeta_fd_frontal_v1.0.bin");
        seeta_faceDetection->SetMinFaceSize(70);
        seeta_faceDetection->SetScoreThresh(2.f);
        seeta_faceDetection->SetImagePyramidScaleFactor(0.8f);
        seeta_faceDetection->SetWindowStep(4, 4);
        seeta_faceAlignMent = new seeta::FaceAlignment("model/seeta_fa_v1.1.bin");
        seeta_faceIdentification = new seeta::FaceIdentification();
        int loadResult = seeta_faceIdentification->LoadModel("model/seeta_fr_v1.0.bin");
        LOG_INFO("load face identification with result:%d", loadResult);
    }
    else
    {

    }

    initLock.unlock();
    return f_success;
}

void FaceSdkWapper::release()
{
    if(this->sdk_provider.compare(SEETA_PROVIDER) == 0)
    {
        SafeDeleteObj(seeta_faceDetection);
        SafeDeleteObj(seeta_faceAlignMent);
        SafeDeleteObj(seeta_faceIdentification);
    }

}

std::list<FaceInfo> FaceSdkWapper::faceDetectAndAlign(  cv::Mat &img )
{
    LOG_INFO("try to detect image with height:%d width:%d", img.rows, img.cols);
    std::list<FaceInfo> ret;
    if(this->sdk_provider.compare(SEETA_PROVIDER) == 0)
    {
        cv::Mat img_gray;

        if (img.channels() != 1)
        {
            cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
        }
        else
        {
           img_gray = img;
        }


        seeta::ImageData img_data;
        img_data.data = img_gray.data;
        img_data.width = img_gray.cols;
        img_data.height = img_gray.rows;
        img_data.num_channels = 1;

        std::vector<seeta::FaceInfo> faces = seeta_faceDetection->Detect(img_data);

        foreach (seeta::FaceInfo face, faces) {
          seeta::FacialLandmark points[5];
          seeta_faceAlignMent->PointDetectLandmarks(img_data, face, points);

          FaceInfo info;
          info.height = face.bbox.height;
          info.width = face.bbox.width;
          info.x = face.bbox.x;
          info.y = face.bbox.y;
          info.pitch = face.pitch;
          info.rool = face.roll;
          info.yaw = face.yaw;
          info.score = face.score;
          for (int i = 0; i < 5; i ++)
          {
              info.points[i].x = points[i].x;
              info.points[i].y = points[i].y;
          }

          ret.push_back(info);
        }
    }

    return ret;
}

std::vector<float>& FaceSdkWapper::faceExtractFeature(cv::Mat &img, FaceInfo &faceInfo)
{
    if(this->sdk_provider.compare(SEETA_PROVIDER) == 0)
    {
        if(seeta_face_features.size() != 2048)
        {
            seeta_face_features.resize(2048);
        }

        memcpy(seeta_face_features.data(), seeta_face_features_zero.data(), 2018 * sizeof(float));

        seeta::ImageData src_img_data(img.cols, img.rows, img.channels());
        src_img_data.data = img.data;

        seeta::FacialLandmark pt5[5];
        for (int i = 0; i < 5; ++ i) {
          pt5[i].x = faceInfo.points[i].x;
          pt5[i].y = faceInfo.points[i].y;
        }

        seeta_faceIdentification->ExtractFeatureWithCrop(src_img_data, pt5,
              seeta_face_features.data());

        return seeta_face_features;

    }


    return seeta_face_features;

}

float FaceSdkWapper::calSimilarity(std::vector<float> &feature1, std::vector<float> &feature2)
{
    if(this->sdk_provider.compare(SEETA_PROVIDER) == 0)
    {
        return seeta_faceIdentification->CalcSimilarity(feature1.data(), feature2.data(), feature1.size());
    }

    return 0;
}

void FaceSdkWapper::compareTopN(std::vector<float> &feature,std::list<ImageFileAndFeature> &faces, int n, vector<ImageFileAndFeature> &topFaces, vector<float> &topScore, float minScore)
{
    topFaces.clear();
    topScore.clear();
    foreach (ImageFileAndFeature face, faces) {
        float score = calSimilarity(face.feature, feature);
        if(score >= minScore)
        {
            if(topScore.size() == 0)
            {
                topScore.push_back(score);
                topFaces.push_back(face);
            }
            else
            {

                int i = 0;
                for (i; i < topScore.size(); i++)
                {
                    if(topScore[i] > score)
                    {
                        break;
                    }
                }

                topScore.insert(topScore.begin() + i, score);
                topFaces.insert(topFaces.begin() + i, face);
                if(topFaces.size() > n)
                {
                    topFaces.erase(topFaces.begin());
                    topScore.erase(topScore.begin());
                }

            }
        }
    }
}
