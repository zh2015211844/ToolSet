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

const char *useMsg = "Input command line format: tsltool -i infile infile2 -o outfile [-t] [-c]\n"
                     "\t-i: Indicates that the input file follows;\n"
                     "\t-o: Indicates that the output file follows;\n"
                     "\t-t: Indicates forced reallocation of enum values;\n"
                     "\t-c: indicates the compilation format;\n\n"
                     "\tnote: If there is no -c,compiled into binary format;\n"
                     "\t      otherwise, it is compiled into binary format and into C language code;\n";
using namespace std;
// 输入文件集合
list<string> input;
// 输出文件
string output;
// 输出C语言代码标志
bool cflg = false;
// 强制重新分配枚举
bool fflg = false;

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
                fflg = true;
                ++i;
            } // 输出C代码
            else if (strcmp(argv[i], "-c") == 0)
            {
                cflg = true;
                ++i;
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
        // 解析成功
        /* ::printf("input files:\n");
         for (auto i = input.begin(); i != input.end(); ++i)
             ::printf("\t%s\n", i->c_str());
         ::printf("output file: %s\n", output.c_str());
         ::printf("cfg: %s\n\n", cflg == true ? "binary and c-code" : "binary");
        */
        return;
    } while (false);
    ::printf(useMsg);
    ::exit(0);
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
                ::printf("Unknown escape character[ \\%c] !\n", data[index + 1]);
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
        return false;

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
        if (index >= size)
            break;
        // 强制分配枚举值
        if (fflg == true)
            item.mEnum = 0;
        // 校验枚举值是否重复,重复设置为0
        auto eit = tslEnum.find(item.mEnum);
        if (eit != tslEnum.end())
        {
            ::printf("Warning: Duplicate enum`s value [ %s | %s ] on line %d!\n",
                     item.mName.c_str(), eit->second.mName.c_str(), tlsLine);
            item.mEnum = 0;
        }

        // 校验枚举名称是否重复
        if (tslMap.find(item.mName) != tslMap.end())
        {
            ::printf("Error: Duplicate enume`s name [ %s ] on line %d!\n",
                     item.mName.c_str(), tlsLine);
            return false;
        }
        auto it = tslMap.insert(pair<string, Item>(item.mName, item));
        if (item.mEnum != 0)
            tslEnum.insert(pair<uint32_t, const Item &>(item.mEnum, it.first->second));
    }
    if (retFlg == false)
        ::printf("Error parsing file on line %d!\n", tlsLine);
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
            ::printf("Fail to open file[ %s ]!\n", iter->c_str());
            ::exit(0);
        }
        ::printf("Start parsing[ %s ] ...\n", iter->c_str());
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
        ::printf("Fail to pasrer files!count: %d \n", errCnt);
        ::exit(0);
    }
}

void CheckEnum()
{
    int32_t nEnum = 1;
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
    char hexes[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
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
    ::printf("\nGenerating data...\n");
    // 二进制数据
    vector<uint8_t> vBin;
    // 预填充数据
    //  1.魔数：4 byte 为 tsls 字母的值
    vBin.push_back('t');
    vBin.push_back('s');
    vBin.push_back('l');
    vBin.push_back('t');
    // 2.EMS总数
    uint32_t count = tslMap.size();
    ::printf("Total EMS:%u\n", count);
    vBin.push_back(count >> 24);
    vBin.push_back(count >> 16);
    vBin.push_back(count >> 8);
    vBin.push_back(count);
    // 3.EMS数据长度
    vBin.push_back(0);
    vBin.push_back(0);
    vBin.push_back(0);
    vBin.push_back(0);
    // 记录文件头大小
    uint32_t binBase = vBin.size();
    // 代码数据
    string szCH, szCSA, szCSE;
    // 预填充数据
    szCH.append("enum{\n\t");
    szCSA.append("const char* gTslArr[]={\n\t");
    if (fflg == false)
        szCSE.append("\nunsigned int gTslEnum[]={\n\t");
    // 生成数据
    count = 0;
    for (auto i = tslEnum.begin(); i != tslEnum.end(); ++i)
    {
        // 生成二进制数据
        //  1.填充枚举值 4 byte
        uint32_t nEnum = i->second.mEnum;
        vBin.push_back(nEnum >> 24);
        vBin.push_back(nEnum >> 16);
        vBin.push_back(nEnum >> 8);
        vBin.push_back(nEnum);
        // 2.填充字符串长度 2 byte,包括 null 字符
        uint16_t nLen = i->second.mTsl.size() + 1;
        vBin.push_back(nLen >> 8);
        vBin.push_back(nLen);
        // 3.填充字符串数据,包括 null 字符
        auto aData = i->second.mTsl;
        for (uint16_t l = 0; l < nLen; ++l)
            vBin.push_back(aData[l]);

        // 生成C代码数据
        if (cflg == true)
        {
            if ((count + 1) % 5 == 0)
            {
                szCH.append("\n\t");
                szCSA.append("\n\t");
                szCSE.append("\n\t");
            }
            ++count;
            // 1.生成枚举头文件
            szCH.append(i->second.mName);
            szCH.push_back('=');
            auto sEnum = toHex(i->second.mEnum, 4); //= std::to_string(i->second.mEnum);
            szCH.append(sEnum);
            szCH.push_back(',');
            // 2.生成C语言代码
            // 2.1字符串数组
            szCSA.push_back('"');
            szCSA.append(i->second.mTsl);
            szCSA.append("\",");
            // 2.2枚举值数组
            if (fflg == false)
            {
                szCSE.append(i->second.mName);
                szCSE.push_back(',');
            }
        }
    }
    // 修复数据
    uint32_t size = vBin.size() - binBase;
    ::printf("TEMS length:%u\n\n", size);
    const uint32_t offset = 8;
    vBin[offset] = size >> 24;
    vBin[offset + 1] = size >> 16;
    vBin[offset + 2] = size >> 8;
    vBin[offset + 3] = size;
    szCH.append("\n};");
    szCSA.append("\n};");

    if(fflg==false)
        szCSE.append("\n};");

    // 输出到文件
    //  1.二进制文件
    string tslBin = output + ".tsl";
    FILE *fout = fopen(tslBin.c_str(), "wb");
    if (fout == nullptr)
        ::printf("Fail to open file[ %s ]!\n", tslBin.c_str());
    else
    {
        ::printf("Generate file[ %s ]...\n", tslBin.c_str());
        fwrite(vBin.data(), sizeof(uint8_t), vBin.size(), fout);
        fclose(fout);
    }
    // 2.代码文件
    if (cflg == true)
    {
        string tslH = output + ".tsl.h";
        fout = fopen(tslH.c_str(), "wb");
        if (fout == nullptr)
            ::printf("Fail to open file[ %s ]!\n", tslH.c_str());
        else
        {
            ::printf("Generate file[ %s ]...\n", tslH.c_str());
            fwrite(szCH.data(), sizeof(char), szCH.size(), fout);
            fclose(fout);
        }
        string tslC = output + ".tsl.c";
        fout = fopen(tslC.c_str(), "wb");
        if (fout == nullptr)
            ::printf("Fail to open file[ %s ]!\n", tslC.c_str());
        else
        {
            ::printf("Generate file[ %s ]...\n", tslC.c_str());
            // 获取文件名称
            string name;
            auto pos = tslH.find_last_of('\\');
            if (pos == -1)
                pos = 0;
            else
                ++pos;
            name = tslH.substr(pos);
            tslH = "#include \"./" + name + "\"\n";
            fwrite(tslH.data(), sizeof(char), tslH.size(), fout);
            fwrite(szCSA.data(), sizeof(char), szCSA.size(), fout);
            fwrite(szCSE.data(), sizeof(char), szCSE.size(), fout);
            fclose(fout);
        }
    }
    ::printf("\nEnding...\n");
}
