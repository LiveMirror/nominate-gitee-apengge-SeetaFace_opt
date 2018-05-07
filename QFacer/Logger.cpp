
#include "Logger.h"
#include <sys/stat.h>
#include <io.h>
#include <direct.h>

Logger* Logger::instance = NULL;
LoggerLevel Logger::logLevel = LoggerLevelDebug;

#define ACCESS _access
#define MKDIR(a) _mkdir((a))


int CreatDir(const char *pDir)
{
    int i = 0;
    int iRet;
    int iLen;
    char* pszDir;

    if (NULL == pDir)
    {
        return 0;
    }

    pszDir = _strdup(pDir);
    iLen = strlen(pszDir);

    // 创建中间目录
    for (i = 0; i < iLen; i++)
    {
        if (pszDir[i] == '\\' || pszDir[i] == '/')
        {
            pszDir[i] = '\0';

            //如果不存在, 创建
            iRet = ACCESS(pszDir, 0);
            if (iRet != 0)
            {
                iRet = MKDIR(pszDir);
                if (iRet != 0)
                {
                    return -1;
                }
            }
            //支持linux, 将所有\换成/
            pszDir[i] = '/';
        }
    }

    iRet = MKDIR(pszDir);
    free(pszDir);
    return iRet;
}

BOOL FindFirstFileExists(LPCTSTR lpPath, DWORD dwFilter)
{
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(lpPath, &fd);
    BOOL bFilter = (FALSE == dwFilter) ? TRUE : fd.dwFileAttributes & dwFilter;
    BOOL RetValue = ((hFind != INVALID_HANDLE_VALUE) && bFilter) ? TRUE : FALSE;
    FindClose(hFind);
    return RetValue;
}


BOOL FilePathExists(QString filePath)
{


    WCHAR * wFilePath = new WCHAR[filePath.length() + 1];
    memset(wFilePath, 0, (filePath.size() + 1) * sizeof(WCHAR));
    filePath.toWCharArray(wFilePath);
    BOOL ret =  FindFirstFileExists(wFilePath, FALSE);

    SafeDeleteArray(wFilePath);
    return ret;
}

SString getBufferPrintInfo(const void* buffer, int length)
{
	unsigned char * _buffer = (unsigned char*)buffer;
	char out[20000] = { 0 };

	char temp[50] = { 0 };

	unsigned char oneLine[100] = { 0 };

	if (length > 2000)
	{
		length = 2000;
	}
	if (buffer && length > 0)
	{
		int i = 0;
		for (i; i < length; i++)
		{
			if (i % 16 == 0)
			{
				if (i > 0)
				{
					strcat(out, "|");
				}
				if (strlen((char *)oneLine) > 0)
				{
					strcat(out, (char *)oneLine);
				}

				strcat(out, "\n");
				sprintf(temp, "%04d:", i);
				strcat(out, temp);
				memset(oneLine, 0, 100);
			}

			sprintf(temp, "  %02x", _buffer[i]);
			oneLine[i % 16] = _buffer[i];
			if ((unsigned char)oneLine[i % 16] < (unsigned char)32 || (unsigned char)oneLine[i % 16] > (unsigned char)128)
			{
				oneLine[i % 16] = '.';
			}
			strcat(out, temp);
		}

		int tail = length % 16;
		//先补充16 - tail个空字符串
		for (int i = 0; i < 16 - tail; i++)
		{
			strcat(out, "    ");
		}

		strcat(out, "|");
		memset(oneLine, 0, 100);

		for (int i = 0; i < tail; i++)
		{
			oneLine[i] = _buffer[length - tail + i];
			if ((unsigned char)oneLine[i] < (unsigned char)32 || (unsigned char)oneLine[i] > (unsigned char)128)
			{
				oneLine[i] = '.';
			}
		}

		strcat(out, (char *)oneLine);

		return out;
	}
	else
	{
		return "";
	}
}



SString getCurrentTimeInChina()
{
	SYSTEMTIME st;

	GetLocalTime(&st);

	char strDate[100] = {0};

	_snprintf(strDate,99, "%04d-%02d-%02d %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    return strDate;
}

Logger * Logger::sharedInstance()
{
	if (Logger::instance == NULL)
	{
		Logger::instance = new Logger();
	}

	return Logger::instance;
}
void Logger::cfg(LoggerLevel logLevel, QString logFile, int maxFileLenMB, int maxFileCount, LoggerMode mode)
{
	this->logLevel = logLevel;
	this->logFile = logFile;

    CreatDir(getParentFilePath(logFile.toStdString()).c_str());

	this->maxFileLength = maxFileLength;
	this->maxFileCount = maxFileCount;

	this->mode = mode;

	if (isSave2File())
	{
		doTransfer();
	}
}


LoggerLevel Logger::getLogLevel()
{
	return Logger::logLevel;
}

QString Logger::getLogFile()
{
	return this->logFile;
}

FILE * Logger::getFLogFile()
{
	return fLogFile;
}

bool Logger::isCfged()
{

	return logFile.size() > 0 && logLevel > LoggerLevelDefalt && fLogFile != NULL;
}


void Logger::fileSizeIncreased(int size)
{

	currentFileSize = currentFileSize + size;
	checkSize();

}

void Logger::checkSize()
{
	if (currentFileSize > maxFileLength*1024*1024)
	{
		doTransfer();
	}
}

bool Logger::isSave2File()
{
    return mode == LoggerModeOnlyFile || mode == LoggerModeOnlyBothFileAndQDebug;
}

bool Logger::isSave2QDebug()
{
    return mode == LoggerModeOnlyQDebug || mode == LoggerModeOnlyBothFileAndQDebug;
}

void Logger::doTransfer()
{
	if (fLogFile)
	{
		fflush(fLogFile);
		fclose(fLogFile);
		fLogFile = NULL;
	}

    SString fileName =  getLogFile().toStdString();

	for (int i = maxFileCount; i >= 2; i--)
	{
		
		char temp[20] = { '\0' };

		sprintf_s(temp, 19, "%d", i);

		char temp2[20] = { '\0' };

		sprintf_s(temp2, 19, "%d", i - 1);


        SString iFileName = fileName;
		iFileName.insert(iFileName.find_last_of('.'), temp);

        SString i_1FileName = fileName;
		i_1FileName.insert(i_1FileName.find_last_of('.'), temp2);

		//第一步，删除掉iFileName，将i_1FileName重命名为iFileName
        if (FilePathExists(QString::fromStdString(iFileName)))
		{
			remove(iFileName.c_str());
		}
		
        if (FilePathExists(QString::fromStdString(i_1FileName)))
		{
			rename(i_1FileName.c_str(), iFileName.c_str());
		}
	}

    SString fileName1 = fileName;
	fileName1.insert(fileName1.find_last_of('.'), "1");

    fopen_s(&(this->fLogFile), fileName1.c_str(), "ab+");

	currentFileSize = 0;
}

Logger::Logger()
{
    logFile = "";
	logLevel = LoggerLevelDefalt;
	fLogFile = NULL;

	maxFileLength = 0;

	maxFileCount = 0;

	currentFileSize = 0;

    oneLog[0] = '\0';

}


Logger::~Logger()
{
	if (fLogFile != NULL)
	{
		fclose(fLogFile);
	}
}
