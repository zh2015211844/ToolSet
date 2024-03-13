#include <cstdint>
#include <cstdio>
#include <filesystem>

struct I18nLoader
{
    I18nLoader() : mData(nullptr) {}
    ~I18nLoader();
    // 加载文件,检查版本是否一致
    bool load(const char *filename, const uint32_t version);
    
    // 通过枚举值翻译到字符串
    const char *translate(uint32_t id);
    

#if 1
    // 输出测试
    void dump() const
    {
        for (size_t i = 0; i < mData->mTotal; ++i)
        {
            auto item = mData->mTsls[i].mTsl;
            printf("0x%X: %s\n", item->mEnum, item->mStr);
        }
    }
#endif
private:
    // 翻译单元
    struct TslItem
    {
        uint32_t mEnum; // 枚举ID
        uint16_t mLen;  // 字符串长度
        char mStr[];    // 翻译字符串
    };

    // 文件头
    struct FileHeader
    {
        uint8_t mMagic[4]; // 魔数标识
        uint32_t mHash;    // 文件hash
        uint32_t mVersion; // 文件版本
        uint32_t mTotal;   // 翻译单元数量
        uint32_t mSerial;  // 标识是否连续
        uint32_t mSize;    // 翻译单元数据大小
        union
        {
            uintptr_t mOffset; // 翻译单元偏移
            TslItem *mTsl;     // 翻译单元
        } mTsls[];             // 翻译单元数组
    };

    FileHeader *mData; // 文件数据

    // 检查数据是否有效
    bool checkData(const uint32_t version,const size_t total) const;
   


};