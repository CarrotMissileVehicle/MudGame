/**
 * @file Renderer.h
 * @brief Renderer 抽象：View 层唯一输出出口。
 *
 * StdoutRenderer 输出到终端；StringRenderer 捕获到内存字符串（供测试断言）。
 * 所有面板只持 Renderer 引用，不直接使用 std::cout。
 */
#pragma once

#include <string>

namespace mud::view
{
    /** @brief 输出设备抽象：print 追加换行，print_raw 原样输出。 */
    class Renderer
    {
    public:
        virtual ~Renderer() = default;

        /** @brief 输出一行（自动追加换行）。 */
        virtual void print(const std::string& line) = 0;

        /** @brief 原样输出（不追加换行）。 */
        virtual void print_raw(const std::string& s) = 0;
    };

    /** @brief 标准输出实现。 */
    class StdoutRenderer final : public Renderer
    {
    public:
        void print(const std::string& line) override;
        void print_raw(const std::string& s) override;
    };

    /** @brief 字符串捕获实现（测试断言 / 缓冲渲染用）。 */
    class StringRenderer final : public Renderer
    {
    public:
        void print(const std::string& line) override;
        void print_raw(const std::string& s) override;

        /** @brief 已捕获的全部输出文本。 */
        [[nodiscard]] const std::string& text() const { return text_; }

    private:
        std::string text_;
    };
} // namespace mud::view
