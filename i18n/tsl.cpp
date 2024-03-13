#include <tsl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <list>
#include <set>
#include <vector>
#include <filesystem>

const char *useMsg = "命令格式: i18n.exe -i infile infile2 -o outfile [-v 123456] [-t] [-l c/c++] [-h] \n"
                     "选项含义：\n"
                     "\t-i :指示后面是输入文件;\n"
                     "\t-o :指示后面是输出文件\n"
                     "\t-v: 指示文件的版本号,默认为 1;\n"
                     "\t-t: 指示强制分配枚举值,从 0 开始分配;\n"
                     "\t-l: 指示编译输出编程语言代码;\n"
                     "\t\tc  : 指示编译输出 C 编程语言代码;\n"
                     "\t\tc++: 指示编译输出 C++ 编程语言代码;\n\n"
                     "\t注解: 如果没有指定 -l 标志，则只输出二进制文件\n"
                     "\t      否则，输出指定的语言代码，以及二进制文件\n"
                     "\t文件格式：\n"
                     "\t枚举名称,枚举值,翻译字符串\n"
                     "\t\tmenu_edit, 1 , \"编辑\"\n"
                     "\t\tmenu_copy, 2 , \"复制\"\n"
                     "\t\t...\n"
                     "\t其中'#' 字符开始的一行为注释，会被跳过。\n";
using namespace std;
// 输入文件集合
list<string> input;
// 输出文件
string output;
// 输出语言
enum class Language
{
    unknown = 0,
    C,
    Cpp
};
// 输出C语言代码标志
Language lang = Language::unknown;
// 强制重新分配枚举
bool force_flg = false;

// 翻译文件版本号
uint32_t version = 1;

// 枚举项
struct Item
{
    string mName;   // 枚举名称
    uint32_t mEnum; // 枚举值
    string mTsl;    // 映射的转义信息
};
// 枚举项映射
map<string, Item> tslMap;
// 枚举集合
map<uint32_t, const Item &> tslEnum;
// 解析文件的行号
uint32_t tlsLine = 0;

void ParserCmd(int argc, const char *argv[])
{
    do
    {
        // 校验参数个数
        if (argc < 5)
            break;
        // 开始解析参数
        int i = 1;
        while (i < argc)
        {
            // 输入文件
            if (strcmp(argv[i], "-i") == 0)
            {
                ++i;
                if (i >= argc)
                {
                    i = -1;
                    break;
                }
                for (; i < argc; ++i)
                {
                    if (argv[i][0] != '-')
                        input.push_back(argv[i]);
                    else
                        break;
                }
            }
            // 输出文件
            else if (strcmp(argv[i], "-o") == 0)
            {
                ++i;
                if (i >= argc)
                {
                    i = -1;
                    break;
                }
                if (argv[i][0] != '-')
                {
                    output = argv[i];
                    ++i;
                }
                else
                {
                    i = -1;
                    break;
                }
            } // 强制分配枚举值
            else if (strcmp(argv[i], "-t") == 0)
            {
                force_flg = true;
                ++i;
            } // 输出C代码
            else if (strcmp(argv[i], "-v") == 0)
            {
                ++i;
                if (i >= argc)
                {
                    i = -1;
                    break;
                }
                if (argv[i][0] != '-')
                {
                    // 检查版本号必须是数字
                    auto vstr = argv[i];
                    while (*vstr)
                    {
                        if (isdigit(*vstr))
                        {
                            ++vstr;
                        }
                        else
                        {
                            i = -1;
                            break;
                        }
                    }
                    version = stoul(argv[i]);
                    ++i;
                }
                else
                {
                    i = -1;
                    break;
                }
            }
            else if (strcmp(argv[i], "-l") == 0)
            {
                ++i;
                if (i >= argc)
                {
                    i = -1;
                    break;
                }
                if (strcmp(argv[i], "c") == 0)
                {
                    lang = Language::C;
                    ++i;
                }
                else if (strcmp(argv[i], "c++") == 0)
                {
                    lang = Language::Cpp;
                    ++i;
                }
                else
                {
                    i = -1;
                    break;
                }
            }
            // 存在错误
            else
            {
                i = -1;
                break;
            }

        } // 存在错误
        if (i == -1 ||
            input.empty() == true ||
            output.empty() == true)
            break;
        return;
    } while (false);
    printf("命令存在错误！或者输入输出文件为空！\n\n使用帮助：\n");
    printf(useMsg);
    exit(0);
}

// 跳过空白字符
inline void SkipBlank(uint32_t &index, uint32_t size, const char data[])
{
    while (true)
    {
        // 注释
        if (data[index] == '#')
        {
            ++index;
            while (index < size && data[index++] != '\n')
                ;
            ++tlsLine;
            break;
        }
        // 空白字符
        else if (data[index] == ',' ||
                 data[index] == ' ' ||
                 data[index] == '\t' ||
                 data[index] == '\r')
            ++index;
        else if (data[index] == '\n')
        {
            ++tlsLine;
            ++index;
        }
        else
            break;
    }
}
// 解析枚举名称
bool ParserName(uint32_t &index, uint32_t size, const char data[], string &name)
{
    name.clear();
    const char ch = data[index];
    // C语言标识符规则
    if (isalpha(data[index]) == 0 && data[index] != '_')
        return false;
    while (index < size)
    {
        const char ch = data[index];
        if (isalpha(data[index]) != 0 ||
            isdigit(data[index]) != 0 ||
            data[index] == '_')
        {
            name.push_back(data[index++]);
        }
        else
            return true;
    }
    return false;
}
// 解析枚举值
bool ParserEnum(uint32_t &index, uint32_t size, const char data[], uint32_t &nEnum)
{
    string szNum;
    bool hexFlg = false;
    const char ch = data[index];
    // 10/16进制
    while (index < size)
    {
        if ((data[index] == 'x' || data[index] == 'X') &&
            hexFlg == false &&
            szNum.size() == 1 &&
            szNum[0] == '0')
        {
            hexFlg = true;
            szNum.push_back(data[index++]);
        }
        else if (hexFlg == false &&
                 isdigit(data[index]) != 0)
        {
            szNum.push_back(data[index++]);
        }
        else if (hexFlg == true &&
                 (isdigit(data[index]) != 0 ||
                  ((data[index] >= 'a' && data[index] <= 'f') ||
                   (data[index] >= 'A' && data[index] <= 'F'))))
        {
            szNum.push_back(data[index++]);
        }
        else
            break;
    }
    // 十进制
    if (hexFlg == false)
        nEnum = std::atoi(szNum.c_str());
    else // 十六进制
    {
        nEnum = 0;

        for (auto i = 0; i < szNum.size(); ++i)
        {
            if (szNum[i] >= '0' && szNum[i] <= '9')
                nEnum = nEnum * 16 + szNum[i] - '0'; // 0到9的字符串
            else if (szNum[i] >= 'A' && szNum[i] <= 'F')
                nEnum = nEnum * 16 + szNum[i] - 'A' + 10; // 大写十六进制的ABCDEF的字符串的情况
            else if (szNum[i] >= 'a' && szNum[i] <= 'f')
                nEnum = nEnum * 16 + szNum[i] - 'a' + 10;
        }
    }
    return true;
}
// 解析映射信息
bool ParserTsl(uint32_t &index, uint32_t size, const char data[], string &tsl)
{
    tsl.clear();
    // C语言字符串规则
    if (data[index] != '"')
        return false;
    ++index;
    while (index < size)
    {
        if (data[index] == '\\' && index + 1 < size)
        {
            if (data[index + 1] == '\\')
                tsl.push_back('\\');
            else if (data[index + 1] == 't')
                tsl.push_back('\t');
            else if (data[index + 1] == 'r')
                tsl.push_back('\r');
            else if (data[index + 1] == 'n')
                tsl.push_back('\n');
            else if (data[index + 1] == '"')
                tsl.push_back('"');
            else
            {
                ::printf("未知转义字符‘ \\%c’ !\n", data[index + 1]);
                return false;
            }
            index += 2;
        }
        else if (data[index] == '"')
        {
            ++index;
            break;
        }
        else
            tsl.push_back(data[index++]);
    }

    if (tsl.size() > 0xFFFF)
    {
        ::printf("%4d :字符串数据太长(len >65535)！\n", tlsLine);
        return false;
    }

    return true;
}
// 解析文本
bool ParserTxt(const string &path, const string &txt)
{
    uint32_t index = 0;
    const char *data = txt.data();
    Item item;
    uint32_t size = txt.length();
    // 跳过第一行
    SkipBlank(index, size, data);
    bool retFlg = true;
    while (index < size)
    {
        SkipBlank(index, size, data);
        if (index >= size || data[index] == 0)
            break;
        // 解析枚举名称
        if (ParserName(index, size, data, item.mName) == false)
        {
            retFlg = false;
            break;
        }
        SkipBlank(index, size, data);
        if (index >= size)
            break;
        // 解析枚举值
        if (ParserEnum(index, size, data, item.mEnum) == false)
        {
            retFlg = false;
            break;
        }
        SkipBlank(index, size, data);
        if (index >= size)
            break;
        // 解析映射信息
        if (ParserTsl(index, size, data, item.mTsl) == false)
        {
            retFlg = false;
            break;
        }
        SkipBlank(index, size, data);
        /*if (index >= size)
            break;*/
        // 强制分配枚举值
        if (force_flg == true)
            item.mEnum = 0;
        // 校验枚举值是否重复,重复设置为0
        auto eit = tslEnum.find(item.mEnum);
        if (eit != tslEnum.end())
        {
            printf("%4d :警告,枚举名称为 ‘%s’ 与 ‘%s’ 的枚举值存在重复  !\n",
                   tlsLine, item.mName.c_str(), eit->second.mName.c_str());
            item.mEnum = 0;
        }

        // 校验枚举名称是否重复
        if (tslMap.find(item.mName) != tslMap.end())
        {
            printf("%4d :错误， 枚举名称重复 ‘%s’ !\n",
                   item.mName.c_str(), tlsLine);
            return false;
        }
        auto it = tslMap.insert(pair<string, Item>(item.mName, item));
        if (item.mEnum != 0)
            tslEnum.insert(pair<uint32_t, const Item &>(item.mEnum, it.first->second));
    }
    if (retFlg == false)
        printf("%4d :解析文件发生错误!\n", tlsLine);
    return retFlg;
}
void ParserFile()
{
    int32_t errCnt = 0; // 错误计数
    string txt;
    for (auto iter = input.begin(); iter != input.end(); ++iter)
    {
        // 打开文件
        FILE *fp = fopen(iter->c_str(), "r");
        if (fp == nullptr)
        {
            ::printf("[ %s ]：打开文件失败!\n", iter->c_str());
            ::exit(0);
        }
        ::printf("[ %s ]：解析文件... \n", iter->c_str());
        // 获取大小
        fseek(fp, 0, SEEK_END);
        auto size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        // 分配内存
        txt.resize(size);
        // 读取文件
        fread(txt.data(), sizeof(char), size, fp);
        // 解析具体文件
        tlsLine = 1;
        if (ParserTxt(*iter, txt) == false)
            ++errCnt;
        // 关闭文件
        fclose(fp);
    }
    // 如果解析过程中存在错误
    if (errCnt != 0)
    {
        ::printf("解析文件错误！错误文件数: %d \n", errCnt);
        ::exit(0);
    }
}

void CheckEnum()
{
    int32_t nEnum = 0;
    for (auto i = tslMap.begin(); i != tslMap.end(); ++i)
    {
        // 调整枚举值
        if (i->second.mEnum == 0x0)
        {
            while (true)
            {
                if (tslEnum.find(nEnum) != tslEnum.end())
                {
                    ++nEnum;
                    continue;
                }
                else
                    break;
            }
            tslEnum.insert(pair<uint32_t, const Item &>(nEnum, i->second));
            i->second.mEnum = nEnum++;
        }
    }
}
string toHex(uint32_t num, uint32_t min)
{
    static char hexes[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    string hex;
    // min = min | 2;
    uint32_t nibble;
    do
    {
        nibble = num & 0x0f;
        num = num >> 4;
        hex = hexes[nibble] + hex;
    } while (num);
    if (hex.size() < min)
    {
        auto i = min - hex.size();
        while (i-- > 0)
            hex = '0' + hex;
    }
    return "0x" + hex;
}
void DumpFile()
{
    switch (lang)
    {
    case Language::C:
        DumpFileC();
        break;
    case Language::Cpp:
        DumpFileCpp();
        break;
    }
    DumpBinFile();
}

struct Crc32
{
    Crc32(bool init = false)
    {
        if (true == init)
        { // Poly = 0xedb88320 WinRAR Poly
            uint32_t Val;
            for (uint32_t i = 0; i < 256; i++)
            {
                Val = i;
                for (uint32_t k = 0; k < 8; k++)
                {
                    if (Val & 1)
                        Val = 0xedb88320 ^ (Val >> 1);
                    else
                        Val = Val >> 1;
                }
                printf("0x%08x, ", Val);
                if (0 == ((i + 1) % 6))
                    printf("\n");
            }
            printf("\n");
        }
    }
    uint32_t cal(uint32_t hash, const uint8_t *data, size_t len)
    {
        size_t i;
        hash = hash ^ 0xFFFFFFFF;
        for (i = 0; i < len; i++)
        {
            hash = table[(hash ^ data[i]) & 0xFF] ^ (hash >> 8);
        }
        return (hash ^ 0xFFFFFFFF);
    }

private:
    static uint32_t table[256];
};
uint32_t Crc32::table[] = {
    0x00000000,
    0x77073096,
    0xee0e612c,
    0x990951ba,
    0x076dc419,
    0x706af48f,
    0xe963a535,
    0x9e6495a3,
    0x0edb8832,
    0x79dcb8a4,
    0xe0d5e91e,
    0x97d2d988,
    0x09b64c2b,
    0x7eb17cbd,
    0xe7b82d07,
    0x90bf1d91,
    0x1db71064,
    0x6ab020f2,
    0xf3b97148,
    0x84be41de,
    0x1adad47d,
    0x6ddde4eb,
    0xf4d4b551,
    0x83d385c7,
    0x136c9856,
    0x646ba8c0,
    0xfd62f97a,
    0x8a65c9ec,
    0x14015c4f,
    0x63066cd9,
    0xfa0f3d63,
    0x8d080df5,
    0x3b6e20c8,
    0x4c69105e,
    0xd56041e4,
    0xa2677172,
    0x3c03e4d1,
    0x4b04d447,
    0xd20d85fd,
    0xa50ab56b,
    0x35b5a8fa,
    0x42b2986c,
    0xdbbbc9d6,
    0xacbcf940,
    0x32d86ce3,
    0x45df5c75,
    0xdcd60dcf,
    0xabd13d59,
    0x26d930ac,
    0x51de003a,
    0xc8d75180,
    0xbfd06116,
    0x21b4f4b5,
    0x56b3c423,
    0xcfba9599,
    0xb8bda50f,
    0x2802b89e,
    0x5f058808,
    0xc60cd9b2,
    0xb10be924,
    0x2f6f7c87,
    0x58684c11,
    0xc1611dab,
    0xb6662d3d,
    0x76dc4190,
    0x01db7106,
    0x98d220bc,
    0xefd5102a,
    0x71b18589,
    0x06b6b51f,
    0x9fbfe4a5,
    0xe8b8d433,
    0x7807c9a2,
    0x0f00f934,
    0x9609a88e,
    0xe10e9818,
    0x7f6a0dbb,
    0x086d3d2d,
    0x91646c97,
    0xe6635c01,
    0x6b6b51f4,
    0x1c6c6162,
    0x856530d8,
    0xf262004e,
    0x6c0695ed,
    0x1b01a57b,
    0x8208f4c1,
    0xf50fc457,
    0x65b0d9c6,
    0x12b7e950,
    0x8bbeb8ea,
    0xfcb9887c,
    0x62dd1ddf,
    0x15da2d49,
    0x8cd37cf3,
    0xfbd44c65,
    0x4db26158,
    0x3ab551ce,
    0xa3bc0074,
    0xd4bb30e2,
    0x4adfa541,
    0x3dd895d7,
    0xa4d1c46d,
    0xd3d6f4fb,
    0x4369e96a,
    0x346ed9fc,
    0xad678846,
    0xda60b8d0,
    0x44042d73,
    0x33031de5,
    0xaa0a4c5f,
    0xdd0d7cc9,
    0x5005713c,
    0x270241aa,
    0xbe0b1010,
    0xc90c2086,
    0x5768b525,
    0x206f85b3,
    0xb966d409,
    0xce61e49f,
    0x5edef90e,
    0x29d9c998,
    0xb0d09822,
    0xc7d7a8b4,
    0x59b33d17,
    0x2eb40d81,
    0xb7bd5c3b,
    0xc0ba6cad,
    0xedb88320,
    0x9abfb3b6,
    0x03b6e20c,
    0x74b1d29a,
    0xead54739,
    0x9dd277af,
    0x04db2615,
    0x73dc1683,
    0xe3630b12,
    0x94643b84,
    0x0d6d6a3e,
    0x7a6a5aa8,
    0xe40ecf0b,
    0x9309ff9d,
    0x0a00ae27,
    0x7d079eb1,
    0xf00f9344,
    0x8708a3d2,
    0x1e01f268,
    0x6906c2fe,
    0xf762575d,
    0x806567cb,
    0x196c3671,
    0x6e6b06e7,
    0xfed41b76,
    0x89d32be0,
    0x10da7a5a,
    0x67dd4acc,
    0xf9b9df6f,
    0x8ebeeff9,
    0x17b7be43,
    0x60b08ed5,
    0xd6d6a3e8,
    0xa1d1937e,
    0x38d8c2c4,
    0x4fdff252,
    0xd1bb67f1,
    0xa6bc5767,
    0x3fb506dd,
    0x48b2364b,
    0xd80d2bda,
    0xaf0a1b4c,
    0x36034af6,
    0x41047a60,
    0xdf60efc3,
    0xa867df55,
    0x316e8eef,
    0x4669be79,
    0xcb61b38c,
    0xbc66831a,
    0x256fd2a0,
    0x5268e236,
    0xcc0c7795,
    0xbb0b4703,
    0x220216b9,
    0x5505262f,
    0xc5ba3bbe,
    0xb2bd0b28,
    0x2bb45a92,
    0x5cb36a04,
    0xc2d7ffa7,
    0xb5d0cf31,
    0x2cd99e8b,
    0x5bdeae1d,
    0x9b64c2b0,
    0xec63f226,
    0x756aa39c,
    0x026d930a,
    0x9c0906a9,
    0xeb0e363f,
    0x72076785,
    0x05005713,
    0x95bf4a82,
    0xe2b87a14,
    0x7bb12bae,
    0x0cb61b38,
    0x92d28e9b,
    0xe5d5be0d,
    0x7cdcefb7,
    0x0bdbdf21,
    0x86d3d2d4,
    0xf1d4e242,
    0x68ddb3f8,
    0x1fda836e,
    0x81be16cd,
    0xf6b9265b,
    0x6fb077e1,
    0x18b74777,
    0x88085ae6,
    0xff0f6a70,
    0x66063bca,
    0x11010b5c,
    0x8f659eff,
    0xf862ae69,
    0x616bffd3,
    0x166ccf45,
    0xa00ae278,
    0xd70dd2ee,
    0x4e048354,
    0x3903b3c2,
    0xa7672661,
    0xd06016f7,
    0x4969474d,
    0x3e6e77db,
    0xaed16a4a,
    0xd9d65adc,
    0x40df0b66,
    0x37d83bf0,
    0xa9bcae53,
    0xdebb9ec5,
    0x47b2cf7f,
    0x30b5ffe9,
    0xbdbdf21c,
    0xcabac28a,
    0x53b39330,
    0x24b4a3a6,
    0xbad03605,
    0xcdd70693,
    0x54de5729,
    0x23d967bf,
    0xb3667a2e,
    0xc4614ab8,
    0x5d681b02,
    0x2a6f2b94,
    0xb40bbe37,
    0xc30c8ea1,
    0x5a05df1b,
    0x2d02ef8d,
};

struct BinBuffer
{
    const auto &getData() const { return buffer; }

    // 合并
    void push(BinBuffer &bin)
    {
        this->buffer.insert(buffer.end(), bin.buffer.begin(), bin.buffer.end());
    }

    auto push(uint8_t data)
    {
        buffer.push_back(data);
    }
    auto push(uint16_t data)
    {
        buffer.push_back(data);
        buffer.push_back(data >> 8);
    }
    auto push(uint32_t data)
    {
        buffer.push_back(data);
        buffer.push_back(data >> 8);
        buffer.push_back(data >> 16);
        buffer.push_back(data >> 24);
    }
    auto push(uintptr_t data)
    {
        buffer.push_back(data);
        buffer.push_back(data >> 8);
        buffer.push_back(data >> 16);
        buffer.push_back(data >> 24);
        if (sizeof(data) == 8)
        {
            buffer.push_back(data >> 32);
            buffer.push_back(data >> 40);
            buffer.push_back(data >> 48);
            buffer.push_back(data >> 56);
        }
    }

    auto push(const string &str)
    {
        uint16_t len = str.length();
        push(len);
        for (auto &c : str)
        {
            buffer.push_back(c);
        }
        buffer.push_back(0);
        auto size = buffer.size();
        auto mod = size % 4;
        switch (mod)
        {
        case 1:
            buffer.push_back(0);
            buffer.push_back(0);
            buffer.push_back(0);
            break;
        case 2:
            buffer.push_back(0);
            break;
        case 3:
            buffer.push_back(0);
            break;
        }
    }

    auto getSize()
    {
        return buffer.size();
    }

    // 计算哈希值，并更新修正值
    void hash(bool init = false)
    {
        Crc32 crc32(init);
        uint32_t hash = crc32.cal(0, buffer.data() + 8, buffer.size() - 8);
        buffer[4] = hash;
        buffer[5] = hash >> 8;
        buffer[6] = hash >> 16;
        buffer[7] = hash >> 24;
    }

private:
    std::vector<uint8_t> buffer;
};

void DumpBinFile()
{
    printf("生成二进制文件中...\n");

    BinBuffer vbin, voffset, vtsl;

    //  1.魔数：4 byte 为 i18n 字母的值
    vbin.push((uint8_t)'i');
    vbin.push((uint8_t)'1');
    vbin.push((uint8_t)'8');
    vbin.push((uint8_t)'n');
    //  2.校验：4 byte crc32 校验
    vbin.push(0u);
    //  3.版号：4 byte
    vbin.push(version);
    //  4.翻译单元数量
    uint32_t count = tslMap.size();
    ::printf("翻译单元数量:%u\n", count);
    vbin.push(count);
    // 5.翻译单元数据生成
    for (auto &kv : tslMap)
    {
        auto &key = kv.first;
        auto &value = kv.second;
        uintptr_t offset = vtsl.getSize();
        voffset.push(offset); // 记录偏移

        vtsl.push(value.mEnum); // 记录翻译枚举值
        vtsl.push(value.mTsl);  // 记录翻译语言
    }
    // 6.
    uint32_t serial = 0; // 编号是否连续
    if (force_flg == true)
        serial = -1;
    vbin.push(serial);
    // 7.翻译数据大小
    uint32_t tslsize = vtsl.getSize();
    vbin.push(tslsize);

    // 8.合并数据
    vbin.push(voffset);
    vbin.push(vtsl);

    // 9.计算哈希值
    vbin.hash();

    // 10.保存到文件中
    string tslBin = output + ".tsl";
    FILE *fout = fopen(tslBin.c_str(), "wb");
    if (fout == nullptr)
        ::printf("[ %s ]：创建并打开文件失败!\n", tslBin.c_str());
    else
    {
        ::printf("[ %s ]：写入文件...\n", tslBin.c_str());
        fwrite(vbin.getData().data(), sizeof(uint8_t), vbin.getSize(), fout);
        fclose(fout);
    }
}

void DumpFileC()
{
    printf("生成 C 语言代码中...\n");

    string enumdef; // 枚举定义
    string tslimp;  // 翻译数据定义

    enumdef = "#ifndef _I18N_Tsl_\n#define _I18N_Tsl_\n\n"
              "/*----------------------\n"
              "   多语言翻译枚举ID定义\n"
              "    自动生成，请勿修改\n"
              "----------------------*/\n"
              "enum Tsl_i18n{\n";

    tslimp = "#ifdef _Need_I18N_Tsl_String_\n"
             "/*----------------------\n"
             "   多语言翻译字符串定义\n"
             "    自动生成，请勿修改\n"
             "----------------------*/\n"
             "struct{\n"
             "\tunsigned int mID;// 翻译枚举ID\n"
             "\tconst char* mTsl;// 翻译字符串\n"
             "}Tsl_i18n_Items[]={\n";

    for (auto &kv : tslMap)
    {
        auto &key = kv.first;
        auto &value = kv.second;

        enumdef += "\t" + key + "=";
        enumdef += std::to_string(value.mEnum) + ",\n";

        tslimp += "\t{";
        tslimp += key + ",\"";
        tslimp += value.mTsl;
        tslimp += "\"},\n";
    }

    enumdef += "};\n\n#endif\n";
    tslimp += "};\n\n#endif\n";

    // 保存到文件中
    string path = output + ".tsl.h";
    FILE *fout = fopen(path.c_str(), "wb");
    if (fout == nullptr)
        ::printf("[ %s ]：创建并打开文件失败!\n", path.c_str());
    else
    {
        ::printf("[ %s ]：写入文件...\n", path.c_str());
        fwrite(enumdef.data(), sizeof(uint8_t), enumdef.size(), fout);
        fclose(fout);
    }

    path = output + ".tsl.c";
    fout = fopen(path.c_str(), "wb");
    if (fout == nullptr)
        ::printf("[ %s ]：创建并打开文件失败!\n", path.c_str());
    else
    {
        ::printf("[ %s ]：写入文件...\n", path.c_str());
        fwrite(tslimp.data(), sizeof(uint8_t), tslimp.size(), fout);
        fclose(fout);
    }
}

void DumpFileCpp()
{
    printf("生成 C++ 语言代码中...\n");

    string enumdef; // 枚举定义
    string tslimp;  // 翻译数据定义

    enumdef = "#ifndef _I18N_Tsl_\n#define _I18N_Tsl_\n\n"
              "/*----------------------\n"
              "   多语言翻译枚举ID定义\n"
              "    自动生成，请勿修改\n"
              "----------------------*/\n"
              "struct Tsl_i18n{\n"
              "\tenum :unsigned int{\n";

    tslimp = "#ifdef _Need_I18N_Tsl_String_\n"
             "/*----------------------\n"
             "   多语言翻译字符串定义\n"
             "    自动生成，请勿修改\n"
             "----------------------*/\n"
             "struct{\n"
             "\tunsigned int mID;// 翻译枚举ID\n"
             "\tconst char* mTsl;// 翻译字符串\n"
             "}Tsl_i18n_Items[]={\n";

    for (auto &kv : tslMap)
    {
        auto &key = kv.first;
        auto &value = kv.second;

        enumdef += "\t\t" + key + "=";
        enumdef += std::to_string(value.mEnum) + ",\n";

        tslimp += "\t{";
        tslimp += "Tsl_i18n::" + key + ",\"";
        tslimp += value.mTsl;
        tslimp += "\"},\n";
    }

    enumdef += "\t};\n};\n\n#endif\n";
    tslimp += "};\n\n#endif\n";

    // 保存到文件中
    string path = output + ".tsl.hpp";
    FILE *fout = fopen(path.c_str(), "wb");
    if (fout == nullptr)
        ::printf("[ %s ]：创建并打开文件失败!\n", path.c_str());
    else
    {
        ::printf("[ %s ]：写入文件...\n", path.c_str());
        fwrite(enumdef.data(), sizeof(uint8_t), enumdef.size(), fout);
        fclose(fout);
    }

    path = output + ".tsl.cpp";
    fout = fopen(path.c_str(), "wb");
    if (fout == nullptr)
        ::printf("[ %s ]：创建并打开文件失败!\n", path.c_str());
    else
    {
        ::printf("[ %s ]：写入文件...\n", path.c_str());
        fwrite(tslimp.data(), sizeof(uint8_t), tslimp.size(), fout);
        fclose(fout);
    }
}
