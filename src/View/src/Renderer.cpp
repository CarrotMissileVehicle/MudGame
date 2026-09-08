/**
 * @file Renderer.cpp
 * @brief Renderer 抽象的标准输出与字符串实现。
 */
#include "Renderer.h"

#include <iostream>

namespace mud::view
{
    void StdoutRenderer::print(const std::string& line) { std::cout << line << "\n"; }

    void StdoutRenderer::print_raw(const std::string& s) { std::cout << s; }

    void StringRenderer::print(const std::string& line)
    {
        text_ += line;
        text_ += "\n";
    }

    void StringRenderer::print_raw(const std::string& s) { text_ += s; }
} // namespace mud::view