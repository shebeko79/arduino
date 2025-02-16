#pragma once

constexpr int FAIL_SAFE_DELAY = 500;

constexpr float WHEEL_PULSES_PER_ROTATION=90.0;
constexpr float WHEEL_DIAMETER=6.5*0.0254; //meters
constexpr float WHEEL_PERIMETER=M_PI*WHEEL_DIAMETER;
constexpr float WHEEL_PULSES_PER_METER=WHEEL_PULSES_PER_ROTATION/WHEEL_PERIMETER;
 
constexpr float MAX_SPEED = 4.0; //meters per second
constexpr float OFF_SPEED = 0.01; //meters per second

constexpr float RELATIVE_MAX_SPEED = 0.3; //meters per second
constexpr float RELATIVE_MAX_ROT = 0.2; //meters per second

constexpr float PID_ki=0.1/MAX_SPEED;
constexpr float PID_kp=1.0/MAX_SPEED;
constexpr float PID_kd=0.05/MAX_SPEED;


constexpr unsigned TIMER_MS=10;
