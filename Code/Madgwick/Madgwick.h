#ifndef __MADGWICK_H__
#define __MADGWICK_H__

#include "cpp_main.h"
#include <math.h>

#define PI 3.14159265
#define g  9.80655

#define ACCEL_BETA_THRESHOLD 0.5f       // 加速度计数据与重力加速度的差值阈值，单位为m/s^2。当加速度计数据与重力加速度的差值小于该阈值时，认为加速度计数据可靠，可以使用较高的beta值进行姿态估计；否则认为加速度计数据不可靠，使用较低的beta值以减少对姿态估计的影响。

#define MADGWICK_BETA_DEF 0.98f         // Madgwick算法的默认beta值，适用于加速度计数据可靠的情况。较高的beta值会使算法更快地响应加速度计数据，但可能会引入更多的噪声。
#define MADGWICK_BETA_FAST 0.001f       // Madgwick算法的快速响应beta值，适用于加速度计数据不可靠的情况。较低的beta值会使算法更慢地响应加速度计数据，但可以减少噪声对姿态估计的影响。

class EULER
{
    public:
        EULER(void);

        void MadgwickAHRSupdateIMU(float ax, float ay, float az, float gx, float gy, float gz);
        void MadgwickAHRSupdate(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz);

        // void ESKFupdataIMU(float ax, float ay, float az, float gx, float gy, float gz);
        // void ESKFupdata(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz);

        inline float get_q0() { return _q0; }
        inline float get_q1() { return _q1; }
        inline float get_q2() { return _q2; }
        inline float get_q3() { return _q3; }

        inline float get_pitch() { _pitch = asinf(-2.0f * (_q1*_q3 - _q0*_q2)) * (180.0f / PI); return _pitch; }
        inline float get_roll()  { _roll = atan2f(_q0*_q1 + _q2*_q3, 0.5f - _q1*_q1 - _q2*_q2) * (180.0f / PI); return _roll; }
        inline float get_yaw()   { _yaw = atan2f(2.0f*(_q1*_q2 + _q0*_q3), _q0*_q0 + _q1*_q1 - _q2*_q2 - _q3*_q3) * (180.0f / PI); return _yaw; }

    private:
        uint32_t getTick(void);
        float invSqrt(float x);
        void Norm_Data(float *x,float *y,float *z);
        void Norm_Data_Q(float *w,float *x,float *y,float *z);
        float lowPassFilter(float value, float last_value, uint8_t alpha_shift);

        float _betaDef;
        float _q0, _q1, _q2, _q3;
        float _pitch;
        float _roll;
        float _yaw;

        uint32_t _last_time;
};

#endif
