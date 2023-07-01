#include <res.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <list>
#include <set>
#include <vector>
#include <filesystem>

const char *useMsg = "Input command line format: restool -i infile infile2 -o outfile \n"
                     "\t-i: Indicates that the input file follows;\n"
                     "\t-o: Indicates that the output file follows;\n";
using namespace std;

// 输入文件集合
list<string> input;
// 输出文件
string output;
// 输出数据
string outtxt;

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
#ifdef _Tool_debug
        ::printf("input files:\n");
        for (auto i = input.begin(); i != input.end(); ++i)
            ::printf("\t%s\n", i->c_str());
        ::printf("output file: %s\n", output.c_str());
#endif
        return;
    } while (false);
    ::printf(useMsg);
    ::exit(0);
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
void GeneratTxt(const string &path, const vector<uint8_t> &txt)
{
    if (txt.empty())
    {
        ::printf("Empty file!\n");
        return;
    }
    // 获取文件名称
    string name;
    auto pos = path.find_last_of('\\');
    if (pos == -1)
        pos = 0;
    else
        ++pos;
    name = path.substr(pos);
    // 除去扩展名
    pos = name.find_first_of('.');
    while (pos != -1)
    {
        name.replace(pos, 1, "_");
        pos = name.find_first_of('.');
        //= name.substr(0, pos);
    };
    // 添加文件长度
    outtxt.append("unsigned int ");
    outtxt.append(name);
    outtxt.append("_byte = ");
    outtxt += std::to_string(txt.size());
    outtxt.append(";\n");
    // 添加数组信息
    outtxt.append("const char ");
    outtxt.append(name);
    outtxt.append("_data[] ={\n\t");
    // 生成数据
    uint32_t count = 0;
    for (auto it = txt.begin(); it != txt.end(); ++it)
    {
        if ((count + 1) % 5 == 0)
            outtxt.append("\n\t");
        outtxt.append(toHex(*it, 2));
        outtxt.append(", ");
        ++count;
    }
    outtxt.append("\n};\n");
}
void DumpFile()
{
    int32_t errCnt = 0; // 错误计数
    vector<uint8_t> fdata;
    for (auto iter = input.begin(); iter != input.end(); ++iter)
    {
        // 打开文件
        FILE *fp = fopen(iter->c_str(), "rb");
        if (fp == nullptr)
        {
            ::printf("Fail to open file[ %s ]!\n", iter->c_str());
            ::exit(0);
        }
        ::printf("Start compiling[ %s ] ...\n", iter->c_str());
        // 获取大小
        fseek(fp, 0, SEEK_END);
        auto size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        // 分配内存
        fdata.resize(size);
        // 读取文件
        fread(fdata.data(), sizeof(char), size, fp);
        // 编译具体文件
        GeneratTxt(*iter, fdata);
        // 关闭文件
        fclose(fp);
    }
    if (outtxt.empty())
    {
        ::printf("The generated data is empty!\n");
        return;
    }
    // 输出到文件
    FILE *fout = fopen(output.c_str(), "wb");
    if (fout == nullptr)
        ::printf("Fail to open file[ %s ]!\n", output.c_str());
    else
    {
        ::printf("Generate file[ %s ]...\n", output.c_str());
        fwrite(outtxt.data(), sizeof(uint8_t), outtxt.size(), fout);
        fclose(fout);
    }
}
