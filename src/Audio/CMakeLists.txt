# audio 库：基于 Windows MCI 的循环背景音乐播放器（winmm）。
add_library(audio
    src/MusicPlayer.cpp
)

target_include_directories(audio
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# winmm：Windows 多媒体支持（mciSendStringW）。MinGW 为 -lwinmm，MSVC 为 winmm.lib。
target_link_libraries(audio
    PRIVATE
        winmm
)