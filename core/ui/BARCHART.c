/*
    TL-ESTA 项目 ： 柱状图组件，遵循 COMPONENT_SPEC.md v1.0 规范
    主要贡献者 : Xingwen37(wuzeyu) HEU ESTA 2026
    设计特点：
        1. OOC 三结构体模式，与 WAVE 组件架构一致
        2. 支持自动/手动柱体布局，数值标签与坐标轴可独立开关
        3. 主题系统：DEFAULT（黑底绿柱）/ LIGHT（白底蓝柱）
        4. 运行时单柱更新（UpdateBar）与批量更新（UpdateAll）
        5. 完全通过 SCREEN_DRAW_* 宏绘制，平台无关
    版本记录：
        2026.5.8  完成 v1 核心功能
*/


#include "ui/BARCHART.h"
#include <string.h>

BARCHART_TypeDef BARCHART_State[BARCHART_MAX_NUM];

/* ================== Config Setter ================== */

ESTA_StatusTypeDef BARCHART_ConfigSetDisplayRange(BARCHART_Config_TypeDef *config,
    uint16_t display_num_min, uint16_t display_num_max) {
    if (config == NULL) return ESTA_ERROR;
    config->display_num_min = display_num_min;
    config->display_num_max = display_num_max;
    return ESTA_OK;
}

ESTA_StatusTypeDef BARCHART_ConfigSetBarCount(BARCHART_Config_TypeDef *config,
    uint16_t bar_count) {
    if (config == NULL) return ESTA_ERROR;
    config->bar_count = bar_count;
    return ESTA_OK;
}

ESTA_StatusTypeDef BARCHART_ConfigSetBarLayout(BARCHART_Config_TypeDef *config,
    uint16_t bar_width, uint16_t bar_spacing) {
    if (config == NULL) return ESTA_ERROR;
    config->bar_width = bar_width;
    config->bar_spacing = bar_spacing;
    return ESTA_OK;
}

ESTA_StatusTypeDef BARCHART_ConfigSetData(BARCHART_Config_TypeDef *config,
    uint16_t *data_values, uint16_t data_count) {
    if (config == NULL) return ESTA_ERROR;
    (void)data_count;
    config->data_values = data_values;
    return ESTA_OK;
}

ESTA_StatusTypeDef BARCHART_ConfigSetDisplayOptions(BARCHART_Config_TypeDef *config,
    bool is_display_value, bool is_display_axis) {
    if (config == NULL) return ESTA_ERROR;
    config->is_display_value = is_display_value;
    config->is_display_axis = is_display_axis;
    return ESTA_OK;
}

ESTA_StatusTypeDef BARCHART_ConfigSetTheme(BARCHART_Config_TypeDef *config,
    BARCHART_theme_type theme_type) {
    if (config == NULL) return ESTA_ERROR;
    config->theme_type = theme_type;
    return ESTA_OK;
}

/* ================== Lifecycle ================== */

/**
 * @brief  初始化柱状图实例
 * @param  inst    : 实例索引，如 BARCHART_INST(0)
 * @param  pConfig : 指向配置结构体的指针
 * @return ESTA_StatusTypeDef
 */
ESTA_StatusTypeDef BARCHART_Init(int inst, BARCHART_Config_TypeDef *BARCHART_Init) {
    if (!IS_VALID_BARCHART_INST(inst)) return ESTA_ERROR;
    if (BARCHART_Init == NULL) return ESTA_ERROR;

    BARCHART_WRITE_CONFIG_INIT(inst, x_origin);
    BARCHART_WRITE_CONFIG_INIT(inst, y_origin);
    BARCHART_WRITE_CONFIG_INIT(inst, x_width);
    BARCHART_WRITE_CONFIG_INIT(inst, y_width);

    if (BARCHART_Init->display_num_min >= BARCHART_Init->display_num_max)
        return ESTA_ERROR;
    BARCHART_WRITE_CONFIG_INIT(inst, display_num_min);
    BARCHART_WRITE_CONFIG_INIT(inst, display_num_max);

    uint16_t bar_count = BARCHART_Init->bar_count;
    if (bar_count == 0 || bar_count > BARCHART_MAX_BARS) {
        bar_count = (bar_count > BARCHART_MAX_BARS) ? BARCHART_MAX_BARS : 1;
    }
    BARCHART_WRITE_CONFIG(inst, bar_count, bar_count);

    BARCHART_WRITE_CONFIG_INIT(inst, bar_width);
    BARCHART_WRITE_CONFIG_INIT(inst, bar_spacing);

    /* 深拷贝数据到 Private 缓冲区 */
    if (BARCHART_Init->data_values != NULL) {
        for (int i = 0; i < bar_count; i++) {
            BARCHART_PRIVATE_MEMBER_ARRAY(inst, data_buff, i) =
                BARCHART_Init->data_values[i];
        }
        BARCHART_WRITE_CONFIG(inst, data_values, NULL);
    } else {
        memset(BARCHART_INST_ADDR(inst).BARCHART_Private.data_buff, 0,
               sizeof(BARCHART_INST_ADDR(inst).BARCHART_Private.data_buff));
    }

    BARCHART_WRITE_CONFIG_INIT(inst, is_display_value);
    BARCHART_WRITE_CONFIG_INIT(inst, is_display_axis);

    if (!IS_VALID_BARCHART_THEME(BARCHART_CONFIG_MEMBER(inst, theme_type)))
        return ESTA_ERROR;
    BARCHART_WRITE_CONFIG_INIT(inst, theme_type);

    /* 计算内部绘图区 */
    uint16_t area_ox = BARCHART_Init->x_origin + 4;
    uint16_t area_oy = BARCHART_Init->y_origin;
    uint16_t area_width  = BARCHART_Init->x_width - 8;
    uint16_t area_height = BARCHART_Init->y_width;

    if (BARCHART_Init->is_display_value) {
        area_oy     += CHAR_PIXEL_HEIGHT + 2;
        area_height -= CHAR_PIXEL_HEIGHT + 2;
    }
    if (BARCHART_Init->is_display_axis) {
        area_height -= 2;
    }

    uint16_t baseline_y = area_oy + area_height;

    /* 计算柱体尺寸 */
    uint16_t bw = BARCHART_Init->bar_width;
    uint16_t bs = BARCHART_Init->bar_spacing;

    if (bw == 0 && bs == 0) {
        uint16_t total_parts = bar_count * 3;
        uint16_t part = (total_parts > 0 && area_width >= total_parts)
            ? area_width / total_parts : 1;
        if (part < 1) part = 1;
        bw = part * 2;
        bs = part;
    }

    /* 钳位：确保所有柱体能放入绘图区 */
    uint16_t total_needed = bar_count * bw + (bar_count - 1) * bs;
    if (total_needed > area_width && bw > 1) {
        uint16_t excess = total_needed - area_width;
        uint16_t shrink = (excess / bar_count) + 1;
        if (bw > shrink) bw -= shrink; else bw = 1;
    }

    /* 存储 Private 布局参数 */
    BARCHART_WRITE_PRIVATE(inst, area_ox, area_ox);
    BARCHART_WRITE_PRIVATE(inst, area_oy, area_oy);
    BARCHART_WRITE_PRIVATE(inst, area_width, area_width);
    BARCHART_WRITE_PRIVATE(inst, area_height, area_height);
    BARCHART_WRITE_PRIVATE(inst, bar_width_actual, bw);
    BARCHART_WRITE_PRIVATE(inst, bar_spacing_actual, bs);
    BARCHART_WRITE_PRIVATE(inst, bar_count_actual, bar_count);
    BARCHART_WRITE_PRIVATE(inst, baseline_y, baseline_y);

    return ESTA_OK;
}

/**
 * @brief  复位柱状图实例
 * @param  inst : 实例索引
 * @return ESTA_StatusTypeDef
 */
ESTA_StatusTypeDef BARCHART_DeInit(int inst) {
    if (!IS_VALID_BARCHART_INST(inst)) return ESTA_ERROR;

    BARCHART_WRITE_CONFIG(inst, x_origin, 0);
    BARCHART_WRITE_CONFIG(inst, y_origin, 0);
    BARCHART_WRITE_CONFIG(inst, x_width, 0);
    BARCHART_WRITE_CONFIG(inst, y_width, 0);
    BARCHART_WRITE_CONFIG(inst, display_num_min, 0);
    BARCHART_WRITE_CONFIG(inst, display_num_max, 0);
    BARCHART_WRITE_CONFIG(inst, bar_count, 0);
    BARCHART_WRITE_CONFIG(inst, bar_width, 0);
    BARCHART_WRITE_CONFIG(inst, bar_spacing, 0);
    BARCHART_WRITE_CONFIG(inst, data_values, NULL);
    BARCHART_WRITE_CONFIG(inst, is_display_value, false);
    BARCHART_WRITE_CONFIG(inst, is_display_axis, false);
    BARCHART_WRITE_CONFIG(inst, theme_type, BARCHART_THEME_DEFAULT);

    memset(BARCHART_INST_ADDR(inst).BARCHART_Private.data_buff, 0,
           sizeof(BARCHART_INST_ADDR(inst).BARCHART_Private.data_buff));
    BARCHART_WRITE_PRIVATE(inst, area_ox, 0);
    BARCHART_WRITE_PRIVATE(inst, area_oy, 0);
    BARCHART_WRITE_PRIVATE(inst, area_width, 0);
    BARCHART_WRITE_PRIVATE(inst, area_height, 0);
    BARCHART_WRITE_PRIVATE(inst, bar_width_actual, 0);
    BARCHART_WRITE_PRIVATE(inst, bar_spacing_actual, 0);
    BARCHART_WRITE_PRIVATE(inst, bar_count_actual, 0);
    BARCHART_WRITE_PRIVATE(inst, baseline_y, 0);

    return ESTA_OK;
}

/* ================== Display / Operations ================== */

/**
 * @brief  绘制外框与坐标轴
 * @param  inst : 实例索引
 * @return ESTA_StatusTypeDef
 */
ESTA_StatusTypeDef BARCHART_FrameDisplay(int inst) {
    if (!IS_VALID_BARCHART_INST(inst)) return ESTA_ERROR;

    uint16_t x_origin       = BARCHART_CONFIG_MEMBER(inst, x_origin);
    uint16_t y_origin       = BARCHART_CONFIG_MEMBER(inst, y_origin);
    uint16_t x_width        = BARCHART_CONFIG_MEMBER(inst, x_width);
    uint16_t y_width        = BARCHART_CONFIG_MEMBER(inst, y_width);
    volatile bool is_display_axis = BARCHART_CONFIG_MEMBER(inst, is_display_axis);
    uint16_t theme_type     = BARCHART_CONFIG_MEMBER(inst, theme_type);

    uint16_t area_ox        = BARCHART_PRIVATE_MEMBER(inst, area_ox);
    uint16_t area_oy        = BARCHART_PRIVATE_MEMBER(inst, area_oy);
    uint16_t area_width     = BARCHART_PRIVATE_MEMBER(inst, area_width);
    uint16_t baseline_y     = BARCHART_PRIVATE_MEMBER(inst, baseline_y);

    uint16_t frame_color    = ESTA_GetThemeColor(theme_type, BARCHART_THEME_FRAME_INDEX);
    uint16_t axis_color     = ESTA_GetThemeColor(theme_type, BARCHART_THEME_AXIS_INDEX);

    /* 外框 */
    SCREEN_DRAW_RECTANGLE(x_origin, y_origin,
        x_origin + x_width, y_origin + y_width, frame_color);

    /* 坐标轴 */
    if (is_display_axis) {
        SCREEN_DRAW_LINE(area_ox, baseline_y,
            area_ox + area_width, baseline_y, axis_color);
        SCREEN_DRAW_LINE(area_ox, area_oy,
            area_ox, baseline_y, axis_color);
    }

    return ESTA_OK;
}

/**
 * @brief  绘制所有柱体与数值标签
 * @param  inst : 实例索引
 * @return ESTA_StatusTypeDef
 */
ESTA_StatusTypeDef BARCHART_BarDisplay(int inst) {
    if (!IS_VALID_BARCHART_INST(inst)) return ESTA_ERROR;

    uint16_t display_num_min   = BARCHART_CONFIG_MEMBER(inst, display_num_min);
    uint16_t display_num_max   = BARCHART_CONFIG_MEMBER(inst, display_num_max);
    volatile bool is_display_value = BARCHART_CONFIG_MEMBER(inst, is_display_value);
    uint16_t theme_type        = BARCHART_CONFIG_MEMBER(inst, theme_type);

    uint16_t area_ox        = BARCHART_PRIVATE_MEMBER(inst, area_ox);
    uint16_t area_height    = BARCHART_PRIVATE_MEMBER(inst, area_height);
    uint16_t bw             = BARCHART_PRIVATE_MEMBER(inst, bar_width_actual);
    uint16_t bs             = BARCHART_PRIVATE_MEMBER(inst, bar_spacing_actual);
    uint16_t bar_count      = BARCHART_PRIVATE_MEMBER(inst, bar_count_actual);
    uint16_t baseline_y     = BARCHART_PRIVATE_MEMBER(inst, baseline_y);

    uint16_t display_range  = display_num_max - display_num_min;
    uint16_t frame_color    = ESTA_GetThemeColor(theme_type, BARCHART_THEME_FRAME_INDEX);
    uint16_t bar_color      = ESTA_GetThemeColor(theme_type, BARCHART_THEME_BAR_INDEX);
    uint16_t label_color    = ESTA_GetThemeColor(theme_type, BARCHART_THEME_LABEL_INDEX);

    uint16_t step = bw + bs;

    for (int i = 0; i < bar_count; i++) {
        uint16_t value = BARCHART_PRIVATE_MEMBER_ARRAY(inst, data_buff, i);
        uint16_t x_left = area_ox + i * step;
        uint16_t x_right = x_left + bw;

        uint16_t display_value = ui_limit(display_num_max, display_num_min, value);
        uint16_t bar_height = ui_coor_normal(area_height, display_range,
            display_value - display_num_min);
        uint16_t y_top = baseline_y - bar_height;

        SCREEN_FILL(x_left, y_top, x_right, baseline_y, bar_color);
        SCREEN_DRAW_RECTANGLE(x_left, y_top, x_right, baseline_y, frame_color);

        if (is_display_value) {
            uint16_t digits = ui_num_digits(value);
            if (digits == 0) digits = 1;
            uint16_t label_x = x_left + (bw - digits * CHAR_PIXEL_WIDTH) / 2;
            uint16_t label_y = (y_top >= CHAR_PIXEL_HEIGHT)
                ? y_top - CHAR_PIXEL_HEIGHT : 0;
            SCREEN_DRAW_NUM(label_x, label_y, value, digits, label_color);
        }
    }

    return ESTA_OK;
}

/**
 * @brief  全量重绘：清除背景 + 重绘柱体
 * @param  inst : 实例索引
 * @return ESTA_StatusTypeDef
 */
ESTA_StatusTypeDef BARCHART_ReDraw(int inst) {
    if (!IS_VALID_BARCHART_INST(inst)) return ESTA_ERROR;
    ESTA_RETURN_IF_ERROR(BARCHART_Clear(inst));
    ESTA_RETURN_IF_ERROR(BARCHART_BarDisplay(inst));
    return ESTA_OK;
}

/**
 * @brief  清除整个组件区域（背景填充 + 重绘坐标轴）
 * @param  inst : 实例索引
 * @return ESTA_StatusTypeDef
 */
ESTA_StatusTypeDef BARCHART_Clear(int inst) {
    if (!IS_VALID_BARCHART_INST(inst)) return ESTA_ERROR;

    uint16_t x_origin       = BARCHART_CONFIG_MEMBER(inst, x_origin);
    uint16_t y_origin       = BARCHART_CONFIG_MEMBER(inst, y_origin);
    uint16_t x_width        = BARCHART_CONFIG_MEMBER(inst, x_width);
    uint16_t y_width        = BARCHART_CONFIG_MEMBER(inst, y_width);
    volatile bool is_display_axis = BARCHART_CONFIG_MEMBER(inst, is_display_axis);
    uint16_t theme_type     = BARCHART_CONFIG_MEMBER(inst, theme_type);

    uint16_t area_ox        = BARCHART_PRIVATE_MEMBER(inst, area_ox);
    uint16_t area_oy        = BARCHART_PRIVATE_MEMBER(inst, area_oy);
    uint16_t area_width     = BARCHART_PRIVATE_MEMBER(inst, area_width);
    uint16_t baseline_y     = BARCHART_PRIVATE_MEMBER(inst, baseline_y);

    uint16_t bg_color       = ESTA_GetThemeColor(theme_type, BARCHART_THEME_BACKGROUND_INDEX);
    uint16_t axis_color     = ESTA_GetThemeColor(theme_type, BARCHART_THEME_AXIS_INDEX);

    SCREEN_FILL(x_origin, y_origin, x_origin + x_width,
        y_origin + y_width, bg_color);

    if (is_display_axis) {
        SCREEN_DRAW_LINE(area_ox, baseline_y,
            area_ox + area_width, baseline_y, axis_color);
        SCREEN_DRAW_LINE(area_ox, area_oy,
            area_ox, baseline_y, axis_color);
    }

    return ESTA_OK;
}

/**
 * @brief  更新单个柱体的值并重绘
 * @param  inst      : 实例索引
 * @param  bar_index : 柱体索引 (0-based)
 * @param  new_value : 新数值
 * @return ESTA_StatusTypeDef
 */
ESTA_StatusTypeDef BARCHART_UpdateBar(int inst, uint16_t bar_index, uint16_t new_value) {
    if (!IS_VALID_BARCHART_INST(inst)) return ESTA_ERROR;
    if (bar_index >= BARCHART_PRIVATE_MEMBER(inst, bar_count_actual))
        return ESTA_ERROR;

    uint16_t display_num_min   = BARCHART_CONFIG_MEMBER(inst, display_num_min);
    uint16_t display_num_max   = BARCHART_CONFIG_MEMBER(inst, display_num_max);
    volatile bool is_display_value = BARCHART_CONFIG_MEMBER(inst, is_display_value);
    uint16_t theme_type        = BARCHART_CONFIG_MEMBER(inst, theme_type);

    uint16_t area_ox        = BARCHART_PRIVATE_MEMBER(inst, area_ox);
    uint16_t area_height    = BARCHART_PRIVATE_MEMBER(inst, area_height);
    uint16_t bw             = BARCHART_PRIVATE_MEMBER(inst, bar_width_actual);
    uint16_t bs             = BARCHART_PRIVATE_MEMBER(inst, bar_spacing_actual);
    uint16_t baseline_y     = BARCHART_PRIVATE_MEMBER(inst, baseline_y);

    uint16_t display_range  = display_num_max - display_num_min;
    uint16_t bg_color       = ESTA_GetThemeColor(theme_type, BARCHART_THEME_BACKGROUND_INDEX);
    uint16_t bar_color      = ESTA_GetThemeColor(theme_type, BARCHART_THEME_BAR_INDEX);
    uint16_t frame_color    = ESTA_GetThemeColor(theme_type, BARCHART_THEME_FRAME_INDEX);
    uint16_t label_color    = ESTA_GetThemeColor(theme_type, BARCHART_THEME_LABEL_INDEX);

    uint16_t step = bw + bs;
    uint16_t x_left  = area_ox + bar_index * step;
    uint16_t x_right = x_left + bw;

    /* 用背景色擦除旧柱体区域（包括标签区域） */
    uint16_t erase_top = BARCHART_PRIVATE_MEMBER(inst, area_oy);
    SCREEN_FILL(x_left, erase_top, x_right, baseline_y, bg_color);

    /* 更新数据 */
    uint16_t clamped_value = ui_limit(display_num_max, display_num_min, new_value);
    BARCHART_PRIVATE_MEMBER_ARRAY(inst, data_buff, bar_index) = clamped_value;

    /* 绘制新柱体 */
    uint16_t bar_height = ui_coor_normal(area_height, display_range,
        clamped_value - display_num_min);
    uint16_t y_top = baseline_y - bar_height;

    SCREEN_FILL(x_left, y_top, x_right, baseline_y, bar_color);
    SCREEN_DRAW_RECTANGLE(x_left, y_top, x_right, baseline_y, frame_color);

    if (is_display_value) {
        uint16_t digits = ui_num_digits(new_value);
        if (digits == 0) digits = 1;
        uint16_t label_x = x_left + (bw - digits * CHAR_PIXEL_WIDTH) / 2;
        uint16_t label_y = (y_top >= CHAR_PIXEL_HEIGHT)
            ? y_top - CHAR_PIXEL_HEIGHT : 0;
        SCREEN_DRAW_NUM(label_x, label_y, new_value, digits, label_color);
    }

    return ESTA_OK;
}

/**
 * @brief  批量更新所有柱体的值
 * @param  inst       : 实例索引
 * @param  new_values : 新数值数组
 * @param  count      : 数组长度
 * @return ESTA_StatusTypeDef
 */
ESTA_StatusTypeDef BARCHART_UpdateAll(int inst, const uint16_t *new_values, uint16_t count) {
    if (!IS_VALID_BARCHART_INST(inst)) return ESTA_ERROR;
    if (new_values == NULL) return ESTA_ERROR;

    uint16_t bar_count = BARCHART_PRIVATE_MEMBER(inst, bar_count_actual);
    if (count > bar_count) count = bar_count;

    for (int i = 0; i < count; i++) {
        BARCHART_PRIVATE_MEMBER_ARRAY(inst, data_buff, i) = new_values[i];
    }

    ESTA_RETURN_IF_ERROR(BARCHART_Clear(inst));
    ESTA_RETURN_IF_ERROR(BARCHART_BarDisplay(inst));
    return ESTA_OK;
}
