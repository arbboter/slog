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
        FM_SELF,        // 日志文件仅存储自身等级的日志
        FM_TREE,        // 日志文件存储自身等级及以上的日志
        FM_ALL,         // 所以日志都存储到当前设定的日志等级文件中
    };

    enum DATE_FMT
    {
        DATE_FMT_LOG,
        DATE_FMT_DATE,
    };
    
    // 日志信息
    typedef struct _log_info_
    {
        unsigned int    nTid;           // 线程ID
        unsigned int    nPid;           // 进程ID
        unsigned int    nLine;          // 行号
        int             nLogLv;         // 日志登记
        string          strFile;        // 文件名
        string          strFunc;        // 函数名
        string          strTag;         // 日志等级标签名
        string          strMsg;         // 日志信息
        string          strDateTime;    // 日志时间
    }LogInfo;

public:
    // 从路径获取文件名
    string GetFileName(const string& strPath);
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
    void   Lock(int nLv);
    void   Unlock(int nLv);


private:
    CSlog()
    {
        for (int i=0; i<=LV_MAX; i++)
        {
            m_fpLog[i] = NULL;
            InitializeCriticalSection(&m_csFile[i]);
        }

        Init();
        m_fmLog = FM_ALL;
        m_bEnableTS = false;
    }

    ~CSlog(void)
    {
        Uninit();

        for (int i=0; i<=LV_MAX; i++)
        {
            m_fpLog[i] = NULL;
            DeleteCriticalSection(&m_csFile[i]);
        }
    }

    bool Init();
    bool Uninit();

private:
    static CSlog*  m_pInst;     // 日志句柄

protected:
    CRITICAL_SECTION    m_csFile[LV_MAX+1];         // 日志文件锁
    FILE*               m_fpLog[LV_MAX+1];          // 日志文件句柄
    LV                  m_lvLog;                    // 日志等级
    string              m_strLogPath;               // 日志文件路径
    string              m_strAppName;               // 日志程序名称，日志文件名中包含该字段
    size_t              m_nMaxFileSize;             // 单个日志文件最大值
    int                 m_nMaxFileNum;              // 单级日志文件最大数
    FILE_MODE           m_fmLog;                    // 日志文件模式
    bool                m_bEnableTS;                // 是否启用线程安全
};

#define SLogInst        (CSlog::Inst())


#define LOGLv(lv,fmt, ...)  do{\
                                CSlog::LogInfo lg;\
                                string strLog;\
                                SLogInst->LogFormate(lg, lv, __LINE__, __FUNCTION__, __FILE__, fmt, ##__VA_ARGS__);\
                                strLog = SLogInst->BuildInfo(lg);\
                                SLogInst->WriteLog(lv, strLog);\
                            }while(0);

#define LOGLvH(lv,tag)      do{\
                                string strLog;\
                                strLog = SLogInst->BuildHeader(tag);\
                                SLogInst->WriteLog(lv, strLog);\
                             }while(0);

// 格式化日志接口
#define LOGD(fmt,...)   LOGLv(CSlog::LV_DEBUG,fmt,  ##__VA_ARGS__)
#define LOGI(fmt,...)   LOGLv(CSlog::LV_INFO ,fmt,  ##__VA_ARGS__)
#define LOGW(fmt,...)   LOGLv(CSlog::LV_WARN ,fmt,  ##__VA_ARGS__)
#define LOGE(fmt,...)   LOGLv(CSlog::LV_ERROR,fmt,  ##__VA_ARGS__)
#define LOGA(fmt,...)   LOGLv(CSlog::LV_ALARM,fmt,  ##__VA_ARGS__)
#define LOGF(fmt,...)   LOGLv(CSlog::LV_FATAL,fmt,  ##__VA_ARGS__)

// 日志分段标签，默认标签当前函数名
#define LOGDH()   LOGLvH(CSlog::LV_DEBUG,__FUNCTION__)
#define LOGIH()   LOGLvH(CSlog::LV_INFO ,__FUNCTION__)
#define LOGWH()   LOGLvH(CSlog::LV_WARN ,__FUNCTION__)
#define LOGEH()   LOGLvH(CSlog::LV_ERROR,__FUNCTION__)
#define LOGAH()   LOGLvH(CSlog::LV_ALARM,__FUNCTION__)
#define LOGFH()   LOGLvH(CSlog::LV_FATAL,__FUNCTION__)

// 日志分段标签
#define LOGDHT(tag)   LOGLvH(CSlog::LV_DEBUG,tag)
#define LOGIHT(tag)   LOGLvH(CSlog::LV_INFO ,tag)
#define LOGWHT(tag)   LOGLvH(CSlog::LV_WARN ,tag)
#define LOGEHT(tag)   LOGLvH(CSlog::LV_ERROR,tag)
#define LOGAHT(tag)   LOGLvH(CSlog::LV_ALARM,tag)
#define LOGFHT(tag)   LOGLvH(CSlog::LV_FATAL,tag)

#endif
