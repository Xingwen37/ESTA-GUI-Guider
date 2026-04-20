#ifndef __OSC_LIB
#define __OSC_LIB


/* INCLUDE */

// 支持硬件抽象层
// 如果你需要移植自己的屏幕，就将这段宏注释掉
// 然后在下面添加自己的屏幕函数
#define SCREEN_USE_ILI9341

#ifdef   SCREEN_USE_ILI9341
#include "main.h"
#include "ili9341_driver.h"


// (x1, y1)起始点 (x2, y2)终止点
#define SCREEN_DRAW_LINE(x1, y1, x2, y2, COLOR) \
            ILI9341_draw_line(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_RECTANGLE(x1, y1, x2, y2, COLOR) \
            ILI9341_draw_rectangle(x1, y1, x2, y2, COLOR)
#define SCREEN_FILL(x1, y1, x2, y2, COLOR) \
            ILI9341_fill(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_NUM(x, y, num, len,COLOR) \
            ILI9341_draw_num(x, y, num, len,COLOR)

#else /* SCREEN_USE_ILI9341 */
// 在这里包含你自己的屏幕库
// 并在宏后写你自己的屏幕函数
#define SCREEN_DRAW_LINE(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_RECTANGLE(x1, y1, x2, y2, COLOR)
#define SCREEN_FILL(x1, y1, x2, y2, COLOR)
#define SCREEN_DRAW_NUM(x, y, num, len,COLOR)
#endif /* SCREEN_USE_ILI9341 */


/* GLOBAL MARCO */
// 最大示波器实例个数
#define MAX_OSC_NUM         2

// 单个示波器实例通道数
//TODO: 实现多通道的示波器 ,现在只支持至多双通道
#define MAX_OSC_CHANNEL     2

// 最大标尺个数，标尺过多且宽度不足可能导致标尺重叠
#define OSC_MAX_RULER_Y_NUM   5
#define OSC_MAX_RULER_X_NUM   5

// 一个字符的宽度和高度
#define CHAR_PIXEL_WIDTH    8
#define CHAR_PIXEL_HEIGHT   16


/* USER MARCO OR ENUM */
/* INST MARCO */
#define OSC_INST(i)                   (i)
#define OSC_INST_ADDR(i)              OSC_State[OSC_INST(i)]


/* THEME SETTINGS */
typedef enum {
    OSC_THEME_DEFAULT = 0,
    OSC_THEME_LIGHT ,
    //用于检查边界条件，并非主题类型！所有添加的类型都放在OSC_THEME_COUNT 这个枚举变量上面
    OSC_THEME_COUNT 
} OSC_theme_type;

typedef enum {
    OSC_THEME_FRAME_INDEX = 0  ,
    OSC_THEME_RULER_INDEX      ,
    OSC_THEME_WAVE_CH0_INDEX   ,
    OSC_THEME_WAVE_CH1_INDEX   ,
    OSC_THEME_BACKGROUND_INDEX ,
    //用于检查边界条件，并非主题类型！所有添加的类型都放在OSC_THEME_INDEX_COUNT 这个枚举变量上面
    OSC_THEME_INDEX_COUNT 
} OSC_theme_color_index_type;

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
#define OSC_CONFIG_MEMBER(OSCx, reg_name)  \
            OSC_INST_ADDR(OSCx).OSC_Config.reg_name
#define OSC_PRIVATE_MEMBER(OSCx, reg_name)  \
            OSC_INST_ADDR(OSCx).OSC_Private.reg_name

#define OSC_CONFIG_MEMBER_ARRAY(OSCx, reg_name, NO)  \
            OSC_INST_ADDR(OSCx).OSC_Config.reg_name[NO]
#define OSC_PRIVATE_MEMBER_ARRAY(OSCx, reg_name, NO)  \
            OSC_INST_ADDR(OSCx).OSC_Private.reg_name[NO]

#define OSC_WRITE_CONFIG(OSCx, reg_name, reg_value)   \
            OSC_INST_ADDR(OSCx).OSC_Config.reg_name = reg_value
#define OSC_WRITE_PRIVATE(OSCx, reg_name, reg_value)   \
            OSC_INST_ADDR(OSCx).OSC_Private.reg_name = reg_value

#define OSC_WRITE_CONFIG_INIT(OSCx, reg_name)         \
            OSC_INST_ADDR(OSCx).OSC_Config.reg_name = OSC_Init->reg_name


/* ASSERT MARCO OR ENUM*/
/* Defensive Programming */
typedef enum
{
  OSC_OK       = 0x00,
  OSC_ERROR    = 0x01,
  OSC_FULL     = 0x02
} OSC_StatusTypeDef;

#define IS_VALID_OSC_INST(OSCx) ((int)OSCx < MAX_OSC_NUM && (int)OSCx >= 0)
#define IS_VALID_THEME(THEMEx)  ((int)THEMEx < OSC_THEME_COUNT && (int)THEMEx >= 0)
#define IS_VALID_CHNUM(CHNUM)   ((int)CHNUM < MAX_OSC_CHANNEL && (int)CHNUM >= 0)


/* Software REGS typedef of OSC */
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
    /* 标尺设置 */
    // TODO: 标尺显示负数,小数的情况
    // TODO: 非线性标尺（如dB）
    volatile bool  is_display_ruler_x; // 是否显示x轴标尺
    volatile bool  is_display_ruler_y; // 是否显示y轴标尺
    /* ruler 这个地址只在初始化时用于传参，用于将标尺数据保存在Private中, 其余时间为NULL */
    uint16_t       *ruler_y;           // y轴标尺传入的地址
    uint16_t       ruler_count_y;      // y轴标尺数量，最多OSC_MAX_RULER_NUM
    uint16_t       ruler_num_digits_y; // y轴标尺显示数字的最大位数
    /* ruler 这个地址只在初始化时用于传参，用于将标尺数据保存在Private中, 其余时间为NULL */
    uint16_t       *ruler_x;           // x轴标尺传入的地址
    uint16_t       ruler_count_x;      // x轴标尺数量，最多OSC_MAX_RULER_NUM
    uint16_t       ruler_zero_value_x; // x轴标尺坐标为0的值
    uint16_t       ruler_full_value_x; // x轴标尺坐标最大的值
    uint16_t       ruler_num_digits_x; // x轴标尺显示数字的最大位数
    /* 屏幕满是否自动刷新屏幕 */
    volatile bool  is_auto_clear;
    /* 示波器主题 */
    OSC_theme_type theme_type;
} OSC_Config_TypeDef;

typedef struct {
    /* Private regs */
    /* 标尺的数据存储在这里 */
    uint16_t    ruler_buff_y[OSC_MAX_RULER_Y_NUM]; 
    uint16_t    ruler_buff_x[OSC_MAX_RULER_X_NUM]; 
    uint16_t    last_index;
    uint16_t    x_coor_last;
    uint16_t    y_coor_last_CH0;
    uint16_t    y_coor_last_CH1;
} OSC_Private_Typedef;

/* be like Class in C++ */
typedef struct {
    /* Public regs */
    OSC_Config_TypeDef     OSC_Config;
    /* Private regs */
    OSC_Private_Typedef    OSC_Private;
} OSC_TypeDef;


/* function prototype */
OSC_StatusTypeDef OSC_Init(int OSCx, OSC_Config_TypeDef *OSC_Init);
OSC_StatusTypeDef OSC_DeInit(int OSCx);
OSC_StatusTypeDef OSC_RulerDisplay(int OSCx);
OSC_StatusTypeDef OSC_FrameDisplay(int OSCx);
OSC_StatusTypeDef OSC_CurveClear(int OSCx);
OSC_StatusTypeDef OSC_ReDraw(int OSCx);
OSC_StatusTypeDef OSC_CurveDraw(int OSCx, uint16_t data_CH0, uint16_t data_CH1);
uint16_t OSC_GetThemeColor(OSC_theme_type theme, OSC_theme_color_index_type color_type);

#endif