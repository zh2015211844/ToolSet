#ifndef __TSL_H__
#define __TSL_H__

//解析命令行
void ParserCmd(int argc,const char *argv[]);
//解析文件
void ParserFile();
//校验
void CheckEnum();

//生二进制文件
void DumpFile();
//生二进制文件
void DumpBinFile();
//生代码文件
void DumpFileC();
void DumpFileCpp();
#endif // __TSL_H__