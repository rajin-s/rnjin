#include "unit_test.hpp"
#include "unit_test/bitmask.test.hpp"
#include "unit_test/file.test.hpp"
#include "unit_test/log.test.hpp"
#include "unit_test/math.test.hpp"
#define TESTS GET_TEST(bitmask), GET_TEST(file), GET_TEST(log), GET_TEST(math)