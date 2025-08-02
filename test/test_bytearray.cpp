//
// Created by guo on 2025/8/1.
//

#include <gtest/gtest.h>
#include <random>
#include "Log/Log.h"
#include "Network/ByteArray.h"
#include <numeric>

static solar::Logger::ptr g_logger = SOLAR_LOG_ROOT();

template<typename T,typename WriteFun, typename ReadFun>
static void test_read_write_single_type(const std::vector<T>& vec, WriteFun write_fun, ReadFun read_fun, size_t base_size) {
    solar::ByteArray::ptr ba = std::make_shared<solar::ByteArray>(base_size);
    for (const T i : vec) {
        (ba.get()->*write_fun)(i);
    }
    ba->setPosition(0);
    for (T i : vec) {
        T v = (ba.get()->*read_fun)();
        EXPECT_EQ(i, v);
    }
    EXPECT_EQ(ba->getReadSize(), 0);
}

template<typename T,typename WriteFun, typename ReadFun>
static void test_read_write_integral(WriteFun write_fun, ReadFun read_fun) {
    std::vector<T> vec;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    for (int i = 0; i < 1000; ++i) {
        vec.push_back(dist(gen));
    }
    test_read_write_single_type<T>(vec, write_fun, read_fun, 1);
    test_read_write_single_type<T>(vec, write_fun, read_fun, 2);
    test_read_write_single_type<T>(vec, write_fun, read_fun, 3);
    test_read_write_single_type<T>(vec, write_fun, read_fun, 4);
    test_read_write_single_type<T>(vec, write_fun, read_fun, 100);
}

TEST(TestByteArray, test_fixed_read_write_int8_t) {
    test_read_write_integral<int8_t>(&solar::ByteArray::writeFint8, &solar::ByteArray::readFint8);
}

TEST(TestByteArray, test_fixed_read_write_int16_t) {
    test_read_write_integral<int16_t>(&solar::ByteArray::writeFint16, &solar::ByteArray::readFint16);
}

TEST(TestByteArray, test_fixed_read_write_int32_t) {
    test_read_write_integral<int32_t>(&solar::ByteArray::writeFint32, &solar::ByteArray::readFint32);
}

TEST(TestByteArray, test_fixed_read_write_int64_t) {
    test_read_write_integral<int64_t>(&solar::ByteArray::writeFint64, &solar::ByteArray::readFint64);
}

TEST(TestByteArray, test_fixed_read_write_uint8_t) {
    test_read_write_integral<uint8_t>(&solar::ByteArray::writeFuint8, &solar::ByteArray::readFuint8);
}

TEST(TestByteArray, test_fixed_read_write_uint16_t) {
    test_read_write_integral<uint16_t>(&solar::ByteArray::writeFuint16, &solar::ByteArray::readFuint16);
}

TEST(TestByteArray, test_fixed_read_write_uint32_t) {
    test_read_write_integral<uint32_t>(&solar::ByteArray::writeFuint32, &solar::ByteArray::readFuint32);
}

TEST(TestByteArray, test_fixed_read_write_uint64_t) {
    test_read_write_integral<uint64_t>(&solar::ByteArray::writeFuint64, &solar::ByteArray::readFuint64);
}

TEST(TestByteArray, test_read_write_int32_t) {
    test_read_write_integral<int32_t>(&solar::ByteArray::writeInt32, &solar::ByteArray::readInt32);
}

TEST(TestByteArray, test_read_write_uint32_t) {
    test_read_write_integral<uint32_t>(&solar::ByteArray::writeUint32, &solar::ByteArray::readUint32);
}

TEST(TestByteArray, test_read_write_int64_t) {
    test_read_write_integral<int64_t>(&solar::ByteArray::writeInt64, &solar::ByteArray::readInt64);
}

TEST(TestByteArray, test_read_write_uint64_t) {
    test_read_write_integral<uint64_t>(&solar::ByteArray::writeUint64, &solar::ByteArray::readUint64);
}

template<typename T,typename WriteFun, typename ReadFun>
void test_read_write_float_point(WriteFun write_fun, ReadFun read_fun) {
    std::vector<T> vec;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<T> dist(-100, 100);
    for (int i = 0; i < 1000; ++i) {
        vec.push_back(dist(gen));
    }
    test_read_write_single_type<T>(vec, write_fun, read_fun, 1);
    test_read_write_single_type<T>(vec, write_fun, read_fun, 2);
    test_read_write_single_type<T>(vec, write_fun, read_fun, 3);
    test_read_write_single_type<T>(vec, write_fun, read_fun, 4);
    test_read_write_single_type<T>(vec, write_fun, read_fun, 100);
}

TEST(TestByteArray, test_fixed_read_write_float) {
    test_read_write_float_point<float>(&solar::ByteArray::writeFloat, &solar::ByteArray::readFloat);
}

TEST(TestByteArray, test_fixed_read_write_double) {
    test_read_write_float_point<double>(&solar::ByteArray::writeDouble, &solar::ByteArray::readDouble);
}

template<typename T,typename WriteFun, typename ReadFun>
static void test_to_and_from_file(const std::vector<T>& vec, WriteFun write_fun, ReadFun read_fun, size_t base_size, const std::string& filename) {
    solar::ByteArray::ptr ba = std::make_shared<solar::ByteArray>(base_size);
    for (const T i : vec) {
        (ba.get()->*write_fun)(i);
    }
    ba->setPosition(0);
    EXPECT_EQ(ba->getPosition(), 0);
    EXPECT_TRUE(ba->writeToFile(filename));
    ba->setPosition(0);

    solar::ByteArray::ptr ba2 = std::make_shared<solar::ByteArray>(base_size * 2);
    ba2->readFromFile(filename);
    ba2->setPosition(0);

    EXPECT_EQ(ba->toString(), ba2->toString());
    for (T i : vec) {
        T v = (ba2.get()->*read_fun)();
        EXPECT_EQ(i, v);
    }
    EXPECT_EQ(ba2->getReadSize(), 0);
}

template<typename T,typename WriteFun, typename ReadFun>
static void test_to_and_from_file(const std::vector<T>& vec, WriteFun write_fun, ReadFun read_fun, const std::string& filename) {
    test_to_and_from_file<T>(vec, write_fun, read_fun, 1, filename);
    test_to_and_from_file<T>(vec, write_fun, read_fun, 2, filename);
    test_to_and_from_file<T>(vec, write_fun, read_fun, 3, filename);
    test_to_and_from_file<T>(vec, write_fun, read_fun, 4, filename);
    test_to_and_from_file<T>(vec, write_fun, read_fun, 100, filename);
}
template<typename T,typename WriteFun, typename ReadFun>
static void test_to_and_from_file(WriteFun write_fun, ReadFun read_fun, const std::string& filename) {
    std::vector<T> vec;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    for (int i = 0; i < 1000; ++i) {
        vec.push_back(dist(gen));
    }
    test_to_and_from_file<T>(vec, write_fun, read_fun, 1, filename);
    test_to_and_from_file<T>(vec, write_fun, read_fun, 2, filename);
    test_to_and_from_file<T>(vec, write_fun, read_fun, 3, filename);
    test_to_and_from_file<T>(vec, write_fun, read_fun, 4, filename);
    test_to_and_from_file<T>(vec, write_fun, read_fun, 100, filename);
}

TEST(TestByteArray, test_to_and_from_file) {
    test_to_and_from_file<int8_t>(&solar::ByteArray::writeFint8, &solar::ByteArray::readFint8, "/tmp/test_to_and_from_file_Fint8");
    test_to_and_from_file<uint8_t>(&solar::ByteArray::writeFuint8, &solar::ByteArray::readFuint8, "/tmp/test_to_and_from_file_Fuint8");

    test_to_and_from_file<int16_t>(&solar::ByteArray::writeFint16, &solar::ByteArray::readFint16, "/tmp/test_to_and_from_file_Fint16");
    test_to_and_from_file<uint16_t>(&solar::ByteArray::writeFuint16, &solar::ByteArray::readFuint16, "/tmp/test_to_and_from_file_Fuint16");

    test_to_and_from_file<int32_t>(&solar::ByteArray::writeFint32, &solar::ByteArray::readFint32, "/tmp/test_to_and_from_file_Fint32");
    test_to_and_from_file<uint32_t>(&solar::ByteArray::writeFuint32, &solar::ByteArray::readFuint32, "/tmp/test_to_and_from_file_Fuint32");

    test_to_and_from_file<int32_t>(&solar::ByteArray::writeInt32, &solar::ByteArray::readInt32, "/tmp/test_to_and_from_file_Int32");
    test_to_and_from_file<uint32_t>(&solar::ByteArray::writeUint32, &solar::ByteArray::readUint32, "/tmp/test_to_and_from_file_Uint32");

    test_to_and_from_file<int64_t>(&solar::ByteArray::writeFint64, &solar::ByteArray::readFint64, "/tmp/test_to_and_from_file_Fint64");
    test_to_and_from_file<uint64_t>(&solar::ByteArray::writeFuint64, &solar::ByteArray::readFuint64, "/tmp/test_to_and_from_file_Fuint64");

    test_to_and_from_file<int64_t>(&solar::ByteArray::writeInt64, &solar::ByteArray::readInt64, "/tmp/test_to_and_from_file_Int64");
    test_to_and_from_file<uint64_t>(&solar::ByteArray::writeUint64, &solar::ByteArray::readUint64, "/tmp/test_to_and_from_file_Uint64");
}