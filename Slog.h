#ifndef __SLOG_H_20161031__
#define __SLOG_H_20161031__

#include <string>
#include <vector>
#include <stdarg.h>
#include <Windows.h>

using namespace std;
class CSlog
{
public:
    enum LV
    {
        LV_DEBUG    = 0,
        LV_INFO     = 1,
        LV_WARN     = 2,
        LV_ERROR    = 3,
        LV_ALARM    = 4,
        LV_FATAL    = 5,

        LV_MAX,
    };

    enum FILE_MODE
    {
        FM_CURRENT,     // 日志文件仅记录等于当前日志等级的日志
        FM_EVERY,       // 日志文件记录所有大于等于当前日志等级的日志
    };

    enum DATE_FMT
    {
        DATE_FMT_LOG,
        DATE_FMT_DATE,
    };
    
    // 日志信息
    typedef struct _log_info_
    {
        unsigned int    nTid;
        unsigned int    nPid;
        unsigned int    nLine;
        int             nLogLv;
        string          strFile;
        string          strFunc;
        string          strTag;
        string          strMsg;
        string          strDateTime;
    }LogInfo;

public:
    // 文件是否存在
    bool FileExist(const string& strFile);
    // 获取文件大小
    size_t GetFileSize(const string& strFile);
    // 创建多级目录
    int MakeMultiPath(const string& strPath);
    // 获取路径单元，分割路径
    int GetPathUnit(const string& strPath, vector<string>& vecPath);
    // 字符串分割 
    int Split(const string& strSrc, const string& strDim, vector<string>& vecItems);
    // 获取当前时间
    string GetDateTime(const DATE_FMT& fmt);
    // 格式化字符串到string
    string Formate(const char * pFmt, ...);
    string Formate(const char * pFmt, va_list va);

public:
    // 对外提供的参数调整设置
    void    SetCfg(const LV& lv, const string& strName, const string& strPath="");
    void    SetFileMode(const FILE_MODE& fm);
    void    SetFileRoll(const size_t& nMaxFileSize, const int& nMaxFileNum);
    static  CSlog*  Inst();

public:
    // 生成日志信息
    void    LogFormate(LogInfo& oInfo, const LV& lv, const int nLine, const char* pFunc, const char* pFile, const char* pFmt, ...);
    string  BuildInfo(const LogInfo& oInfo);
    string  BuildHeader(const string& strTag);
    // 写日志
    size_t  WriteLog(const LV& lv, const string& strLog);

protected:
    // 日志滚动
    bool    RollFile(const LV& lv, int nStart=0);
    // 获取写入文件
    FILE*   ObtainFile(const LV& lv);
    // 获取日志文件名
    string GetName(const LV& lv, int nIdx);
    int    CloseFile(const LV& lv);


private:
    CSlog()
    {
        for (int i=0; i<=LV_MAX; i++)
        {
            m_fpLog[i] = NULL;
        }

        Init();
        m_bMakeDir = false;
        m_fmLog = FM_EVERY;
    }

    ~CSlog(void)
    {
        Uninit();
    }

    bool Init();
    bool Uninit();

private:
    static CSlog*  m_pInst;

protected:
    CRITICAL_SECTION    m_csFile[LV_MAX+1];
    FILE*               m_fpLog[LV_MAX+1];
    LV                  m_lvLog;
    string              m_strLogPath;
    string              m_strLogName;
    size_t              m_nMaxFileSize;
    int                 m_nMaxFileNum;
    bool                m_bMakeDir;
    FILE_MODE           m_fmLog;
};

#define SLogInst        (CSlog::Inst())


#define LOGLv(lv,fmt, ...)  do{\
                                CSlog::LogInfo lg;\
                                string strLog;\
                                SLogInst->LogFormate(lg, lv, __LINE__, __FUNCTION__, __FILE__, fmt, ##__VA_ARGS__);\
                                strLog = SLogInst->BuildInfo(lg);\
                                SLogInst->WriteLog(lv, strLog);\
                            }while(0);

// 格式化日志接口
#define LOGD(fmt,...)   LOGLv(CSlog::LV_DEBUG,fmt,  ##__VA_ARGS__)
#define LOGI(fmt,...)   LOGLv(CSlog::LV_INFO ,fmt,  ##__VA_ARGS__)
#define LOGW(fmt,...)   LOGLv(CSlog::LV_WARN ,fmt,  ##__VA_ARGS__)
#define LOGE(fmt,...)   LOGLv(CSlog::LV_ERROR,fmt,  ##__VA_ARGS__)
#define LOGA(fmt,...)   LOGLv(CSlog::LV_ALARM,fmt,  ##__VA_ARGS__)
#define LOGF(fmt,...)   LOGLv(CSlog::LV_FATAL,fmt,  ##__VA_ARGS__)

#endif
