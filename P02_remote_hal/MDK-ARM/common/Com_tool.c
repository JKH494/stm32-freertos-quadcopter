#include "Com_tool.h"

/**
 * @brief 限幅函数，返回int16_t类型
 * 
 */
int16_t Com_limit(int16_t value, int16_t min, int16_t max)
{
    if(value > max)
    {
        return max;
    }
    else if(value < min)
    {
        return min;
    }
    else
    {
        return value;
    }
}
