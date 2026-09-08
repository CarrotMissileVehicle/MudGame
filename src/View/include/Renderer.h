/**
 * @file Renderer.h
 * @brief 视图输出抽象：全部 View 组件经由 Renderer 输出，便于测试替换。
 */
#pragma once

#include <string>

namespace mud::view
{
    /**
     * @brief 文本输出抽象。View 层唯一输出出口（替代散落的 std::cout）。
     */
    class Renderer
    {
    public:
        virtual ~Renderer() = default;

        /// 输出一行（自动换行）。
        virtual void print(const std::string& line) = 0;

        /// 输出原始文本（不换行，用于提示符）。
        virtual void print_raw(const std::string& s) = 0;
    };

    /** @brief 标准输出实现（stdout）。 */
    class StdoutRenderer : public Renderer
    {
    public:
        void print(const std::string& line) override;
        void print_raw(const std::string& s) override;
    };

    /** @brief 字符串实现：把输出累积进缓冲区，供单元测试断言。 */
    class StringRenderer : public Renderer
    {
    public:
        void print(const std::string& line) override;
        void print_raw(const std::string& s) override;

        /// 返回当前累积的全部输出。
        const std::string& text() const noexcept { return text_; }

        /// 清空缓冲区。
        void clear() noexcept { text_.clear(); }

    private:
        std::string text_;
    };
} // namespace mud::view