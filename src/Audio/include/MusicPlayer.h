/**
 * @file MusicPlayer.h
 * @brief 基于 Windows MCI 的循环背景音乐播放器。
 *
 * 封装 mciSendStringW（winmm），打开 MP3 后由 MCI 设备自动循环播放，
 * 不占用额外线程、不阻塞游戏主循环。
 */
#pragma once

#include <string>

namespace mud::audio
{
    /**
     * @brief 非阻塞循环 BGM 播放器（RAII：析构自动停止并关闭）。
     *
     * 线程安全：仅由主线程调用 start/stop。
     */
    class MusicPlayer
    {
    public:
        MusicPlayer() = default;
        ~MusicPlayer();

        MusicPlayer(const MusicPlayer&) = delete;
        MusicPlayer& operator=(const MusicPlayer&) = delete;

        /**
         * @brief 打开指定音频文件并循环播放。
         * @param file 音频文件路径（MP3）。
         * @return true 打开且开始循环播放成功；false 失败（文件缺失或设备不可用）。
         * @note 若此前已打开则先关闭再重新打开（幂等）。
         */
        bool start(const std::string& file);

        /** @brief 停止并关闭设备（幂等，可重复调用）。 */
        void stop();

        /** @brief 是否已打开设备。 */
        bool is_open() const noexcept { return open_; }

    private:
        bool open_ = false;
        std::wstring alias_{L"mud_bgm"};
    };
} // namespace mud::audio