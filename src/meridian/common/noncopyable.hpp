#ifndef MERIDIAN_COMMON_NONCOPYABLE_HPP
#define MERIDIAN_COMMON_NONCOPYABLE_HPP

namespace meridian {
namespace common {

class Noncopyable
{
private:
    Noncopyable(const Noncopyable&) =delete;
    Noncopyable& operator=(const Noncopyable&) =delete;
public:
    Noncopyable()
    {}
};

} // namespace common
} // namespace meridian

#endif /* MERIDIAN_COMMON_NONCOPYABLE_HPP */
