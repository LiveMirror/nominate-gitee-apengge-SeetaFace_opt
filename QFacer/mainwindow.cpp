#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "common.h"
#include <QFileDialog>
#include "Logger.h"
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMessageBox>
#include "facesdkwapper.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    faceToDBTask = NULL;
    //ui->dbSavePathSelectPushButton->setHidden(true);
    setAcceptDrops(true);
    faceSDKWapper.initWithGPU(0,SEETA_PROVIDER);

    nScoreLables.push_back(ui->nCompareTop1ScoreLabel);
    nScoreLables.push_back(ui->nCompareTop2ScoreLabel);
    nScoreLables.push_back(ui->nCompareTop3ScoreLabel);
    nScoreLables.push_back(ui->nCompareTop4ScoreLabel);
    nScoreLables.push_back(ui->nCompareTop5ScoreLabel);

    nFacePushButtons.push_back(ui->nCompareTop1ImageLabel);
    nFacePushButtons.push_back(ui->nCompareTop2ImageLabel);
    nFacePushButtons.push_back(ui->nCompareTop3ImageLabel);
    nFacePushButtons.push_back(ui->nCompareTop4ImageLabel);
    nFacePushButtons.push_back(ui->nCompareTop5ImageLabel);

}

MainWindow::~MainWindow()
{
    faceSDKWapper.release();
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    //LOG_INFO("drag event pos, x:%d, y:%d", e->pos().x(), e->pos().y())
    if(e->mimeData()->hasUrls()) //只能打开jpeg图片
    {
        if(ui->tabWidget->currentIndex() == 1 || ui->tabWidget->currentIndex() == 2)
        {
             e->acceptProposedAction(); //可以在这个窗口部件上拖放对象;

        }
    }

}

void MainWindow::dropEvent(QDropEvent *e)
{

    QPoint ePoint = e->pos();
    ePoint -=(ui->tabWidget->geometry().topLeft());
     if(ui->tabWidget->currentIndex() == 1)
     {

         //LOG_INFO("drop event pos, x:%d, y:%d", e->pos().x(), e->pos().y());
        // LOG_INFO("drop event pos, x:%d, y:%d", this->mapFromGlobal(e->pos()).x(), this->mapFromGlobal(e->pos()).y());
        // LOG_INFO("right button pos, x:%d, y:%d, w:%d, h:%d",ui->rightImageButton->geometry().x(), ui->rightImageButton->geometry().y()
         //          , ui->rightImageButton->geometry().width(), ui->rightImageButton->geometry().height());

        // LOG_INFO("left button pos, x:%d, y:%d, w:%d, h:%d",ui->leftImageButton->geometry().x(), ui->leftImageButton->geometry().y()
          //         , ui->leftImageButton->geometry().width(), ui->leftImageButton->geometry().height());

         QList<QUrl> urls = e->mimeData()->urls();
         if(ui->rightImageButton->geometry().contains(ePoint))
         {
             QString path = urls.first().toLocalFile();
             if(path.size() > 0)
             {

                 updateIcorOfButton(ui->rightImageButton, path);
                 QIcon icon;
                 QPixmap pixmap1(path);
                 icon.addPixmap(pixmap1);
                 ui->rightImageButton->setIcon(icon);
                 ui->rightImageButton->setIconSize(ui->rightImageButton->size());
                 //ui->leftImageButton->setStyleSheet("border-image: url(:" + path + ");");

                 rightFile = path;
                 LOG_INFO("dorp event with right file:%s", QStr2CStr(rightFile));
             }
         }

         if( ui->leftImageButton->geometry().contains(ePoint))
         {
             QString path = urls.first().toLocalFile();
             if(path.size() > 0)
             {
                  updateIcorOfButton(ui->leftImageButton, path);
                 //ui->leftImageButton->setStyleSheet("border-image: url(:" + path + ");");

                 leftFile = path;
                 LOG_INFO("dorp event with left file:%s", QStr2CStr(leftFile));
             }
         }
     }
     else if (ui->tabWidget->currentIndex() == 2)
     {
         QList<QUrl> urls = e->mimeData()->urls();
         if(ui->nCompareSrcImageButton->geometry().contains(ePoint))
         {
             QString path = urls.first().toLocalFile();
             if(path.size() > 0)
             {
                updateIcorOfButton(ui->nCompareSrcImageButton, path);
                nCompareSrcImgPath = path;
                LOG_INFO("dorp event with nCompareSrcImgPath:%s", QStr2CStr(nCompareSrcImgPath));
             }
         }
     }

}

void MainWindow::updateIcorOfButton(QPushButton *button, QString path)
{
    QIcon icon;
    QPixmap pixmap1(path);
    icon.addPixmap(pixmap1);
    button->setIcon(icon);
    button->setIconSize(button->size());

}



void MainWindow::on_dbSavePathSelectPushButton_clicked()
{

    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("选择保存图片特征的路径");
    fileDialog->setDirectory(".");

    fileDialog->setFileMode(QFileDialog::DirectoryOnly);

    QStringList fileNames;
    if(fileDialog->exec())
    {
        fileNames = fileDialog->selectedFiles();
    }

    if(fileNames.size() > 0)
    {
        ui->dbSavePathLineEdit->setText(fileNames.at(0));
        LOG_INFO("select db save path:%s", QStr2CStr(ui->dbSavePathLineEdit->text()));
    }
}

void MainWindow::on_imageFromSelectPushButton_clicked()
{
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle("选择提取特征值的图片路径");
    fileDialog->setDirectory(".");
    fileDialog->setFileMode(QFileDialog::DirectoryOnly);
    fileDialog->setViewMode(QFileDialog::Detail);

    QStringList fileNames;
    if(fileDialog->exec())
    {
        fileNames = fileDialog->selectedFiles();
    }

    if(fileNames.size() > 0)
    {
        ui->imageFromPathLineEdit->setText(fileNames.at(0));
        LOG_INFO("select image path:%s", QStr2CStr(ui->imageFromPathLineEdit->text()));
        ui->dbSavePathLineEdit->setText(fileNames.at(0));
        LOG_INFO("select db save path:%s", QStr2CStr(ui->dbSavePathLineEdit->text()));
    }

}

void MainWindow::on_createDBStartButton_clicked()
{
    if(ui->imageFromPathLineEdit->text().isEmpty())
    {
        QMessageBox::warning(this, "提醒", "图片路径不能为空");
        return;
    }

    if(ui->dbSavePathLineEdit->text().isEmpty())
    {
        QMessageBox::warning(this, "提醒", "图片库保存路径不能为空");
        return;
    }


    faceToDBTask = new FaceToDBTask(ui->imageFromPathLineEdit->text(),
                                    ui->dbSavePathLineEdit->text(),
                                    ui->dbCreateThreadCount->text().toInt());


    displayFaceToDBDisplayInfo("start face to db task");

    QObject::connect(faceToDBTask, SIGNAL(displayInfo(QString)), this, SLOT(displayFaceToDBDisplayInfo(QString)));
    QObject::connect(faceToDBTask, SIGNAL(onFinish()), this, SLOT(onFaceToDBTaskfinish()));
    QObject::connect(faceToDBTask, SIGNAL(updateProcess(int)), ui->createDBProgressBar, SLOT(setValue(int)));

    ui->createDBStartButton->setEnabled(false);
    ui->createDBStartButton->setText("处理中");

    faceToDBTask->run();

}

void MainWindow::onFaceToDBTaskfinish()
{
    LOG_INFO("onFaceToDBTaskfinish");
    faceToDBTask->waitThreadQuit();
    SafeDeleteObj(faceToDBTask);
    ui->createDBStartButton->setEnabled(true);
    ui->createDBStartButton->setText("启动");
    displayFaceToDBDisplayInfo("点击下面的开始进行创建");
}

void MainWindow::displayFaceToDBDisplayInfo(QString displayInfo)
{
    ui->createDBDisplayInfoLabel->setText(displayInfo);
}

void MainWindow::on_leftImageButton_clicked()
{
    ui->similarityLabel->setText("");
    QString path = QFileDialog::getOpenFileName(this, "Open Image", "", "Image Files(*.jpg *.png)");
    if(path.size() > 0)
    {
        QIcon icon;
        QPixmap pixmap1(path);
        icon.addPixmap(pixmap1);
        ui->leftImageButton->setIcon(icon);
        ui->leftImageButton->setIconSize(ui->leftImageButton->size());
        //ui->leftImageButton->setStyleSheet("border-image: url(:" + path + ");");

        leftFile = path;
    }


}

void MainWindow::on_rightImageButton_clicked()
{
    ui->similarityLabel->setText("");
    QString path = QFileDialog::getOpenFileName(this, "Open Image", "", "Image Files(*.jpg *.png)");
    if(path.size() > 0)
    {
       QIcon icon;
       QPixmap pixmap1(path);
       icon.addPixmap(pixmap1);
       ui->rightImageButton->setIcon(icon);
       ui->rightImageButton->setIconSize(ui->rightImageButton->size());

       rightFile = path;
    }



}

void MainWindow::on_compareButton_clicked()
{
    if(leftFile.size() <= 0 || rightFile.size() <= 0)
    {
        QMessageBox::warning(this, "注意", "请选择图片然后进行比较");
        return;
    }
    cv::Mat leftImage = cv::imread(leftFile.toLocal8Bit().data());
    cv::Mat rightImage = cv::imread(rightFile.toLocal8Bit().data());

    std::list<FaceInfo> leftInfos = faceSDKWapper.faceDetectAndAlign(leftImage);
    std::list<FaceInfo> rightInfos = faceSDKWapper.faceDetectAndAlign(rightImage);

    if(leftInfos.size() != 1)
    {
        QMessageBox::warning(this, "注意", "只支持一个头像进行比较,左边头像个数:" + int2QStr(leftInfos.size()));
        return;
    }

    if(rightInfos.size() != 1)
    {
        QMessageBox::warning(this, "注意", "只支持一个头像进行比较,右边边头像个数:" + int2QStr(rightInfos.size()));
        return;
    }

    std::vector<float> leftFeature = faceSDKWapper.faceExtractFeature(leftImage, leftInfos.front());
    std::vector<float> rightFeature = faceSDKWapper.faceExtractFeature(rightImage, rightInfos.front());

    float similarity = faceSDKWapper.calSimilarity(leftFeature, rightFeature);
    ui->similarityLabel->setText(float2QStr(similarity));
}

void MainWindow::on_nCompareLoadDestDBButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Open Image", "", "Image db(*.db)");

    if(path.size() > 0)
    {
        nCompareFileAndFeatures.clear();
        QStringList fileContent;
        readContentFromFile(path,fileContent);
        foreach (QString oneline, fileContent) {
            QString file = oneline.mid(0, oneline.indexOf("|"));
            QString feature = oneline.mid(oneline.indexOf("|") + 1);
            QStringList featureSplite = feature.split(",");
            std::vector<float> featureVector;
            foreach (QString temp, featureSplite) {
                featureVector.push_back(temp.toFloat());
            }

            ImageFileAndFeature one(file, featureVector);
            nCompareFileAndFeatures.push_back(one);
        }

        LOG_INFO("load imag db record count:%d", nCompareFileAndFeatures.size());
        ui->nComareDestDBCountLabel->setText(int2QStr(nCompareFileAndFeatures.size()));
        ui->nCompareDestDBLineEdit->setText(path);
    }
}

void MainWindow::on_nCompareSrcImageButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Open Image", "", "Image files(*.bmp *.jpg)");
    if(path.size() > 0)
    {
        updateIcorOfButton(ui->nCompareSrcImageButton, path);
        nCompareSrcImgPath = path;
    }
}


void MainWindow::on_nCompareCompareButton_clicked()
{
    foreach ( QPushButton * b, nFacePushButtons) {
        QIcon icon;
        b->setIcon(icon);

    }

    foreach (QLabel * l, nScoreLables) {
        l->setText("0");
    }
    if(nCompareSrcImgPath.size() == 0)
    {
        QMessageBox::warning(this, "提醒", "请选择图片然后进行比较");
        return;
    }

    if(nCompareFileAndFeatures.size() == 0)
    {
        QMessageBox::warning(this, "提醒", "请选择库,并确保库里面有图像数据");
        return;
    }


    std::vector<float> topScore;
    std::vector<ImageFileAndFeature> topFaces;

    cv::Mat img = cv::imread(nCompareSrcImgPath.toLocal8Bit().data());

    std::list<FaceInfo> faceInfos = faceSDKWapper.faceDetectAndAlign(img);
    if(faceInfos.size() != 1)
    {
        QMessageBox::warning(this, "注意", "只支持一个头像进行比较,头像个数:" + int2QStr(faceInfos.size()));
        return;
    }


    std::vector<float> features = faceSDKWapper.faceExtractFeature(img, faceInfos.front());
    faceSDKWapper.compareTopN(features,nCompareFileAndFeatures, 5, topFaces, topScore, ui->nCompareMinScoreLineEdit->text().toFloat()*1.0/100);


    for (int i = 0; i < topScore.size(); i ++)
    {
        (nScoreLables.at(i))->setText(float2QStr(topScore.at(i)));
        updateIcorOfButton(nFacePushButtons.at(i), topFaces.at(i).file);
    }

}
