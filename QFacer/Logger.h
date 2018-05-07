#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <String>
#include <QString>
#include <QMutex>
#include <QDebug>
#include "common.h"




typedef enum {
	LoggerLevelDefalt = -1, 
	LoggerLevelTrace = 0, 
	LoggerLevelDebug, 
	LoggerLevelInfo, 
	LoggerLevelWarn, 
	LoggerLevelError, 
	LoggerLevelFatal, 

}LoggerLevel;


typedef enum {
	LoggerModeOnlyFile = 1,
    LoggerModeOnlyQDebug = 2,
    LoggerModeOnlyBothFileAndQDebug = 3
}LoggerMode;

SString getCurrentTimeInChina();


BOOL FilePathExists(QString filePath);




int CreatDir(const char *pDir);


class Logger
{
public:
	Logger();
	~Logger();

	static Logger * sharedInstance();

    void cfg(LoggerLevel logLevel, QString logFile, int maxFileLenMB, int maxFileCount, LoggerMode mode = LoggerModeOnlyFile);

	LoggerLevel getLogLevel();

    QString getLogFile();

	FILE* getFLogFile();

	bool isCfged();

	void fileSizeIncreased(int size);

	void doTransfer(); 

	bool isSave2File();

    bool isSave2QDebug();

private:
	void checkSize();

public:
	static LoggerLevel logLevel;

    QMutex criticalSection;

    char oneLog[10000];
private:
	static Logger * instance;

    QString logFile;

	FILE * fLogFile;

	int maxFileLength;

	int maxFileCount;

	int currentFileSize;



	LoggerMode mode;


};


#define LOG_TRACE(FMT, ...) \
{\
    if (Logger::sharedInstance()->isCfged()  && Logger::sharedInstance()->getLogLevel() <= LoggerLevelTrace && (Logger::sharedInstance()->isSave2File() || Logger::sharedInstance()->isSave2QDebug())) {\
        Logger::sharedInstance()->criticalSection.lock();\
        _snprintf(Logger::sharedInstance()->oneLog,9999,"%s p[%d]*TRACE|: function: \"%s()\" file: \"%s\" line: \"%u\" MSG: " FMT "\n", getCurrentTimeInChina().c_str(), ::GetCurrentThreadId(), __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);\
        if(Logger::sharedInstance()->isSave2File() ){\
            FILE * f = Logger::sharedInstance()->getFLogFile(); \
            if (f)\
            {\
                Logger::sharedInstance()->fileSizeIncreased(fprintf(f, "%s", Logger::sharedInstance()->oneLog)); \
                fflush(f);\
            }\
        }\
        if(Logger::sharedInstance()->isSave2QDebug())\
        {\
            qDebug()<<Logger::sharedInstance()->oneLog;\
        }\
        Logger::sharedInstance()->criticalSection.unlock();\
    }\
}


#define LOG_DEBUG(FMT, ...) \
{\
    if (Logger::sharedInstance()->isCfged()  && Logger::sharedInstance()->getLogLevel() <= LoggerLevelDebug && (Logger::sharedInstance()->isSave2File() || Logger::sharedInstance()->isSave2QDebug())) {\
        Logger::sharedInstance()->criticalSection.lock();\
        _snprintf(Logger::sharedInstance()->oneLog,9999,"%s p[%d]*DEBUG|: function: \"%s()\" file: \"%s\" line: \"%u\" MSG: " FMT "\n", getCurrentTimeInChina().c_str(), ::GetCurrentThreadId(), __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);\
        if(Logger::sharedInstance()->isSave2File() ){\
            FILE * f = Logger::sharedInstance()->getFLogFile(); \
            if (f)\
            {\
                Logger::sharedInstance()->fileSizeIncreased(fprintf(f, "%s", Logger::sharedInstance()->oneLog)); \
                fflush(f);\
            }\
        }\
        if(Logger::sharedInstance()->isSave2QDebug())\
        {\
            qDebug()<<Logger::sharedInstance()->oneLog;\
        }\
        Logger::sharedInstance()->criticalSection.unlock();\
    }\
}

#define LOG_INFO(FMT, ...) \
{\
    if (Logger::sharedInstance()->isCfged()  && Logger::sharedInstance()->getLogLevel() <= LoggerLevelInfo && (Logger::sharedInstance()->isSave2File() || Logger::sharedInstance()->isSave2QDebug())) {\
        Logger::sharedInstance()->criticalSection.lock();\
        _snprintf(Logger::sharedInstance()->oneLog,9999,"%s p[%d]*INFO: function: \"%s()\" file: \"%s\" line: \"%u\" MSG: " FMT "\n", getCurrentTimeInChina().c_str(), ::GetCurrentThreadId(), __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);\
        if(Logger::sharedInstance()->isSave2File() ){\
            FILE * f = Logger::sharedInstance()->getFLogFile(); \
            if (f)\
            {\
                Logger::sharedInstance()->fileSizeIncreased(fprintf(f, "%s", Logger::sharedInstance()->oneLog)); \
                fflush(f);\
            }\
        }\
        if(Logger::sharedInstance()->isSave2QDebug())\
        {\
            qDebug()<<Logger::sharedInstance()->oneLog;\
        }\
        Logger::sharedInstance()->criticalSection.unlock();\
    }\
}
#define LOG_WARN(FMT, ...) \
{\
    if (Logger::sharedInstance()->isCfged()  && Logger::sharedInstance()->getLogLevel() <= LoggerLevelWarn && (Logger::sharedInstance()->isSave2File() || Logger::sharedInstance()->isSave2QDebug())) {\
        Logger::sharedInstance()->criticalSection.lock();\
        _snprintf(Logger::sharedInstance()->oneLog,9999,"%s p[%d]*WARN: function: \"%s()\" file: \"%s\" line: \"%u\" MSG: " FMT "\n", getCurrentTimeInChina().c_str(), ::GetCurrentThreadId(), __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);\
        if(Logger::sharedInstance()->isSave2File() ){\
            FILE * f = Logger::sharedInstance()->getFLogFile(); \
            if (f)\
            {\
                Logger::sharedInstance()->fileSizeIncreased(fprintf(f, "%s", Logger::sharedInstance()->oneLog)); \
                fflush(f);\
            }\
        }\
        if(Logger::sharedInstance()->isSave2QDebug())\
        {\
            qDebug()<<Logger::sharedInstance()->oneLog;\
        }\
        Logger::sharedInstance()->criticalSection.unlock();\
    }\
}


#define LOG_ERROR(FMT, ...) \
{\
    if (Logger::sharedInstance()->isCfged()  && Logger::sharedInstance()->getLogLevel() <= LoggerLevelError && (Logger::sharedInstance()->isSave2File() || Logger::sharedInstance()->isSave2QDebug())) {\
        Logger::sharedInstance()->criticalSection.lock();\
        _snprintf(Logger::sharedInstance()->oneLog,9999,"%s p[%d]*ERROR: function: \"%s()\" file: \"%s\" line: \"%u\" MSG: " FMT "\n", getCurrentTimeInChina().c_str(), ::GetCurrentThreadId(), __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);\
        if(Logger::sharedInstance()->isSave2File() ){\
            FILE * f = Logger::sharedInstance()->getFLogFile(); \
            if (f)\
            {\
                Logger::sharedInstance()->fileSizeIncreased(fprintf(f, "%s", Logger::sharedInstance()->oneLog)); \
                fflush(f);\
            }\
        }\
        if(Logger::sharedInstance()->isSave2QDebug())\
        {\
            qDebug()<<Logger::sharedInstance()->oneLog;\
        }\
        Logger::sharedInstance()->criticalSection.unlock();\
    }\
}

#define LOG_FATAL(FMT, ...) \
{\
    if (Logger::sharedInstance()->isCfged()  && Logger::sharedInstance()->getLogLevel() <= LoggerLevelFatal && (Logger::sharedInstance()->isSave2File() || Logger::sharedInstance()->isSave2QDebug())) {\
        Logger::sharedInstance()->criticalSection.lock();\
        _snprintf(Logger::sharedInstance()->oneLog,9999,"%s p[%d]*FALTAL: function: \"%s()\" file: \"%s\" line: \"%u\" MSG: " FMT "\n", getCurrentTimeInChina().c_str(), ::GetCurrentThreadId(), __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);\
        if(Logger::sharedInstance()->isSave2File() ){\
            FILE * f = Logger::sharedInstance()->getFLogFile(); \
            if (f)\
            {\
                Logger::sharedInstance()->fileSizeIncreased(fprintf(f, "%s", Logger::sharedInstance()->oneLog)); \
                fflush(f);\
            }\
        }\
        if(Logger::sharedInstance()->isSave2QDebug())\
        {\
            qDebug()<<Logger::sharedInstance()->oneLog;\
        }\
        Logger::sharedInstance()->criticalSection.unlock();\
    }\
}


SString getBufferPrintInfo(const void* buffer, int length);

#define LOG_PRINT_BUFFER(buffer, length) LOG_INFO("%s", getBufferPrintInfo(buffer,length).c_str())

#endif


