#ifndef MERIDIAN_COMMON_ASSERT_HPP
#define MERIDIAN_COMMON_ASSERT_HPP

#include <deque>
#include <string>
#include <utility>
#include <cstdint>

namespace meridian {
namespace common {

class AssertHelper
{
public:
    /**
     * Ender of macro expansion
     */
    AssertHelper& MERIDIAN_ASSERT_HELPER_A;
    AssertHelper& MERIDIAN_ASSERT_HELPER_B;

    AssertHelper(const char* cond, const char* fn, int line)
      : MERIDIAN_ASSERT_HELPER_A(*this),
        MERIDIAN_ASSERT_HELPER_B(*this),
        mConditionStr(cond),
        mFilename(fn),
        mLine(line)
    {}
    ~AssertHelper();

    AssertHelper& show(const char* msg, const std::string& x)
    {
        mValues.push_back(std::make_pair(std::string(msg), x));
        return *this;
    }
    AssertHelper& show(const char* msg, bool x)
    {
        mValues.push_back(std::make_pair(std::string(msg), x ? "true" : "false"));
        return *this;
    }
    AssertHelper& show(const char* msg, char x)
    {
        mValues.push_back(std::make_pair(std::string(msg), std::string(1, x)));
        return *this;
    }
    AssertHelper& show(const char* msg, int8_t x)
    {
        return show(msg, (int64_t) x);
    }
    AssertHelper& show(const char* msg, uint8_t x)
    {
        return show(msg, (uint64_t) x);
    }
    AssertHelper& show(const char* msg, int16_t x)
    {
        return show(msg, (int64_t) x);
    }
    AssertHelper& show(const char* msg, uint16_t x)
    {
        return show(msg, (uint64_t) x);
    }
    AssertHelper& show(const char* msg, int32_t x)
    {
        return show(msg, (int64_t) x);
    }
    AssertHelper& show(const char* msg, uint32_t x)
    {
        return show(msg, (uint64_t) x);
    }
    AssertHelper& show(const char* msg, double x);
    AssertHelper& show(const char* msg, int64_t x);
    AssertHelper& show(const char* msg, uint64_t x);

    
    void Msg(const std::string& msg)
    {
        mMessage = msg;
    }

private:
    char const * const mConditionStr;
    char const * const mFilename;
    int mLine;
    std::deque<std::pair<std::string, std::string> > mValues;
    std::string mMessage;
};

} // namespace common
} // namespace meridian

#define MERIDIAN_ASSERT_HELPER_A(x) \
    MERIDIAN_ASSERT_HELPER_OP(x, B)
#define MERIDIAN_ASSERT_HELPER_B(x) \
    MERIDIAN_ASSERT_HELPER_OP(x, A)
#define MERIDIAN_ASSERT_HELPER_OP(x, next) \
    show(#x, (x)). MERIDIAN_ASSERT_HELPER_##next

#define MERIDIAN_ASSERT(cond) \
    if (__builtin_expect((cond) != 0, true)) {} \
    else ::meridian::common \
        ::AssertHelper(#cond, __FILE__, __LINE__). MERIDIAN_ASSERT_HELPER_A

#endif /* MERIDIAN_COMMON_ASSERT_HPP */

