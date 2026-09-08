/**
 * @file view_primitives.cpp
 * @brief View 层通用渲染原语实现。
 */
#include "view_primitives.h"

#include <iomanip>
#include <sstream>

namespace mud::view
{
    void print_separator(Renderer& r, int width, char ch)
    {
        r.print(std::string(static_cast<std::size_t>(width), ch));
    }

    void print_title(Renderer& r, const std::string& title, char ch)
    {
        const int width = 60;
        const std::string side(static_cast<std::size_t>(width), ch);
        r.print(side);
        r.print(title);
        r.print(side);
    }

    void print_pair(Renderer& r, const std::string& label, const std::string& value)
    {
        r.print(label + ": " + value);
    }

    std::string format_time(const mud::time::GameDateTime& t)
    {
        std::ostringstream os;
        os << t.year << "年" << t.month << "月" << t.day << "日 "
           << std::setw(2) << std::setfill('0') << t.hour << ":" << std::setw(2) << t.minute;
        return os.str();
    }

    void print_prompt(Renderer& r) { r.print_raw("> "); }
} // namespace mud::view