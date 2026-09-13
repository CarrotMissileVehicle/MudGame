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
    } // namespace

    MusicPlayer::~MusicPlayer()
    {
        stop();
    }

    bool MusicPlayer::start(const std::string& file)
    {
        stop(); // 幂等：若残留旧设备先关闭

        // 路径转宽字符（MCI 命令使用宽字符串；路径须加引号以容忍空格）
        // 注意：窄→宽为字节扩展，ASCII 资源路径（编译期 MUDGAME_RES_DIR）足够。
        // MCI 对反斜杠更友好，统一将 '/' 规范化为 '\'。
        std::wstring wpath(file.begin(), file.end());
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