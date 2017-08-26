/**
 * Copyright (C) 2017-present Jung-Sang Ahn <jungsang.ahn@gmail.com>
 * All rights reserved.
 *
 * https://github.com/greensky00
 *
 * Test Suite
 * Version: 0.1.17
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#define _CLM_D_GRAY    "\033[1;30m"
#define _CLM_GREEN     "\033[32m"
#define _CLM_B_GREEN   "\033[1;32m"
#define _CLM_RED       "\033[31m"
#define _CLM_B_RED     "\033[1;31m"
#define _CLM_BROWN     "\033[33m"
#define _CLM_BLUE      "\033[34m"
#define _CLM_B_BLUE    "\033[1;34m"
#define _CLM_MAGENTA   "\033[35m"
#define _CLM_B_MAGENTA "\033[1;35m"
#define _CLM_CYAN      "\033[36m"
#define _CLM_END       "\033[0m"

#define _CL_D_GRAY(str)    _CLM_D_GRAY  str _CLM_END
#define _CL_GREEN(str)     _CLM_GREEN   str _CLM_END
#define _CL_RED(str)       _CLM_RED     str _CLM_END
#define _CL_MAGENTA(str)   _CLM_MAGENTA str _CLM_END
#define _CL_BROWN(str)     _CLM_BROWN   str _CLM_END
#define _CL_B_MAGENTA(str) _CLM_MAGENTA str _CLM_END
#define _CL_CYAN(str)      _CLM_CYAN    str _CLM_END


#define CHK_EQ(exp_value, value)                                         \
    if ((exp_value) != (value)) {                                        \
        std::cout                                                        \
        << "\n    " _CLM_GREEN << __FILE__ << _CLM_END ":"               \
        << _CLM_GREEN << __LINE__ << _CLM_END << ", "                    \
        << _CLM_CYAN << __func__ << "()" _CLM_END << "\n"                \
        << "    value of: " _CLM_B_BLUE #value _CLM_END "\n"             \
        << "    expected: " _CLM_B_GREEN << (exp_value) << _CLM_END "\n" \
        << "      actual: " _CLM_B_RED << (value) << _CLM_END "\n";      \
        return -1;                                                       \
    }

#define CHK_OK(value)                                                \
    if (!(value)) {                                                  \
        std::cout                                                    \
        << "\n    " _CLM_GREEN << __FILE__ << _CLM_END ":"           \
        << _CLM_GREEN << __LINE__ << _CLM_END << ", "                \
        << _CLM_CYAN << __func__ << "()" _CLM_END << "\n"            \
        << "    value of: " _CLM_B_BLUE #value _CLM_END "\n"         \
        << "    expected: " _CLM_B_GREEN << "true" << _CLM_END "\n"  \
        << "      actual: " _CLM_B_RED << "false" << _CLM_END "\n";  \
        return -1;                                                   \
    }

#define CHK_NOT(value)                                               \
    if (value) {                                                     \
        std::cout                                                    \
        << "\n    " _CLM_GREEN << __FILE__ << _CLM_END ":"           \
        << _CLM_GREEN << __LINE__ << _CLM_END << ", "                \
        << _CLM_CYAN << __func__ << "()" _CLM_END << "\n"            \
        << "    value of: " _CLM_B_BLUE #value _CLM_END "\n"         \
        << "    expected: " _CLM_B_GREEN << "false" << _CLM_END "\n" \
        << "      actual: " _CLM_B_RED << "true" << _CLM_END "\n";   \
        return -1;                                                   \
    }

#define CHK_NULL(value)                                                \
    if (value) {                                                       \
        std::cout                                                      \
        << "\n    " _CLM_GREEN << __FILE__ << _CLM_END ":"             \
        << _CLM_GREEN << __LINE__ << _CLM_END << ", "                  \
        << _CLM_CYAN << __func__ << "()" _CLM_END << "\n"              \
        << "    value of: " _CLM_B_BLUE #value _CLM_END "\n"           \
        << "    expected: " _CLM_B_GREEN << "NULL" << _CLM_END "\n";   \
        printf("      actual: " _CLM_B_RED "%p" _CLM_END "\n", value); \
        return -1;                                                     \
    }

#define CHK_NONNULL(value)                                              \
    if (!(value)) {                                                     \
        std::cout                                                       \
        << "\n    " _CLM_GREEN << __FILE__ << _CLM_END ":"              \
        << _CLM_GREEN << __LINE__ << _CLM_END << ", "                   \
        << _CLM_CYAN << __func__ << "()" _CLM_END << "\n"               \
        << "    value of: " _CLM_B_BLUE #value _CLM_END "\n"            \
        << "    expected: " _CLM_B_GREEN << "non-NULL" << _CLM_END "\n" \
        << "      actual: " _CLM_B_RED << "NULL" << _CLM_END "\n";      \
        return -1;                                                      \
    }

#define CHK_SM(smaller, greater) \
    if (!((smaller) < (greater))) {                           \
        std::cout                                             \
        << "\n    " _CLM_GREEN << __FILE__ << _CLM_END ":"    \
        << _CLM_GREEN << __LINE__ << _CLM_END << ", "         \
        << _CLM_CYAN << __func__ << "()" _CLM_END << "\n"     \
        << "    expected: "                                   \
        << _CLM_B_BLUE #smaller " < " #greater _CLM_END "\n"  \
        << "    value of "                                    \
        << _CLM_B_GREEN #smaller _CLM_END ": "                \
        << _CLM_B_RED << smaller << _CLM_END "\n"             \
        << "    value of "                                    \
        << _CLM_B_GREEN #greater _CLM_END ": "                \
        << _CLM_B_RED << greater << _CLM_END "\n";            \
        return -1;                                            \
    }

#define CHK_SMEQ(smaller , greater)                           \
    if (!((smaller) <= (greater))) {                          \
        std::cout                                             \
        << "\n    " _CLM_GREEN << __FILE__ << _CLM_END ":"    \
        << _CLM_GREEN << __LINE__ << _CLM_END << ", "         \
        << _CLM_CYAN << __func__ << "()" _CLM_END << "\n"     \
        << "    expected: "                                   \
        << _CLM_B_BLUE #smaller " <= " #greater _CLM_END "\n" \
        << "    value of "                                    \
        << _CLM_B_GREEN #smaller _CLM_END ": "                \
        << _CLM_B_RED << smaller << _CLM_END "\n"             \
        << "    value of "                                    \
        << _CLM_B_GREEN #greater _CLM_END ": "                \
        << _CLM_B_RED << greater << _CLM_END "\n";            \
        return -1;                                            \
    }

#define CHK_GT(greater, smaller)                              \
    if (!((greater) > (smaller))) {                           \
        std::cout                                             \
        << "\n    " _CLM_GREEN << __FILE__ << _CLM_END ":"    \
        << _CLM_GREEN << __LINE__ << _CLM_END << ", "         \
        << _CLM_CYAN << __func__ << "()" _CLM_END << "\n"     \
        << "    expected: "                                   \
        << _CLM_B_BLUE #greater " > " #smaller _CLM_END "\n"  \
        << "    value of "                                    \
        << _CLM_B_GREEN #greater _CLM_END ": "                \
        << _CLM_B_RED << greater << _CLM_END "\n"             \
        << "    value of "                                    \
        << _CLM_B_GREEN #smaller _CLM_END ": "                \
        << _CLM_B_RED << smaller << _CLM_END "\n";            \
        return -1;                                            \
    }

#define CHK_GTEQ(greater, smaller)                            \
    if (!((greater) >= (smaller))) {                          \
        std::cout                                             \
        << "\n    " _CLM_GREEN << __FILE__ << _CLM_END ":"    \
        << _CLM_GREEN << __LINE__ << _CLM_END << ", "         \
        << _CLM_CYAN << __func__ << "()" _CLM_END << "\n"     \
        << "    expected: "                                   \
        << _CLM_B_BLUE #greater " >= " #smaller _CLM_END "\n" \
        << "    value of "                                    \
        << _CLM_B_GREEN #greater _CLM_END ": "                \
        << _CLM_B_RED << greater << _CLM_END "\n"             \
        << "    value of "                                    \
        << _CLM_B_GREEN #smaller _CLM_END ": "                \
        << _CLM_B_RED << smaller << _CLM_END "\n";            \
        return -1;                                            \
    }


using test_func = std::function<int()>;

class TestArgsBase;
using test_func_args = std::function<int(TestArgsBase*)>;

class TestSuite;
class TestArgsBase {
public:
    virtual ~TestArgsBase() { }
    void setCallback(std::string test_name,
                     test_func_args func,
                     TestSuite* test_instance) {
        testName = test_name;
        testFunction = func;
        testInstance = test_instance;
    }

    void testAll() { testAllInternal(0); }

    virtual void setParam(size_t param_no, size_t param_idx) {
        (void)param_no;
        (void)param_idx;
    }

    virtual size_t getNumSteps(size_t param_no) {
        (void)param_no;
        return 0;
    }

    virtual size_t getNumParams() { return 0; }
    virtual std::string toString() { return ""; }

private:
    inline void testAllInternal(size_t depth);

    std::string testName;
    test_func_args testFunction;
    TestSuite* testInstance;
};

class TestArgsWrapper {
public:
    TestArgsWrapper(TestArgsBase* _test_args) : test_args(_test_args) {}
    ~TestArgsWrapper() {
        delete test_args;
    }
    TestArgsBase* getArgs() const { return test_args; }
private:
    TestArgsBase* test_args;
};

enum class StepType {
    LINEAR,
    EXPONENTIAL
};

template<typename T>
class TestRange {
public:
    TestRange() {
        type = RangeType::NONE;
    }

    // Constructor for given values
    TestRange(const std::vector<T>& _array) :
        type(RangeType::ARRAY), array(_array)
    { }

    // Constructor for regular steps
    TestRange(T _begin, T _end, T _step,
              StepType _type = StepType::LINEAR) :
        begin(_begin), end(_end), step(_step)
    {
        if (_type == StepType::LINEAR) {
            type = RangeType::LINEAR;
        } else {
            type = RangeType::EXPONENTIAL;
        }
    }

    T getEntry(size_t idx) {
        if (type == RangeType::ARRAY) {
            return array[idx];
        } else if (type == RangeType::LINEAR) {
            return begin + step * idx;
        } else if (type == RangeType::EXPONENTIAL) {
            return begin * std::pow(step, idx);
        }

        return begin;
    }

    size_t getSteps() {
        if (type == RangeType::ARRAY) {
            return array.size();
        } else if (type == RangeType::LINEAR) {
            return ((end - begin) / step) + 1;
        } else if (type == RangeType::EXPONENTIAL) {
            size_t coe = end / begin;
            double steps_double = (double)std::log(coe) / std::log(step);
            return steps_double + 1;
        }

        return 0;
    }

private:
    enum class RangeType {
        NONE,
        ARRAY,
        LINEAR,
        EXPONENTIAL
    };

    RangeType type;
    std::vector<T> array;
    T begin;
    T end;
    T step;
};

struct TestOptions {
    TestOptions()
        : printTestMessage(false)
        {}

    bool printTestMessage;
};

class TestSuite {
public:
    TestSuite()
        : cntPass(0),
          cntFail(0),
          startTimeGlobal(std::chrono::system_clock::now()) {}

    TestSuite(int argc, char **argv)
        : cntPass(0),
          cntFail(0),
          startTimeGlobal(std::chrono::system_clock::now()) {
        if (argc >= 2) {
            filter = argv[1];
        }
    }

    ~TestSuite() {
        std::chrono::time_point<std::chrono::system_clock> cur_time =
                std::chrono::system_clock::now();;
        std::chrono::duration<double> elapsed = cur_time - startTimeGlobal;
        std::string time_str = usToString(elapsed.count() * 1000000);

        printf(_CL_GREEN("%zu") " tests passed", cntPass);
        if (cntFail) {
            printf(", " _CL_RED("%zu") " tests failed", cntFail);
        }
        printf(" out of " _CL_CYAN("%zu") " (" _CL_BROWN("%s") ")\n",
               cntPass+cntFail, time_str.c_str());
    }

    static std::string getTestFileName(std::string prefix) {
        std::string ret = prefix;
        int rnd_num = std::rand();
        ret += "_";
        ret += std::to_string(rnd_num);
        return ret;
    }

    static void clearTestFile(std::string prefix) {
        int r;
        (void)r;
        std::string command = "rm -rf ";
        command += prefix;
        command += "*";
        r = system(command.c_str());
    }

    class Timer {
    public:
        Timer(size_t _duration_ms) : duration_ms(_duration_ms) {
            start = std::chrono::system_clock::now();
        }
        bool timeover() {
            auto cur = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed = cur - start;
            if (duration_ms < elapsed.count() * 1000) return true;
            return false;
        }
    private:
        std::chrono::time_point<std::chrono::system_clock> start;
        size_t duration_ms;
    };

    inline void doTest(std::string test_name,
                       test_func func);

    inline void doTest(std::string test_name,
                       test_func_args func,
                       TestArgsWrapper& args_wrapper);

    template<typename T, typename F>
    inline void doTest(std::string test_name,
                       F func,
                       TestRange<T> range);

    inline void doTestCB(std::string test_name,
                         test_func_args func,
                         TestArgsBase* args);

    TestOptions options;

private:
    bool matchFilter(std::string test_name) {
        if (!filter.empty() &&
            test_name.find(filter) == std::string::npos) {
            // Doesn't match with the given filter.
            return false;
        }
        return true;
    }

    void readyTest(std::string& test_name) {
        printf("[ " "...." " ] %s\n", test_name.c_str());
        if (options.printTestMessage) {
            printf(_CL_D_GRAY("   === TEST MESSAGE (BEGIN) ===\n"));
        }
        fflush(stdout);

        startTimeLocal = std::chrono::system_clock::now();
    }

    void reportTestResult(std::string& test_name,
                          int result) {
        std::chrono::time_point<std::chrono::system_clock> cur_time =
                std::chrono::system_clock::now();;
        std::chrono::duration<double> elapsed = cur_time - startTimeLocal;
        std::string time_str = usToString(elapsed.count() * 1000000);

        if (result < 0) {
            printf("[ " _CL_RED("FAIL") " ] %s (" _CL_BROWN("%s") ")\n",
                   test_name.c_str(),
                   time_str.c_str());
            cntFail++;
        } else {
            if (options.printTestMessage) {
                printf(_CL_D_GRAY("   === TEST MESSAGE (END) ===\n"));
            } else {
                // Move a line up.
                printf("\033[1A");
                // Clear current line.
                printf("\r");
                // And then overwrite.
            }
            printf("[ " _CL_GREEN("PASS") " ] %s (" _CL_BROWN("%s") ")\n",
                   test_name.c_str(),
                   time_str.c_str());
            cntPass++;
        }
    }

    std::string usToString(uint64_t us) {
        std::stringstream ss;
        if (us < 1000) {
            // us
            ss << std::fixed << std::setprecision(0) << us << " us";
        } else if (us < 1000000) {
            // ms
            double tmp = static_cast<double>(us / 1000.0);
            ss << std::fixed << std::setprecision(1) << tmp << " ms";
        } else if (us < (uint64_t)600 * 1000000) {
            // second: 1 s -- 600 s (10 mins)
            double tmp = static_cast<double>(us / 1000000.0);
            ss << std::fixed << std::setprecision(1) << tmp << " s";
        } else {
            // minute
            double tmp = static_cast<double>(us / 60.0 / 1000000.0);
            ss << std::fixed << std::setprecision(0) << tmp << " m";
        }
        return ss.str();
    }

    size_t cntPass;
    size_t cntFail;
    std::string filter;
    // Start time of each test.
    std::chrono::time_point<std::chrono::system_clock> startTimeLocal;
    // Start time of the entire test suite.
    std::chrono::time_point<std::chrono::system_clock> startTimeGlobal;
};


// ===== Functor =====

struct TestArgsSetParamFunctor {
    template<typename T>
    void operator()(T* t, TestRange<T>& r, size_t param_idx) const {
        *t = r.getEntry(param_idx);
    }
};

template<std::size_t I = 0,
         typename FuncT,
         typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
TestArgsSetParamScan(int,
                     std::tuple<Tp*...> &,
                     std::tuple<TestRange<Tp>...> &,
                     FuncT,
                     size_t) { }

template<std::size_t I = 0,
         typename FuncT,
         typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), void>::type
TestArgsSetParamScan(int index,
                     std::tuple<Tp*...>& t,
                     std::tuple<TestRange<Tp>...>& r,
                     FuncT f,
                     size_t param_idx) {
    if (index == 0) f(std::get<I>(t), std::get<I>(r), param_idx);
    TestArgsSetParamScan<I + 1, FuncT, Tp...>(index-1, t, r, f, param_idx);
}
struct TestArgsGetNumStepsFunctor {
    template<typename T>
    void operator()(T* t, TestRange<T>& r, size_t& steps_ret) const {
        (void)t;
        steps_ret = r.getSteps();
    }
};

template<std::size_t I = 0,
         typename FuncT,
         typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
TestArgsGetStepsScan(int,
                     std::tuple<Tp*...> &,
                     std::tuple<TestRange<Tp>...> &,
                     FuncT,
                     size_t) { }

template<std::size_t I = 0,
         typename FuncT,
         typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), void>::type
TestArgsGetStepsScan(int index,
                     std::tuple<Tp*...>& t,
                     std::tuple<TestRange<Tp>...>& r,
                     FuncT f,
                     size_t& steps_ret) {
    if (index == 0) f(std::get<I>(t), std::get<I>(r), steps_ret);
    TestArgsGetStepsScan<I + 1, FuncT, Tp...>(index-1, t, r, f, steps_ret);
}

#define TEST_ARGS_CONTENTS()                               \
    void setParam(size_t param_no, size_t param_idx) {     \
        TestArgsSetParamScan(param_no, args, ranges,       \
                             TestArgsSetParamFunctor(),    \
                             param_idx); }                 \
    size_t getNumSteps(size_t param_no) {                  \
        size_t ret = 0;                                    \
        TestArgsGetStepsScan(param_no, args, ranges,       \
                             TestArgsGetNumStepsFunctor(), \
                             ret);                         \
        return ret; }                                      \
    size_t getNumParams() {                                \
        return std::tuple_size<decltype(args)>::value;     \
    }


// ===== TestArgsBase =====

void TestArgsBase::testAllInternal(size_t depth) {
    size_t i;
    size_t n_params = getNumParams();
    size_t n_steps = getNumSteps(depth);

    for (i=0; i<n_steps; ++i) {
        setParam(depth, i);
        if (depth+1 < n_params) {
            testAllInternal(depth+1);
        } else {
            std::string test_name;
            std::string args_name = toString();
            if (!args_name.empty()) {
                test_name = testName + " (" + args_name + ")";
            }
            testInstance->doTestCB(test_name,
                                   testFunction,
                                   this);
        }
    }
}



// ===== TestSuite =====

void TestSuite::doTest(std::string test_name,
                       test_func func)
{
    if (!matchFilter(test_name)) return;

    readyTest(test_name);
    int ret = func();
    reportTestResult(test_name, ret);
}

void TestSuite::doTest(std::string test_name,
                       test_func_args func,
                       TestArgsWrapper& args_wrapper) {
    if (!matchFilter(test_name)) return;

    TestArgsBase* args = args_wrapper.getArgs();
    args->setCallback(test_name, func, this);
    args->testAll();
}

void TestSuite::doTestCB(std::string test_name,
                         test_func_args func,
                         TestArgsBase* args) {
    readyTest(test_name);
    int ret = func(args);
    reportTestResult(test_name, ret);
}

template<typename T, typename F>
void TestSuite::doTest(std::string test_name,
                       F func,
                       TestRange<T> range) {
    if (!matchFilter(test_name)) return;

    size_t n = range.getSteps();
    size_t i;

    for (i=0; i<n; ++i) {
        std::string actual_test_name = test_name;
        std::stringstream ss;
        T cur_arg = range.getEntry(i);
        ss << cur_arg;
        actual_test_name += " (" + ss.str() + ")";

        readyTest(actual_test_name);
        int ret = func(range.getEntry(i));
        reportTestResult(actual_test_name, ret);
    }
}



// ===== Parameter macros =====

#define DEFINE_PARAMS_2(name,                                  \
                        type1, param1, range1,                 \
                        type2, param2, range2)                 \
    class name ## _class : public TestArgsBase {               \
    public:                                                    \
        name ## _class() {                                     \
            args = std::make_tuple(&param1, &param2);          \
            ranges = std::make_tuple(                          \
                         TestRange<type1>range1,               \
                         TestRange<type2>range2 );             \
        }                                                      \
        std::string toString() {                               \
            std::stringstream ss;                              \
            ss << param1 << ", " << param2;                    \
            return ss.str();                                   \
        }                                                      \
        TEST_ARGS_CONTENTS()                                   \
        type1 param1;                                          \
        type2 param2;                                          \
    private:                                                   \
        std::tuple<type1*, type2*> args;                       \
        std::tuple<TestRange<type1>, TestRange<type2>> ranges; \
    };

#define DEFINE_PARAMS_3(name,                                  \
                        type1, param1, range1,                 \
                        type2, param2, range2,                 \
                        type3, param3, range3)                 \
    class name ## _class : public TestArgsBase {               \
    public:                                                    \
        name ## _class() {                                     \
            args = std::make_tuple(&param1, &param2, &param3); \
            ranges = std::make_tuple(                          \
                         TestRange<type1>range1,               \
                         TestRange<type2>range2,               \
                         TestRange<type3>range3 );             \
        }                                                      \
        std::string toString() {                               \
            std::stringstream ss;                              \
            ss << param1 << ", " << param2 << ", " << param3;  \
            return ss.str();                                   \
        }                                                      \
        TEST_ARGS_CONTENTS()                                   \
        type1 param1;                                          \
        type2 param2;                                          \
        type3 param3;                                          \
    private:                                                   \
        std::tuple<type1*, type2*, type3*> args;               \
        std::tuple<TestRange<type1>,                           \
                   TestRange<type2>,                           \
                   TestRange<type3>> ranges;                   \
    };

#define SET_PARAMS(name) \
    TestArgsWrapper name(new name ## _class())

#define GET_PARAMS(name) \
    name ## _class* name = static_cast<name ## _class*>(TEST_args_base__)

#define PARAM_BASE TestArgsBase* TEST_args_base__

#define TEST_SUITE_AUTO_PREFIX __func__

