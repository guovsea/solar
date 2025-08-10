//
// Created by guo on 2025/7/28.
//

#include "ByteArray.h"

#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "Log/Log.h"
#include "Log/Logger.h"
#include "Util/Endian.h"

namespace {

solar::Logger::ptr g_logger = SOLAR_LOG_NAME("system");

uint32_t EncodeZigzag32(int32_t v) {
    // 将负数绝对值减去 1 存到除最低位以外的位上，符号存到最低位为 1
    if (v < 0) {
        return ((uint32_t)(-v) << 1) - 1;
    } else {
        // 将正数绝对值存到除最低位以外的位上，符号存到最低位为 0
        return v << 1;
    }
}

int32_t DecodeZigzag32(uint32_t v) {
    // 如果为负数，实际上是将减一之后的绝对值异或 -1，等价于绝对值取反 +1，又变成了负数
    // 如果为正数，实际上是将绝对值异或 0，不变
    // ~v + 1 <==> (v - 1) ^ -1
    return (v >> 1) ^ -(v & 1);
}

uint64_t EncodeZigzag64(int64_t v) {
    if (v < 0) {
        return ((uint64_t)(-v) << 1) - 1;
    } else {
        return v << 1;
    }
}


int64_t DecodeZigzag64(uint64_t v) {
    return (v >> 1) ^ -(v & 1);
}

}

namespace solar {
ByteArray::Node::Node(size_t s)
    :ptr{new char[s]}
    ,size{s}
    ,next{nullptr} {
}

ByteArray::Node::Node()
    :ptr{nullptr}
    ,size{0}
    ,next{nullptr}
{
}

ByteArray::Node::~Node() {
    if (ptr) {
        delete[] ptr;
    }
}

ByteArray::ByteArray(size_t base_size)
    :m_baseSize{base_size}
    ,m_position{0}
    ,m_capacity{base_size}
    ,m_size{0}
    ,m_endian{SOLAR_BIG_ENDIAN}
    ,m_root{new Node(base_size)}
    ,m_cur{m_root}
{
}

ByteArray::~ByteArray() {
    Node *tmp = m_root;
    while (tmp) {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    }
}

void ByteArray::writeFint8(int8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::writeFuint8(uint8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::writeFint16(int16_t value) {
    if (m_endian != SOLAR_BYTE_ORDER) {
        value = Byteswap(value);
    }
    write(&value, sizeof(value));

}

void ByteArray::writeFuint16(uint16_t value) {
    if (m_endian != SOLAR_BYTE_ORDER) {
        value = Byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint32(int32_t value) {
    if (m_endian != SOLAR_BYTE_ORDER) {
        value = Byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint32(uint32_t value) {
    if (m_endian != SOLAR_BYTE_ORDER) {
        value = Byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint64(int64_t value) {
    if (m_endian != SOLAR_BYTE_ORDER) {
        value = Byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint64(uint64_t value) {
    if (m_endian != SOLAR_BYTE_ORDER) {
        value = Byteswap(value);
    }
    write(&value, sizeof(value));
}
void ByteArray::writeInt32(int32_t value) {
    writeUint32(EncodeZigzag32((value)));
}

void ByteArray::writeUint32(uint32_t value) {
    // Varint 编码
    uint8_t tmp[5];
    uint8_t i = 0;
    while (value >= 0x80) {
        // 与上 0x80 说明后面还有
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    // 最后一字节的最高位一定是 0， 因为 value < 0x80
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeInt64(int64_t value) {
    writeUint64(EncodeZigzag64(value));
}

void ByteArray::writeUint64(uint64_t value) {
    // Varint 编码
    uint8_t tmp[10];
    uint8_t i = 0;
    while (value >= 0x80) {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeFloat(float value) {
    uint32_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint32(v);
}

void ByteArray::writeDouble(double value) {
    uint64_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint64(v);
}

void ByteArray::writeString16(const std::string &value) {
    writeFuint16(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeString32(const std::string &value) {
    writeFuint32(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeString64(const std::string &value) {
    writeFuint64(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringVint(const std::string &value) {
    writeUint64(value.length());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringWithoutLength(const std::string &value) {
    write(value.c_str(), value.size());
}

int8_t ByteArray::readFint8() {
    int8_t v;
    read(&v, sizeof(v));
    return v;
}

uint8_t ByteArray::readFuint8() {
    uint8_t v;
    read(&v, sizeof(v));
    return v;
}

int16_t ByteArray::readFint16() {
    int16_t v;
    read(&v, sizeof(v));
    if (m_endian != SOLAR_BYTE_ORDER) {
        v = Byteswap(v);
    }
    return v;
}

#define XX(type) \
    type v; \
    read(&v, sizeof(v)); \
    if (m_endian != SOLAR_BYTE_ORDER) { \
        v = Byteswap(v); \
    } \
    return v;

uint16_t ByteArray::readFuint16() {
    XX(uint16_t);
}

int32_t ByteArray::readFint32() {
    XX(int32_t);
}

uint32_t ByteArray::readFuint32() {
    XX(uint32_t);
}

int64_t ByteArray::readFint64() {
    XX(int64_t);
}

uint64_t ByteArray::readFuint64() {
    XX(uint64_t);
}
#undef XX

int32_t ByteArray::readInt32() {
    return DecodeZigzag32(readUint32());

}

uint32_t ByteArray::readUint32() {
    uint32_t result = 0;
    for (int i = 0; i < 32; i += 7) {
        uint8_t b = readFuint8();
        if (b < 0x80) {
            result |= ((uint32_t)b) << i;
            // 最高位位 0， 这是这个数字的最后一位
            break;
        } else {
            // 获取后 7 位中实际存储的内容
            result |= (((uint32_t)(b & 0x7f)) << i);
        }
    }
    return result;
}

int64_t ByteArray::readInt64() {
    return DecodeZigzag64(readUint64());
}

uint64_t ByteArray::readUint64() {
    uint64_t result = 0;
    for (int i = 0; i < 64; i += 7) {
        uint8_t b = readFuint8();
        if (b < 0x80) {
            result |= ((uint64_t)b) << i;
            break;
        } else {
            result |= (((uint64_t)(b & 0x7f)) << i);
        }
    }
    return result;
}

float ByteArray::readFloat() {
    uint32_t v = readFuint32();
    float value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

double ByteArray::readDouble() {
    uint64_t v = readFuint64();
    double value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

std::string ByteArray::readStringF16() {
    uint16_t len = readFuint16();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringF32() {
    uint32_t len = readFuint32();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringF64() {
    uint64_t len = readFuint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringVint() {
    uint64_t len = readUint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

void ByteArray::clear() {
    m_position = m_size = 0;
    m_capacity = m_baseSize;
    Node* tmp = m_root->next;
    while (tmp) {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    }
    m_cur = m_root;
    m_root->next = nullptr;
}

void ByteArray::write(const void *buf, size_t size) {
    if (size == 0) {
        return;
    }
    addAvailable(size);

    size_t block_pos = m_position % m_baseSize; // 分区
    size_t block_avail = m_cur->size - block_pos;
    size_t write_pos = 0;
    while (size > 0) {
        if (block_avail >= size) {
            memcpy(m_cur->ptr + block_pos, (const char*)buf + write_pos, size);
            if (m_cur->size == (block_pos + size)) {
                m_cur = m_cur->next;
            }
            m_position += size;
            write_pos += size;
            size = 0;
        } else {
            memcpy(m_cur->ptr + block_pos, (const char*)buf + write_pos, block_avail);
            m_position += block_avail;
            write_pos += block_avail;
            size -= block_avail;
            m_cur = m_cur->next;
            block_avail = m_cur->size;
            block_pos = 0;
        }
    }
    if (m_position > m_size) {
        m_size = m_position;
    }
}

void ByteArray::read(void *buf, size_t size) {
    if (size > getReadSize()) {
        throw std::out_of_range("not enough len");
    }

    size_t block_pos = m_position % m_baseSize;
    size_t block_avail = m_cur->size - block_pos;
    size_t read_pos = 0;
    while (size > 0) {
        if (block_avail >= size) {
            memcpy((char*)buf + read_pos, m_cur->ptr + block_pos, size);
            if (m_cur->size == block_pos + size) {
                m_cur = m_cur->next;
            }
            m_position += size;
            read_pos += size;
            size = 0;
        } else {
            memcpy((char*)buf + read_pos, m_cur->ptr + block_pos, block_avail);
            m_position += block_avail;
            read_pos += block_avail;
            size -= block_avail;
            m_cur = m_cur->next;
            block_avail = m_cur->size;
            block_pos = 0;
        }
    }
}

void ByteArray::read(void *buf, size_t size, size_t position) const {
    if (size + position > m_size) {
        throw std::out_of_range("not enough len");
    }

    size_t block_pos = m_position % m_baseSize;
    size_t block_avail = m_cur->size - block_pos;
    size_t read_pos = 0;
    Node* cur = m_cur;
    while (size > 0) {
        if (block_avail >= size) {
            memcpy((char*)buf + read_pos, cur->ptr + block_pos, size);
            if (cur->size == block_pos + size) {
                cur = cur->next;
            }
            position += size;
            read_pos += size;
            size = 0;
        } else {
            memcpy((char*)buf + read_pos, cur->ptr + block_pos, block_avail);
            position += block_avail;
            read_pos += block_avail;
            size -= block_avail;
            cur = cur->next;
            block_avail = cur->size;
            block_pos = 0;
        }
    }
}

void ByteArray::setPosition(size_t v) {
    if (v > m_capacity) {
        throw std::out_of_range("set_position out of range");
    }
    m_position = v;
    if (m_position > m_size) {
        m_size = m_position;
    }
    m_cur = m_root;
    while (v > m_cur->size) {
        v -= m_cur->size;
        m_cur = m_cur->next;
    }
    if (v == m_cur->size) {
        m_cur = m_cur->next;
    }
}

bool ByteArray::writeToFile(const std::string & name) {
    std::ofstream ofs;
    ofs.open(name, std::ios::trunc | std::ios::binary);
    if (!ofs) { SOLAR_LOG_ERROR(g_logger) << "writeToFile name=" << name
            << " error, " << errno << " errstr=" << strerror(errno);
        return false;
    }

    int64_t read_size = getReadSize();
    int64_t pos = m_position;
    Node* cur = m_cur;

    while (read_size > 0) {
        int diff = pos % m_baseSize;
        int64_t len = (read_size > (int64_t)m_baseSize ? m_baseSize : read_size) - diff;
        ofs.write(cur->ptr + diff, len);
        cur = cur->next;
        pos += len;
        read_size -= len;
    }
    return true;

}

bool ByteArray::readFromFile(const std::string &name) {
    std::ifstream ifs;
    ifs.open(name, std::ios::binary);
    if (!ifs) { SOLAR_LOG_ERROR(g_logger) << "readFromFile name=" << name
            << " error, " << errno << " errstr=" << strerror(errno);
        return false;
    }
    std::shared_ptr<char[]> buff(new char[m_baseSize]);
    while (!ifs.eof()) {
        ifs.read(buff.get(), m_baseSize);
        write(buff.get(), ifs.gcount());
    }
    return true;
}

bool ByteArray::isLittleEndian() const {
    return m_endian == SOLAR_LITTLE_ENDIAN;
}

void ByteArray::setLittleEndian(bool val) {
    m_endian = val ? SOLAR_LITTLE_ENDIAN : SOLAR_BIG_ENDIAN;
}

std::string ByteArray::toString() const {
    std::string str{};
    str.resize(getReadSize());
    if (str.empty()) {
        return str;
    }
    read(&str[0], str.size(), m_position);
    return str;
}

std::string ByteArray::toHexString() const{
    std::string str = toString();
    std::stringstream ss;
    for (size_t i = 0; i < str.size(); ++i) {
        if (i > 0  && i % 32 == 0) {
            ss << std::endl;
        }
        ss << std::setw(2) << std::setfill('0') << std::hex
            << (int)(uint8_t)str[i] << " ";
    }
    return ss.str();
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec> &buffers, uint64_t len) const {
    len = len > getReadSize() ? getReadSize() : len;
    if (len == 0) {
        return 0;
    }
    size_t block_pos = m_position % m_baseSize;
    size_t block_avail  = m_cur->size - block_pos;
    struct iovec iov;
    Node* cur = m_cur;
    size_t size = len;
    while (len > 0) {
        if (block_avail >= len) {
            iov.iov_base = cur->ptr + block_pos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + block_pos;
            iov.iov_len = block_avail;
            len -= block_avail;
            cur = cur->next;
            block_avail = m_cur->size;
            block_pos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec> &buffers, uint64_t len, uint64_t position) const {
    len = len > getReadSize() ? getReadSize() : len;
    if (len == 0) {
        return 0;
    }
    size_t block_pos = position % m_baseSize;
    size_t count = position / m_baseSize;
    Node* cur = m_cur;
    while (count > 0) {
        cur = cur->next;
        --count;
    }

    size_t block_avail  = m_cur->size - block_pos;
    struct iovec iov;
    size_t size = len;
    while (len > 0) {
        if (block_avail >= len) {
            iov.iov_base = cur->ptr + block_pos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + block_pos;
            iov.iov_len = block_avail;
            len -= block_avail;
            cur = cur->next;
            block_avail = m_cur->size;
            block_pos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

uint64_t ByteArray::getWriteBuffers(std::vector<iovec> &buffers, uint64_t len) {
    if (len == 0) {
        return 0;
    }
    addAvailable(len);
    uint64_t size = len;
    size_t block_pos = m_position % m_baseSize;
    size_t block_avail = m_cur->size - block_pos;
    iovec iov{};
    Node* cur = m_cur;
    while (len > 0) {
        if (block_avail >= len) {
            iov.iov_base = cur->ptr + block_pos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + block_pos;
            iov.iov_len = block_avail;
            len -= block_avail;
            cur = cur->next;
            block_avail = cur->size;
            block_pos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

void ByteArray::addAvailable(size_t size) {
    if (size == 0) {
        return;
    }
    size_t old_avail = getAvailable();
    if (old_avail >= size) {
        return;
    }
    size = size - old_avail;
    size_t count = (size / m_baseSize) + (size % m_baseSize ? 1 : 0);
    Node* tmp = m_root;
    while (tmp->next) {
        tmp = tmp->next;
    }
    Node* first = nullptr;
    for (size_t i = 0; i < count; ++i) {
        tmp->next = new Node(m_baseSize);
        if (first == nullptr) {
            first = tmp->next;
        }
        tmp = tmp->next;
        m_capacity += m_baseSize;
    }
    if (old_avail == 0) {
        m_cur = first;
    }
}

} // solar