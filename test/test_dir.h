#ifndef __SOLAR_TEST_TEST_UTIL_H__
#define __SOLAR_TEST_TEST_UTIL_H__
#ifndef TEST_DIR
#ifdef __TEST_DIR
#define TEST_DIR std::string(__TEST_DIR)
#elif
#define TEST_DIR std::string(".")
#endif
#endif

#endif