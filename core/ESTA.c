
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


#include "ESTA.h"
#include <string.h>

// 不要修改这个数组的名称，我们根据其来寻址
// 也不要直接通过数组修改里面的内容，而是通过库函数修改配置（除非你知道你在做什么）
// 你可以将其视作库函数中外设的基地址
ESTA_TypeDef ESTA_State[MAX_ESTA_NUM];

void ESTA_ConfigSetPositionAndSize(ESTA_Config_TypeDef *config,
                                  uint16_t x_origin, uint16_t y_origin,
                                  uint16_t x_width, uint16_t y_width) {
    if(config == NULL) return;
    config->x_origin = x_origin;
    config->y_origin = y_origin;
    config->x_width = x_width;
    config->y_width = y_width;
}

void ESTA_ConfigSetDisplayRange(ESTA_Config_TypeDef *config,
                               uint16_t display_num_min, uint16_t display_num_max) {
    if(config == NULL) return;
    config->display_num_min = display_num_min;
    config->display_num_max = display_num_max;
}

void ESTA_ConfigSetChannelNum(ESTA_Config_TypeDef *config, uint16_t channel_num) {
    if(config == NULL) return;
    config->channel_num = channel_num;
}

void ESTA_ConfigSetChannelEnabled(ESTA_Config_TypeDef *config, uint8_t channel_mask) {
    if(config == NULL) return;
    config->channel_mask |= channel_mask;
}

void ESTA_ConfigSetChannelDisabled(ESTA_Config_TypeDef *config, uint8_t channel_mask) {
    if(config == NULL) return;
    config->channel_mask &= ~channel_mask;
}

void ESTA_ConfigSetRulerY(ESTA_Config_TypeDef *config, bool is_display,
                         uint16_t *ruler_y, uint16_t ruler_count_y,
                         uint16_t ruler_num_digits_y) {
    if(config == NULL) return;
    config->is_display_ruler_y = is_display;
    config->ruler_y = ruler_y;
    config->ruler_count_y = ruler_count_y;
    config->ruler_num_digits_y = ruler_num_digits_y;
}

void ESTA_ConfigSetRulerX(ESTA_Config_TypeDef *config, bool is_display,
                         uint16_t *ruler_x, uint16_t ruler_count_x,
                         uint16_t ruler_zero_value_x, uint16_t ruler_full_value_x,
                         uint16_t ruler_num_digits_x) {
    if(config == NULL) return;
    config->is_display_ruler_x = is_display;
    config->ruler_x = ruler_x;
    config->ruler_count_x = ruler_count_x;
    config->ruler_zero_value_x = ruler_zero_value_x;
    config->ruler_full_value_x = ruler_full_value_x;
    config->ruler_num_digits_x = ruler_num_digits_x;
}

void ESTA_ConfigSetTheme(ESTA_Config_TypeDef *config, ESTA_theme_type theme_type) {
    if(config == NULL) return;
    config->theme_type = theme_type;
}

void ESTA_ConfigSetAutoClear(ESTA_Config_TypeDef *config, bool is_auto_clear) {
    if(config == NULL) return;
    config->is_auto_clear = is_auto_clear;
}

// 储存主题颜色的数组，你可以在此处添加自己的主题
// 你还需要ESTA_theme_type 枚举内添加你的主题名称
// 这样在配置寄存器的时候你就可以修改theme_type更改主题
uint16_t ESTA_theme[ESTA_THEME_COUNT][ESTA_THEME_INDEX_COUNT] = {
    [ESTA_THEME_DEFAULT] = {
        [ESTA_THEME_FRAME_INDEX]      = __WHITE,
        [ESTA_THEME_RULER_INDEX]      = __GRAY,
        [ESTA_THEME_WAVE_CH0_INDEX]   = __GREEN,
        [ESTA_THEME_WAVE_CH1_INDEX]   = __GBLUE,
        [ESTA_THEME_WAVE_CH2_INDEX]   = __YELLOW,
        [ESTA_THEME_WAVE_CH3_INDEX]   = __RED,
        [ESTA_THEME_BACKGROUND_INDEX] = __BLACK
    },
    [ESTA_THEME_LIGHT]   = {
        [ESTA_THEME_FRAME_INDEX]      = __BLACK,
        [ESTA_THEME_RULER_INDEX]      = __GRAY,
        [ESTA_THEME_WAVE_CH0_INDEX]   = __ORANGE,
        [ESTA_THEME_WAVE_CH1_INDEX]   = __DEEP_BLUE,
        [ESTA_THEME_WAVE_CH2_INDEX]   = __RED,
        [ESTA_THEME_WAVE_CH3_INDEX]   = __BLUE,
        [ESTA_THEME_BACKGROUND_INDEX] = __WHITE
    }
};

/* @brief : 坐标标准化
*/
static uint16_t coor_normal(uint16_t coor_width, uint16_t value_max_range, uint16_t value){
    if(value_max_range == 0 || coor_width == 0) return 0;
    return (uint16_t)((((uint32_t)value * coor_width) + (value_max_range >> 1)) / value_max_range);
}

static uint16_t my_limit(uint16_t max, uint16_t min, uint16_t value){
    if(value > max) return max;
    else if(value < min) return min;
    else return value;
}

static bool is_out_of_bound(uint16_t max, uint16_t min, uint16_t value){
    return (value < min || value > max); 
}

static uint16_t num_digits(uint16_t x){
    int count = 0;
    while(x != 0){
        x /= 10;
        count++;
    }
    return count;
}


/* @brief : 初始化示波器实例
*  @param : int OSCx : 示波器实例，如ESTA_INST(0)或ESTA_INST(1)
*           ESTA_Config_TypeDef *ESTA_Init : 指向ESTA设置结构体的指针
*  @return : enum ESTA_StatusTypeDef 为ESTA_OK则无问题，为ESTA_ERROR则有问题
*  @note   : ESTA_Config_TypeDef 见ESTA.h
*/
ESTA_StatusTypeDef ESTA_Init(int OSCx, ESTA_Config_TypeDef *ESTA_Init) {
    if(!IS_VALID_ESTA_INST(OSCx)) return ESTA_ERROR;

    ESTA_WRITE_CONFIG_INIT(OSCx, x_origin);
    ESTA_WRITE_CONFIG_INIT(OSCx, y_origin);
    ESTA_WRITE_CONFIG_INIT(OSCx, x_width);
    ESTA_WRITE_CONFIG_INIT(OSCx, y_width);
    if(ESTA_Init->display_num_min >= ESTA_Init->display_num_max) return ESTA_ERROR;
    ESTA_WRITE_CONFIG_INIT(OSCx, display_num_min);
    ESTA_WRITE_CONFIG_INIT(OSCx, display_num_max);
    ESTA_WRITE_CONFIG_INIT(OSCx, channel_mask);

    // 初始化Y轴标尺
    if(ESTA_Init->is_display_ruler_y){
        if(ESTA_Init->ruler_y == NULL) return ESTA_ERROR;

        uint16_t ruler_actual_count_y = (ESTA_Init->ruler_count_y > ESTA_MAX_RULER_Y_NUM) ? 
                ESTA_MAX_RULER_Y_NUM : ESTA_Init->ruler_count_y;
        // 优化：先将整个缓存区清零，再拷贝有效数据，消除 if-else 分支
        memset(ESTA_INST_ADDR(OSCx).ESTA_Private.ruler_buff_y, 0, sizeof(ESTA_INST_ADDR(OSCx).ESTA_Private.ruler_buff_y));
        for(int i = 0; i < ruler_actual_count_y; i++){
            ESTA_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_y, i) = ESTA_Init->ruler_y[i];
        }
        ESTA_WRITE_CONFIG_INIT(OSCx, is_display_ruler_y);
        ESTA_WRITE_CONFIG(OSCx, ruler_y, NULL);
        ESTA_WRITE_CONFIG(OSCx, ruler_count_y, ruler_actual_count_y);
        ESTA_WRITE_CONFIG_INIT(OSCx, ruler_num_digits_y);
    }else{
        memset(ESTA_INST_ADDR(OSCx).ESTA_Private.ruler_buff_y, 0, sizeof(ESTA_INST_ADDR(OSCx).ESTA_Private.ruler_buff_y));
        ESTA_WRITE_CONFIG(OSCx, is_display_ruler_y, false);
        ESTA_WRITE_CONFIG(OSCx, ruler_y, NULL);
        ESTA_WRITE_CONFIG(OSCx, ruler_count_y, 0);
        ESTA_WRITE_CONFIG(OSCx, ruler_num_digits_y, 0);
    }
    
    // 初始化X轴标尺
    if(ESTA_Init->is_display_ruler_x){
        if(ESTA_Init->ruler_x == NULL) return ESTA_ERROR;

        uint16_t ruler_actual_count_x = (ESTA_Init->ruler_count_x > ESTA_MAX_RULER_X_NUM) ? 
                ESTA_MAX_RULER_X_NUM : ESTA_Init->ruler_count_x;
        memset(ESTA_INST_ADDR(OSCx).ESTA_Private.ruler_buff_x, 0, sizeof(ESTA_INST_ADDR(OSCx).ESTA_Private.ruler_buff_x));
        for(int i = 0; i < ruler_actual_count_x; i++){
            ESTA_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_x, i) = ESTA_Init->ruler_x[i];
        }
        ESTA_WRITE_CONFIG_INIT(OSCx, is_display_ruler_x);
        ESTA_WRITE_CONFIG(OSCx, ruler_x, NULL);
        ESTA_WRITE_CONFIG(OSCx, ruler_count_x, ruler_actual_count_x);
        ESTA_WRITE_CONFIG_INIT(OSCx, ruler_zero_value_x);
        ESTA_WRITE_CONFIG_INIT(OSCx, ruler_full_value_x);
        ESTA_WRITE_CONFIG_INIT(OSCx, ruler_num_digits_x);
    }else{
        memset(ESTA_INST_ADDR(OSCx).ESTA_Private.ruler_buff_x, 0, sizeof(ESTA_INST_ADDR(OSCx).ESTA_Private.ruler_buff_x));
        ESTA_WRITE_CONFIG(OSCx, is_display_ruler_x, false);
        ESTA_WRITE_CONFIG(OSCx, ruler_x, NULL);
        ESTA_WRITE_CONFIG(OSCx, ruler_count_x, 0);
        ESTA_WRITE_CONFIG(OSCx, ruler_zero_value_x, 0);
        ESTA_WRITE_CONFIG(OSCx, ruler_full_value_x, 0);
        ESTA_WRITE_CONFIG(OSCx, ruler_num_digits_x, 0);
    }

    if(!IS_VALID_CHNUM(ESTA_CONFIG_MEMBER(OSCx, channel_num))) return ESTA_ERROR;
    ESTA_WRITE_CONFIG_INIT(OSCx, channel_num);

    if(!IS_VALID_THEME(ESTA_CONFIG_MEMBER(OSCx, theme_type))) return ESTA_ERROR;
    ESTA_WRITE_CONFIG_INIT(OSCx, theme_type);
    ESTA_WRITE_CONFIG_INIT(OSCx, is_auto_clear);

    /* Private regs */
    ESTA_WRITE_PRIVATE(OSCx, last_index, 0);
    ESTA_WRITE_PRIVATE(OSCx, x_coor_last, ESTA_Init->x_origin);
    for(int i = 0; i < MAX_ESTA_CHANNEL; i++){
        ESTA_WRITE_PRIVATE(OSCx, y_coor_last_CH[i], 0);
    }

    return ESTA_OK;
}

/* @brief : 复位示波器实例
*  @param : int OSCx : 示波器实例，如ESTA_INST(0)或ESTA_INST(1)
*  @return : enum ESTA_StatusTypeDef 为ESTA_OK则无问题，为ESTA_ERROR则有问题
*/
ESTA_StatusTypeDef ESTA_DeInit(int OSCx) {
    if(!IS_VALID_ESTA_INST(OSCx)) return ESTA_ERROR;

    ESTA_WRITE_CONFIG(OSCx, x_origin, 0);
    ESTA_WRITE_CONFIG(OSCx, y_origin, 0);
    ESTA_WRITE_CONFIG(OSCx, x_width, 0);
    ESTA_WRITE_CONFIG(OSCx, y_width, 0);
    ESTA_WRITE_CONFIG(OSCx, display_num_min, 0);
    ESTA_WRITE_CONFIG(OSCx, display_num_max, 0);
    ESTA_WRITE_CONFIG(OSCx, is_display_ruler_y, false);
    ESTA_WRITE_CONFIG(OSCx, ruler_y, NULL);
    ESTA_WRITE_CONFIG(OSCx, ruler_count_y, 0);
    ESTA_WRITE_CONFIG(OSCx, ruler_num_digits_y, 0);
    ESTA_WRITE_CONFIG(OSCx, is_display_ruler_x, false);
    ESTA_WRITE_CONFIG(OSCx, ruler_x, NULL);
    ESTA_WRITE_CONFIG(OSCx, ruler_count_x, 0);
    ESTA_WRITE_CONFIG(OSCx, ruler_num_digits_x, 0);
    ESTA_WRITE_CONFIG(OSCx, ruler_zero_value_x, 0);
    ESTA_WRITE_CONFIG(OSCx, ruler_full_value_x, 0);
    ESTA_WRITE_CONFIG(OSCx, theme_type, ESTA_THEME_DEFAULT);
    ESTA_WRITE_CONFIG(OSCx, is_auto_clear, true);

    for(int i = 0; i < ESTA_MAX_RULER_Y_NUM; i++){
        ESTA_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_y, i) = 0;
    }
    for(int i = 0; i < ESTA_MAX_RULER_X_NUM; i++){
        ESTA_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_x, i) = 0;
    }
    ESTA_WRITE_PRIVATE(OSCx, last_index, 0);
    ESTA_WRITE_PRIVATE(OSCx, x_coor_last, 0);
    for(int i = 0; i < MAX_ESTA_CHANNEL; i++){
        ESTA_WRITE_PRIVATE(OSCx, y_coor_last_CH[i], 0);
    }

    return ESTA_OK;
}

/* @brief : 获得主题颜色
*  @param : ESTA_theme_type theme 主题名称
*           ESTA_theme_color_index_type color_type 颜色类型索引
*  @return : uint16_t 颜色值
*/
uint16_t ESTA_GetThemeColor(ESTA_theme_type theme, ESTA_theme_color_index_type color_type) {
    if(theme >= ESTA_THEME_COUNT) theme = ESTA_THEME_DEFAULT;
    if(color_type >= ESTA_THEME_INDEX_COUNT) color_type = ESTA_THEME_FRAME_INDEX;
    
    return ESTA_theme[theme][color_type];
}

/* @brief : 绘制示波器标尺
*  @param : int OSCx : 示波器实例，如ESTA_INST(0)或ESTA_INST(1)
*  @return : enum ESTA_StatusTypeDef 为ESTA_OK则无问题，为ESTA_ERROR则有问题
*/
ESTA_StatusTypeDef ESTA_RulerDisplay(int OSCx) {
    volatile bool is_display_ruler_y = ESTA_CONFIG_MEMBER(OSCx, is_display_ruler_y);
    volatile bool is_display_ruler_x = ESTA_CONFIG_MEMBER(OSCx, is_display_ruler_x);
    if(!(is_display_ruler_x || is_display_ruler_y)) return ESTA_OK;

    if(!IS_VALID_ESTA_INST(OSCx)) return ESTA_ERROR;

    uint16_t x_origin             = ESTA_CONFIG_MEMBER(OSCx, x_origin); 
    uint16_t y_origin             = ESTA_CONFIG_MEMBER(OSCx, y_origin); 
    uint16_t x_width              = ESTA_CONFIG_MEMBER(OSCx, x_width);
    uint16_t y_width              = ESTA_CONFIG_MEMBER(OSCx, y_width);
    uint16_t display_num_min      = ESTA_CONFIG_MEMBER(OSCx, display_num_min);
    uint16_t display_num_max      = ESTA_CONFIG_MEMBER(OSCx, display_num_max);
    uint16_t ruler_count_y        = ESTA_CONFIG_MEMBER(OSCx, ruler_count_y);
    uint16_t ruler_num_digits_y   = ESTA_CONFIG_MEMBER(OSCx, ruler_num_digits_y);
    uint16_t ruler_count_x        = ESTA_CONFIG_MEMBER(OSCx, ruler_count_x);
    uint16_t ruler_zero_value_x   = ESTA_CONFIG_MEMBER(OSCx, ruler_zero_value_x);
    uint16_t ruler_full_value_x   = ESTA_CONFIG_MEMBER(OSCx, ruler_full_value_x);
    uint16_t ruler_num_digits_x   = ESTA_CONFIG_MEMBER(OSCx, ruler_num_digits_x);
    uint16_t theme_type  = ESTA_CONFIG_MEMBER(OSCx, theme_type);
    uint16_t frame_color = ESTA_GetThemeColor(theme_type, ESTA_THEME_FRAME_INDEX);
    uint16_t ruler_color = ESTA_GetThemeColor(theme_type, ESTA_THEME_RULER_INDEX);

    uint16_t y_frame_width = (is_display_ruler_x) ? 
            (y_width - CHAR_PIXEL_HEIGHT) : y_width;
    uint16_t x_frame_width = (is_display_ruler_y) ? 
            (x_width - ruler_num_digits_y * CHAR_PIXEL_WIDTH) : x_width;

    if(is_display_ruler_y){
        uint16_t display_range = display_num_max - display_num_min;

        for(int i = 0; i < ruler_count_y; i++){
            uint16_t ruler = ESTA_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_y, i);
            uint16_t ruler_y_coor = 0;
            ruler_y_coor = y_origin + y_frame_width - 
                coor_normal(y_frame_width, display_range, ruler - display_num_min);
            ruler_y_coor = my_limit(y_origin + y_frame_width - 2, y_origin + 2, ruler_y_coor);
            uint16_t ruler_y_num_coor = 
                my_limit(y_origin + y_frame_width, y_origin, 
                ruler_y_coor - CHAR_PIXEL_HEIGHT / 2);
            // 防止无符号整数溢出
            if((int16_t)ruler_y_coor - CHAR_PIXEL_HEIGHT / 2 < 0) ruler_y_num_coor = 0;
            SCREEN_DRAW_LINE(x_origin, ruler_y_coor, 
                x_origin + x_frame_width, ruler_y_coor, ruler_color);
                //原注释：有时屏幕显示数字需要画两次 但在模拟器却没有出现该情况，推测是硬件驱动的问题
                SCREEN_DRAW_NUM(x_origin + x_frame_width, ruler_y_num_coor, 
                    ruler, ruler_num_digits_y, ruler_color);
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
            uint16_t ruler = ESTA_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_x, i);
            uint16_t ruler_actual_digits = num_digits(ruler);
            uint16_t ruler_x_coor = 0;
            ruler_x_coor = x_origin + 
                coor_normal(x_frame_width, x_ruler_range, ruler - ruler_zero_value_x);
            ruler_x_coor = 
                my_limit(x_origin + x_frame_width - 2, x_origin + 2, ruler_x_coor);
            uint16_t ruler_x_num_coor = 
                my_limit(x_origin + x_frame_width, x_origin, 
                ruler_x_coor - ruler_actual_digits * CHAR_PIXEL_WIDTH / 2);
            // 防止无符号整数溢出
            if((int16_t)ruler_x_coor - ruler_actual_digits * CHAR_PIXEL_WIDTH / 2 < 0) {
                ruler_x_num_coor = 0;
            }
            SCREEN_DRAW_LINE(ruler_x_coor, y_origin, 
                ruler_x_coor, y_origin + y_frame_width, ruler_color);
                //原注释：有时屏幕显示数字需要画两次 //但在模拟器却没有出现该情况，推测是硬件驱动的问题
                SCREEN_DRAW_NUM(ruler_x_num_coor, y_origin + y_frame_width, 
                    ruler, ruler_actual_digits, ruler_color);
        }
    }
    return ESTA_OK;
}

/* @brief : 绘制示波器框架
*  @param : int OSCx : 示波器实例，如ESTA_INST(0)或ESTA_INST(1)
*  @return : enum ESTA_StatusTypeDef 为ESTA_OK则无问题，为ESTA_ERROR则有问题
*/
ESTA_StatusTypeDef ESTA_FrameDisplay(int OSCx) {
    if(!IS_VALID_ESTA_INST(OSCx)) return ESTA_ERROR;
    uint16_t x_origin    = ESTA_CONFIG_MEMBER(OSCx, x_origin); 
    uint16_t y_origin    = ESTA_CONFIG_MEMBER(OSCx, y_origin); 
    uint16_t x_width     = ESTA_CONFIG_MEMBER(OSCx, x_width);
    uint16_t y_width     = ESTA_CONFIG_MEMBER(OSCx, y_width);
    volatile bool is_display_ruler_y = ESTA_CONFIG_MEMBER(OSCx, is_display_ruler_y);
    volatile bool is_display_ruler_x = ESTA_CONFIG_MEMBER(OSCx, is_display_ruler_x);
    uint16_t ruler_num_digits_y = ESTA_CONFIG_MEMBER(OSCx, ruler_num_digits_y);
    uint16_t theme_type  = ESTA_CONFIG_MEMBER(OSCx, theme_type);
    uint16_t frame_color = ESTA_GetThemeColor(theme_type, ESTA_THEME_FRAME_INDEX);

    uint16_t y_frame_width = (is_display_ruler_x) ? 
            (y_width - CHAR_PIXEL_HEIGHT) : y_width;
    uint16_t x_frame_width = (is_display_ruler_y) ? 
            (x_width - ruler_num_digits_y * CHAR_PIXEL_WIDTH) : x_width;
    uint16_t x_outline_width = x_width;
    SCREEN_DRAW_RECTANGLE(x_origin, y_origin, 
            x_origin + x_frame_width, y_origin + y_frame_width, frame_color);
    SCREEN_DRAW_RECTANGLE(x_origin, y_origin, 
            x_origin + x_outline_width, y_origin + y_width, frame_color);
    return ESTA_OK;
}

/* @brief : 清除曲线
*  @param : int OSCx : 示波器实例，如ESTA_INST(0)或ESTA_INST(1)
*  @return : enum ESTA_StatusTypeDef 为ESTA_OK则无问题，为ESTA_ERROR则有问题
*/
ESTA_StatusTypeDef ESTA_CurveClear(int OSCx) {
    if(!IS_VALID_ESTA_INST(OSCx)) return ESTA_ERROR;
    uint16_t x_origin    = ESTA_CONFIG_MEMBER(OSCx, x_origin); 
    uint16_t y_origin    = ESTA_CONFIG_MEMBER(OSCx, y_origin); 
    uint16_t x_width     = ESTA_CONFIG_MEMBER(OSCx, x_width);
    uint16_t y_width     = ESTA_CONFIG_MEMBER(OSCx, y_width);
    volatile bool is_display_ruler_y = ESTA_CONFIG_MEMBER(OSCx, is_display_ruler_y);
    uint16_t ruler_num_digits_y = ESTA_CONFIG_MEMBER(OSCx, ruler_num_digits_y);
    uint16_t theme_type  = ESTA_CONFIG_MEMBER(OSCx, theme_type);
    uint16_t frame_color = ESTA_GetThemeColor(theme_type, ESTA_THEME_FRAME_INDEX);
    uint16_t bg_color    = ESTA_GetThemeColor(theme_type, ESTA_THEME_BACKGROUND_INDEX);

    SCREEN_FILL(x_origin, y_origin, x_origin + x_width, y_origin + y_width, bg_color);
    SCREEN_DRAW_RECTANGLE(x_origin, y_origin, 
            x_origin + x_width, y_origin + y_width, frame_color);
    // reset the index of ESTA
    ESTA_WRITE_PRIVATE(OSCx, last_index, 0);
    ESTA_WRITE_PRIVATE(OSCx, x_coor_last, x_origin);
    for(int i = 0; i < MAX_ESTA_CHANNEL; i++){
        ESTA_WRITE_PRIVATE(OSCx, y_coor_last_CH[i], 0);
    }
    if(is_display_ruler_y) {
        if(ESTA_RulerDisplay(OSCx) == ESTA_ERROR) return ESTA_ERROR;
    }
    return ESTA_OK;
}

/* @brief : 绘制示波器框架与（如有）标尺
*  @param : int OSCx : 示波器实例，如ESTA_INST(0)或ESTA_INST(1)
*           ESTA_Config_TypeDef *ESTA_Init : 指向ESTA设置结构体的指针
*  @return : enum ESTA_StatusTypeDef 为ESTA_OK则无问题，为ESTA_ERROR则有问题
*/
ESTA_StatusTypeDef ESTA_ReDraw(int OSCx) {
    if(!IS_VALID_ESTA_INST(OSCx)) return ESTA_ERROR;
    if(ESTA_FrameDisplay(OSCx) == ESTA_ERROR) return ESTA_ERROR;
    if(ESTA_CurveClear(OSCx) == ESTA_ERROR) return ESTA_ERROR;
    return ESTA_OK;
}

/* @brief : 向示波器发送一个数据，绘制曲线，（可选）曲线满自动清屏
*  @param : int OSCx : 示波器实例，如ESTA_INST(0)或ESTA_INST(1)
*           uint16_t data_CH0 : CH0通道的数据
*           uint16_t data_CH1 : CH1通道的数据，若为单通道置零即可
*  @return : enum ESTA_StatusTypeDef 为ESTA_OK则无问题，为ESTA_ERROR则有问题
*            若示波器实例is_auto_clear == false，不会自动清屏，同时屏幕满时返回ESTA_FULL
*/
ESTA_StatusTypeDef ESTA_CurveDraw(int OSCx, uint16_t data_CH[]) {
    if(!IS_VALID_ESTA_INST(OSCx)) return ESTA_ERROR;
    uint16_t x_origin                = ESTA_CONFIG_MEMBER(OSCx, x_origin); 
    uint16_t y_origin                = ESTA_CONFIG_MEMBER(OSCx, y_origin); 
    uint16_t x_width                 = ESTA_CONFIG_MEMBER(OSCx, x_width);
    uint16_t y_width                 = ESTA_CONFIG_MEMBER(OSCx, y_width);
    volatile bool is_display_ruler_x = ESTA_CONFIG_MEMBER(OSCx, is_display_ruler_x);
    volatile bool is_display_ruler_y = ESTA_CONFIG_MEMBER(OSCx, is_display_ruler_y);
    uint16_t ruler_num_digits_y      = ESTA_CONFIG_MEMBER(OSCx, ruler_num_digits_y);
    uint16_t display_num_min         = ESTA_CONFIG_MEMBER(OSCx, display_num_min);
    uint16_t display_num_max         = ESTA_CONFIG_MEMBER(OSCx, display_num_max);
    uint16_t channel_num             = ESTA_CONFIG_MEMBER(OSCx, channel_num);
    uint16_t theme_type              = ESTA_CONFIG_MEMBER(OSCx, theme_type);
    volatile bool is_auto_clear      = ESTA_CONFIG_MEMBER(OSCx, is_auto_clear);
    uint8_t channel_mask             = ESTA_CONFIG_MEMBER(OSCx, channel_mask);

    uint16_t active_channel_num = (channel_num > MAX_ESTA_CHANNEL) ? MAX_ESTA_CHANNEL : channel_num;

    uint16_t wave_CH_color[MAX_ESTA_CHANNEL] = {0};
    for(int i = 0; i < active_channel_num; i++){
        if(is_channel_enabled(channel_mask, CH0 << i)) {
            wave_CH_color[i] = ESTA_GetThemeColor(theme_type, ESTA_THEME_WAVE_CH0_INDEX + i);
        }
    }

    uint16_t last_index       = ESTA_PRIVATE_MEMBER(OSCx, last_index);
    uint16_t x_coor_last      = ESTA_PRIVATE_MEMBER(OSCx, x_coor_last);
    uint16_t y_coor_last_CH[MAX_ESTA_CHANNEL];
    for(int i = 0; i < active_channel_num; i++){
        y_coor_last_CH[i] = ESTA_PRIVATE_MEMBER(OSCx, y_coor_last_CH[i]);
    }

    uint16_t y_frame_width = (is_display_ruler_x) ? 
            (y_width - CHAR_PIXEL_HEIGHT) : y_width;
    uint16_t x_frame_width = (is_display_ruler_y) ? 
            (x_width - ruler_num_digits_y * CHAR_PIXEL_WIDTH) : x_width;

    if(last_index >= x_frame_width) {
        if (!is_auto_clear) {
            // 重置绘制状态
            ESTA_WRITE_PRIVATE(OSCx, last_index, 0);
            ESTA_WRITE_PRIVATE(OSCx, x_coor_last, x_origin);
            return ESTA_FULL;
        }
        last_index = 0;
        ESTA_CurveClear(OSCx);
    }

    uint16_t display_range = display_num_max - display_num_min;
    uint16_t x_coor = x_origin + last_index;

    for(int i = 0; i < active_channel_num; i++) {
        uint8_t ch_mask = (uint8_t)(CH0 << i);
        if(!is_channel_enabled(channel_mask, ch_mask)) continue;

        if(is_out_of_bound(display_num_max, display_num_min, data_CH[i])) return ESTA_ERROR;

        uint16_t y_display_value = my_limit(display_num_max, display_num_min, data_CH[i]);
        uint16_t y_coor = y_origin + y_frame_width -
                coor_normal(y_frame_width, display_range, y_display_value);

        if(last_index > 0) {
            SCREEN_DRAW_LINE(x_coor, y_coor,
                x_coor_last, y_coor_last_CH[i], wave_CH_color[i]);
        }
        ESTA_PRIVATE_MEMBER(OSCx, y_coor_last_CH[i]) = y_coor;
    }

    ESTA_PRIVATE_MEMBER(OSCx, x_coor_last) = x_coor;
    last_index++;
    ESTA_PRIVATE_MEMBER(OSCx, last_index)  = last_index;

    return ESTA_OK;
}
