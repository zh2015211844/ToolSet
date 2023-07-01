#include <res.h>

int main(int argc, const char *argv[])
{
#ifdef _Tool_debug
    const char *argvs[] = {
        "tts",
        "-i",
        "E:\\Project\\Tools\\ResTool\\test\\test1.bin.txt",
        "-i",
        "E:\\Project\\Tools\\ResTool\\test\\test2.bin.txt",
        "-o",
        "E:\\Project\\Tools\\ResTool\\test\\test.h",
    };
    //解析命令行
    ParserCmd(sizeof(argvs) / sizeof(char *), argvs);
#else
    ParserCmd(argc, argv);
#endif
    //生成文件
    DumpFile();
    return 0;
}