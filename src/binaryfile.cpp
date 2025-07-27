#include "binaryfile.hpp"
#include <raylib.h>

bool BinaryFile::open(const char* filename)
{
    clear();

    int bytesRead;
    data = LoadFileData(filename, &bytesRead);

    if (data == nullptr)
    {
        return false;
    }

    fileSize = bytesRead;
    position = 0;
    readOnly = true;

    return true;
}

bool BinaryFile::create(const void* buffer, u32 size)
{
    clear();

    if (buffer == nullptr || size == 0)
    {
        return false;
    }

    data = malloc(size);
    if (data == nullptr)
    {
        return false;
    }

    memcpy(data, buffer, size);
    fileSize = size;
    position = 0;
    readOnly = false;

    return true;
}

bool BinaryFile::save(const char* filename)
{
    if (data == nullptr || fileSize == 0)
    {
        return false;
    }

    return SaveFileData(filename, data, fileSize);
}

void BinaryFile::clear()
{
    if (data != nullptr)
    {
        LogInfo("Freeing file data.");
        if (readOnly)
        {
            UnloadFileData((unsigned char*)data);
        }
        else
        {
            free(data);
        }
        data = nullptr;
    }

    position = 0;
    fileSize = 0;
    readOnly = false;
}

u32 BinaryFile::getFileSize() { return fileSize; }
u32 BinaryFile::ftell() { return position; }

u32 BinaryFile::seek(u32 offset, u32 origin)
{
    u32 newPosition = position;

    switch (origin)
    {
        case SEEK_SET: newPosition = offset; break;
        case SEEK_CUR: newPosition = position + offset; break;
        case SEEK_END: newPosition = fileSize + offset; break;
    }

    if (newPosition > fileSize)
    {
        newPosition = fileSize;
    }

    position = newPosition;
    return position;
}

u32 BinaryFile::readBytes(void* buffer, u32 size)
{
    if (data == nullptr || buffer == nullptr || size == 0
        || position >= fileSize)
    {
        return 0;
    }

    u32 bytesToRead = size;
    if (position + bytesToRead > fileSize)
    {
        bytesToRead = fileSize - position;
    }

    memcpy(buffer, (u8*)data + position, bytesToRead);
    position += bytesToRead;

    return bytesToRead;
}

u32 BinaryFile::writeBytes(const void* buffer, u32 size)
{
    if (readOnly || data == nullptr || buffer == nullptr || size == 0)
    {
        return 0;
    }

    // Verifica se precisa expandir o buffer
    if (position + size > fileSize)
    {
        u32 newSize = position + size;
        void* newData = realloc(data, newSize);
        if (newData == nullptr)
        {
            return 0;
        }
        data = newData;
        fileSize = newSize;
    }

    memcpy((u8*)data + position, buffer, size);
    position += size;

    return size;
}

 
float BinaryFile::readFloat()
{
    float value = 0.0f;
    readBytes(&value, 4); // Sempre 4 bytes
    return value;
}

u8 BinaryFile::readByte()
{
    u8 value = 0;
    readBytes(&value, 1); // Sempre 1 byte
    return value;
}

s16 BinaryFile::readShort()
{
    s16 value = 0;
    readBytes(&value, 2); // Sempre 2 bytes
    return value;
}

u16 BinaryFile::readUShort()
{
    u16 value = 0;
    readBytes(&value, 2); // Sempre 2 bytes
    return value;
}

s32 BinaryFile::readInt()
{
    s32 value = 0;
    readBytes(&value, 4); // Sempre 4 bytes
    return value;
}

u32 BinaryFile::readUInt()
{
    u32 value = 0;
    readBytes(&value, 4); // Sempre 4 bytes
    return value;
}

s64 BinaryFile::readLong()
{
    s64 value = 0;
    readBytes(&value, 8); // Sempre 8 bytes
    return value;
}

u64 BinaryFile::readULong()
{
    u64 value = 0;
    readBytes(&value, 8); // Sempre 8 bytes
    return value;
}

// Métodos de escrita com tamanhos fixos
void BinaryFile::writeFloat(float value)
{
    writeBytes(&value, 4); // Sempre 4 bytes
}

void BinaryFile::writeByte(u8 value)
{
    writeBytes(&value, 1); // Sempre 1 byte
}

void BinaryFile::writeShort(s16 value)
{
    writeBytes(&value, 2); // Sempre 2 bytes
}

void BinaryFile::writeUShort(u16 value)
{
    writeBytes(&value, 2); // Sempre 2 bytes
}

void BinaryFile::writeInt(s32 value)
{
    writeBytes(&value, 4); // Sempre 4 bytes
}

void BinaryFile::writeUInt(u32 value)
{
    writeBytes(&value, 4); // Sempre 4 bytes
}

void BinaryFile::writeLong(s64 value)
{
    writeBytes(&value, 8); // Sempre 8 bytes
}

void BinaryFile::writeULong(u64 value)
{
    writeBytes(&value, 8); // Sempre 8 bytes
}