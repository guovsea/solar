//
// Created by guo on 2025/7/28.
//

#ifndef __SOLAR_BYTEARRAY_H__
#define __SOLAR_BYTEARRAY_H__

#include <memory>
#include <string>
#include <stdint.h>

namespace solar {
class ByteArray {
public:
    typedef std::shared_ptr<ByteArray> ptr;
    struct Node {
        Node(size_t s);
        Node();
        ~Node();
        char* ptr;
        size_t size;
        Node* next;
    };

    ByteArray(size_t base_size = 4096);
    ~ByteArray();

    // write
    void writeFint8  (const int8_t& value);
    void writeFuint8 (const uint8_t& value);
    void writeFint16 (const int16_t& value);
    void writeFuint16(const uint16_t& value);
    void writeFint32 (const int32_t& value);
    void writeFuint32(const uint32_t& value);
    void writeFint64 (const int64_t& value);
    void writeFuint64(const uint64_t& value);

    // 压缩
    void writeInt32(const int32_t& value);
    void writeUint32(const uint32_t& value);
    void writeInt64(const int64_t& value);
    void writeUint64(const uint64_t& value);

    void writeFloat(const float& value);
    void writeDouble(const double& value);
    // length:int16, data
    void writeString16(const std::string& value);
    // length:int32, data
    void writeString32(const std::string& value);
    // length:int64, data
    void writeString64(const std::string& value);
    // length:varint, data
    void writeStringVint(const std::string& value);
    // data
    void writeStringWithoutLength(const std::string& value);

    // read
    int8_t   readFint8();
    uint8_t  readFuint8();
    int16_t  readFint16();
    uint16_t readFuint16();
    int64_t  readFint64();
    uint64_t readFuint64();

    int32_t  readInt32();
    uint32_t readUint32();
    int64_t  readInt64();
    uint64_t readUint64();

    float    readFloat();
    double   readDouble();

    // length:int16, data
    std::string readStringF16();
    // length:int32, data
    std::string readStringF32();
    // length:int64, data
    std::string readStringF64();
    // length:varint, data
    std::string readStringVint();

    // 操作
    void clear();
    void write(const void* buf, size_t size);
    void read(char* buf, size_t size);

    size_t getPosition() const { return m_position; }
    void setPosition(size_t v);

    bool writeToFile(const std::string& name);
    bool readFromFile(const std::string& name);

    size_t getBaseSize() const { return m_baseSize; }

    /**
     *
     * @return 当前可读数据大小
     */
    size_t getReadSize() const {return m_size - m_position; }

    bool isLittleEndian() const;
    void setLittleEndian(bool val);
private:
    void addCapacity(size_t size);

    /**
     *
     * @return 当前剩余容量
     */
    size_t getCapacity() const { return m_capacity - m_position; }
private:
    size_t m_baseSize;
    size_t m_position;
    size_t m_capacity;
    size_t m_size;
    int m_endian;

    Node* m_root;
    Node* m_cur;
};
} // solar

#endif //__SOLAR_BYTEARRAY_H__