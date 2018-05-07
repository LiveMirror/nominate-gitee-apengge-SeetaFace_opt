#ifndef FACETODBTASK_H
#define FACETODBTASK_H
#include "Runable.h"
#include "QObject"
#include <QMutex>
#include <QSemaphore>

#include "facesdkwapper.h"

class FaceToDBTask : public QObject, public Runable
{
    Q_OBJECT
public:
    FaceToDBTask(QString imageDir, QString dbDir, int subTaskCount);

    virtual int start();

    QString nextImage();

    void saveFileAndFeature(QString file, std::vector<float>& feature);
    signals:

    void onFinish();

    void displayInfo(QString info);

    // 0 -- 100
    void updateProcess(int process);





public:

    QString imageDir;

    QString dbDir;

    int subTaskCount;

    QStringList images;

    std::list<ImageFileAndFeature> fileAndFeatures;

    int totalImageCount;

    QMutex lock;

    QMutex fileAndFeaturesLock;

};

#endif // FACETODBTASK_H
