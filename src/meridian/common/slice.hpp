#ifndef MERIDIAN_COMMON_SLICE_HPP
#define MERIDIAN_COMMON_SLICE_HPP

#include <string>
#include <cstring>
#include <cstdint>

namespace meridian {
namespace common {

class Slice
{
    const void* mBegin;
    const void* mEnd;

public:
    Slice()
      : mBegin(nullptr), mEnd(nullptr)
    {}
    explicit Slice(const std::string& s)
      : mBegin(s.data()), mEnd(s.data() + s.length())
    {}
    explicit Slice(const char* s)
      : mBegin(s), mEnd(nullptr)
    {
        mEnd = (unit8_t*) mBegin + strlen(s);
    }
    Slice(const void* b, const void* e)
      : mBegin(b), mEnd(e)
    {}
    Slice(const void* s, int64_t len)
      : mBegin(s), mEnd((uint8_t*) s + len)
    {}
    Slice(const Slice& s)
      : mBegin(s.mBegin), mEnd(s.mEnd)
    {}
    Slice(Slice&& s)
      : mBegin(s.mBegin), mEnd(s.mEnd)
    {
        s.mBegin = nullptr;
        s.mEnd = nullptr;
    }
    Slice& operator=(const Slice& s)
    {
        mBegin = s.mBegin;
        mEnd = s.mEnd;
    }

    const void* data() const
    {
        return mBegin;
    }
    intptr_t size() const
    {
        return static_cast<uint8_t*>(mEnd) - static_cast<uint8_t*>(mBegin);
    }
    uint8_t operator[](intptr_t offset) const
    {
        return static_cast<uint8_t*>(mBegin)[offset];
    }
    Slice subslice(intptr_t from) const
    {
        return Slice(static_cast<uint8_t*>(mBegin) + from, mEnd);
    }
    Slice subslice(intptr_t from, intptr_t to) const
    {
        return Slice(
            static_cast<uint8_t*>(mBegin) + from,
            static_cast<uint8_t*>(mBegin) + to);
    }
};

} // namespace common
} // namespace meridian

#endif /* MERIDIAN_COMMON_SLICE_HPP */

