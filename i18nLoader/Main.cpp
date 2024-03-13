#include "I18nLoader.hpp"

int main(int argc, const char *argv[])
{
    // 设置编码为中文UTF8
    setlocale(LC_ALL, "zh-CN.UTF-8");

    I18nLoader loader;

    if (true == loader.load("C:\\Users\\kil\\Desktop\\ToolSet\\i18n\\test\\tts.tsl", 100))
    {
        loader.dump();
    }
    auto tsl = loader.translate(0);
    return 0;
}