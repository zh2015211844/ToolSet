#include <tsl.h>
#include <clocale>
int main(int argc, const char *argv[])
{
    // 设置编码为中文UTF8
    setlocale(LC_ALL, "zh-CN.UTF-8");

#ifdef _Tool_debug
    const char *argvs[] = {
        "tts",
        "-i",
        "C:\\Users\\kil\\Desktop\\ToolSet\\i18n\\test\\tts.1",
        "-i",
        "C:\\Users\\kil\\Desktop\\ToolSet\\i18n\\test\\tts.2",
        "-o",
        "C:\\Users\\kil\\Desktop\\ToolSet\\i18n\\test\\tts",
        "-t",
        "-v",
        "100",
        "-l",
        "c++",
    };
    // 解析命令行
    ParserCmd(sizeof(argvs) / sizeof(char *), argvs);
#else
    ParserCmd(argc, argv);
#endif
    // 解析文件
    ParserFile();
    // 校验
    CheckEnum();
    // 生成文件
    DumpFile();
    return 0;
}