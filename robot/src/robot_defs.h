#pragma once
#include <math.h>

constexpr int FAIL_SAFE_DELAY = 5000;

constexpr float WHEEL_PULSES_PER_ROTATION=90.0;
constexpr float WHEEL_DIAMETER=6.5*0.0254; //meters
constexpr float WHEEL_PERIMETER=M_PI*WHEEL_DIAMETER;
constexpr float WHEEL_PULSES_PER_METER=WHEEL_PULSES_PER_ROTATION/WHEEL_PERIMETER;
 
constexpr float MAX_SPEED = 4.0; //Estimated speed of unloaded wheel on maximum power (meters per second)
constexpr float OFF_SPEED = 0.01; //meters per second
constexpr float CLOSE_TO_ZERO_ACCELERATION = 0.02;
constexpr float CLOSE_TO_ZERO_SPEED_DIFF = 0.05;
constexpr float UP_SPEED_CORRECTION = 0.01;
constexpr float DOWN_SPEED_CORRECTION = 0.03;
constexpr float BREAK_MAX_SPEED_DIFF = 1.0;
constexpr float BREAK_MAX_SPEED_DIFF_WITH_ACCELERATION = 0.5;
constexpr float KICK_PWM_CORRECTION = 0.1;
constexpr float MIN_SPEED2PWM = 1.0/MAX_SPEED/4;
constexpr float MAX_SPEED2PWM = 1.0/MAX_SPEED*16;
constexpr unsigned long MAIN_CYCLE_DELAY = 50;
constexpr float EXPECTED_PWM_DELAY = MAIN_CYCLE_DELAY/1000.0;

constexpr float RELATIVE_MAX_SPEED = 0.3; //meters per second
constexpr float RELATIVE_MAX_ROT = 0.2; //meters per second


constexpr unsigned TIMER_MS=10;
