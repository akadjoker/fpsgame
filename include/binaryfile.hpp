#pragma once
#include "Config.hpp"
 
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2


class BinaryFile 
{
public:
    BinaryFile() {};
    bool open(const char* filename);
    bool create(const void* buffer, u32 size);
    bool save(const char* filename);
    void clear();


    u32 getFileSize();
    u32 ftell();

    u32 seek(u32 offset, u32 origin);
    u32 readBytes(void* buffer, u32 size);
    u32 writeBytes(const void* buffer, u32 size);
    

    float readFloat();
    u8 readByte();
    s16 readShort();
    u16 readUShort();

    s32 readInt();

    u32 readUInt();

    s64 readLong();

    u64 readULong();
    

 
    void writeFloat(float value);
    void writeByte(u8 value);
    void writeShort(s16 value);
    void writeUShort(u16 value);
    void writeInt(s32 value);

    void writeUInt(u32 value);


    void writeLong(s64 value);


    void writeULong(u64 value);


        bool isEof() { return position >= fileSize; }


    private:
        u32 position{ 0 };
        u32 fileSize{ 0 };
        void* data{ nullptr };
        bool readOnly{ false };
    };
