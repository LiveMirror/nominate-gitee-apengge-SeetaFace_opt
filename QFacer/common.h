#ifndef COMMON_H
#define COMMON_H

#include <windows.h>
#include <list>
#include <iostream>
using namespace std;

#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QTime>


#include <signal.h>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

typedef struct DiskInfo
{
    QString disk;

    float totalMB;

    float freeMB;

}DiskInfo;

typedef std::string SString;

bool isAppNameRunning(QString appName);
bool isAppPidRunning(int pid);
bool isSocketOnListenning(int port);

int getPidByListenPort(int port);

int getMemoryUsageKBByPid(int pid);


int killAppByPID(int pid);

int getPidByAppName(QString appName);


SString getFileSuffixFromPath(SString filePath);

SString getFileNameFromPath(SString filePath);

SString getParentFilePath(SString filePath);

QStringList getImagesFilesFromPath(QString path);

int readPidFromFile(const QString & pidFile, QString &pid);
int readContentFromFile(const QString & file, QStringList & content, QString::SplitBehavior behavior=QString::SkipEmptyParts);
int saveContentToFile(const QString &file, byte * buffer, int buffLen);

void printArgvs(QStringList &result, QString cmd,QStringList & grepStrs, int grepBegin = 0, int grepLength = -1);

void execShell(QStringList &result, QString cmd,QStringList & grepStrs, int grepBegin = 0, int grepLength = -1);

void execCmdWithOutResult(QString cmd);


int execCmdWithOutResultEX(void* cmd);

int getSysRegErrorReportDontOpenUI();

void updateSysRegErrorReportDontOpenU(uint newValue);

bool isSelfAutoStart();

int setSelfAutoStart(bool autoStart);

MEMORYSTATUSEX getWin_MemStatus();

double CpuUseage();

list<DiskInfo> getWin_DiskUsage();


#define ZERO_TIME QTime(0,0,0,0)

QString sayTime(QDateTime &time);

QDateTime parsTime(QString &time);


#define boolToYesOrNo(b) (b?"yes":"no")

#define f_success 0
#define f_fail -1

#define SafeDeleteObj(obj) \
{\
if (obj)\
{\
    delete obj;\
    obj = NULL;\
}\
}

#define SafeDeleteArray(array) \
{\
if (array)\
{\
    delete []array;\
    array = NULL;\
}\
};

#define SafeClearListAndDelEle(eletype, array)\
{foreach (eletype * temp, array) {\
    SafeDeleteObj(temp);\
}\
array.clear();}

#define SafeCloseFile(file)\
{\
    if(file)\
    {\
        fflush(file);\
        fclose(file);\
        file = NULL;\
    }\
};

#define QStr2CStr(qstr) qstr.toStdString().c_str()

#define float2QStr(value) QString("%1").arg(value)

SString float2SString(float value);

#define int2QStr(value) QString("%1").arg(value)

#define boolTo0Or1(b) (b?1:0)

#define intToSuccessOrFail(value) (value ? "成功" : "失败")

#endif // COMMON_H
