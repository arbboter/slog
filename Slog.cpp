#include "stdafx.h"
#include "Slog.h"
#include <sstream>
#include <time.h>
#include <stdarg.h>
#include <Windows.h>

using namespace std;


// 关闭特定编译警告
#pragma warning(disable:4996)


CSlog* CSlog::m_pInst = NULL;
char   g_pLogTag[][4] = {"DBG", "INF", "WRN", "ERR", "ARM", "FTL", "NUL"};


int CSlog::MakeMultiPath(const string& strPath)
{
    int nMakes = 0;
    string strCurPath;

    vector<string> vecPath;
    GetPathUnit(strPath, vecPath);

    for (size_t i=0; i<vecPath.size(); i++)
    {
        strCurPath += vecPath[i] + "/";
        if(!FileExist(strCurPath))
        {
            CreateDirectory(strCurPath.c_str(), NULL);
            nMakes += 1;
        }
    }

    return nMakes;
}

int CSlog::GetPathUnit(const string& strPath, vector<string>& vecPath)
{
    // 路径分为绝对路径和相对路径
    // 如:./log/,./../log,D:/log/20161031

    // 格式化路径，将\替换为/
    size_t nPos = 0;
    string strWinDim = "\\";
    string strFmtDim = "/";
    string strFmtPath = strPath;
    while((nPos=strFmtPath.find(strWinDim,nPos)) != string::npos)
    {
        strFmtPath.replace(nPos, strWinDim.length(), strFmtDim);
        nPos += strWinDim.length();
    }

    // 分割路径
    return Split(strFmtPath, strFmtDim, vecPath);
}

int CSlog::Split(const string& strSrc, const string& strDim, vector<string>& vecItems)
{
    size_t nPos = 0;
    size_t nLast = 0;
    while((nPos=strSrc.find(strDim,nPos)) != string::npos)
    {
        vecItems.push_back(strSrc.substr(nLast, nPos-nLast));
        nPos += strDim.length();
        nLast =  nPos;
    }
    // 处理尾巴
    size_t nLen = strSrc.size();
    if(nLast < nLen)
    {
        vecItems.push_back(strSrc.substr(nLast, nLen-nLast));
    }
    return vecItems.size();
}

void CSlog::SetCfg(const LV& lv, const string& strName, const string& strLogPath)
{
    if(!strLogPath.empty())
    {
        m_strLogPath = strLogPath;
        size_t nEnd = strLogPath.length()-1;
        if(strLogPath[nEnd] != '/' && strLogPath[nEnd]!='\\' )
        {
            m_strLogPath += "/";
        }
    }
    m_strAppName = strName;
    m_lvLog = lv;

    // 关闭文件
    Uninit();
}

CSlog* CSlog::Inst()
{
    // 非线程安全
    if(m_pInst == NULL)
    {
        m_pInst = new CSlog();
    }
    return m_pInst;
}

string CSlog::BuildInfo(const LogInfo& oInfo)
{
    stringstream ssLog;

    // [2016-10-31 15:16:52] [PTD:123,456] [FLF:main.cpp,68,GetUserName] #ERROR# This is a FATAL Log 998001
    ssLog << "[" << oInfo.strDateTime << "] "
          << "[PTD:" << oInfo.nPid << "," << oInfo.nTid << "] "
          << "[FLF:" << GetFileName(oInfo.strFile) << "," << oInfo.nLine << "," << oInfo.strFunc << "] "
          << "#" << oInfo.strTag << "# "
          << oInfo.strMsg << "\n";
    
    return ssLog.str();
}

string CSlog::BuildHeader(const string& strTag)
{
    stringstream ssLog;
    const char* pLineTag = "---------------------";

    // 日志头信息
    ssLog << pLineTag << strTag << "    " <<  __DATE__ << " " << __TIME__ << pLineTag << "\n";
    
    return ssLog.str();
}

std::string CSlog::GetDateTime(const DATE_FMT& fmt)
{
    time_t t = time(NULL); 
    char szBuf[64] = {0};
    if(fmt == DATE_FMT_LOG)
    {
        strftime(szBuf, sizeof(szBuf), "%Y%m%d %H:%M:%S",localtime(&t) ); 
    }
    else if(DATE_FMT_DATE)
    {
        strftime(szBuf, sizeof(szBuf), "%Y%m%d",localtime(&t) ); 
    }
    return szBuf;
}


void CSlog::LogFormate(LogInfo& oInfo, const LV& lv, const int nLine, const char* pFunc, const char* pFile, const char* pFmt, ...)
{
    va_list args;

    oInfo.strDateTime = GetDateTime(DATE_FMT_LOG);
    oInfo.nLogLv = lv;
    oInfo.nLine = nLine;
    oInfo.strFunc = pFunc;
    oInfo.strFile = pFile;
    oInfo.nPid = GetCurrentProcessId();
    oInfo.nTid = GetCurrentThreadId();
    oInfo.strTag = g_pLogTag[lv];

    va_start(args, pFmt);
    oInfo.strMsg = Formate(pFmt, args);
    va_end(args);
}

std::string CSlog::Formate(const char * pFmt, ...)
{
    va_list args;
    string  strRet;

    va_start(args, pFmt);
    strRet = Formate(pFmt, args);
    va_end(args);

    return strRet;
}

std::string CSlog::Formate(const char * pFmt, va_list va)
{
    char    szBuf[256] = {0};
    int     nLen = 0;
    string  strRet;

    nLen = vsprintf_s(szBuf, sizeof(szBuf)-1, pFmt, va);

    // 重试次数
    int nRetry = 0;
    // 返回负数，或者返回数值是总容纳数量
    if(nLen<0 || (nLen+1)>=sizeof(szBuf))
    {
        nRetry = 10;
        nLen = sizeof(szBuf);
    }
    else
    {
        strRet = szBuf;
    }

    // 扩大数据容量重试
    while(nRetry > 0)
    {
        nLen += nLen;
        char* pBuf = new char[nLen];
        if(pBuf == NULL)
        {
            // 内存分配失败
            break;
        }

        memset(pBuf, 0, sizeof(char)*nLen);
        int nRetLen = vsprintf_s(pBuf, nLen-1, pFmt, va);
        if(nRetLen<0 || (nRetLen+1)>=nLen)
        {
            nRetry--;
        }
        else
        {
            strRet = pBuf;
            nRetry = 0;
        }

        delete[] pBuf;
        pBuf = NULL;
    }

    return strRet;
}

size_t CSlog::WriteLog(const LV& lv, const string& strLog)
{
    if((lv<m_lvLog) || (lv>=LV_MAX))
    {
        return 0;
    }

    FILE* fp = NULL;
    LV wlv = lv;

    switch (m_fmLog)
    {
    case FM_ALL:
        {
            wlv = m_lvLog;
            break;
        }
    case FM_TREE:
    case FM_SELF:
        {
            wlv = lv;
            break;
        }
    }

    // 写当前日志
    fp= ObtainFile(wlv);
    if(fp)
    {
        Lock(wlv);
        fwrite(strLog.c_str(), strLog.length(), 1, fp);
        Unlock(wlv);
    }
    
    // 写下级日志
    if(m_fmLog == FM_TREE)
    {
        WriteLog((LV)(lv-1), strLog);
    }
    return strLog.length();
}

bool CSlog::Init()
{
    bool bRet = true;

    m_lvLog = LV_DEBUG;
    m_strLogPath = "./log/";
    m_strAppName = "slog";
    m_nMaxFileSize = 1024*1024*20;
    m_nMaxFileNum = 20;

    for (int i=0; i<=LV_MAX; i++)
    {
        CloseFile((LV)i);
    }

    return bRet;
}

bool CSlog::Uninit()
{
    bool bRet = true;

    for (int i=0; i<=LV_MAX; i++)
    {
        CloseFile((LV)i);
    }

    return bRet;
}

size_t CSlog::GetFileSize(const string& strFile)
{
    size_t nSize = 0;
#if 0
    struct _stat oInfo;
    _stat(strFile.c_str(), &oInfo);
    nSize = oInfo.st_size;
#else
    HANDLE hFile = CreateFile(strFile.c_str(), FILE_READ_EA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        nSize = ::GetFileSize(hFile, NULL);
        CloseHandle(hFile);
    }
#endif
  
    return nSize;
}

bool CSlog::RollFile(const LV& lv, int nStart)
{
    bool bRet = true;
    string strPath;
    string strNewPath;

    for (int i=m_nMaxFileNum; i>0; i--)
    {
        strPath = m_strLogPath + GetName(lv, i-1);
        strNewPath = m_strLogPath + GetName(lv, i);
        
        // 删除旧文件
        if(FileExist(strNewPath.c_str()))
        {
            DeleteFile(strNewPath.c_str());
        }

        // 复制文件
        if(FileExist(strPath.c_str()))
        {
            bRet &= (MoveFile(strPath.c_str(), strNewPath.c_str()) == TRUE);
        }
    }
    DeleteFile(strPath.c_str());
    return bRet;
}

FILE* CSlog::ObtainFile(const LV& lv)
{
    string strName = m_strLogPath + GetName(lv, 0);

    int nTryNum = 2;

    do
    {
        nTryNum -= 1;
        if(m_fpLog[lv] == NULL)
        {
            m_fpLog[lv] = fopen(strName.c_str(), "a");
            if(m_fpLog[lv] == NULL)
            {
                // 打开文件失败，尝试创建目录
                MakeMultiPath(m_strLogPath);
            }
        }
        else if(GetFileSize(strName.c_str()) >= m_nMaxFileSize)
        {
            CloseFile(lv);
            RollFile(lv);
            continue;
        }
    }while(nTryNum>0);

    return m_fpLog[lv];
}

std::string CSlog::GetName(const LV& lv, int nIdx)
{
    string strRet = m_strAppName + "_" + GetDateTime(DATE_FMT_DATE) + "_" + g_pLogTag[lv];

    if(nIdx == 0)
    {
        strRet += ".log";
    }
    else
    {
        char szBuf[64] = {0};
        sprintf_s(szBuf, sizeof(szBuf), "_%02d.log", nIdx);
        strRet += szBuf;
    }
    return strRet;
}

int CSlog::CloseFile(const LV& lv)
{
    int nRet = 0;
    if(m_fpLog[lv] != NULL)
    {
        nRet = fclose(m_fpLog[lv]);
        m_fpLog[lv] = NULL;
    }
    return nRet;
}

bool CSlog::FileExist(const string& strFile)
{
    // 复制文件
    return (INVALID_FILE_ATTRIBUTES != GetFileAttributes(strFile.c_str()));
}

void CSlog::SetFileMode(const FILE_MODE& fm)
{
    m_fmLog = fm;
}

void CSlog::SetFileRoll(const size_t& nMaxFileSize, const int& nMaxFileNum)
{
    m_nMaxFileSize = nMaxFileSize;
    m_nMaxFileNum = nMaxFileNum;
}

std::string CSlog::GetFileName(const string& strPath)
{
    string  strName;
    vector<string> vecPath;

    GetPathUnit(strPath, vecPath);
    if(vecPath.size() > 0)
    {
        strName = vecPath[vecPath.size()-1];
    }
    return strName;
}

void CSlog::Lock(int nLv)
{
    if(m_bEnableTS)
    {
        EnterCriticalSection(&m_csFile[nLv]);
    }
}

void CSlog::Unlock(int nLv)
{
    if(m_bEnableTS)
    {
        LeaveCriticalSection(&m_csFile[nLv]);
    }
}
