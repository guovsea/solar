//
// Created by guo on 2025/7/26.
//

#ifndef __SOLAR_ENDIAN_H__
#define __SOLAR_ENDIAN_H__

#define SOLAR_LITTLE_ENDIAN 1234
#define SOLAR_BIG_ENDIAN 4321

#include <byteswap.h>
#include <type_traits>

namespace solar {
template<typename T>
std::enable_if_t<sizeof(T) == sizeof(uint64_t), T> Byteswap(T value) {
    return static_cast<T>(bswap_64(static_cast<uint64_t>(value)));
}

template<typename T>
std::enable_if_t<sizeof(T) == sizeof(uint32_t), T> Byteswap(T value) {
    return static_cast<T>(bswap_32(static_cast<uint32_t>(value)));
}
template<typename T>
std::enable_if_t<sizeof(T) == sizeof(uint16_t), T> Byteswap(T value) {
    return static_cast<T>(bswap_16(static_cast<uint16_t>(value)));
}

#if BYTE_ORDER == BIG_ENDIAN
#define SOLAR_BYTE_ORDER SOLAR_BIG_ENDIAN
#else
#define SOLAR_BYTE_ORDER SOLAR_LITTLE_ENDIAN
#endif

#if SOLAR_BYTE_ORDER == SOLAR_BIG_ENDIAN
/**
 *
 * @brief 只在小端机上执行
 * @return
 */
template<typename T>
T ByteswapOnLittleEndian(T t) {
    return t;
}

/**
 *
 * @brief 只在大端机上执行
 * @return
 */
template<typename T>
T ByteswapOnBigEndian(T t) {
    return Byteswap(t);
}
#else
/**
 *
 * @brief 只在小端机上执行
 * @return
 */
template<typename T>
T ByteswapOnLittleEndian(T t) {
    return Byteswap(t);
}

/**
 *
 * @brief 只在大端机上执行
 * @return
 */
template<typename T>
T ByteswapOnBigEndian(T t) {
    return t;
}
#endif

} // namespace solar
#endif //__SOLAR_ENDIAN_H__
