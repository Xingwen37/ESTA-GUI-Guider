
/*
    TL-OSC 项目 ： 使用OOP in C (OOC) 写法的示波器库，仿HAL库搭建
    主要贡献者 : TongLewis(yangyutong) HEU ESTA 2025
    设计特点：
        1. 调用简单：使用仿HAL模式的方式设计，只需配置抽象寄存器即可调用，无需复杂配置。
        2. 功能强大：可绘制波形与X轴，Y轴标尺以及标尺数字。
        3. 代码规范：变量规范，宏丰富抽象层次高，错误处理方式规范，可读性强。
        4. 移植方便：设计屏幕函数抽象层，使用不同屏幕只需修改一处代码，轻松移植。
        5. 资料齐全：代码注释，样例，文档强力支持，快速上手。
        6. 外观精致：示波器主题自定义简单，也可以使用预设主题。
    版本记录：
        2025.7.16 完成概念设计
        2025.7.17 完成基本框架设计
        2025.7.18 完善文档与样例，发布BetaV0.1版本
    该项目还处于内测版本，如果你有更好的想法/想添加的功能/主题等，欢迎与我联系。
    TIPS:将编译优化开至-O2效果更好
*/


#include "OSC.h"

// 不要修改这个数组的名称，我们根据其来寻址
// 也不要直接通过数组修改里面的内容，而是通过库函数修改配置（除非你知道你在做什么）
// 你可以将其视作库函数中外设的基地址
OSC_TypeDef OSC_State[MAX_OSC_NUM];

// 储存主题颜色的数组，你可以在此处添加自己的主题
// 你还需要OSC_theme_type 枚举内添加你的主题名称
// 这样在配置寄存器的时候你就可以修改theme_type更改主题
uint16_t OSC_theme[OSC_THEME_COUNT][OSC_THEME_INDEX_COUNT] = {
    [OSC_THEME_DEFAULT] = {
        [OSC_THEME_FRAME_INDEX]      = __WHITE,
        [OSC_THEME_RULER_INDEX]      = __GRAY,
        [OSC_THEME_WAVE_CH0_INDEX]   = __GREEN,
        [OSC_THEME_WAVE_CH1_INDEX]   = __GBLUE,
        [OSC_THEME_BACKGROUND_INDEX] = __BLACK
    },
    [OSC_THEME_LIGHT]   = {
        [OSC_THEME_FRAME_INDEX]      = __BLACK,
        [OSC_THEME_RULER_INDEX]      = __GRAY,
        [OSC_THEME_WAVE_CH0_INDEX]   = __ORANGE,
        [OSC_THEME_WAVE_CH1_INDEX]   = __DEEP_BLUE,
        [OSC_THEME_BACKGROUND_INDEX] = __WHITE
    }
};

/* @brief : 坐标标准化
*/
static uint16_t coor_normal(uint16_t coor_width, uint16_t value_max_range, uint16_t value){
    if(value_max_range == 0 || coor_width == 0) return 0;
    return (uint16_t)((double)value / (double)value_max_range * (double)coor_width);
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

static void delay_ms(int ms) {
  HAL_Delay(ms);
}

/* @brief : 初始化示波器实例
*  @param : int OSCx : 示波器实例，如OSC_INST(0)或OSC_INST(1)
*           OSC_Config_TypeDef *OSC_Init : 指向OSC设置结构体的指针
*  @return : enum OSC_StatusTypeDef 为OSC_OK则无问题，为OSC_ERROR则有问题
*  @note   : OSC_Config_TypeDef 见OSC.h
*/
OSC_StatusTypeDef OSC_Init(int OSCx, OSC_Config_TypeDef *OSC_Init) {
    if(!IS_VALID_OSC_INST(OSCx)) return OSC_ERROR;

    OSC_WRITE_CONFIG_INIT(OSCx, x_origin);
    OSC_WRITE_CONFIG_INIT(OSCx, y_origin);
    OSC_WRITE_CONFIG_INIT(OSCx, x_width);
    OSC_WRITE_CONFIG_INIT(OSCx, y_width);
    if(OSC_Init->display_num_min >= OSC_Init->display_num_max) return OSC_ERROR;
    OSC_WRITE_CONFIG_INIT(OSCx, display_num_min);
    OSC_WRITE_CONFIG_INIT(OSCx, display_num_max);

    // 初始化Y轴标尺
    if(OSC_Init->is_display_ruler_y){
        if(OSC_Init->ruler_y == NULL) return OSC_ERROR;

        uint16_t ruler_actual_count_y = (OSC_Init->ruler_count_y > OSC_MAX_RULER_Y_NUM) ? 
                OSC_MAX_RULER_Y_NUM : OSC_Init->ruler_count_y;
        for(int i = 0; i < OSC_MAX_RULER_Y_NUM; i++){
            if(i < ruler_actual_count_y) {
                OSC_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_y, i) = OSC_Init->ruler_y[i];
            }else{
                OSC_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_y, i) = 0;
            }   
        }
        OSC_WRITE_CONFIG_INIT(OSCx, is_display_ruler_y);
        OSC_WRITE_CONFIG(OSCx, ruler_y, NULL);
        OSC_WRITE_CONFIG(OSCx, ruler_count_y, ruler_actual_count_y);
        OSC_WRITE_CONFIG_INIT(OSCx, ruler_num_digits_y);
    }else{
        for(int i = 0; i < OSC_MAX_RULER_Y_NUM; i++){
            OSC_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_y, i) = 0;
        }
        OSC_WRITE_CONFIG(OSCx, is_display_ruler_y, false);
        OSC_WRITE_CONFIG(OSCx, ruler_y, NULL);
        OSC_WRITE_CONFIG(OSCx, ruler_count_y, 0);
        OSC_WRITE_CONFIG(OSCx, ruler_num_digits_y, 0);
    }
    
    // 初始化X轴标尺
    if(OSC_Init->is_display_ruler_x){
        if(OSC_Init->ruler_x == NULL) return OSC_ERROR;

        uint16_t ruler_actual_count_x = (OSC_Init->ruler_count_x > OSC_MAX_RULER_X_NUM) ? 
                OSC_MAX_RULER_X_NUM : OSC_Init->ruler_count_x;
        for(int i = 0; i < OSC_MAX_RULER_X_NUM; i++){
            if(i < ruler_actual_count_x) {
                OSC_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_x, i) = OSC_Init->ruler_x[i];
            }else{
                OSC_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_x, i) = 0;
            }   
        }
        OSC_WRITE_CONFIG_INIT(OSCx, is_display_ruler_x);
        OSC_WRITE_CONFIG(OSCx, ruler_x, NULL);
        OSC_WRITE_CONFIG(OSCx, ruler_count_x, ruler_actual_count_x);
        OSC_WRITE_CONFIG_INIT(OSCx, ruler_zero_value_x);
        OSC_WRITE_CONFIG_INIT(OSCx, ruler_full_value_x);
        OSC_WRITE_CONFIG_INIT(OSCx, ruler_num_digits_x);
    }else{
        for(int i = 0; i < OSC_MAX_RULER_X_NUM; i++){
            OSC_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_x, i) = 0;
        }
        OSC_WRITE_CONFIG(OSCx, is_display_ruler_x, false);
        OSC_WRITE_CONFIG(OSCx, ruler_x, NULL);
        OSC_WRITE_CONFIG(OSCx, ruler_count_x, 0);
        OSC_WRITE_CONFIG(OSCx, ruler_zero_value_x, 0);
        OSC_WRITE_CONFIG(OSCx, ruler_full_value_x, 0);
        OSC_WRITE_CONFIG(OSCx, ruler_num_digits_x, 0);
    }

    if(!IS_VALID_CHNUM(OSC_CONFIG_MEMBER(OSCx, channel_num))) return OSC_ERROR;
    OSC_WRITE_CONFIG_INIT(OSCx, channel_num);

    if(!IS_VALID_THEME(OSC_CONFIG_MEMBER(OSCx, theme_type))) return OSC_ERROR;
    OSC_WRITE_CONFIG_INIT(OSCx, theme_type);
    OSC_WRITE_CONFIG_INIT(OSCx, is_auto_clear);

    /* Private regs */
    OSC_WRITE_PRIVATE(OSCx, last_index, 0);
    OSC_WRITE_PRIVATE(OSCx, x_coor_last, OSC_Init->x_origin);
    OSC_WRITE_PRIVATE(OSCx, y_coor_last_CH0, 0);
    OSC_WRITE_PRIVATE(OSCx, y_coor_last_CH1, 0);

    return OSC_OK;
}

/* @brief : 复位示波器实例
*  @param : int OSCx : 示波器实例，如OSC_INST(0)或OSC_INST(1)
*  @return : enum OSC_StatusTypeDef 为OSC_OK则无问题，为OSC_ERROR则有问题
*/
OSC_StatusTypeDef OSC_DeInit(int OSCx) {
    if(!IS_VALID_OSC_INST(OSCx)) return OSC_ERROR;

    OSC_WRITE_CONFIG(OSCx, x_origin, 0);
    OSC_WRITE_CONFIG(OSCx, y_origin, 0);
    OSC_WRITE_CONFIG(OSCx, x_width, 0);
    OSC_WRITE_CONFIG(OSCx, y_width, 0);
    OSC_WRITE_CONFIG(OSCx, display_num_min, 0);
    OSC_WRITE_CONFIG(OSCx, display_num_max, 0);
    OSC_WRITE_CONFIG(OSCx, is_display_ruler_y, false);
    OSC_WRITE_CONFIG(OSCx, ruler_y, NULL);
    OSC_WRITE_CONFIG(OSCx, ruler_count_y, 0);
    OSC_WRITE_CONFIG(OSCx, ruler_num_digits_y, 0);
    OSC_WRITE_CONFIG(OSCx, is_display_ruler_x, false);
    OSC_WRITE_CONFIG(OSCx, ruler_x, NULL);
    OSC_WRITE_CONFIG(OSCx, ruler_count_x, 0);
    OSC_WRITE_CONFIG(OSCx, ruler_num_digits_x, 0);
    OSC_WRITE_CONFIG(OSCx, ruler_zero_value_x, 0);
    OSC_WRITE_CONFIG(OSCx, ruler_full_value_x, 0);
    OSC_WRITE_CONFIG(OSCx, theme_type, OSC_THEME_DEFAULT);
    OSC_WRITE_CONFIG(OSCx, is_auto_clear, true);

    for(int i = 0; i < OSC_MAX_RULER_Y_NUM; i++){
        OSC_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_y, i) = 0;
    }
    for(int i = 0; i < OSC_MAX_RULER_X_NUM; i++){
        OSC_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_x, i) = 0;
    }
    OSC_WRITE_PRIVATE(OSCx, last_index, 0);
    OSC_WRITE_PRIVATE(OSCx, x_coor_last, 0);
    OSC_WRITE_PRIVATE(OSCx, y_coor_last_CH0, 0);
    OSC_WRITE_PRIVATE(OSCx, y_coor_last_CH1, 0);

    return OSC_OK;
}

/* @brief : 获得主题颜色
*  @param : OSC_theme_type theme 主题名称
*           OSC_theme_color_index_type color_type 颜色类型索引
*  @return : uint16_t 颜色值
*/
uint16_t OSC_GetThemeColor(OSC_theme_type theme, OSC_theme_color_index_type color_type) {
    if(theme >= OSC_THEME_COUNT) theme = OSC_THEME_DEFAULT;
    if(color_type >= OSC_THEME_INDEX_COUNT) color_type = OSC_THEME_FRAME_INDEX;
    
    return OSC_theme[theme][color_type];
}

/* @brief : 绘制示波器标尺
*  @param : int OSCx : 示波器实例，如OSC_INST(0)或OSC_INST(1)
*  @return : enum OSC_StatusTypeDef 为OSC_OK则无问题，为OSC_ERROR则有问题
*/
OSC_StatusTypeDef OSC_RulerDisplay(int OSCx) {
    volatile bool is_display_ruler_y = OSC_CONFIG_MEMBER(OSCx, is_display_ruler_y);
    volatile bool is_display_ruler_x = OSC_CONFIG_MEMBER(OSCx, is_display_ruler_x);
    if(!(is_display_ruler_x | is_display_ruler_y)) return OSC_OK;

    if(!IS_VALID_OSC_INST(OSCx)) return OSC_ERROR;

    uint16_t x_origin             = OSC_CONFIG_MEMBER(OSCx, x_origin); 
    uint16_t y_origin             = OSC_CONFIG_MEMBER(OSCx, y_origin); 
    uint16_t x_width              = OSC_CONFIG_MEMBER(OSCx, x_width);
    uint16_t y_width              = OSC_CONFIG_MEMBER(OSCx, y_width);
    uint16_t display_num_min      = OSC_CONFIG_MEMBER(OSCx, display_num_min);
    uint16_t display_num_max      = OSC_CONFIG_MEMBER(OSCx, display_num_max);
    uint16_t ruler_count_y        = OSC_CONFIG_MEMBER(OSCx, ruler_count_y);
    uint16_t ruler_num_digits_y   = OSC_CONFIG_MEMBER(OSCx, ruler_num_digits_y);
    uint16_t ruler_count_x        = OSC_CONFIG_MEMBER(OSCx, ruler_count_x);
    uint16_t ruler_zero_value_x   = OSC_CONFIG_MEMBER(OSCx, ruler_zero_value_x);
    uint16_t ruler_full_value_x   = OSC_CONFIG_MEMBER(OSCx, ruler_full_value_x);
    uint16_t ruler_num_digits_x   = OSC_CONFIG_MEMBER(OSCx, ruler_num_digits_x);
    uint16_t theme_type  = OSC_CONFIG_MEMBER(OSCx, theme_type);
    uint16_t frame_color = OSC_GetThemeColor(theme_type, OSC_THEME_FRAME_INDEX);
    uint16_t ruler_color = OSC_GetThemeColor(theme_type, OSC_THEME_RULER_INDEX);

    uint16_t y_frame_width = (is_display_ruler_x) ? 
            (y_width - CHAR_PIXEL_HEIGHT) : y_width;
    uint16_t x_frame_width = (is_display_ruler_y) ? 
            (x_width - ruler_num_digits_y * CHAR_PIXEL_WIDTH) : x_width;

    if(is_display_ruler_y){
        uint16_t display_range = display_num_max - display_num_min;

        for(int i = 0; i < ruler_count_y; i++){
            uint16_t ruler = OSC_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_y, i);
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
            for(int i = 0; i < 2; i++){
                // 有时屏幕显示数字需要画两次
                SCREEN_DRAW_NUM(x_origin + x_frame_width, ruler_y_num_coor, 
                    ruler, ruler_num_digits_y, ruler_color);
            }
        }
        //写数字导致边框可能间断，重新绘制
        uint16_t x_outline_width = x_width;
        uint16_t y_outline_width = y_width;
        SCREEN_DRAW_RECTANGLE(x_origin + x_frame_width, y_origin, 
            x_origin + x_outline_width, y_origin + y_outline_width, frame_color);
    }

    if(is_display_ruler_x){
        uint16_t x_ruler_range = ruler_full_value_x - ruler_zero_value_x;
        for(int i = 0; i < ruler_count_x; i++){
            uint16_t ruler = OSC_PRIVATE_MEMBER_ARRAY(OSCx, ruler_buff_x, i);
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
            for(int i = 0; i < 2; i++){
                // 有时屏幕显示数字需要画两次
                SCREEN_DRAW_NUM(ruler_x_num_coor, y_origin + y_frame_width, 
                    ruler, ruler_actual_digits, ruler_color);
            }
        }
    }
    return OSC_OK;
}

/* @brief : 绘制示波器框架
*  @param : int OSCx : 示波器实例，如OSC_INST(0)或OSC_INST(1)
*  @return : enum OSC_StatusTypeDef 为OSC_OK则无问题，为OSC_ERROR则有问题
*/
OSC_StatusTypeDef OSC_FrameDisplay(int OSCx) {
    if(!IS_VALID_OSC_INST(OSCx)) return OSC_ERROR;
    uint16_t x_origin    = OSC_CONFIG_MEMBER(OSCx, x_origin); 
    uint16_t y_origin    = OSC_CONFIG_MEMBER(OSCx, y_origin); 
    uint16_t x_width     = OSC_CONFIG_MEMBER(OSCx, x_width);
    uint16_t y_width     = OSC_CONFIG_MEMBER(OSCx, y_width);
    volatile bool is_display_ruler_y = OSC_CONFIG_MEMBER(OSCx, is_display_ruler_y);
    volatile bool is_display_ruler_x = OSC_CONFIG_MEMBER(OSCx, is_display_ruler_x);
    uint16_t ruler_num_digits_y = OSC_CONFIG_MEMBER(OSCx, ruler_num_digits_y);
    uint16_t theme_type  = OSC_CONFIG_MEMBER(OSCx, theme_type);
    uint16_t frame_color = OSC_GetThemeColor(theme_type, OSC_THEME_FRAME_INDEX);

    uint16_t y_frame_width = (is_display_ruler_x) ? 
            (y_width - CHAR_PIXEL_HEIGHT) : y_width;
    uint16_t x_frame_width = (is_display_ruler_y) ? 
            (x_width - ruler_num_digits_y * CHAR_PIXEL_WIDTH) : x_width;
    uint16_t x_outline_width = x_width;
    SCREEN_DRAW_RECTANGLE(x_origin, y_origin, 
            x_origin + x_frame_width, y_origin + y_frame_width, frame_color);
    SCREEN_DRAW_RECTANGLE(x_origin, y_origin, 
            x_origin + x_outline_width, y_origin + y_width, frame_color);
    return OSC_OK;
}

/* @brief : 清除曲线
*  @param : int OSCx : 示波器实例，如OSC_INST(0)或OSC_INST(1)
*  @return : enum OSC_StatusTypeDef 为OSC_OK则无问题，为OSC_ERROR则有问题
*/
OSC_StatusTypeDef OSC_CurveClear(int OSCx) {
    if(!IS_VALID_OSC_INST(OSCx)) return OSC_ERROR;
    uint16_t x_origin    = OSC_CONFIG_MEMBER(OSCx, x_origin); 
    uint16_t y_origin    = OSC_CONFIG_MEMBER(OSCx, y_origin); 
    uint16_t x_width     = OSC_CONFIG_MEMBER(OSCx, x_width);
    uint16_t y_width     = OSC_CONFIG_MEMBER(OSCx, y_width);
    volatile bool is_display_ruler_y = OSC_CONFIG_MEMBER(OSCx, is_display_ruler_y);
    uint16_t ruler_num_digits_y = OSC_CONFIG_MEMBER(OSCx, ruler_num_digits_y);
    uint16_t theme_type  = OSC_CONFIG_MEMBER(OSCx, theme_type);
    uint16_t frame_color = OSC_GetThemeColor(theme_type, OSC_THEME_FRAME_INDEX);
    uint16_t bg_color    = OSC_GetThemeColor(theme_type, OSC_THEME_BACKGROUND_INDEX);

    SCREEN_FILL(x_origin, y_origin, x_origin + x_width, y_origin + y_width, bg_color);
    SCREEN_DRAW_RECTANGLE(x_origin, y_origin, 
            x_origin + x_width, y_origin + y_width, frame_color);
    // reset the index of OSC
    OSC_WRITE_PRIVATE(OSCx, last_index, 0);
    OSC_WRITE_PRIVATE(OSCx, x_coor_last, x_origin);
    OSC_WRITE_PRIVATE(OSCx, y_coor_last_CH0, 0);
    OSC_WRITE_PRIVATE(OSCx, y_coor_last_CH1, 0);
    if(is_display_ruler_y) {
        if(OSC_RulerDisplay(OSCx) == OSC_ERROR) return OSC_ERROR;
    }
    return OSC_OK;
}

/* @brief : 绘制示波器框架与（如有）标尺
*  @param : int OSCx : 示波器实例，如OSC_INST(0)或OSC_INST(1)
*           OSC_Config_TypeDef *OSC_Init : 指向OSC设置结构体的指针
*  @return : enum OSC_StatusTypeDef 为OSC_OK则无问题，为OSC_ERROR则有问题
*/
OSC_StatusTypeDef OSC_ReDraw(int OSCx) {
    if(!IS_VALID_OSC_INST(OSCx)) return OSC_ERROR;
    if(OSC_FrameDisplay(OSCx) == OSC_ERROR) return OSC_ERROR;
    if(OSC_CurveClear(OSCx) == OSC_ERROR) return OSC_ERROR;
    return OSC_OK;
}

/* @brief : 向示波器发送一个数据，绘制曲线，（可选）曲线满自动清屏
*  @param : int OSCx : 示波器实例，如OSC_INST(0)或OSC_INST(1)
*           uint16_t data_CH0 : CH0通道的数据
*           uint16_t data_CH1 : CH1通道的数据，若为单通道置零即可
*  @return : enum OSC_StatusTypeDef 为OSC_OK则无问题，为OSC_ERROR则有问题
*            若示波器实例is_auto_clear == false，不会自动清屏，同时屏幕满时返回OSC_FULL
*/
OSC_StatusTypeDef OSC_CurveDraw(int OSCx, uint16_t data_CH0, uint16_t data_CH1) {
    if(!IS_VALID_OSC_INST(OSCx)) return OSC_ERROR;
    uint16_t x_origin                = OSC_CONFIG_MEMBER(OSCx, x_origin); 
    uint16_t y_origin                = OSC_CONFIG_MEMBER(OSCx, y_origin); 
    uint16_t x_width                 = OSC_CONFIG_MEMBER(OSCx, x_width);
    uint16_t y_width                 = OSC_CONFIG_MEMBER(OSCx, y_width);
    volatile bool is_display_ruler_x = OSC_CONFIG_MEMBER(OSCx, is_display_ruler_x);
    volatile bool is_display_ruler_y = OSC_CONFIG_MEMBER(OSCx, is_display_ruler_y);
    uint16_t ruler_num_digits_y      = OSC_CONFIG_MEMBER(OSCx, ruler_num_digits_y);
    uint16_t display_num_min         = OSC_CONFIG_MEMBER(OSCx, display_num_min);
    uint16_t display_num_max         = OSC_CONFIG_MEMBER(OSCx, display_num_max);
    uint16_t channel_num             = OSC_CONFIG_MEMBER(OSCx, channel_num);
    uint16_t theme_type              = OSC_CONFIG_MEMBER(OSCx, theme_type);
    volatile bool is_auto_clear      = OSC_CONFIG_MEMBER(OSCx, is_auto_clear);
    uint16_t wave_CH0_color = OSC_GetThemeColor(theme_type, OSC_THEME_WAVE_CH0_INDEX);
    uint16_t wave_CH1_color = OSC_GetThemeColor(theme_type, OSC_THEME_WAVE_CH1_INDEX);

    uint16_t last_index       = OSC_PRIVATE_MEMBER(OSCx, last_index);
    uint16_t x_coor_last      = OSC_PRIVATE_MEMBER(OSCx, x_coor_last);
    uint16_t y_coor_last_CH0  = OSC_PRIVATE_MEMBER(OSCx, y_coor_last_CH0);
    uint16_t y_coor_last_CH1  = OSC_PRIVATE_MEMBER(OSCx, y_coor_last_CH1);

    uint16_t y_frame_width = (is_display_ruler_x) ? 
            (y_width - CHAR_PIXEL_HEIGHT) : y_width;
    uint16_t x_frame_width = (is_display_ruler_y) ? 
            (x_width - ruler_num_digits_y * CHAR_PIXEL_WIDTH) : x_width;

    if(last_index >= x_frame_width) {
        if (!is_auto_clear) {
            // 重置绘制状态
            OSC_WRITE_PRIVATE(OSCx, last_index, 0);
            OSC_WRITE_PRIVATE(OSCx, x_coor_last, x_origin);
            return OSC_FULL;
        }
        last_index = 0;
        OSC_CurveClear(OSCx);
    }

    uint16_t display_range = display_num_max - display_num_min;

    if(is_out_of_bound(display_num_max, display_num_min, data_CH0)) return OSC_ERROR;
    uint16_t y_displayValue_CH0 = my_limit(display_num_max, display_num_min, data_CH0);
    uint16_t x_coor = x_origin + last_index;
    uint16_t y_coor_CH0 = y_origin + y_frame_width - 
                coor_normal(y_frame_width, display_range, y_displayValue_CH0);
    if(last_index > 0) {
        SCREEN_DRAW_LINE(x_coor, y_coor_CH0, 
            x_coor_last, y_coor_last_CH0, wave_CH0_color);
    }
    OSC_PRIVATE_MEMBER(OSCx, x_coor_last) = x_coor;
    OSC_PRIVATE_MEMBER(OSCx, y_coor_last_CH0) = y_coor_CH0;

    //CH1 通道
    if(channel_num == 2) {
        if(is_out_of_bound(display_num_max, display_num_min, data_CH1)) return OSC_ERROR;
        uint16_t y_displayValue_CH1 = my_limit(display_num_max, display_num_min, data_CH1);
        uint16_t y_coor_CH1 = y_origin + y_frame_width - 
                coor_normal(y_frame_width, display_range, y_displayValue_CH1);
        if(last_index > 0) {
            SCREEN_DRAW_LINE(x_coor, y_coor_CH1, 
                x_coor_last, y_coor_last_CH1, wave_CH1_color);
        }
        OSC_PRIVATE_MEMBER(OSCx, y_coor_last_CH1) = y_coor_CH1;
    }
    last_index++;
    OSC_PRIVATE_MEMBER(OSCx, last_index)  = last_index;

    return OSC_OK;
}
