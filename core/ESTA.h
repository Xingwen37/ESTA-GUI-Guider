#ifndef __ESTA_LIB
#define __ESTA_LIB

/* INCLUDE */
#include <stddef.h>  
#include <stdint.h>  
#include <stdbool.h>
#include "helper.h"

// 支持硬件抽象层
// 取消注释以使用对应的底层驱动
// #define SCREEN_USE_ILI9341
#define SCREEN_USE_SDL2 

#ifdef SCREEN_USE_ILI9341
#include "main.h"
#include "ili9341_driver.h"

// (x1, y1)起始点 (x2, y2)终止点
#define SCREEN_DRAW_LINE(x1, y1, x2, y2, COLOR) \
            ILI9341_draw_line(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_RECTANGLE(x1, y1, x2, y2, COLOR) \
            ILI9341_draw_rectangle(x1, y1, x2, y2, COLOR)
#define SCREEN_FILL(x1, y1, x2, y2, COLOR) \
            ILI9341_fill(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_NUM(x, y, num, len, COLOR) \
            ILI9341_draw_num(x, y, num, len, COLOR)

#elif defined(SCREEN_USE_SDL2)
// 引入 SDL2 移植层的头文件
#include "esta_port_sdl2.h"

#define SCREEN_DRAW_LINE(x1, y1, x2, y2, COLOR) \
            ESTA_SDL2_DrawLine(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_RECTANGLE(x1, y1, x2, y2, COLOR) \
            ESTA_SDL2_DrawRectangle(x1, y1, x2, y2, COLOR)
#define SCREEN_FILL(x1, y1, x2, y2, COLOR) \
            ESTA_SDL2_Fill(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_NUM(x, y, num, len, COLOR) \
            ESTA_SDL2_DrawNum(x, y, num, len, COLOR)

#else /* 你的自定义屏幕或其他 */
// 在这里包含你自己的屏幕库
// 并在宏后写你自己的屏幕函数
#define SCREEN_DRAW_LINE(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_RECTANGLE(x1, y1, x2, y2, COLOR)
#define SCREEN_FILL(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_NUM(x, y, num, len, COLOR)
#endif


/* GLOBAL MARCO */
// 最大示波器实例个数
#define MAX_ESTA_NUM         4

// 单个示波器实例通道数
//TODO: 实现多通道的示波器 ,现在支持至四通道,可拓展至八通道
#define MAX_ESTA_CHANNEL     4

/* 通道掩码常量（独热码） */
#define CH0  (1U << 0)  /* 0b00000001 */
#define CH1  (1U << 1)  /* 0b00000010 */
#define CH2  (1U << 2)  /* 0b00000100 */
#define CH3  (1U << 3)  /* 0b00001000 */
#define CH4  (1U << 4)  /* 0b00010000 */
#define CH5  (1U << 5)  /* 0b00100000 */
#define CH6  (1U << 6)  /* 0b01000000 */
#define CH7  (1U << 7)  /* 0b10000000 */



// 最大标尺个数，标尺过多且宽度不足可能导致标尺重叠
#define ESTA_MAX_RULER_Y_NUM   5
#define ESTA_MAX_RULER_X_NUM   5

// 一个字符的宽度和高度
#define CHAR_PIXEL_WIDTH    8
#define CHAR_PIXEL_HEIGHT   16


/* USER MARCO OR ENUM */
/* INST MARCO */
#define ESTA_INST(i)                   (i)
#define ESTA_INST_ADDR(i)              ESTA_State[ESTA_INST(i)]


/* THEME SETTINGS */
typedef enum {
    ESTA_THEME_DEFAULT = 0,
    ESTA_THEME_LIGHT ,
    //用于检查边界条件，并非主题类型！所有添加的类型都放在ESTA_THEME_COUNT 这个枚举变量上面
    ESTA_THEME_COUNT 
} ESTA_theme_type;

typedef enum {
    ESTA_THEME_FRAME_INDEX = 0  ,
    ESTA_THEME_RULER_INDEX      ,
    ESTA_THEME_WAVE_CH0_INDEX   ,
    ESTA_THEME_WAVE_CH1_INDEX   ,
    ESTA_THEME_WAVE_CH2_INDEX   ,
    ESTA_THEME_WAVE_CH3_INDEX   ,
    ESTA_THEME_BACKGROUND_INDEX ,
    //用于检查边界条件，并非主题类型！所有添加的类型都放在ESTA_THEME_INDEX_COUNT 这个枚举变量上面
    ESTA_THEME_INDEX_COUNT 
} ESTA_theme_color_index_type;

/* COLOR MARCO */
/* we use RGB565 */
#define RGB888_To_RGB565(R,G,B)  (uint16_t)((R & 0x1f)<<11|(G & 0x3f)<<5|(B & 0x1f)) 
#define __WHITE         	 0xFFFF
#define __BLACK         	 0x0000	  
#define __BLUE         	     0x001F  
#define __DEEP_BLUE          0x101F
#define __BRED               0XF81F//粉紫
#define __GRED 			     0XFFE0//黄色
#define __GBLUE			     0x059F//浅蓝
#define __RED           	 0xF800
#define __GREEN         	 0x07E0
#define __YELLOW        	 0xFFE0
#define __BROWN 			 0XBC40 //棕色
#define __BRRED 			 0XFC07 //棕红色
#define __GRAY  			 0X8430 //灰色
#define __ORANGE             0XFD20 //橙色
#define __PURPLE             0X8010 


/* WRITE/READ REGS MARCO */
#define ESTA_CONFIG_MEMBER(OSCx, reg_name)  \
            ESTA_INST_ADDR(OSCx).ESTA_Config.reg_name
#define ESTA_PRIVATE_MEMBER(OSCx, reg_name)  \
            ESTA_INST_ADDR(OSCx).ESTA_Private.reg_name

#define ESTA_CONFIG_MEMBER_ARRAY(OSCx, reg_name, NO)  \
            ESTA_INST_ADDR(OSCx).ESTA_Config.reg_name[NO]
#define ESTA_PRIVATE_MEMBER_ARRAY(OSCx, reg_name, NO)  \
            ESTA_INST_ADDR(OSCx).ESTA_Private.reg_name[NO]

#define ESTA_WRITE_CONFIG(OSCx, reg_name, reg_value)   \
            ESTA_INST_ADDR(OSCx).ESTA_Config.reg_name = reg_value
#define ESTA_WRITE_PRIVATE(OSCx, reg_name, reg_value)   \
            ESTA_INST_ADDR(OSCx).ESTA_Private.reg_name = reg_value

#define ESTA_WRITE_CONFIG_INIT(OSCx, reg_name)         \
            ESTA_INST_ADDR(OSCx).ESTA_Config.reg_name = ESTA_Init->reg_name


/* ASSERT MARCO OR ENUM*/
/* Defensive Programming */
typedef enum
{
  ESTA_OK       = 0x00,
  ESTA_ERROR    = 0x01,
  ESTA_FULL     = 0x02
} ESTA_StatusTypeDef;

#define IS_VALID_ESTA_INST(OSCx) ((int)OSCx < MAX_ESTA_NUM && (int)OSCx >= 0)
#define IS_VALID_THEME(THEMEx)  ((int)THEMEx < ESTA_THEME_COUNT && (int)THEMEx >= 0)
#define IS_VALID_CHNUM(CHNUM)   ((int)CHNUM < MAX_ESTA_CHANNEL && (int)CHNUM >= 0)


/* Software REGS typedef of ESTA */
typedef struct {
    /* config regs */
    /* 示波器的坐标原点(x_origin, y_origin) */
    uint16_t       x_origin;
    uint16_t       y_origin;
    /* 示波器的x,y宽度 x_width, y_width */
    uint16_t       x_width;
    uint16_t       y_width;
    /* 示波器显示数字的最大值和最小值 */
    uint16_t       display_num_min;
    uint16_t       display_num_max;
    /* 示波器通道数量（至多两个） */
    uint16_t       channel_num;
    /* 示波器通道掩码 */
    uint8_t        channel_mask;
    /* 标尺设置 */
    // TODO: 标尺显示负数,小数的情况
    // TODO: 非线性标尺（如dB）
    volatile bool  is_display_ruler_x; // 是否显示x轴标尺
    volatile bool  is_display_ruler_y; // 是否显示y轴标尺
    /* ruler 这个地址只在初始化时用于传参，用于将标尺数据保存在Private中, 其余时间为NULL */
    uint16_t       *ruler_y;           // y轴标尺传入的地址
    uint16_t       ruler_count_y;      // y轴标尺数量，最多ESTA_MAX_RULER_NUM
    uint16_t       ruler_num_digits_y; // y轴标尺显示数字的最大位数
    /* ruler 这个地址只在初始化时用于传参，用于将标尺数据保存在Private中, 其余时间为NULL */
    uint16_t       *ruler_x;           // x轴标尺传入的地址
    uint16_t       ruler_count_x;      // x轴标尺数量，最多ESTA_MAX_RULER_NUM
    uint16_t       ruler_zero_value_x; // x轴标尺坐标为0的值
    uint16_t       ruler_full_value_x; // x轴标尺坐标最大的值
    uint16_t       ruler_num_digits_x; // x轴标尺显示数字的最大位数
    /* 屏幕满是否自动刷新屏幕 */
    volatile bool  is_auto_clear;
    /* 示波器主题 */
    ESTA_theme_type theme_type;
} ESTA_Config_TypeDef;

typedef struct {
    /* Private regs */
    /* 标尺的数据存储在这里 */
    uint16_t    ruler_buff_y[ESTA_MAX_RULER_Y_NUM]; 
    uint16_t    ruler_buff_x[ESTA_MAX_RULER_X_NUM]; 
    uint16_t    last_index;
    uint16_t    x_coor_last;
    uint16_t    y_coor_last_CH[MAX_ESTA_CHANNEL];
} ESTA_Private_Typedef;

/* be like Class in C++ */
typedef struct {
    /* Public regs */
    ESTA_Config_TypeDef     ESTA_Config;
    /* Private regs */
    ESTA_Private_Typedef    ESTA_Private;
} ESTA_TypeDef;


/* function prototype */
void ESTA_ConfigSetPositionAndSize(ESTA_Config_TypeDef *config,
                                  uint16_t x_origin, uint16_t y_origin,
                                  uint16_t x_width, uint16_t y_width);
void ESTA_ConfigSetDisplayRange(ESTA_Config_TypeDef *config,
                               uint16_t display_num_min, uint16_t display_num_max);
void ESTA_ConfigSetChannelNum(ESTA_Config_TypeDef *config, uint16_t channel_num);
void ESTA_ConfigSetChannelEnabled(ESTA_Config_TypeDef *config, uint8_t channel_mask);
void ESTA_ConfigSetChannelDisabled(ESTA_Config_TypeDef *config, uint8_t channel_mask);
void ESTA_ConfigSetRulerY(ESTA_Config_TypeDef *config, bool is_display,
                         uint16_t *ruler_y, uint16_t ruler_count_y,
                         uint16_t ruler_num_digits_y);
void ESTA_ConfigSetRulerX(ESTA_Config_TypeDef *config, bool is_display,
                         uint16_t *ruler_x, uint16_t ruler_count_x,
                         uint16_t ruler_zero_value_x, uint16_t ruler_full_value_x,
                         uint16_t ruler_num_digits_x);
void ESTA_ConfigSetTheme(ESTA_Config_TypeDef *config, ESTA_theme_type theme_type);
void ESTA_ConfigSetAutoClear(ESTA_Config_TypeDef *config, bool is_auto_clear);

ESTA_StatusTypeDef ESTA_Init(int OSCx, ESTA_Config_TypeDef *ESTA_Init);
ESTA_StatusTypeDef ESTA_DeInit(int OSCx);
ESTA_StatusTypeDef ESTA_RulerDisplay(int OSCx);
ESTA_StatusTypeDef ESTA_FrameDisplay(int OSCx);
ESTA_StatusTypeDef ESTA_CurveClear(int OSCx);
ESTA_StatusTypeDef ESTA_ReDraw(int OSCx);
ESTA_StatusTypeDef ESTA_CurveDraw(int OSCx, uint16_t data_CH[]); 
uint16_t ESTA_GetThemeColor(ESTA_theme_type theme, ESTA_theme_color_index_type color_type);

#endif