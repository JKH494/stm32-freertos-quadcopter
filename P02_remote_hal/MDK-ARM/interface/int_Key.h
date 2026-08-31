#ifndef INT_KEY_H
#define INT_KEY_H

#include "main.h"
#include "freeRTOS.h"
#include "task.h"

typedef enum
{
    KEY_NONE = 0,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_LEFT_X,
    KEY_RIGHT_X,
    KEY_RIGHT_X_LONG
} Key_type;

/**
 * @brief 获取当前是否有按键按下
 * 
 * @return 返回按键名字或KEY_NONE
 */
Key_type Key_Get(void);

#endif // INT_KEY_H
