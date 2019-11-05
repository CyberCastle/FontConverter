#ifndef __UTILS_H__
#define __UTILS_H__

#include <sstream>
#include <iomanip>

namespace utils
{

// Piece of code obtain from here: https://stackoverflow.com/a/24315631/11454077
static inline void replaceAll(std::string &str, const std::string &from, const std::string &to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
}

// Idea for build this piece of code obtain from here: https://stackoverflow.com/a/5590404/11454077
template <typename... T>
std::string intToStr(T &&... args)
{
    std::ostringstream sstr;
    // Fold Expression. More Info: https://en.cppreference.com/w/cpp/language/fold
    ((sstr << std::dec << args), ...);
    return sstr.str();
}

// Idea for build this piece of code obtain from here: https://stackoverflow.com/a/5590404/11454077
template <typename... T>
std::string intToHex(T &&... args)
{
    std::ostringstream sstr;
    // Fold Expression. More Info: https://en.cppreference.com/w/cpp/language/fold
    ((sstr << "0x" << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << args), ...);
    return sstr.str();
}

// Idea for build this piece of code obtain from here: https://stackoverflow.com/a/5590404/11454077
template <typename... T>
std::string boolToStr(T &&... args)
{
    std::ostringstream sstr;
    // Fold Expression. More Info: https://en.cppreference.com/w/cpp/language/fold
    ((sstr << std::boolalpha << args), ...);
    return sstr.str();
}
} // namespace utils

#endif /* __UTILS_H__ */