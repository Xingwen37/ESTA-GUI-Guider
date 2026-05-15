
/*
    TL-ESTA 项目 ： 使用OOP in C (OOC) 写法的示波器库，仿HAL库搭建
    主要贡献者 : TongLewis(yangyutong) HEU ESTA 2025
                Xingwen37(wuzeyu) HEU ESTA 2026
    设计特点：
        1. 调用简单：使用仿HAL模式的方式设计，只需配置抽象寄存器即可调用，无需复杂配置。
        2. 功能强大：可绘制波形与X轴，Y轴标尺以及标尺数字。
        3. 代码规范：变量规范，宏丰富抽象层次高，错误处理方式规范，可读性强。
        4. 移植方便：设计屏幕函数抽象层，使用不同屏幕只需修改一处代码，轻松移植。
        5. 模拟器支持：使用SDL2模拟器环境，支持Linux平台。
        6. 资料齐全：代码注释，样例，文档强力支持，快速上手。
        7. 外观精致：示波器主题自定义简单，也可以使用预设主题。
    版本记录：
        2025.7.16 完成概念设计
        2025.7.17 完成基本框架设计
        2025.7.18 完善文档与样例，发布BetaV0.1版本
        2026.4.20 完成模拟器环境搭建
    该项目还处于内测版本，如果你有更好的想法/想添加的功能/主题等，欢迎与我联系。
    TIPS:将编译优化开至-O2效果更好
*/


#include "ui/WAVE.h"
#include <string.h>

// 不要修改这个数组的名称，我们根据其来寻址
// 也不要直接通过数组修改里面的内容，而是通过库函数修改配置（除非你知道你在做什么）
// 你可以将其视作库函数中外设的基地址
WAVE_TypeDef WAVE_State[MAX_WAVE_NUM];

static bool WAVE_IsValidFontSize(ESTA_FontSize font_size) {
    return (int)font_size >= 0 && font_size < ESTA_FONT_SIZE_COUNT;
}

static void WAVE_CopyUnit(char dst[WAVE_MAX_RULER_UNIT_LEN + 1], const char *src) {
    if (dst == NULL) return;
    if (src == NULL) {
        dst[0] = '\0';
        return;
    }
    strncpy(dst, src, WAVE_MAX_RULER_UNIT_LEN);
    dst[WAVE_MAX_RULER_UNIT_LEN] = '\0';
}

static uint8_t WAVE_StrLen(const char *s) {
    uint8_t len = 0;
    if (s == NULL) return 0;
    while (len < WAVE_MAX_RULER_LABEL_TEXT_LEN && s[len] != '\0') {
        len++;
    }
    return len;
}

static uint8_t WAVE_FormatUInt32(uint32_t value, char *buf, uint8_t size) {
    char tmp[10];
    uint8_t len = 0;
    if (buf == NULL || size == 0) return 0;
    if (value == 0U) {
        buf[0] = '0';
        if (size > 1) buf[1] = '\0';
        return 1;
    }
    while (value > 0U && len < sizeof(tmp)) {
        tmp[len++] = (char)('0' + (value % 10U));
        value /= 10U;
    }
    uint8_t out_len = 0;
    while (len > 0 && out_len + 1 < size) {
        buf[out_len++] = tmp[--len];
    }
    buf[out_len] = '\0';
    return out_len;
}

static uint32_t WAVE_Pow10(uint8_t precision) {
    uint32_t scale = 1U;
    while (precision > 0U) {
        scale *= 10U;
        precision--;
    }
    return scale;
}

static uint8_t WAVE_FormatInt32(int32_t value, char *buf, uint8_t size) {
    if (buf == NULL || size == 0) return 0;
    uint8_t pos = 0;
    uint32_t mag;
    if (value < 0) {
        if (pos + 1 >= size) return 0;
        buf[pos++] = '-';
        mag = (uint32_t)(-(value + 1)) + 1U;
    } else {
        mag = (uint32_t)value;
    }
    pos += WAVE_FormatUInt32(mag, &buf[pos], (uint8_t)(size - pos));
    return pos;
}

static uint8_t WAVE_FormatFloat(float value, uint8_t precision, char *buf, uint8_t size) {
    if (buf == NULL || size == 0) return 0;
    if (precision > 4U) precision = 4U;

    uint8_t pos = 0;
    if (value < 0.0f && pos + 1 < size) {
        buf[pos++] = '-';
        value = -value;
    }

    uint32_t scale = WAVE_Pow10(precision);
    uint32_t scaled = (uint32_t)(value * (float)scale + 0.5f);
    uint32_t integer_part = scaled / scale;
    uint32_t frac_part = scaled % scale;

    pos += WAVE_FormatUInt32(integer_part, &buf[pos], (uint8_t)(size - pos));
    if (precision > 0U && pos + 1 < size) {
        buf[pos++] = '.';
        uint32_t div = scale / 10U;
        for (uint8_t i = 0; i < precision && pos + 1 < size; i++) {
            buf[pos++] = (char)('0' + (frac_part / div) % 10U);
            if (div > 1U) div /= 10U;
        }
    }
    buf[pos] = '\0';
    return pos;
}

static uint8_t WAVE_FormatRulerLabel(const WAVE_RulerLabel_TypeDef *label,
                                     uint8_t precision,
                                     char *buf,
                                     uint8_t size) {
    if (buf == NULL || size == 0 || label == NULL) return 0;
    if (label->value_type == WAVE_RULER_LABEL_FLOAT) {
        return WAVE_FormatFloat(label->float_value, precision, buf, size);
    }
    return WAVE_FormatInt32(label->int_value, buf, size);
}

static WAVE_RulerLabel_TypeDef WAVE_DefaultRulerLabel(uint16_t value) {
    WAVE_RulerLabel_TypeDef label;
    label.value_type = WAVE_RULER_LABEL_INT;
    label.int_value = (int32_t)value;
    label.float_value = (float)value;
    return label;
}

static uint16_t WAVE_GetYLabelReservedWidth(int OSCx) {
    uint16_t count = WAVE_CONFIG_MEMBER(OSCx, ruler_count_y);
    uint16_t min_chars = WAVE_CONFIG_MEMBER(OSCx, ruler_num_digits_y);
    uint8_t precision = WAVE_CONFIG_MEMBER(OSCx, ruler_precision_y);
    ESTA_FontSize font_size = WAVE_CONFIG_MEMBER(OSCx, ruler_font_size_y);
    char label_buf[WAVE_MAX_RULER_LABEL_TEXT_LEN + 1];
    uint16_t max_chars = min_chars;

    for (uint16_t i = 0; i < count; i++) {
        uint8_t len = WAVE_FormatRulerLabel(
            &WAVE_PRIVATE_MEMBER_ARRAY(OSCx, ruler_label_buff_y, i),
            precision, label_buf, sizeof(label_buf));
        if (len > max_chars) max_chars = len;
    }
    return (uint16_t)(max_chars * ui_font_width(font_size));
}

static uint16_t WAVE_GetRightReservedWidth(int OSCx) {
    uint16_t reserved = 0;
    if (WAVE_CONFIG_MEMBER(OSCx, is_display_ruler_y)) {
        reserved = (uint16_t)(reserved + WAVE_GetYLabelReservedWidth(OSCx));
    }
    return reserved;
}

static uint16_t WAVE_GetRightAlignedTextX(uint16_t x_origin, uint16_t width, uint16_t text_width) {
    return (width > text_width) ? (uint16_t)(x_origin + width - text_width) : x_origin;
}

static uint16_t WAVE_GetBottomAlignedTextY(uint16_t y_origin, uint16_t height,
                                           uint16_t reserved_below, uint16_t text_height) {
    uint16_t required_height = (uint16_t)(reserved_below + text_height);
    return (height > required_height) ?
        (uint16_t)(y_origin + height - required_height) : y_origin;
}

static uint16_t WAVE_GetSafeXScale(int OSCx) {
    uint16_t x_scale = WAVE_CONFIG_MEMBER(OSCx, x_scale);
    return (x_scale == 0U) ? 1U : x_scale;
}

uint16_t WAVE_GetPlotWidth(int OSCx) {
    if (!IS_VALID_WAVE_INST(OSCx)) return 0;
    uint16_t x_width = WAVE_CONFIG_MEMBER(OSCx, x_width);
    uint16_t reserved = WAVE_GetRightReservedWidth(OSCx);
    return (x_width > reserved) ? (uint16_t)(x_width - reserved) : x_width;
}

uint16_t WAVE_GetPlotHeight(int OSCx) {
    if (!IS_VALID_WAVE_INST(OSCx)) return 0;
    uint16_t y_width = WAVE_CONFIG_MEMBER(OSCx, y_width);
    if (!WAVE_CONFIG_MEMBER(OSCx, is_display_ruler_x)) return y_width;
    uint16_t x_label_height = ui_font_height(WAVE_CONFIG_MEMBER(OSCx, ruler_font_size_x));
    return (y_width > x_label_height) ? (uint16_t)(y_width - x_label_height) : y_width;
}

uint16_t WAVE_GetSampleCapacity(int OSCx) {
    if (!IS_VALID_WAVE_INST(OSCx)) return 0;
    uint16_t x_scale = WAVE_GetSafeXScale(OSCx);
    uint16_t plot_width = WAVE_GetPlotWidth(OSCx);
    uint16_t capacity = (uint16_t)(plot_width / x_scale);
    return (capacity > 0U) ? capacity : 1U;
}

static const uint16_t WAVE_ColorTable[WAVE_THEME_COUNT][WAVE_THEME_INDEX_COUNT] = {
    [WAVE_THEME_DEFAULT] = {
        [WAVE_THEME_FRAME_INDEX]      = __WHITE,
        [WAVE_THEME_RULER_INDEX]      = __GRAY,
        [WAVE_THEME_WAVE_CH0_INDEX]   = __GREEN,
        [WAVE_THEME_WAVE_CH1_INDEX]   = __GBLUE,
        [WAVE_THEME_WAVE_CH2_INDEX]   = __YELLOW,
        [WAVE_THEME_WAVE_CH3_INDEX]   = __RED,
        [WAVE_THEME_BACKGROUND_INDEX] = __BLACK,
    },
    [WAVE_THEME_LIGHT] = {
        [WAVE_THEME_FRAME_INDEX]      = __BLACK,
        [WAVE_THEME_RULER_INDEX]      = __GRAY,
        [WAVE_THEME_WAVE_CH0_INDEX]   = __ORANGE,
        [WAVE_THEME_WAVE_CH1_INDEX]   = __DEEP_BLUE,
        [WAVE_THEME_WAVE_CH2_INDEX]   = __RED,
        [WAVE_THEME_WAVE_CH3_INDEX]   = __BLUE,
        [WAVE_THEME_BACKGROUND_INDEX] = __WHITE,
    },
};

ESTA_StatusTypeDef WAVE_ConfigSetDisplayRange(WAVE_Config_TypeDef *config,
                               uint16_t display_num_min, uint16_t display_num_max) {
    if(config == NULL) return ESTA_ERROR;
    config->display_num_min = display_num_min;
    config->display_num_max = display_num_max;
    return ESTA_OK;
}

ESTA_StatusTypeDef WAVE_ConfigSetChannelNum(WAVE_Config_TypeDef *config, uint16_t channel_num) {
    if(config == NULL) return ESTA_ERROR;
    config->channel_num = channel_num;
    return ESTA_OK;
}

ESTA_StatusTypeDef WAVE_ConfigSetChannelEnabled(WAVE_Config_TypeDef *config, uint8_t channel_mask) {
    if(config == NULL) return ESTA_ERROR;
    config->channel_mask |= channel_mask;
    return ESTA_OK;
}

ESTA_StatusTypeDef WAVE_ConfigSetChannelDisabled(WAVE_Config_TypeDef *config, uint8_t channel_mask) {
    if(config == NULL) return ESTA_ERROR;
    config->channel_mask &= ~channel_mask;
    return ESTA_OK;
}

ESTA_StatusTypeDef WAVE_ConfigSetRulerY(WAVE_Config_TypeDef *config, bool is_display,
                         uint16_t *ruler_y, uint16_t ruler_count_y,
                         uint16_t ruler_num_digits_y) {
    if(config == NULL) return ESTA_ERROR;
    config->is_display_ruler_y = is_display;
    config->ruler_y = ruler_y;
    config->ruler_count_y = ruler_count_y;
    config->ruler_num_digits_y = ruler_num_digits_y;
    return ESTA_OK;
}

ESTA_StatusTypeDef WAVE_ConfigSetRulerX(WAVE_Config_TypeDef *config, bool is_display,
                         uint16_t *ruler_x, uint16_t ruler_count_x,
                         uint16_t ruler_zero_value_x, uint16_t ruler_full_value_x,
                         uint16_t ruler_num_digits_x) {
    if(config == NULL) return ESTA_ERROR;
    config->is_display_ruler_x = is_display;
    config->ruler_x = ruler_x;
    config->ruler_count_x = ruler_count_x;
    config->ruler_zero_value_x = ruler_zero_value_x;
    config->ruler_full_value_x = ruler_full_value_x;
    config->ruler_num_digits_x = ruler_num_digits_x;
    return ESTA_OK;
}

ESTA_StatusTypeDef WAVE_ConfigSetXScale(WAVE_Config_TypeDef *config, uint16_t x_scale) {
    if(config == NULL) return ESTA_ERROR;
    config->x_scale = (x_scale == 0U) ? 1U : x_scale;
    return ESTA_OK;
}

ESTA_StatusTypeDef WAVE_ConfigSetRulerLabelY(WAVE_Config_TypeDef *config,
                         const WAVE_RulerLabel_TypeDef *ruler_label_y,
                         const char *ruler_unit_y, uint8_t ruler_precision_y) {
    if(config == NULL) return ESTA_ERROR;
    if(ruler_precision_y > 4U) return ESTA_ERROR;
    config->ruler_label_y = ruler_label_y;
    config->ruler_unit_y = ruler_unit_y;
    config->ruler_precision_y = ruler_precision_y;
    return ESTA_OK;
}

ESTA_StatusTypeDef WAVE_ConfigSetRulerLabelX(WAVE_Config_TypeDef *config,
                         const WAVE_RulerLabel_TypeDef *ruler_label_x,
                         const char *ruler_unit_x, uint8_t ruler_precision_x) {
    if(config == NULL) return ESTA_ERROR;
    if(ruler_precision_x > 4U) return ESTA_ERROR;
    config->ruler_label_x = ruler_label_x;
    config->ruler_unit_x = ruler_unit_x;
    config->ruler_precision_x = ruler_precision_x;
    return ESTA_OK;
}

ESTA_StatusTypeDef WAVE_ConfigSetRulerFontSize(WAVE_Config_TypeDef *config,
                         ESTA_FontSize font_size_x, ESTA_FontSize font_size_y) {
    if(config == NULL) return ESTA_ERROR;
    if(!WAVE_IsValidFontSize(font_size_x) || !WAVE_IsValidFontSize(font_size_y)) return ESTA_ERROR;
    config->ruler_font_size_x = font_size_x;
    config->ruler_font_size_y = font_size_y;
    return ESTA_OK;
}

ESTA_StatusTypeDef WAVE_ConfigSetTheme(WAVE_Config_TypeDef *config, WAVE_theme_type theme_type) {
    if(config == NULL) return ESTA_ERROR;
    config->theme_type = theme_type;
    return ESTA_OK;
}

ESTA_StatusTypeDef WAVE_ConfigSetAutoClear(WAVE_Config_TypeDef *config, bool is_auto_clear) {
    if(config == NULL) return ESTA_ERROR;
    config->is_auto_clear = is_auto_clear;
    return ESTA_OK;
}



/* @brief : 初始化示波器实例
*  @param : int OSCx : 示波器实例，如WAVE_INST(0)或WAVE_INST(1)
*           WAVE_Config_TypeDef *WAVE_Init : 指向ESTA设置结构体的指针
*  @return : enum ESTA_StatusTypeDef 为ESTA_OK则无问题，为ESTA_ERROR则有问题
*  @note   : WAVE_Config_TypeDef 见ESTA.h
*/
ESTA_StatusTypeDef WAVE_Init(int OSCx, WAVE_Config_TypeDef *WAVE_Init) {
    if(!IS_VALID_WAVE_INST(OSCx)) return ESTA_ERROR;
    if(WAVE_Init == NULL) return ESTA_ERROR;

    WAVE_WRITE_CONFIG_INIT(OSCx, x_origin);
    WAVE_WRITE_CONFIG_INIT(OSCx, y_origin);
    WAVE_WRITE_CONFIG_INIT(OSCx, x_width);
    WAVE_WRITE_CONFIG_INIT(OSCx, y_width);
    if(WAVE_Init->display_num_min >= WAVE_Init->display_num_max) return ESTA_ERROR;
    WAVE_WRITE_CONFIG_INIT(OSCx, display_num_min);
    WAVE_WRITE_CONFIG_INIT(OSCx, display_num_max);
    WAVE_WRITE_CONFIG(OSCx, x_scale, (WAVE_Init->x_scale == 0U) ? 1U : WAVE_Init->x_scale);
    WAVE_WRITE_CONFIG_INIT(OSCx, channel_mask);

    // 初始化Y轴标尺
    if(WAVE_Init->is_display_ruler_y){
        if(WAVE_Init->ruler_y == NULL) return ESTA_ERROR;

        uint16_t ruler_actual_count_y = (WAVE_Init->ruler_count_y > WAVE_MAX_RULER_Y_NUM) ? 
                WAVE_MAX_RULER_Y_NUM : WAVE_Init->ruler_count_y;
        // 优化：先将整个缓存区清零，再拷贝有效数据，消除 if-else 分支
        memset(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_buff_y, 0, sizeof(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_buff_y));
        memset(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_label_buff_y, 0, sizeof(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_label_buff_y));
        for(int i = 0; i < ruler_actual_count_y; i++){
            WAVE_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_y, i) = WAVE_Init->ruler_y[i];
            if (WAVE_Init->ruler_label_y != NULL) {
                if (!IS_VALID_WAVE_RULER_LABEL_TYPE(WAVE_Init->ruler_label_y[i].value_type)) return ESTA_ERROR;
                WAVE_PRIVATE_MEMBER_ARRAY(OSCx, ruler_label_buff_y, i) = WAVE_Init->ruler_label_y[i];
            } else {
                WAVE_PRIVATE_MEMBER_ARRAY(OSCx, ruler_label_buff_y, i) =
                    WAVE_DefaultRulerLabel(WAVE_Init->ruler_y[i]);
            }
        }
        WAVE_CopyUnit(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_unit_buff_y, WAVE_Init->ruler_unit_y);
        WAVE_WRITE_CONFIG_INIT(OSCx, is_display_ruler_y);
        WAVE_WRITE_CONFIG(OSCx, ruler_y, NULL);
        WAVE_WRITE_CONFIG(OSCx, ruler_label_y, NULL);
        WAVE_WRITE_CONFIG(OSCx, ruler_unit_y, NULL);
        if (WAVE_Init->ruler_precision_y > 4U) return ESTA_ERROR;
        WAVE_WRITE_CONFIG_INIT(OSCx, ruler_precision_y);
        WAVE_WRITE_CONFIG(OSCx, ruler_count_y, ruler_actual_count_y);
        WAVE_WRITE_CONFIG_INIT(OSCx, ruler_num_digits_y);
    }else{
        memset(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_buff_y, 0, sizeof(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_buff_y));
        memset(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_label_buff_y, 0, sizeof(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_label_buff_y));
        WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_unit_buff_y[0] = '\0';
        WAVE_WRITE_CONFIG(OSCx, is_display_ruler_y, false);
        WAVE_WRITE_CONFIG(OSCx, ruler_y, NULL);
        WAVE_WRITE_CONFIG(OSCx, ruler_label_y, NULL);
        WAVE_WRITE_CONFIG(OSCx, ruler_unit_y, NULL);
        WAVE_WRITE_CONFIG(OSCx, ruler_precision_y, 0);
        WAVE_WRITE_CONFIG(OSCx, ruler_count_y, 0);
        WAVE_WRITE_CONFIG(OSCx, ruler_num_digits_y, 0);
    }
    
    // 初始化X轴标尺
    if(WAVE_Init->is_display_ruler_x){
        if(WAVE_Init->ruler_x == NULL) return ESTA_ERROR;

        uint16_t ruler_actual_count_x = (WAVE_Init->ruler_count_x > WAVE_MAX_RULER_X_NUM) ? 
                WAVE_MAX_RULER_X_NUM : WAVE_Init->ruler_count_x;
        memset(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_buff_x, 0, sizeof(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_buff_x));
        memset(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_label_buff_x, 0, sizeof(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_label_buff_x));
        for(int i = 0; i < ruler_actual_count_x; i++){
            WAVE_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_x, i) = WAVE_Init->ruler_x[i];
            if (WAVE_Init->ruler_label_x != NULL) {
                if (!IS_VALID_WAVE_RULER_LABEL_TYPE(WAVE_Init->ruler_label_x[i].value_type)) return ESTA_ERROR;
                WAVE_PRIVATE_MEMBER_ARRAY(OSCx, ruler_label_buff_x, i) = WAVE_Init->ruler_label_x[i];
            } else {
                WAVE_PRIVATE_MEMBER_ARRAY(OSCx, ruler_label_buff_x, i) =
                    WAVE_DefaultRulerLabel(WAVE_Init->ruler_x[i]);
            }
        }
        WAVE_CopyUnit(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_unit_buff_x, WAVE_Init->ruler_unit_x);
        WAVE_WRITE_CONFIG_INIT(OSCx, is_display_ruler_x);
        WAVE_WRITE_CONFIG(OSCx, ruler_x, NULL);
        WAVE_WRITE_CONFIG(OSCx, ruler_label_x, NULL);
        WAVE_WRITE_CONFIG(OSCx, ruler_unit_x, NULL);
        if (WAVE_Init->ruler_precision_x > 4U) return ESTA_ERROR;
        WAVE_WRITE_CONFIG_INIT(OSCx, ruler_precision_x);
        WAVE_WRITE_CONFIG(OSCx, ruler_count_x, ruler_actual_count_x);
        WAVE_WRITE_CONFIG_INIT(OSCx, ruler_zero_value_x);
        WAVE_WRITE_CONFIG_INIT(OSCx, ruler_full_value_x);
        WAVE_WRITE_CONFIG_INIT(OSCx, ruler_num_digits_x);
    }else{
        memset(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_buff_x, 0, sizeof(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_buff_x));
        memset(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_label_buff_x, 0, sizeof(WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_label_buff_x));
        WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_unit_buff_x[0] = '\0';
        WAVE_WRITE_CONFIG(OSCx, is_display_ruler_x, false);
        WAVE_WRITE_CONFIG(OSCx, ruler_x, NULL);
        WAVE_WRITE_CONFIG(OSCx, ruler_label_x, NULL);
        WAVE_WRITE_CONFIG(OSCx, ruler_unit_x, NULL);
        WAVE_WRITE_CONFIG(OSCx, ruler_precision_x, 0);
        WAVE_WRITE_CONFIG(OSCx, ruler_count_x, 0);
        WAVE_WRITE_CONFIG(OSCx, ruler_zero_value_x, 0);
        WAVE_WRITE_CONFIG(OSCx, ruler_full_value_x, 0);
        WAVE_WRITE_CONFIG(OSCx, ruler_num_digits_x, 0);
    }

    if(!IS_VALID_CHNUM(WAVE_CONFIG_MEMBER(OSCx, channel_num))) return ESTA_ERROR;
    WAVE_WRITE_CONFIG_INIT(OSCx, channel_num);

    if(!WAVE_IsValidFontSize(WAVE_Init->ruler_font_size_x) ||
       !WAVE_IsValidFontSize(WAVE_Init->ruler_font_size_y)) return ESTA_ERROR;
    WAVE_WRITE_CONFIG_INIT(OSCx, ruler_font_size_x);
    WAVE_WRITE_CONFIG_INIT(OSCx, ruler_font_size_y);

    if(!IS_VALID_THEME(WAVE_CONFIG_MEMBER(OSCx, theme_type))) return ESTA_ERROR;
    WAVE_WRITE_CONFIG_INIT(OSCx, theme_type);
    WAVE_WRITE_CONFIG_INIT(OSCx, is_auto_clear);

    /* Private regs */
    WAVE_WRITE_PRIVATE(OSCx, last_index, 0);
    WAVE_WRITE_PRIVATE(OSCx, x_coor_last, WAVE_Init->x_origin);
    for(int i = 0; i < MAX_WAVE_CHANNEL; i++){
        WAVE_WRITE_PRIVATE(OSCx, y_coor_last_CH[i], 0);
    }

    return ESTA_OK;
}

/* @brief : 复位示波器实例
*  @param : int OSCx : 示波器实例，如WAVE_INST(0)或WAVE_INST(1)
*  @return : enum ESTA_StatusTypeDef 为ESTA_OK则无问题，为ESTA_ERROR则有问题
*/
ESTA_StatusTypeDef WAVE_DeInit(int OSCx) {
    if(!IS_VALID_WAVE_INST(OSCx)) return ESTA_ERROR;

    WAVE_WRITE_CONFIG(OSCx, x_origin, 0);
    WAVE_WRITE_CONFIG(OSCx, y_origin, 0);
    WAVE_WRITE_CONFIG(OSCx, x_width, 0);
    WAVE_WRITE_CONFIG(OSCx, y_width, 0);
    WAVE_WRITE_CONFIG(OSCx, display_num_min, 0);
    WAVE_WRITE_CONFIG(OSCx, display_num_max, 0);
    WAVE_WRITE_CONFIG(OSCx, x_scale, 1);
    WAVE_WRITE_CONFIG(OSCx, is_display_ruler_y, false);
    WAVE_WRITE_CONFIG(OSCx, ruler_y, NULL);
    WAVE_WRITE_CONFIG(OSCx, ruler_label_y, NULL);
    WAVE_WRITE_CONFIG(OSCx, ruler_unit_y, NULL);
    WAVE_WRITE_CONFIG(OSCx, ruler_precision_y, 0);
    WAVE_WRITE_CONFIG(OSCx, ruler_count_y, 0);
    WAVE_WRITE_CONFIG(OSCx, ruler_num_digits_y, 0);
    WAVE_WRITE_CONFIG(OSCx, is_display_ruler_x, false);
    WAVE_WRITE_CONFIG(OSCx, ruler_x, NULL);
    WAVE_WRITE_CONFIG(OSCx, ruler_label_x, NULL);
    WAVE_WRITE_CONFIG(OSCx, ruler_unit_x, NULL);
    WAVE_WRITE_CONFIG(OSCx, ruler_precision_x, 0);
    WAVE_WRITE_CONFIG(OSCx, ruler_count_x, 0);
    WAVE_WRITE_CONFIG(OSCx, ruler_num_digits_x, 0);
    WAVE_WRITE_CONFIG(OSCx, ruler_zero_value_x, 0);
    WAVE_WRITE_CONFIG(OSCx, ruler_full_value_x, 0);
    WAVE_WRITE_CONFIG(OSCx, ruler_font_size_x, ESTA_FONT_1608);
    WAVE_WRITE_CONFIG(OSCx, ruler_font_size_y, ESTA_FONT_1608);
    WAVE_WRITE_CONFIG(OSCx, theme_type, WAVE_THEME_DEFAULT);
    WAVE_WRITE_CONFIG(OSCx, is_auto_clear, true);

    for(int i = 0; i < WAVE_MAX_RULER_Y_NUM; i++){
        WAVE_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_y, i) = 0;
        WAVE_PRIVATE_MEMBER_ARRAY(OSCx, ruler_label_buff_y, i) = WAVE_DefaultRulerLabel(0);
    }
    for(int i = 0; i < WAVE_MAX_RULER_X_NUM; i++){
        WAVE_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_x, i) = 0;
        WAVE_PRIVATE_MEMBER_ARRAY(OSCx, ruler_label_buff_x, i) = WAVE_DefaultRulerLabel(0);
    }
    WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_unit_buff_y[0] = '\0';
    WAVE_INST_ADDR(OSCx).WAVE_Private.ruler_unit_buff_x[0] = '\0';
    WAVE_WRITE_PRIVATE(OSCx, last_index, 0);
    WAVE_WRITE_PRIVATE(OSCx, x_coor_last, 0);
    for(int i = 0; i < MAX_WAVE_CHANNEL; i++){
        WAVE_WRITE_PRIVATE(OSCx, y_coor_last_CH[i], 0);
    }

    return ESTA_OK;
}

/* @brief : 获得主题颜色
*  @param : WAVE_theme_type theme 主题名称
*           WAVE_theme_color_index_type color_type 颜色类型索引
*  @return : uint16_t 颜色值
*/

/* @brief : 绘制示波器标尺
*  @param : int OSCx : 示波器实例，如WAVE_INST(0)或WAVE_INST(1)
*  @return : enum ESTA_StatusTypeDef 为ESTA_OK则无问题，为ESTA_ERROR则有问题
*/
ESTA_StatusTypeDef WAVE_RulerDisplay(int OSCx) {
    if(!IS_VALID_WAVE_INST(OSCx)) return ESTA_ERROR;

    volatile bool is_display_ruler_y = WAVE_CONFIG_MEMBER(OSCx, is_display_ruler_y);
    volatile bool is_display_ruler_x = WAVE_CONFIG_MEMBER(OSCx, is_display_ruler_x);
    if(!(is_display_ruler_x || is_display_ruler_y)) return ESTA_OK;

    uint16_t x_origin             = WAVE_CONFIG_MEMBER(OSCx, x_origin); 
    uint16_t y_origin             = WAVE_CONFIG_MEMBER(OSCx, y_origin); 
    uint16_t display_num_min      = WAVE_CONFIG_MEMBER(OSCx, display_num_min);
    uint16_t display_num_max      = WAVE_CONFIG_MEMBER(OSCx, display_num_max);
    uint16_t ruler_count_y        = WAVE_CONFIG_MEMBER(OSCx, ruler_count_y);
    uint16_t ruler_count_x        = WAVE_CONFIG_MEMBER(OSCx, ruler_count_x);
    uint16_t ruler_zero_value_x   = WAVE_CONFIG_MEMBER(OSCx, ruler_zero_value_x);
    uint16_t ruler_full_value_x   = WAVE_CONFIG_MEMBER(OSCx, ruler_full_value_x);
    uint8_t ruler_precision_y     = WAVE_CONFIG_MEMBER(OSCx, ruler_precision_y);
    uint8_t ruler_precision_x     = WAVE_CONFIG_MEMBER(OSCx, ruler_precision_x);
    ESTA_FontSize ruler_font_size_x = WAVE_CONFIG_MEMBER(OSCx, ruler_font_size_x);
    ESTA_FontSize ruler_font_size_y = WAVE_CONFIG_MEMBER(OSCx, ruler_font_size_y);
    uint16_t theme_type  = WAVE_CONFIG_MEMBER(OSCx, theme_type);
    uint16_t ruler_color = ESTA_THEME_COLOR(WAVE_ColorTable, theme_type,WAVE_THEME_RULER_INDEX);
    uint16_t ruler_font_width_x = ui_font_width(ruler_font_size_x);
    uint16_t ruler_font_height_y = ui_font_height(ruler_font_size_y);

    uint16_t y_frame_width = WAVE_GetPlotHeight(OSCx);
    uint16_t x_frame_width = WAVE_GetPlotWidth(OSCx);
    const char *unit_y = WAVE_PRIVATE_MEMBER(OSCx, ruler_unit_buff_y);
    const char *unit_x = WAVE_PRIVATE_MEMBER(OSCx, ruler_unit_buff_x);
    char label_buf[WAVE_MAX_RULER_LABEL_TEXT_LEN + 1];

    if(is_display_ruler_y){
        uint16_t display_range = display_num_max - display_num_min;

        for(int i = 0; i < ruler_count_y; i++){
            uint16_t ruler = WAVE_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_y, i);
            uint8_t label_len = WAVE_FormatRulerLabel(
                &WAVE_PRIVATE_MEMBER_ARRAY(OSCx, ruler_label_buff_y, i),
                ruler_precision_y, label_buf, sizeof(label_buf));
            uint16_t ruler_y_coor = 0;
            ruler_y_coor = y_origin + y_frame_width - 
                ui_coor_normal(y_frame_width, display_range, ruler - display_num_min);
            ruler_y_coor = ui_limit(y_origin + y_frame_width - 2, y_origin + 2, ruler_y_coor);
            uint16_t ruler_y_num_coor = 
                ui_limit(y_origin + y_frame_width, y_origin, 
                ruler_y_coor - ruler_font_height_y / 2);
            // 防止无符号整数溢出
            if((int16_t)ruler_y_coor - ruler_font_height_y / 2 < 0) ruler_y_num_coor = 0;
            SCREEN_DRAW_LINE(x_origin, ruler_y_coor, 
                x_origin + x_frame_width, ruler_y_coor, ruler_color);
                //原注释：有时屏幕显示数字需要画两次 但在模拟器却没有出现该情况，推测是硬件驱动的问题
                SCREEN_DRAW_STRING_FONT(x_origin + x_frame_width, ruler_y_num_coor,
                    label_buf, label_len, ruler_font_size_y, ruler_color);
        }
        //原注释：写数字导致边框可能间断，重新绘制 但在模拟器却没有出现该情况，推测是硬件驱动的问题
        /*
        uint16_t x_outline_width = x_width;
        uint16_t y_outline_width = y_width;
        SCREEN_DRAW_RECTANGLE(x_origin + x_frame_width, y_origin, 
        x_origin + x_outline_width, y_origin + y_outline_width, frame_color);
        */
    }

    if(is_display_ruler_x){
        uint16_t x_ruler_range = ruler_full_value_x - ruler_zero_value_x;
        for(int i = 0; i < ruler_count_x; i++){
            uint16_t ruler = WAVE_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_x, i);
            uint8_t label_len = WAVE_FormatRulerLabel(
                &WAVE_PRIVATE_MEMBER_ARRAY(OSCx, ruler_label_buff_x, i),
                ruler_precision_x, label_buf, sizeof(label_buf));
            uint16_t label_width = (uint16_t)(label_len * ruler_font_width_x);
            uint16_t ruler_x_coor = 0;
            ruler_x_coor = x_origin + 
                ui_coor_normal(x_frame_width, x_ruler_range, ruler - ruler_zero_value_x);
            ruler_x_coor = 
                ui_limit(x_origin + x_frame_width - 2, x_origin + 2, ruler_x_coor);
            uint16_t ruler_x_num_coor = 
                ui_limit(x_origin + x_frame_width, x_origin, 
                ruler_x_coor - label_width / 2);
            // 防止无符号整数溢出
            if((int16_t)ruler_x_coor - label_width / 2 < 0) {
                ruler_x_num_coor = 0;
            }
            SCREEN_DRAW_LINE(ruler_x_coor, y_origin, 
                ruler_x_coor, y_origin + y_frame_width, ruler_color);
                //原注释：有时屏幕显示数字需要画两次 //但在模拟器却没有出现该情况，推测是硬件驱动的问题
            uint16_t label_right_limit = (x_frame_width > label_width) ?
                (uint16_t)(x_origin + x_frame_width - label_width) : x_origin;
            if (ruler_x_num_coor > label_right_limit) {
                ruler_x_num_coor = label_right_limit;
            }
            SCREEN_DRAW_STRING_FONT(ruler_x_num_coor, y_origin + y_frame_width,
                label_buf, label_len, ruler_font_size_x, ruler_color);
        }
    }
    uint8_t unit_len_x = is_display_ruler_x ? WAVE_StrLen(unit_x) : 0U;
    uint8_t unit_len_y = is_display_ruler_y ? WAVE_StrLen(unit_y) : 0U;
    uint16_t unit_x_height = ui_font_height(ruler_font_size_x);
    uint16_t unit_y_height = ui_font_height(ruler_font_size_y);

    if (unit_len_x > 0U) {
        uint16_t unit_x_width = (uint16_t)(unit_len_x * ruler_font_width_x);
        uint16_t unit_x_coor = WAVE_GetRightAlignedTextX(x_origin, x_frame_width, unit_x_width);
        uint16_t unit_y_coor = WAVE_GetBottomAlignedTextY(y_origin, y_frame_width, 0U, unit_x_height);
        SCREEN_DRAW_STRING_FONT(unit_x_coor, unit_y_coor,
            unit_x, unit_len_x, ruler_font_size_x, ruler_color);
    }
    if (unit_len_y > 0U) {
        uint16_t ruler_font_width_y = ui_font_width(ruler_font_size_y);
        uint16_t unit_y_width = (uint16_t)(unit_len_y * ruler_font_width_y);
        uint16_t reserved_below = (unit_len_x > 0U) ? unit_x_height : 0U;
        uint16_t unit_x_coor = WAVE_GetRightAlignedTextX(x_origin, x_frame_width, unit_y_width);
        uint16_t unit_y_coor = WAVE_GetBottomAlignedTextY(y_origin, y_frame_width,
            reserved_below, unit_y_height);
        SCREEN_DRAW_STRING_FONT(unit_x_coor, unit_y_coor,
            unit_y, unit_len_y, ruler_font_size_y, ruler_color);
    }
    return ESTA_OK;
}

/* @brief : 绘制示波器框架
*  @param : int OSCx : 示波器实例，如WAVE_INST(0)或WAVE_INST(1)
*  @return : enum ESTA_StatusTypeDef 为ESTA_OK则无问题，为ESTA_ERROR则有问题
*/
ESTA_StatusTypeDef WAVE_FrameDisplay(int OSCx) {
    if(!IS_VALID_WAVE_INST(OSCx)) return ESTA_ERROR;
    uint16_t x_origin    = WAVE_CONFIG_MEMBER(OSCx, x_origin); 
    uint16_t y_origin    = WAVE_CONFIG_MEMBER(OSCx, y_origin); 
    uint16_t x_width     = WAVE_CONFIG_MEMBER(OSCx, x_width);
    uint16_t y_width     = WAVE_CONFIG_MEMBER(OSCx, y_width);
    uint16_t theme_type  = WAVE_CONFIG_MEMBER(OSCx, theme_type);
    uint16_t frame_color = ESTA_THEME_COLOR(WAVE_ColorTable, theme_type,WAVE_THEME_FRAME_INDEX);
    uint16_t y_frame_width = WAVE_GetPlotHeight(OSCx);
    uint16_t x_frame_width = WAVE_GetPlotWidth(OSCx);
    uint16_t x_outline_width = x_width;
    SCREEN_DRAW_RECTANGLE(x_origin, y_origin, 
            x_origin + x_frame_width, y_origin + y_frame_width, frame_color);
    SCREEN_DRAW_RECTANGLE(x_origin, y_origin, 
            x_origin + x_outline_width, y_origin + y_width, frame_color);
    return ESTA_OK;
}

/* @brief : 清除曲线
*  @param : int OSCx : 示波器实例，如WAVE_INST(0)或WAVE_INST(1)
*  @return : enum ESTA_StatusTypeDef 为ESTA_OK则无问题，为ESTA_ERROR则有问题
*/
ESTA_StatusTypeDef WAVE_CurveClear(int OSCx) {
    if(!IS_VALID_WAVE_INST(OSCx)) return ESTA_ERROR;
    uint16_t x_origin    = WAVE_CONFIG_MEMBER(OSCx, x_origin); 
    uint16_t y_origin    = WAVE_CONFIG_MEMBER(OSCx, y_origin); 
    uint16_t x_width     = WAVE_CONFIG_MEMBER(OSCx, x_width);
    uint16_t y_width     = WAVE_CONFIG_MEMBER(OSCx, y_width);
    volatile bool is_display_ruler_y = WAVE_CONFIG_MEMBER(OSCx, is_display_ruler_y);
    uint16_t theme_type  = WAVE_CONFIG_MEMBER(OSCx, theme_type);
    uint16_t frame_color = ESTA_THEME_COLOR(WAVE_ColorTable, theme_type,WAVE_THEME_FRAME_INDEX);
    uint16_t bg_color    = ESTA_THEME_COLOR(WAVE_ColorTable, theme_type,WAVE_THEME_BACKGROUND_INDEX);

    SCREEN_FILL(x_origin, y_origin, x_origin + x_width, y_origin + y_width, bg_color);
    SCREEN_DRAW_RECTANGLE(x_origin, y_origin, 
            x_origin + x_width, y_origin + y_width, frame_color);
    // reset the index of ESTA
    WAVE_WRITE_PRIVATE(OSCx, last_index, 0);
    WAVE_WRITE_PRIVATE(OSCx, x_coor_last, x_origin);
    for(int i = 0; i < MAX_WAVE_CHANNEL; i++){
        WAVE_WRITE_PRIVATE(OSCx, y_coor_last_CH[i], 0);
    }
    if(is_display_ruler_y) {
        ESTA_RETURN_IF_ERROR(WAVE_RulerDisplay(OSCx));
    }
    return ESTA_OK;
}

/* @brief : 绘制示波器框架与（如有）标尺
*  @param : int OSCx : 示波器实例，如WAVE_INST(0)或WAVE_INST(1)
*           WAVE_Config_TypeDef *WAVE_Init : 指向ESTA设置结构体的指针
*  @return : enum ESTA_StatusTypeDef 为ESTA_OK则无问题，为ESTA_ERROR则有问题
*/
ESTA_StatusTypeDef WAVE_ReDraw(int OSCx) {
    if(!IS_VALID_WAVE_INST(OSCx)) return ESTA_ERROR;
    ESTA_RETURN_IF_ERROR(WAVE_FrameDisplay(OSCx));
    ESTA_RETURN_IF_ERROR(WAVE_CurveClear(OSCx));
    return ESTA_OK;
}

/* @brief : 向示波器发送一个数据，绘制曲线，（可选）曲线满自动清屏
*  @param : int OSCx : 示波器实例，如WAVE_INST(0)或WAVE_INST(1)
*           uint16_t data_CH0 : CH0通道的数据
*           uint16_t data_CH1 : CH1通道的数据，若为单通道置零即可
*  @return : enum ESTA_StatusTypeDef 为ESTA_OK则无问题，为ESTA_ERROR则有问题
*            若示波器实例is_auto_clear == false，不会自动清屏，同时屏幕满时返回ESTA_FULL
*/
ESTA_StatusTypeDef WAVE_CurveDraw(int OSCx, uint16_t data_CH[]) {
    if(!IS_VALID_WAVE_INST(OSCx)) return ESTA_ERROR;
    uint16_t x_origin                = WAVE_CONFIG_MEMBER(OSCx, x_origin); 
    uint16_t y_origin                = WAVE_CONFIG_MEMBER(OSCx, y_origin); 
    uint16_t display_num_min         = WAVE_CONFIG_MEMBER(OSCx, display_num_min);
    uint16_t display_num_max         = WAVE_CONFIG_MEMBER(OSCx, display_num_max);
    uint16_t channel_num             = WAVE_CONFIG_MEMBER(OSCx, channel_num);
    uint16_t theme_type              = WAVE_CONFIG_MEMBER(OSCx, theme_type);
    volatile bool is_auto_clear      = WAVE_CONFIG_MEMBER(OSCx, is_auto_clear);
    uint8_t channel_mask             = WAVE_CONFIG_MEMBER(OSCx, channel_mask);

    uint16_t active_channel_num = (channel_num > MAX_WAVE_CHANNEL) ? MAX_WAVE_CHANNEL : channel_num;

    uint16_t wave_CH_color[MAX_WAVE_CHANNEL] = {0};
    for(int i = 0; i < active_channel_num; i++){
        if(is_channel_enabled(channel_mask, CH0 << i)) {
            wave_CH_color[i] = ESTA_THEME_COLOR(WAVE_ColorTable, theme_type,WAVE_THEME_WAVE_CH0_INDEX + i);
        }
    }

    uint16_t last_index       = WAVE_PRIVATE_MEMBER(OSCx, last_index);
    uint16_t x_coor_last      = WAVE_PRIVATE_MEMBER(OSCx, x_coor_last);
    uint16_t y_coor_last_CH[MAX_WAVE_CHANNEL];
    for(int i = 0; i < active_channel_num; i++){
        y_coor_last_CH[i] = WAVE_PRIVATE_MEMBER(OSCx, y_coor_last_CH[i]);
    }

    uint16_t y_frame_width = WAVE_GetPlotHeight(OSCx);
    uint16_t sample_capacity = WAVE_GetSampleCapacity(OSCx);

    if(last_index >= sample_capacity) {
        if (!is_auto_clear) {
            // 重置绘制状态
            WAVE_WRITE_PRIVATE(OSCx, last_index, 0);
            WAVE_WRITE_PRIVATE(OSCx, x_coor_last, x_origin);
            return ESTA_FULL;
        }
        last_index = 0;
        WAVE_CurveClear(OSCx);
    }

    uint16_t display_range = display_num_max - display_num_min;
    uint16_t x_scale = WAVE_GetSafeXScale(OSCx);
    uint16_t x_coor = (uint16_t)(x_origin + last_index * x_scale);

    for(int i = 0; i < active_channel_num; i++) {
        uint8_t ch_mask = (uint8_t)(CH0 << i);
        if(!is_channel_enabled(channel_mask, ch_mask)) continue;

        if(ui_is_out_of_bound(display_num_max, display_num_min, data_CH[i])) return ESTA_ERROR;

        uint16_t y_display_value = ui_limit(display_num_max, display_num_min, data_CH[i]);
        uint16_t y_coor = y_origin + y_frame_width -
                ui_coor_normal(y_frame_width, display_range, y_display_value);

        if(last_index > 0) {
            SCREEN_DRAW_LINE(x_coor, y_coor,
                x_coor_last, y_coor_last_CH[i], wave_CH_color[i]);
        }
        WAVE_PRIVATE_MEMBER(OSCx, y_coor_last_CH[i]) = y_coor;
    }

    WAVE_PRIVATE_MEMBER(OSCx, x_coor_last) = x_coor;
    last_index++;
    WAVE_PRIVATE_MEMBER(OSCx, last_index)  = last_index;

    return ESTA_OK;
}

ESTA_StatusTypeDef WAVE_CurveDrawBatch(int OSCx, int ch_idx,
                        const uint16_t *data, uint16_t count) {
    if (!IS_VALID_WAVE_INST(OSCx)) return ESTA_ERROR;
    if (!IS_VALID_CHNUM(ch_idx)) return ESTA_ERROR;
    if (data == NULL || count == 0) return ESTA_ERROR;

    uint8_t channel_mask = WAVE_CONFIG_MEMBER(OSCx, channel_mask);
    if (!is_channel_enabled(channel_mask, (uint8_t)(CH0 << ch_idx))) return ESTA_ERROR;

    uint16_t x_origin                = WAVE_CONFIG_MEMBER(OSCx, x_origin);
    uint16_t y_origin                = WAVE_CONFIG_MEMBER(OSCx, y_origin);
    uint16_t display_num_min         = WAVE_CONFIG_MEMBER(OSCx, display_num_min);
    uint16_t display_num_max         = WAVE_CONFIG_MEMBER(OSCx, display_num_max);
    uint16_t theme_type              = WAVE_CONFIG_MEMBER(OSCx, theme_type);
    volatile bool is_auto_clear      = WAVE_CONFIG_MEMBER(OSCx, is_auto_clear);

    uint16_t y_frame_width = WAVE_GetPlotHeight(OSCx);
    uint16_t sample_capacity = WAVE_GetSampleCapacity(OSCx);
    uint16_t display_range = display_num_max - display_num_min;
    uint16_t x_scale = WAVE_GetSafeXScale(OSCx);

    uint16_t wave_color = ESTA_THEME_COLOR(WAVE_ColorTable, theme_type,
                            WAVE_THEME_WAVE_CH0_INDEX + ch_idx);

    uint16_t last_index  = WAVE_PRIVATE_MEMBER(OSCx, last_index);
    uint16_t x_coor_last = WAVE_PRIVATE_MEMBER(OSCx, x_coor_last);
    uint16_t y_coor_last = WAVE_PRIVATE_MEMBER(OSCx, y_coor_last_CH[ch_idx]);

    for (uint16_t i = 0; i < count; i++) {
        if (last_index >= sample_capacity) {
            if (!is_auto_clear) {
                WAVE_WRITE_PRIVATE(OSCx, last_index, 0);
                WAVE_WRITE_PRIVATE(OSCx, x_coor_last, x_origin);
                return ESTA_FULL;
            }
            WAVE_CurveClear(OSCx);
            last_index  = 0;
            x_coor_last = x_origin;
            y_coor_last = 0;
        }

        if (ui_is_out_of_bound(display_num_max, display_num_min, data[i]))
            return ESTA_ERROR;

        uint16_t y_display_value = ui_limit(display_num_max, display_num_min, data[i]);
        uint16_t y_coor = y_origin + y_frame_width -
                ui_coor_normal(y_frame_width, display_range, y_display_value);
        uint16_t x_coor = (uint16_t)(x_origin + last_index * x_scale);

        if (last_index > 0) {
            SCREEN_DRAW_LINE(x_coor, y_coor,
                x_coor_last, y_coor_last, wave_color);
        }

        y_coor_last = y_coor;
        x_coor_last = x_coor;
        last_index++;
    }

    WAVE_PRIVATE_MEMBER(OSCx, y_coor_last_CH[ch_idx]) = y_coor_last;
    WAVE_PRIVATE_MEMBER(OSCx, x_coor_last) = x_coor_last;
    WAVE_PRIVATE_MEMBER(OSCx, last_index)  = last_index;

    return ESTA_OK;
}
