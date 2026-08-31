#include "App_flight.h"

Gyro_Accel_Struct gyro_accel_data = {0};
Euler_Struct euler_angle = {0};
Gyro_Struct gyro_last = {0};

extern Remote_data remote_data;
//表示当前飞行模式
extern Flight_State flight_state;
//BMP280初始化返回值
bool bmp280_ready = false;
//定高时的高度
extern float BMP_asl;
//后续高度
//记录定高目标值和油门
float pre2;
float temp2;
float BMP_asl2;
extern uint16_t BMP_thr;

//电机结构体
motor_struct left_top_motor = {.tim = &htim3, .channel = TIM_CHANNEL_1, .speed = 00};
motor_struct left_bottom_motor = {.tim = &htim4, .channel = TIM_CHANNEL_4, .speed = 00};
motor_struct right_top_motor = {.tim = &htim2, .channel = TIM_CHANNEL_2, .speed = 00};
motor_struct right_bottom_motor = {.tim = &htim1, .channel = TIM_CHANNEL_3, .speed = 00};

//俯仰角pid
PID_Struct pid_pitch = {.Kp = 5.00, .Ki = 0.00, .Kd = 0.00};   //外环
PID_Struct pid_gyro_y = {.Kp = 4.00, .Ki = 0.00, .Kd = 0.50};  //内环
//横滚角pid
PID_Struct pid_roll = {.Kp = 5.00, .Ki = 0.00, .Kd = 0.00};   //外环
PID_Struct pid_gyro_x = {.Kp = 4.00, .Ki = 0.00, .Kd = 0.50};  //内环
//偏航角pid
PID_Struct pid_yaw = {.Kp = 3.00, .Ki = 0.00, .Kd = 0.00};   //外环
PID_Struct pid_gyro_z = {.Kp = 5.00, .Ki = 0.00, .Kd = 0.00};  //内环

//定高pid
PID_Struct pid_height = {.Kp = 2.00, .Ki = 0.00, .Kd = 0.00};


/**
 * @brief 初始化MPU6050,电机，BMP280
 * 
 */
void App_flight_init(void)
{
    //初始化BMP280
    bmp280_ready = BMP280Init();
    //初始化MPU6050
    Int_MPU6050_init();
    //初始化电机
    int_motor_start(&left_top_motor);
    int_motor_start(&left_bottom_motor);
    int_motor_start(&right_top_motor);
    int_motor_start(&right_bottom_motor);
}


/**
 * @brief 根据陀螺仪数据计算欧拉角
 */
void App_flight_get_euler_angle(void)
{
    //使用MPU6050获取陀螺仪数据
    Int_MPU6050_get_Data(&gyro_accel_data);
    
    //使用低通滤波
    gyro_accel_data.gyro.Gyro_X = Common_Filter_LowPass(gyro_accel_data.gyro.Gyro_X,gyro_last.Gyro_X);
    gyro_accel_data.gyro.Gyro_Y = Common_Filter_LowPass(gyro_accel_data.gyro.Gyro_Y,gyro_last.Gyro_Y);
    gyro_accel_data.gyro.Gyro_Z = Common_Filter_LowPass(gyro_accel_data.gyro.Gyro_Z,gyro_last.Gyro_Z);
    
    //更新最后的陀螺仪数据
    gyro_last = gyro_accel_data.gyro;

    //打印查看陀螺仪数据
    // debug_printf(":%d,%d,%d\n",gyro_accel_data.gyro.Gyro_X,gyro_accel_data.gyro.Gyro_Y,
    //     gyro_accel_data.gyro.Gyro_Z);

    //使用卡尔曼滤波
    gyro_accel_data.accel.Accel_X = Common_Filter_KalmanFilter(&kfs[0],gyro_accel_data.accel.Accel_X);
    gyro_accel_data.accel.Accel_Y = Common_Filter_KalmanFilter(&kfs[1],gyro_accel_data.accel.Accel_Y);
    gyro_accel_data.accel.Accel_Z = Common_Filter_KalmanFilter(&kfs[2],gyro_accel_data.accel.Accel_Z);

    //打印查看加速度数据
    // debug_printf(":%d,%d,%d\n",gyro_accel_data.accel.Accel_X,gyro_accel_data.accel.Accel_Y,
    //     gyro_accel_data.accel.Accel_Z);

    // //姿态解算 加速度：横滚角，俯仰角  角速度：偏航角
    // euler_angle.Pitch = atan2(gyro_accel_data.accel.Accel_X * 1.0,gyro_accel_data.accel.Accel_Z) * 180 / 3.1415;
    // euler_angle.Roll = atan2(gyro_accel_data.accel.Accel_Y * 1.0,gyro_accel_data.accel.Accel_Z) * 180 / 3.1415;
    // // 2000:陀螺仪满量程 32768:16位ADC满量程 0.006:测量周期6ms
    // euler_angle.Yaw += (gyro_accel_data.gyro.Gyro_Z * 2000.0 / 32768.0) * 0.006;

    //使用四元解算
    Common_IMU_GetEulerAngle(&gyro_accel_data,&euler_angle,0.006);

    //打印查看欧拉角数据
    //debug_printf(":%d,%d,%d\n",(int16_t)euler_angle.Pitch,(int16_t)euler_angle.Roll,(int16_t)euler_angle.Yaw);
}


/**
 * @brief 根据欧拉角pid控制
 * 
 */
void App_flight_euler_pid(void)
{
    //俯仰角
     //外环目标值 = 遥控器传输值
    pid_pitch.target = ( remote_data.pit - 500 ) / 50.0 ;
     //外环测量值 = 欧拉角俯仰角
    pid_pitch.measure = euler_angle.Pitch;
     //内环测量值
    pid_gyro_y.measure = gyro_accel_data.gyro.Gyro_Y * 2000.0 / 32768.0;

    //进行pid计算
    Com_PID_Calc_Chain(&pid_pitch,&pid_gyro_y);

    //打印查看pid计算结果
    // debug_printf(":%.2f,%.2f,%.2f\n",pid_pitch.error,pid_pitch.output,pid_gyro_y.output);

    //横滚角
     //外环目标值 = 遥控器传输值
    pid_roll.target = ( remote_data.rol - 500 ) / 50.0 ;
     //外环测量值 = 欧拉角横滚角
    pid_roll.measure = euler_angle.Roll;
     //内环测量值
    pid_gyro_x.measure = gyro_accel_data.gyro.Gyro_X * 2000.0 / 32768.0;

    //进行pid计算
    Com_PID_Calc_Chain(&pid_roll,&pid_gyro_x);

    //偏航角
    //外环目标值 = 遥控器传输值
    pid_yaw.target = ( remote_data.yaw - 500 ) / 50.0 ;
    //外环测量值 = 欧拉角偏航角
    pid_yaw.measure = euler_angle.Yaw;
    //内环测量值
    pid_gyro_z.measure = gyro_accel_data.gyro.Gyro_Z * 2000.0 / 32768.0;

    //进行pid计算
    Com_PID_Calc_Chain(&pid_yaw,&pid_gyro_z);

}

/**
 * @brief 根据bmp280数据进行pid计算
 * 
 */
void App_flight_bmp280_pid(void)
{
    if(bmp280_ready)
    {
        if (BMP280GetData(&pre2, &temp2, &BMP_asl2))
        {
            pid_height.target = BMP_asl;
            pid_height.measure = BMP_asl2;
            Com_PID_Calc_BMP280(&pid_height);
        }
    }
}

/**
 * @brief 根据PID控制量控制电机
 * 
 */
void App_flight_control_motor(void)
{
    //switch判断飞行状态
    switch (flight_state)
    {
        case IDLE :
            //设置电机速度为0
            left_top_motor.speed = 0;
            left_bottom_motor.speed = 0;
            right_top_motor.speed = 0;
            right_bottom_motor.speed = 0;
            break;
        case NORMAL :
            //俯仰角，向前飞，pid_gyro_y.output输出为负    需要的负反馈：向后飞 -> 前两个电机速度快，后两个电机速度慢 
            left_top_motor.speed = remote_data.thr - pid_gyro_y.output + pid_gyro_x.output + 60 + Com_limit_output(pid_gyro_z.output, 100, -100);
            left_bottom_motor.speed = remote_data.thr + pid_gyro_y.output + pid_gyro_x.output + 60 - Com_limit_output(pid_gyro_z.output, 100, -100);
            right_top_motor.speed = remote_data.thr - pid_gyro_y.output - pid_gyro_x.output - 50 - Com_limit_output(pid_gyro_z.output, 100, -100);
            right_bottom_motor.speed = remote_data.thr + pid_gyro_y.output - pid_gyro_x.output - 50 + Com_limit_output(pid_gyro_z.output, 100, -100);
            break;
        case FIX_HEIGHT :
            left_top_motor.speed = BMP_thr - pid_gyro_y.output + pid_gyro_x.output + 60 + Com_limit_output(pid_gyro_z.output, 100, -100) + Com_limit_output(pid_height.output, 100, -100);
            left_bottom_motor.speed = BMP_thr + pid_gyro_y.output + pid_gyro_x.output + 60 - Com_limit_output(pid_gyro_z.output, 100, -100) + Com_limit_output(pid_height.output, 100, -100);
            right_top_motor.speed = BMP_thr - pid_gyro_y.output - pid_gyro_x.output - 50 - Com_limit_output(pid_gyro_z.output, 100, -100) + Com_limit_output(pid_height.output, 100, -100);
            right_bottom_motor.speed = BMP_thr + pid_gyro_y.output - pid_gyro_x.output - 50 + Com_limit_output(pid_gyro_z.output, 100, -100) + Com_limit_output(pid_height.output, 100, -100);
            break;
        case FAIL :
            break;
        default:
            break;
    }

    //限幅
    left_top_motor.speed = Com_limit_output(left_top_motor.speed, 910, 0);
    left_bottom_motor.speed = Com_limit_output(left_bottom_motor.speed, 910, 0);
    right_top_motor.speed = Com_limit_output(right_top_motor.speed, 800, 0);
    right_bottom_motor.speed = Com_limit_output(right_bottom_motor.speed, 800, 0);

    //安全限制 => 当油门低于50时，将电机速度设置为0
    if(remote_data.thr <= 50 )
    {
        left_top_motor.speed = 0;
        left_bottom_motor.speed = 0;
        right_top_motor.speed = 0;
        right_bottom_motor.speed = 0;

        if (flight_state == FIX_HEIGHT)
        {
            // flight_state = NORMAL;
            pid_height.output = 0.0f;
            pid_height.integral = 0.0f;
        }
    }

    //设置电机速度
    int_motor_set_speed(&left_top_motor);
    int_motor_set_speed(&left_bottom_motor);
    int_motor_set_speed(&right_top_motor);
    int_motor_set_speed(&right_bottom_motor);

}

