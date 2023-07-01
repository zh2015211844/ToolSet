#ifndef __TSL_H__
#define __TSL_H__

//解析命令行
void ParserCmd(int argc,const char *argv[]);
//解析文件
void ParserFile();
//校验
void CheckEnum();
//生文件
void DumpFile();

#endif // __TSL_H__