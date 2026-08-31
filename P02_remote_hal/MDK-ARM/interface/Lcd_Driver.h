#ifndef _LCD_DRIVER_H
#define _LCD_DRIVER_H 
//#include "sys.h"
//#include "stdlib.h"	
#include <stdint.h>
#include "main.h"

#define u8 uint8_t

#define u16 uint16_t

#define u32 uint32_t  


//----------------OLED端口定义----------------- 
/***************根据自己需求更改****************/
#define OLED_SCL_PROT OLED_SCL_GPIO_Port
#define OLED_SCL_PIN  OLED_SCL_Pin
#define OLED_SCL_GPIO_CLK   RCC_APB2Periph_GPIOB
#define OLED_SDA_PROT OLED_SDA_GPIO_Port
#define OLED_SDA_PIN  OLED_SDA_Pin
#define OLED_SDA_GPIO_CLK   RCC_APB2Periph_GPIOB
/*********************END**********************/

#define OLED_SCL_Clr() HAL_GPIO_WritePin(OLED_SCL_PROT,OLED_SCL_PIN,GPIO_PIN_RESET)//SCL
#define OLED_SCL_Set() HAL_GPIO_WritePin(OLED_SCL_PROT,OLED_SCL_PIN,GPIO_PIN_SET)

#define OLED_SDA_Clr() HAL_GPIO_WritePin(OLED_SDA_PROT,OLED_SDA_PIN,GPIO_PIN_RESET)//DIN
#define OLED_SDA_Set() HAL_GPIO_WritePin(OLED_SDA_PROT,OLED_SDA_PIN,GPIO_PIN_SET)
#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

void OLED_ClearPoint(u8 x,u8 y);
void OLED_ColorTurn(u8 i);
void OLED_DisplayTurn(u8 i);
void OLED_I2C_Start(void);
void OLED_I2C_Stop(void);
void OLED_I2C_WaitAck(void);
void OLED_Send_Byte(u8 dat);
void OLED_WR_Byte(u8 dat,u8 mode);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_Refresh(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_DrawLine(u8 x1,u8 y1,u8 x2,u8 y2,u8 mode);
void OLED_DrawCircle(u8 x,u8 y,u8 r);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1,u8 mode);
void OLED_ShowChar6x8(u8 x,u8 y,u8 chr,u8 mode);
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 size1,u8 mode);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size1,u8 mode);
void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size1,u8 mode);
void OLED_ScrollDisplay(u8 num,u8 space,u8 mode);
void OLED_ShowPicture(u8 x,u8 y,u8 sizex,u8 sizey,u8 BMP[],u8 mode);
void OLED_Init(void);
void Display_Init(void);

#endif

