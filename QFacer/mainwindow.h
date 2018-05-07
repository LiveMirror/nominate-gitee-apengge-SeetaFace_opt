#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "facetodbtask.h"
#include "facesdkwapper.h"
#include <QPushButton>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

    void updateIcorOfButton(QPushButton * button, QString path);

private slots:
    void on_imageFromSelectPushButton_clicked();
    
    void on_dbSavePathSelectPushButton_clicked();
    
    void on_createDBStartButton_clicked();

    void onFaceToDBTaskfinish();

    void displayFaceToDBDisplayInfo(QString displayInfo);

    void on_leftImageButton_clicked();

    void on_rightImageButton_clicked();

    void on_compareButton_clicked();

    void on_nCompareLoadDestDBButton_clicked();

    void on_nCompareSrcImageButton_clicked();



    void on_nCompareCompareButton_clicked();

private:
    Ui::MainWindow *ui;


    FaceToDBTask * faceToDBTask;

    //1:1 比对
    QString leftFile;
    QString rightFile;

    FaceSdkWapper faceSDKWapper;
    //1:n 比对
    QString nCompareDestDBPath;

    QString nCompareSrcImgPath;

    list<ImageFileAndFeature> nCompareFileAndFeatures;

    vector<QPushButton *> nFacePushButtons;
    vector<QLabel *>nScoreLables;

};

#endif // MAINWINDOW_H
