#include "main.h"
#include "i2c.h"
#include "oled.h"       // 建议你自己建一个 oled.h，把函数声明放进去
#include "OLED_Font.h"  // 字库头文件

// 外部声明 I2C 句柄 (由 CubeMX 生成)
extern I2C_HandleTypeDef hi2c1;
extern const unsigned char OLED_F8x16[][16];

// 定义 OLED 的 I2C 地址 (大多数是 0x78，如果不亮试试 0x7A)
#define OLED_I2C_ADDR 0x78

/**
  * @brief  OLED写命令
  */
void OLED_WriteCommand(uint8_t Command)
{
    uint8_t buf[2];
    buf[0] = 0x00; // 0x00代表写命令
    buf[1] = Command;
    // 注意：把 HAL_MAX_DELAY 改成 10，万一通信失败，等10ms就退出，防止程序卡死！
    HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDR, buf, 2, 10); 
}

/**
  * @brief  OLED写数据
  */
void OLED_WriteData(uint8_t Data)
{
    uint8_t buf[2];
    buf[0] = 0x40; // 0x40代表写数据
    buf[1] = Data;
    // 注意：把 HAL_MAX_DELAY 改成 10，防止死机
    HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDR, buf, 2, 10);
}

/**
  * @brief  OLED设置光标位置
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    OLED_WriteCommand(0xB0 | Y);                    // 设置Y位置
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));    // 设置X位置高4位
    OLED_WriteCommand(0x00 | (X & 0x0F));           // 设置X位置低4位
}

/**
  * @brief  OLED清屏
  */
void OLED_Clear(void)
{  
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        OLED_SetCursor(j, 0);
        for(i = 0; i < 128; i++)
        {
            OLED_WriteData(0x00);
        }
    }
}

/**
  * @brief  OLED显示一个字符 (Line: 1~4, Column: 1~16)
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
    uint8_t i;
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);       // 上半部分
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);
    }
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);   // 下半部分
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);
    }
}

/**
  * @brief  OLED显示字符串
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(Line, Column + i, String[i]);
    }
}

/**
  * @brief  OLED次方函数
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

/**
  * @brief  OLED显示数字（十进制，正数）
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)							
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/**
  * @brief  OLED初始化
  */
void OLED_Init(void)
{
    HAL_Delay(200); // 留足上电时间
    
    OLED_WriteCommand(0xAE); // 关闭显示
    
    OLED_WriteCommand(0xD5); // 设置显示时钟分频比/振荡器频率
    OLED_WriteCommand(0x80);
    
    OLED_WriteCommand(0xA8); // 设置多路复用率
    OLED_WriteCommand(0x3F);
    
    OLED_WriteCommand(0xD3); // 设置显示偏移
    OLED_WriteCommand(0x00);
    
    OLED_WriteCommand(0x40); // 设置显示开始行
    
    OLED_WriteCommand(0xA1); // 设置左右方向，0xA1正常 0xA0左右反置
    OLED_WriteCommand(0xC8); // 设置上下方向，0xC8正常 0xC0上下反置

    OLED_WriteCommand(0xDA); // 设置COM引脚硬件配置
    OLED_WriteCommand(0x12);
    
    OLED_WriteCommand(0x81); // 设置对比度控制
    OLED_WriteCommand(0xCF);

    OLED_WriteCommand(0xD9); // 设置预充电周期
    OLED_WriteCommand(0xF1);

    OLED_WriteCommand(0xDB); // 设置VCOMH取消选择级别
    OLED_WriteCommand(0x30);

    OLED_WriteCommand(0xA4); // 设置整个显示打开/关闭
    OLED_WriteCommand(0xA6); // 设置正常/倒转显示
    OLED_WriteCommand(0x8D); // 设置充电泵
    OLED_WriteCommand(0x14);

    OLED_WriteCommand(0xAF); // 开启显示
    OLED_Clear();            // OLED清屏
}
