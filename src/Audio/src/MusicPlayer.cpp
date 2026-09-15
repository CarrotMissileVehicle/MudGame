/**
 * @file MusicPlayer.cpp
 * @brief Windows MCI 循环背景音乐播放器实现。
 *
 * 依次尝试：
 *   1. open ... type mpegvideo alias mud_bgm（显式设备类型，覆盖 MP3）
 *   2. open ... alias mud_bgm（按扩展名自动选择设备）
 * 成功后发 play mud_bgm repeat 实现无缝循环。
 */
#include "MusicPlayer.h"

#include <windows.h>
#include <mmsystem.h>

#include <cstddef>

namespace mud::audio
{
    namespace
    {
        /** @brief 发送一条 MCI 字符串命令，成功返回 true。 */
        bool mci_ok(const std::wstring& cmd)
        {
            // 返回 0 表示成功；出错时返回值非 0。
            return mciSendStringW(cmd.c_str(), nullptr, 0, nullptr) == 0;
        }

        /**
         * @brief 窄字符串→宽字符串（项目源码/路径为 UTF-8，MSVC /utf-8）。
         *
         * 逐字节零扩展（std::wstring(s.begin(), s.end())）会使非 ASCII 字符
         * 变成错误码元，导致 MCI 找不到文件；此处优先按 UTF-8 转换，
         * 失败时退回系统 ANSI 代码页。
         */
        std::wstring to_wide(const std::string& s)
        {
            if (s.empty())
                return {};
            const int len = static_cast<int>(s.size());
            int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                        s.data(), len, nullptr, 0);
            UINT cp = CP_UTF8;
            if (n <= 0)
            {
                cp = CP_ACP;
                n = MultiByteToWideChar(cp, 0, s.data(), len, nullptr, 0);
            }
            if (n <= 0)
                return {};
            std::wstring out(static_cast<std::size_t>(n), L'\0');
            MultiByteToWideChar(cp, 0, s.data(), len, out.data(), n);
            return out;
        }
    } // namespace

    MusicPlayer::~MusicPlayer()
    {
        stop();
    }

    bool MusicPlayer::start(const std::string& file)
    {
        stop(); // 幂等：若残留旧设备先关闭

        // 路径经 UTF-8→宽字符转换后拼入 MCI 命令串。路径中含双引号或控制字符
        // 会提前终结引号参数并注入额外 MCI 指令（命令注入），一律拒绝。
        for (const char ch : file)
            if (ch == '"' || (static_cast<unsigned char>(ch) < 0x20))
                return false;

        // MCI 对反斜杠更友好，统一将 '/' 规范化为 '\'。
        std::wstring wpath = to_wide(file);
        if (wpath.empty())
            return false;
        for (auto& ch : wpath)
            if (ch == L'/')
                ch = L'\\';

        std::wstring cmd = L"open \"" + wpath + L"\" type mpegvideo alias " + alias_;
        if (!mci_ok(cmd))
        {
            // 显式设备类型失败 → 让系统按扩展名自动选择设备
            cmd = L"open \"" + wpath + L"\" alias " + alias_;
            if (!mci_ok(cmd))
                return false;
        }
        open_ = true;

        // 循环播放（由 MCI 设备内部循环）
        const std::wstring playCmd = L"play " + alias_ + L" repeat";
        if (!mci_ok(playCmd))
        {
            stop();
            return false;
        }
        return true;
    }

    void MusicPlayer::stop()
    {
        if (!open_)
            return;
        (void)mci_ok(L"stop " + alias_);
        (void)mci_ok(L"close " + alias_);
        open_ = false;
    }
} // namespace mud::audio