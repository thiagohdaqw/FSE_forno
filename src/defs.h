#ifndef __DEFS_H__
#define __DEFS_H__

typedef struct
{
    float kp;
    float ki;
    float kd;
} PID;


typedef struct
{
    float intern_temperature;
    float extern_temperature;
    float user_temperature;
    float resistor_percent;
    float fan_percent;
    PID pid;
} States;



#endif