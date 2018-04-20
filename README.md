# slog说明

a light weight c++ log for windows

## 特性

* 本日志类追求实现使用简单快捷，无第三方依赖，仅需注意使用的地方需要预先使用`using namespace`声明名字空间`CMS`
* 日志支持五种日志等级，从低到高依次为：LV_DEBUG,LV_INFO,LV_WARN,LV_ERROR,LV_FATAL
* 日志文件支持滚动模式，默认文件个数为20个，大小为20MB，可通过接口自定义设置
* 对外日志接口可支持非文本二进制数据，需调用`BLOGI`类的`B`为前缀的接口
* 支持日志头，用于手工为日志分段，且支持程序编译时间的输出
* 日志内容文件模式记录支持三种模式:
  * FM_SELF: 日志文件仅存储自身等级的日志
  * FM_TREE: 日志文件存储自身等级及以上的日志
  * FM_ALL: 所以日志都存储到当前设定的日志等级文件中，默认使用该方式
* 实现细节:
  * 内部按日志级别维护一个文件列表数组，分别记录对应级别的日志文件指针
  * 调用接口写日志时，为了提高性，减少IO影响，开辟一个单独的队列和线程分别存储程序日志和写日志到文件，如此调用日志接口的代码开销仅限于把日志写入队列，日志本身线程负责将该日志写入文件

## example

```cpp
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

```
