//
// Created by guo on 2025/8/10.
//

#ifndef __SOLAR_STREAM_H__
#define __SOLAR_STREAM_H__

#include <memory>
#include "ByteArray.h"

namespace solar {
class Stream {
public:
    typedef std::shared_ptr<Stream> ptr;
    virtual ~Stream();

    /**
     *
     * @param buffer
     * @param length
     * @return 读取的字节数，错误返回 -1
     */
    virtual int read(void *buffer, size_t length) = 0;
    virtual int read(ByteArray::ptr ba, size_t length) = 0;

    /**
     * @brief 多次 read，直到 read 完所有希望读取的数据
     * @param buffer
     * @param length 希望读取的数据的字节数
     * @return
     */
    virtual int readFixSize(void *buffer, size_t length);
    virtual int readFixSize(ByteArray::ptr ba, size_t length);

    /**
     *
     * @param buffer 写入的字节数，错误返回 -1
     * @param length
     * @return
     */
    virtual int write(const void *buffer, size_t length) = 0;
    virtual int write(ByteArray::ptr ba, size_t length) = 0;

    /**
     * @brief 多次 write 直到 write 完所有希望读取的数据
     * @param buffer
     * @param length 希望写入的数据的字节数
     * @return
     */
    virtual int writeFixSize(const void *buffer, size_t length);
    virtual int writeFixSize(ByteArray::ptr ba, size_t length);

    virtual void close() = 0;
};
}

#endif //__SOLAR_STREAM_H__