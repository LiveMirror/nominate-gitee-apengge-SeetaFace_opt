#include "common.h"

#include <QProcess>
#include "Logger.h"
#include <QFile>
#include <QSettings>
#include "SimpleRunable.h"
#include <QApplication>
#include <QDir>
#include <QDirIterator>

#include <stdlib.h>

#define SYS_REG_RUN "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"

bool isAppNameRunning(QString appName)
{
    QString cmd = "tasklist";

    QStringList greps;

    greps << appName + " ";

    QStringList result;
    execShell(result, cmd, greps);
    bool ret = (result.size() > 0);
    if(ret)
    {
        LOG_INFO("appName:%s is running", QStr2CStr(appName));
    }
    else
    {
        LOG_INFO("appName:%s is not running", QStr2CStr(appName));
    }

    return ret;
}


bool isAppPidRunning(int pid)
{
    QString cmd = "tasklist";

    QStringList greps;

    greps << int2QStr(pid) + " ";

    QStringList result;
    execShell(result, cmd, greps, 0, 35);
    bool ret = (result.size() > 0);
    if(ret)
    {
        LOG_INFO("app pid:%d is running", pid);
    }
    else
    {
        LOG_INFO("app pid:%d is not running", pid);
    }

    return ret;
}

bool isSocketOnListenning(int port)
{
    //QString cmd = "netstat -an -o -p tcp";
    QString cmd = "netstat -ano -p tcp";
    QStringList greps;
    greps << ":" + int2QStr(port) << "LISTENING";

    QStringList result;
    execShell(result, cmd, greps);
    bool ret = (result.size() > 0);
    if(ret)
    {
        LOG_INFO("port:%d is on listenning", port);
    }
    else
    {
        LOG_INFO("port:%d isnot on listenning", port);
    }

    return ret;
}

void printArgvs(QStringList & result, QString cmd,QStringList & grepStrs,int grepBegin, int grepLength)
{
    result.clear();

    QProcess p(0);
    p.start(cmd);
    p.waitForStarted();
    p.waitForFinished(-1);
    QString cmdOutput =  QString::fromLocal8Bit(p.readAllStandardOutput());
    LOG_INFO("argvs:\n%s", QStr2CStr(cmdOutput));
}

void execShell(QStringList & result, QString cmd,QStringList & grepStrs,int grepBegin, int grepLength)
{
    result.clear();

    QProcess p(0);
    p.start(cmd);
    p.waitForStarted();
    p.waitForFinished(-1);
    QString cmdOutput =  QString::fromLocal8Bit(p.readAllStandardOutput());
    LOG_INFO("cmd:%s with output:%s", QStr2CStr(cmd), QStr2CStr(cmdOutput));
    result = cmdOutput.split("\n", QString::SkipEmptyParts);
    for (int i = 0; i < grepStrs.size(); i++)
    {
        const QString & grepStr = grepStrs.at(i);
        for (int j = result.size() -1; j >= 0; j--)
        {
            const QString &oneResult = result.at(j);

            QString subStr = oneResult.mid(grepBegin, grepLength);

            if(subStr.indexOf(grepStr) < 0)
            {
                result.removeAt(j);
            }
        }
    }
    LOG_INFO("try to start cmd:%s, after greped  with strings:%s, result:%s", QStr2CStr(cmd), QStr2CStr(grepStrs.join("|")), QStr2CStr(result.join("\n")));
}

int execCmdWithOutResultEX(void* cmd)
{

    QString* strCmd = (QString  *)cmd;
    LOG_INFO("exec cmd:%s without result", QStr2CStr((*strCmd)));
    QProcess *p = new QProcess(0);
    p->startDetached("cmd", QStringList()<<"/c"<<*strCmd);

    SafeDeleteObj(strCmd);

    return f_success;
}

void execCmdWithOutResult(QString cmd)
{
    QString *strCmd = new QString(cmd);
    (new SimpleRunable(execCmdWithOutResultEX, strCmd))->run();
}

QString sayTime(QDateTime &time)
{
    return time.toString("yyyy-MM-dd hh:mm:ss:zzz");
}

QDateTime parsTime(QString &time)
{
    return QDateTime::fromString(time, "yyyy-MM-dd hh:mm:ss:zzz");
}

int readPidFromFile(const QString & pidFile, QString &pid)
{
    QStringList content;
    if(f_success == readContentFromFile(pidFile, content, QString::SkipEmptyParts))
    {
        if(content.length() > 0)
        {
            pid =  content.at(0);
            return f_success;
        }
        else
        {
            LOG_ERROR("pidFile is empty");
            return f_fail;
        }
    }
    else
    {
        return f_fail;
    }
}

int readContentFromFile(const QString & file, QStringList & content, QString::SplitBehavior behavior)
{
    content.clear();
    QFile f(file);
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        LOG_ERROR("try to open file:%s with readonly and text mode failed", QStr2CStr(file));
        return f_fail;
    }
    else
    {
        QTextStream txtInput(&f);
        QString lineStr;
        while(!txtInput.atEnd())
        {
            lineStr = txtInput.readLine();
            if(behavior == QString::SkipEmptyParts && lineStr.length() <= 0)
            {
                continue;
            }
            else
            {
               content << lineStr;
            }
        }

        f.close();
    }

    return f_success;
}


int saveContentToFile(const QString &file, byte *buffer, int buffLen)
{
    QFile f(file);
    if(!f.open(QIODevice::WriteOnly ))
    {
        LOG_ERROR("try to open file:%s with WriteOnly", QStr2CStr(file));
        return f_fail;
    }
    else
    {
        QByteArray array((char *)buffer, buffLen);
        f.write(array);
        f.close();
    }

    return f_success;
}

int getSysRegErrorReportDontOpenUI()
{
    int ret = -1;

    QSettings *reg = new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\Windows\ Error\ Reporting",
    QSettings::NativeFormat);
    ret = reg->value("DontShowUI",0).toUInt();
    SafeDeleteObj(reg);

//    HKEY hKEY;
//    if (ERROR_SUCCESS == ::RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\Windows\ Error\ Reporting", 0, KEY_READ | KEY_WRITE, &hKEY))
//    {
//        DWORD dwValue;
//        DWORD dwSize = sizeof(DWORD);
//        DWORD dwType = REG_DWORD;
//        if (::RegQueryValueExA(hKEY, "DontShowUI", 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
//        {
//            LOG_INFO("got the error report DontShowUI sys reg value:%ul success", dwValue);

//            ret = dwValue;
//        }
//        else
//        {
//            LOG_ERROR("get the error report DontShowUI sys reg value falied");
//        }

//        ::RegCloseKey(hKEY);
//    }
//    else
//    {
//        LOG_ERROR("try to open sysreg:%s failed", "HKEY_CURRENT_USER\Software\ Microsoft\Windows\Windows Error Reporting\DontShowUI");
//    }

    return ret;
}

void updateSysRegErrorReportDontOpenU(uint newValue)
{
    QSettings *reg = new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\Windows\ Error\ Reporting",
    QSettings::NativeFormat);
    reg->setValue("DontShowUI", newValue);

}

//windows 内存 使用率
MEMORYSTATUSEX getWin_MemStatus(){
    MEMORYSTATUSEX ms;
     ms.dwLength = sizeof (ms);
    ::GlobalMemoryStatusEx(&ms);
    LOG_INFO("got one mem status, dwTotalPhys:%u,dwAvailPhys:%u, dwMemoryLoad:%ld", ms.ullTotalPhys, ms.ullAvailPhys, ms.dwMemoryLoad);
    return ms;
}

LONGLONG CompareFileTime2(FILETIME time1, FILETIME time2)
{
    LONGLONG a = ((LONGLONG)time1.dwHighDateTime << 32) | time1.dwLowDateTime;
    LONGLONG b = ((LONGLONG)time2.dwHighDateTime << 32) | time2.dwLowDateTime;


    return   b - a;
}

QStringList getImagesFilesFromPath(QString path)
{
    //判断路径是否存在
    QDir dir(path);
    if(!dir.exists())
    {
        return QStringList();
    }

    //获取所选文件类型过滤器
    QStringList filters;
    //文件过滤
    filters<<QString("*.jpeg")<<QString("*.jpg")<<QString("*.png")<<QString("*.bmp");

    //定义迭代器并设置过滤器
    QDirIterator dir_iterator(path,
        filters,
        QDir::Files | QDir::NoSymLinks,
        QDirIterator::Subdirectories);
    QStringList string_list;
    while(dir_iterator.hasNext())
    {
        dir_iterator.next();
        QFileInfo file_info = dir_iterator.fileInfo();
        QString absolute_file_path = file_info.absoluteFilePath();
        string_list.append(absolute_file_path);
    }
    return string_list;
}

double CpuUseage()
{
    FILETIME m_preidleTime;
    FILETIME m_prekernelTime;
    FILETIME m_preuserTime;

    GetSystemTimes(&m_preidleTime, &m_prekernelTime, &m_preuserTime);
    Sleep(1000);

    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;
    GetSystemTimes(&idleTime, &kernelTime, &userTime);


    int idle = CompareFileTime2(m_preidleTime, idleTime);
    int kernel = CompareFileTime2(m_prekernelTime, kernelTime);
    int user = CompareFileTime2(m_preuserTime, userTime);


    if (kernel + user == 0)
        return 0.0;
    //（总的时间-空闲时间）/总的时间=占用cpu的时间就是使用率
    double cpu = abs((kernel + user - idle) * 100 / (kernel + user));

    m_preidleTime = idleTime;
    m_prekernelTime = kernelTime;
    m_preuserTime = userTime;
    LOG_INFO("cpu:%f", cpu);
    return cpu;
}

//获取 WIN 硬盘使用情况
list<DiskInfo> getWin_DiskUsage(){
    list<DiskInfo> ret;

    int DiskCount = 0;
    DWORD logicalDrives = GetLogicalDrives();
    //利用GetLogicalDrives()函数可以获取系统中逻辑驱动器的数量，函数返回的是一个32位无符号整型数据。
    while (logicalDrives)//通过循环操作查看每一位数据是否为1，如果为1则磁盘为真,如果为0则磁盘不存在。
    {
        if (logicalDrives & 1)//通过位运算的逻辑与操作，判断是否为1
        {
            ++DiskCount;
        }
        logicalDrives = logicalDrives >> 1;//通过位运算的右移操作保证每循环一次所检查的位置向右移动一位。
        //DiskInfo = DiskInfo/2;
    }
    //-----------------------------------------------------------------------------------------
    int DSLength = GetLogicalDriveStringsA(0, NULL);
    //通过GetLogicalDriveStrings()函数获取所有驱动器字符串信息长度。
    char* DStr = new char[DSLength];//用获取的长度在堆区创建一个c风格的字符串数组
    GetLogicalDriveStringsA(DSLength, DStr);
    //通过GetLogicalDriveStrings将字符串信息复制到堆区数组中,其中保存了所有驱动器的信息。

    int DType;
    int si = 0;
    BOOL fResult;
    unsigned long long i64FreeBytesToCaller;
    unsigned long long i64TotalBytes;
    unsigned long long i64FreeBytes;

    for (int i = 0; i<DSLength / 4; ++i)//为了显示每个驱动器的状态，则通过循环输出实现，由于DStr内部保存的数据是A:\NULLB:\NULLC:\NULL，这样的信息，所以DSLength/4可以获得具体大循环范围
    {
        char dir[4] = { DStr[si], ':', '\\', '\0' };
        DType = GetDriveTypeA(DStr + i * 4);
        //GetDriveType函数，可以获取驱动器类型，参数为驱动器的根目录
        if (DType == DRIVE_FIXED)
        {
            fResult = GetDiskFreeSpaceExA(
                dir,
                (PULARGE_INTEGER)&i64FreeBytesToCaller,
                (PULARGE_INTEGER)&i64TotalBytes,
                (PULARGE_INTEGER)&i64FreeBytes);
            DiskInfo info;
            info.disk = dir;
            info.totalMB = (float)i64TotalBytes / 1024 / 1024;
            info.freeMB =  (float)i64FreeBytesToCaller / 1024 / 1024 ;
            ret.push_back(info);
            LOG_INFO("disk:%s, totalMB:%f, freeMB:%f", QStr2CStr(info.disk), info.totalMB, info.freeMB);
        }

        si += 4;
    }

    return ret;
}

bool isSelfAutoStart()
{
//    QString application_name = QApplication::applicationName();
//    QSettings *settings = new QSettings(SYS_REG_RUN, QSettings::NativeFormat);
//    bool ret = settings->contains(application_name);
//    delete settings;
//    return ret;
    QString startBatFile = "C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\StartUp\\watchDogStart.bat";

    QFile file(startBatFile);
    if(file.exists())
    {

        QStringList fileContent;
        if(f_success == readContentFromFile(startBatFile, fileContent) && fileContent.size() > 0
                && fileContent.at(0).compare("start " + QCoreApplication::applicationFilePath()) == 0)
        {
            return true;
        }
        else
        {
            LOG_ERROR("start bat file:%s exist, but content is invalid, will remove this file");
            file.remove();
            return false;
        }
    }
    else
    {
        return false;
    }
}

int setSelfAutoStart(bool autoStart)
{
//    QString application_name = QApplication::applicationName();
//    QSettings *settings = new QSettings(SYS_REG_RUN, QSettings::NativeFormat);
//    if(autoStart)
//    {
//        QString application_path = QApplication::applicationFilePath();
//        settings->setValue(application_name, application_path.replace("/", "\\"));
//    }
//    else
//    {
//        settings->remove(application_name);
//    }
//    delete settings;

    QString startBatFile = "C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\StartUp\\watchDogStart.bat";

    QFile file(startBatFile);

    if(autoStart)
    {
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            LOG_ERROR("can't open start bat file:%s", QStr2CStr(startBatFile));
            return f_fail;
        }
        else
        {
            QTextStream out(&file);
            out<< "start " + QCoreApplication::applicationFilePath() <<endl;
            out.flush();
        }
        file.close();
    }
    else
    {
        if(file.exists())
        {
            file.remove();
        }

    }

    return f_success;
}

int getPidByListenPort(int port)
{
    QString cmd = "netstat -an -o -p tcp";
    QStringList greps;
    greps << ":" + int2QStr(port) + " " << "LISTENING";

    QStringList result;
    execShell(result, cmd, greps);
    bool ret = (result.size() > 0);
    if(ret)
    {
        QString line = result.front();
        QString pid = line.mid(line.lastIndexOf("LISTENING") + strlen("LISTENING")+1).trimmed();
        LOG_INFO("port:%d is on listenning, pid = %s",port,QStr2CStr(pid));
        return pid.toInt();
    }
    else
    {
        LOG_INFO("port:%d isnot on listenning", port);
        return -1;
    }

}

int getMemoryUsageKBByPid(int pid)
{
    QString cmd = " tasklist /FO CSV /FI \"PID eq " + int2QStr(pid) + "\"";
    QStringList greps;
    greps << "\"" + int2QStr(pid) + "\"";

    QStringList result;
    execShell(result, cmd, greps);
    bool ret = (result.size() > 0);
    if(ret)
    {
        QString line = result.front();
        QString usageKB = line.split("\",\"").last();


        usageKB.replace("\"", "");
        usageKB.replace(" ", "");
        usageKB.replace("k", "");
        usageKB.replace("K", "");
        usageKB.replace(",", "");
        usageKB.replace("\r","");
        usageKB.replace("\n","");
        LOG_INFO("pid:%d's memory usage is %skb",pid,QStr2CStr(usageKB));
        return usageKB.toInt();
    }
    else
    {
        LOG_INFO("pid:%d is not alive", pid);
        return -1;
    }
}

int killAppByPID(int pid)
{
    LOG_INFO("kill app by pid:%d", pid);
    if(pid >= 0)
    {
        QString cmd = "taskkill /PID  " + int2QStr(pid);
        execCmdWithOutResultEX(new QString(cmd));

        Sleep(8000);
        if(isAppPidRunning(pid))
        {
            LOG_INFO("8 seconds after kill not by force, pid:%d is still running, will kill it by force");
            QString cmd = "taskkill /F /PID  " + int2QStr(pid);
            execCmdWithOutResultEX(new QString(cmd));
            while(isAppPidRunning(pid))
            {
                Sleep(2);
                LOG_INFO("after kill by force, pid:%d is still running, wait until it exist", pid);
            }
        }
    }

    return f_success;
}

int getPidByAppName(QString appName)
{
    QString cmd = "tasklist";

    QStringList greps;

    greps << appName + " ";

    QStringList result;
    execShell(result, cmd, greps);
    bool ret = (result.size() > 0);
    if(ret)
    {
        QString line = result.front();
        int pid = atoi(QStr2CStr(line.mid(appName.length()).trimmed()));
        LOG_INFO("appName:%s is running, pid:%d", QStr2CStr(appName), pid);
        return pid;
    }
    else
    {
        LOG_INFO("appName:%s is not running", QStr2CStr(appName));
        return -1;
    }
}


SString getFileNameFromPath(SString filePath)
{
    int iIndex1 = filePath.find_last_of('/');
    int iIndex2 = filePath.find_last_of('\\');

    int iIndex = iIndex1;
    if (iIndex1 == SString::npos || (iIndex1 != SString::npos && iIndex2 != SString::npos && iIndex2 > iIndex1))
    {
        iIndex = iIndex2;
    }

    if (iIndex == SString::npos || iIndex >= filePath.size() - 1)
    {
        return "";
    }
    else
    {
        return filePath.substr(iIndex + 1);
    }
}

SString getParentFilePath(SString filePath)
{

    SString temp = filePath;
    if (temp.length() > 0)
    {
        if (temp[temp.length() - 1] == '/' || temp[temp.length() - 1] == '\\')
        {
            temp.erase(temp.length() - 1, 1);
        }

        if (temp.length() > 0)
        {
            int iIndex1 = temp.find_last_of('/');
            int iIndex2 = temp.find_last_of('\\');

            int iIndex = iIndex1;
            if (iIndex1 == SString::npos || (iIndex1 != SString::npos && iIndex2 != SString::npos && iIndex2 > iIndex1))
            {
                iIndex = iIndex2;
            }
            if (iIndex == SString::npos)
            {
                return "";
            }
            else
            {
                return temp.substr(0, iIndex);
            }
        }
    }

    return "";

}



SString float2SString(float value)
{
    char ret[50] = {'\0'};
    sprintf(ret, "%f", value);
    return ret;
}
