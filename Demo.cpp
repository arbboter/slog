#include <stdio.h>
#include "Slog.h"

using namespace CMS;
int main(int argc, char *argv[])
{
    for (int i = 0; i < 1000; i++)
    {
        LOGD("#%d# Hello, i'm a loger.", i);
        LOGI("#%d# Hello, i'm a loger.", i);
        LOGW("#%d# Hello, i'm a loger.", i);
        LOGE("#%d# Hello, i'm a loger.", i);
        LOGF("#%d# Hello, i'm a loger.", i);
        if (i == 100)
        {
            // 设置日志滚动时文件大小上限，文件个数上限
            SLogInst->SetFileRoll(1024 * 1024, 10);
            // 设置日志等级和日志文件名前缀
            SLogInst->SetCfg(CSlog::LV_ERROR, "Roll");
            // 设置日志文件模式：FM_SELF-记录到当前等级日志文件
            SLogInst->SetFileMode(CSlog::FM_SELF);
        }
    }
    return 0;
}
