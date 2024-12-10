#pragma once

constexpr float WHEEL_PULSES_PER_ROTATION=90.0;
constexpr float WHEEL_DIAMETER=6.5*0.0254; //meters
constexpr float WHEEL_PERIMETER=M_PI*WHEEL_DIAMETER;
constexpr float WHEEL_PULSES_PER_METER=WHEEL_PULSES_PER_ROTATION/WHEEL_PERIMETER;
 
constexpr float MAX_SPEED = 4.0; //meters per second
constexpr float MAX_ROT_DIFF = 0.2; //meters per second

constexpr float PID_ki=0.0;
constexpr float PID_kp=1.0;
constexpr float PID_kd=0.0;

constexpr float SPEED_TO_PWM=WHEEL_PULSES_PER_METER/3.0;

constexpr unsigned TIMER_MS=10;
