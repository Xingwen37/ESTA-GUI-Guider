#include "helper.h"
#include <stdio.h>

/* ---------- 编码函数：将通道号转换为掩码或组合掩码 ---------- */

/**
 * @brief 将单个通道号 (1~8) 转换为对应的位掩码
 * @param channel 通道号，1~8
 * @return 对应的掩码，若通道号无效则返回 0
 */
inline uint8_t channel_to_mask(int channel) {
    if (channel >= 1 && channel <= 8) {
        return (uint8_t)(1U << (channel - 1));
    }
    return 0U;
}

/**
 * @brief 开启指定通道（修改掩码）
 * @param mask 指向当前掩码的指针
 * @param channel 要开启的通道号 (1~8)
 */
inline void enable_channel(uint8_t *mask, int channel) {
    if (mask && channel >= 1 && channel <= 8) {
        *mask |= (uint8_t)(1U << (channel - 1));
    }
}

/**
 * @brief 关闭指定通道（修改掩码）
 * @param mask 指向当前掩码的指针
 * @param channel 要关闭的通道号 (1~8)
 */
inline void disable_channel(uint8_t *mask, int channel) {
    if (mask && channel >= 1 && channel <= 8) {
        *mask &= (uint8_t)~(1U << (channel - 1));
    }
}

/* ---------- 解码函数：检查掩码中某个通道的状态 ---------- */

/**
 * @brief 检查指定通道是否开启
 * @param mask 通道掩码
 * @param channel 通道号 (1~8)
 * @return true 表示通道开启，false 表示关闭或通道号无效
 */
inline bool is_channel_enabled(uint8_t mask, int channel) {
    if (channel < 1 || channel > 8) {
        return false;
    }
    return (mask & (1U << (channel - 1))) != 0;
}

/* ---------- 译码函数：将掩码转换为可读的通道列表 ---------- */

/**
 * @brief 将掩码解析为通道号字符串，例如 "1,3,5"
 * @param mask 通道掩码
 * @param buffer 输出缓冲区
 * @param bufsize 缓冲区大小
 * @return 返回 buffer 指针，方便链式调用
 */
inline char* decode_channels(uint8_t mask, char *buffer, size_t bufsize) {
    if (!buffer || bufsize == 0) {
        return NULL;
    }

    size_t pos = 0;
    bool first = true;
    
    for (int ch = 1; ch <= 8; ++ch) {
        if (mask & (1U << (ch - 1))) {
            if (!first) {
                pos += snprintf(buffer + pos, bufsize - pos, ",");
            }
            pos += snprintf(buffer + pos, bufsize - pos, "%d", ch);
            first = false;
            
            if (pos >= bufsize - 1) {
                break;  /* 缓冲区已满，停止写入 */
            }
        }
    }
    
    if (first) {
        /* 没有开启任何通道 */
        snprintf(buffer, bufsize, "none");
    }
    
    return buffer;
}

/**
 * @brief 打印当前开启的通道列表（调试用）
 * @param mask 通道掩码
 */
inline void print_enabled_channels(uint8_t mask) {
    char buf[32];
    decode_channels(mask, buf, sizeof(buf));
    printf("Enabled channels: %s (mask = 0x%02X)\n", buf, mask);
}


