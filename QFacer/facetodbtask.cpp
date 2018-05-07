#include "facetodbtask.h"
#include <QDir>

#include "SimpleRunable.h"
#include "facesdkwapper.h"
#include <QFile>
#include <QIODevice>

int FaceToDBTaskSubTask(void *p)
{
    FaceSdkWapper facesdk;
    facesdk.initWithGPU(0, SEETA_PROVIDER);
    FaceToDBTask * task  = (FaceToDBTask *)p;

    QString image;
    while (!((image = task->nextImage()).isEmpty()))
    {
        cv::Mat img = cv::imread(image.toLocal8Bit().data());
        std::list<FaceInfo> facesInfo = facesdk.faceDetectAndAlign(img);
        LOG_INFO("file:%s, havs faces count:%d", QStr2CStr(image), facesInfo.size());
        if(facesInfo.size() == 1)
        {
            task->saveFileAndFeature(image, facesdk.faceExtractFeature(img, facesInfo.front()));
        }

    }

    facesdk.release();
    return f_success;
}

FaceToDBTask::FaceToDBTask(QString imageDir, QString dbDir, int subTaskCount) : imageDir(imageDir),
    dbDir(dbDir), subTaskCount(subTaskCount), totalImageCount(0)
{
    if(subTaskCount <= 0 || subTaskCount >= 100)
    {
        LOG_ERROR("invalid sub task count:%d, set to 4 by default", subTaskCount);
        subTaskCount = 4;
    }
}



int FaceToDBTask::start()
{
    emit this->updateProcess(0);
    emit this->displayInfo("loading images");

    images = getImagesFilesFromPath(imageDir);
    totalImageCount = images.size();
    std::list<SimpleRunable *> runables;
    for (int i = 0; i < subTaskCount; i++)
    {
        runables.push_back(new SimpleRunable(FaceToDBTaskSubTask, this));
    }

    foreach (SimpleRunable* runable, runables) {
        runable->releaseSelfWhenExist = false;
        runable->run();
    }

    foreach (SimpleRunable* runable, runables) {
        runable->waitThreadQuit();
        SafeDeleteObj(runable);
    }

    emit displayInfo("writting feature to local file");

    QFile file(dbDir + "/" + SEETA_PROVIDER + ".db");
    file.open(QIODevice::ReadWrite|QIODevice::Text);
    QTextStream in(&file);
    foreach (ImageFileAndFeature temp, fileAndFeatures) {
        in <<   QStr2CStr(temp.file) << "|";
        for (int i = 0; i < 2048; i ++)
        {
            if(i != 0)
            {
                in << ",";
            }
            in << float2SString(temp.feature[i]).c_str();
        }
        in << "\n";
    }
    file.close();
    fileAndFeatures.clear();

    emit this->updateProcess(100);
    emit this->onFinish();
    totalImageCount = 0;
    return f_success;
}

QString FaceToDBTask::nextImage()
{
    QString ret = "";
    lock.lock();
    if(images.size() > 0)
    {
        ret = images.front();
        images.pop_front();
        emit this->displayInfo("handing:" + int2QStr(totalImageCount - images.size()) + "/" + int2QStr(totalImageCount));
        emit this->updateProcess(100.0 - images.size() * 100.0 / totalImageCount);
    }

    lock.unlock();
    return ret;
}

void FaceToDBTask::saveFileAndFeature(QString file, std::vector<float> &feature)
{
    fileAndFeaturesLock.lock();
    ImageFileAndFeature it(file,feature);
    fileAndFeatures.push_back(it);
    fileAndFeaturesLock.unlock();;
}
