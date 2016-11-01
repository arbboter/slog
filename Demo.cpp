#include "stdafx.h"
#include <stdio.h>
#include "Slog.h"

int main(int argc, char *argv[])
{
    for (int i=0; i<100000; i++)
    {
        LOGD("#%d# Hello, i'm a loger.", i);
        LOGI("#%d# Hello, i'm a loger.", i);
        LOGW("#%d# Hello, i'm a loger.", i);
        LOGE("#%d# Hello, i'm a loger.", i);
        LOGA("#%d# Hello, i'm a loger.", i);
        LOGF("#%d# Hello, i'm a loger.", i);
        if(i==100)
        {
            SLogInst->SetFileRoll(1024*1024, 10);
            SLogInst->SetCfg(CSlog::LV_ERROR, "Roll");
            SLogInst->SetFileMode(CSlog::FM_CURRENT);
        }
    }
    return 0;
}

