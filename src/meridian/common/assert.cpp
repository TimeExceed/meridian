#include "assert.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>

using namespace std;

namespace meridian {
namespace common {

AssertHelper::~AssertHelper()
{
    ostringstream oss;
    oss << "Assertion failed in " << mFilename << ":" << mLine << endl;
    if (!mMessage.empty()) {
        oss << "Message: " << mMessage << endl;
    }
    oss << "Expression: " << mConditionStr << endl;
    for (int i = 0, sz = mValues.size(); i < sz; ++i) {
        if (i == 0) {
            oss << "Values: ";
        } else {
            oss << "        ";
        }
        oss << mValues[i].first << "=" << mValues[i].second << endl;
    }
    cerr << oss.str();

#ifdef NDEBUG
    _Exit(1);
#else
    abort();
#endif
}

namespace {

template<typename T>
void append(deque<pair<string, string>>* vs, const char* msg, T x)
{
    ostringstream oss;
    oss << x;
    vs->push_back(make_pair(string(msg), oss.str()));
}

} // namespace

AssertHelper& AssertHelper::show(const char* msg, double x)
{
    append(&mValues, msg, x);
    return *this;
}

AssertHelper& AssertHelper::show(const char* msg, int64_t x)
{
    append(&mValues, msg, x);
    return *this;
}

AssertHelper& AssertHelper::show(const char* msg, uint64_t x)
{
    append(&mValues, msg, x);
    return *this;
}

} // namespace common
} // namespace meridian

