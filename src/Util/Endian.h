//
// Created by guo on 2025/7/26.
//

#ifndef __SOLAR_ENDIAN_H__
#define __SOLAR_ENDIAN_H__

#include  <byteswap.h>
#include <type_traits>

namespace solar {
template<typename T>
std::enable_if_t<sizeof(T) == sizeof(uint64_t)>
Byteswap(T value) {
    return static_cast<T>(bswap_64(static_cast<uint64_t>(value)));
}

template<typename T>
std::enable_if_t<sizeof(T) == sizeof(uint32_t)>
Byteswap(T value) {
    return static_cast<T>(bswap_64(static_cast<uint32_t>(value)));
}
template<typename T>
std::enable_if_t<sizeof(T) == sizeof(uint16_t)>
Byteswap(T value) {
    return static_cast<T>(bswap_64(static_cast<uint16_t>(value)));
}

#if BYTE_ORDER == BIG_ENDIAN
#define SOLAR_BYTE_ORDER SOLAR_BIG_ENDIAN
#else
#define SOLAR_BYTE_ORDER SOLAR_LITTLE_ENDIAN
#endif

#if SOLAR_BYTE_ORDER == SOLAR_BIG_ENDIAN
template<typename T>
T ByteswapOnLittleEndian(T t) {
    return t;
}
template<typename T>
T ByteswapOnBigEndian(T t) {
    return byteswap(t);
}
#else
template<typename T>
T ByteswapOnLittileEndian(T t) {
    return byteswap(t);
}
template<typename T>
T ByteswapOnBigEndian(T t) {
    return t;
}
#endif

}
#endif //__SOLAR_ENDIAN_H__