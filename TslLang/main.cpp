#include <tsl.h>

int main(int argc, const char *argv[])
{
#ifdef _Tool_debug
    const char *argvs[] = {
        "tts",
        "-i",
        "E:\\Project\\Tools\\TslTool\\test\\tts.1",
        "-i",
        "E:\\Project\\Tools\\TslTool\\test\\tts.2",
        "-o",
        "E:\\Project\\Tools\\TslTool\\test\\tts",
        "-t",
        "-c",
    };
    //解析命令行
    ParserCmd(sizeof(argvs) / sizeof(char *), argvs);
#else
    ParserCmd(argc, argv);
#endif
    //解析文件
    ParserFile();
    //校验
    CheckEnum();
    //生成文件
    DumpFile();
    return 0;
}