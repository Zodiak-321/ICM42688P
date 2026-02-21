#include "Movement.h"

uint32_t MOVEMENT::_get_tick(void)
{
    // Implementation for getting the current tick or time
    return HAL_GetTick();
}

/*============================================================================================*/

MOVEMENT::MOVEMENT(MOVEMENT_MountingOrientation mounting)
    : _mounting(mounting)
{
    switch(mounting)
    {
        case MOVEMENT_MountingOrientation::MOUNT_Z_DOWN:
            _fix_ax = 0.0f;
            _fix_ay = 0.0f;
            _fix_az = g;
            break;
        case MOVEMENT_MountingOrientation::MOUNT_Z_UP:
            _fix_ax = 0.0f;
            _fix_ay = 0.0f;
            _fix_az = -g;
            break;
        case MOVEMENT_MountingOrientation::MOUNT_X_DOWN:
            _fix_ax = g;
            _fix_ay = 0.0f;
            _fix_az = 0.0f;
            break;
        case MOVEMENT_MountingOrientation::MOUNT_X_UP:
            _fix_ax = -g;
            _fix_ay = 0.0f;
            _fix_az = 0.0f;
            break;
        case MOVEMENT_MountingOrientation::MOUNT_Y_DOWN:
            _fix_ax = 0.0f;
            _fix_ay = g;
            _fix_az = 0.0f;
            break;
        case MOVEMENT_MountingOrientation::MOUNT_Y_UP:
            _fix_ax = 0.0f;
            _fix_ay = -g;
            _fix_az = 0.0f;
            break;
        default:
            _fix_ax = 0.0f;
            _fix_ay = 0.0f;
            _fix_az = g;
            break;
    }

    data.accel.accel = 0.0f;
    data.accel.accel_axis.x = 0.0f;
    data.accel.accel_axis.y = 0.0f;
    data.accel.accel_axis.z = 0.0f;
    data.accel.direction.x = 0.0f;
    data.accel.direction.y = 0.0f;
    data.accel.direction.z = 0.0f;

    data.speed.speed = 0.0f;
    data.speed.speed_axis.x = 0.0f;
    data.speed.speed_axis.y = 0.0f;
    data.speed.speed_axis.z = 0.0f;
    data.speed.direction.x = 0.0f;
    data.speed.direction.y = 0.0f;
    data.speed.direction.z = 0.0f;
}

uint32_t MOVEMENT::_pow(uint8_t base, uint8_t exp)
{
    uint32_t result = 1;
    for(uint8_t i = 0; i < exp; i++)
        result *= base;
    return result;
}

float MOVEMENT::_save_n_float(uint8_t n, float value)
{
    float result = (float)((int32_t)(value * _pow(10, n))) / _pow(10, n);
    return result;
}

void MOVEMENT::_get_accel_IMU(float ax, float ay, float az, float pitch, float roll, float yaw)
{
    float ax_corrected = 0;
    float ay_corrected = 0;
    float az_corrected = 0;

    float pitch_rad = pitch * PI / 180.0f;
    float roll_rad = roll * PI / 180.0f;
    float yaw_rad = yaw * PI / 180.0f;

    float sp = sinf(pitch_rad);
    float cp = cosf(pitch_rad);
    float sr = sinf(roll_rad);
    float cr = cosf(roll_rad);
    float sy = sinf(yaw_rad);
    float cy = cosf(yaw_rad);

    ax_corrected = cp * cy * ax + (sr * sp * cy - cr * sy) * ay + (cr * sp * cy + sr * sy) * az;
    ay_corrected = cp * sy * ax + (sr * sp * sy + cr * cy) * ay + (cr * sp * sy - sr * cy) * az;
    az_corrected = -sp * ax + sr * cp * ay + cr * cp * az;

    ax_corrected -= _fix_ax;
    ay_corrected -= _fix_ay;
    az_corrected -= _fix_az;

    data.accel.accel_axis.x = _save_n_float(2, ax_corrected);
    data.accel.accel_axis.y = _save_n_float(2, ay_corrected);
    data.accel.accel_axis.z = _save_n_float(2, az_corrected);

    data.accel.accel = _save_n_float(2, sqrtf(ax_corrected * ax_corrected + ay_corrected * ay_corrected + az_corrected * az_corrected));

    data.accel.direction.x = ax_corrected / data.accel.accel;
    data.accel.direction.y = ay_corrected / data.accel.accel;
    data.accel.direction.z = az_corrected / data.accel.accel;
}

void MOVEMENT::_get_accel_IMU(float ax, float ay, float az, float q0, float q1, float q2, float q3)
{
    float ax_corrected = 0;
    float ay_corrected = 0;
    float az_corrected = 0;

    ax_corrected = (1 - 2*q2*q2 - 2*q3*q3) * ax
             + (2*q1*q2 - 2*q3*q0) * ay
             + (2*q1*q3 + 2*q2*q0) * az;

    ay_corrected = (2*q1*q2 + 2*q3*q0) * ax
             + (1 - 2*q1*q1 - 2*q3*q3) * ay
             + (2*q2*q3 - 2*q1*q0) * az;

    az_corrected = (2*q1*q3 - 2*q2*q0) * ax
             + (2*q2*q3 + 2*q1*q0) * ay
             + (1 - 2*q1*q1 - 2*q2*q2) * az;

    ax_corrected -= _fix_ax;
    ay_corrected -= _fix_ay;
    az_corrected -= _fix_az;

    data.accel.accel_axis.x = _save_n_float(2, ax_corrected);
    data.accel.accel_axis.y = _save_n_float(2, ay_corrected);
    data.accel.accel_axis.z = _save_n_float(2, az_corrected);

    data.accel.accel = _save_n_float(2, sqrtf(ax_corrected * ax_corrected + ay_corrected * ay_corrected + az_corrected * az_corrected));

    data.accel.direction.x = ax_corrected / data.accel.accel;
    data.accel.direction.y = ay_corrected / data.accel.accel;
    data.accel.direction.z = az_corrected / data.accel.accel;
}

void MOVEMENT::_get_speed_IMU(void)
{
    static uint32_t last_tick = 0;
    uint32_t current_tick = _get_tick();
    float dt = (current_tick - last_tick) / 1000.0f;
    last_tick = current_tick;

    data.speed.speed_axis.x += _save_n_float(2, data.accel.accel_axis.x * dt);
    data.speed.speed_axis.y += _save_n_float(2, data.accel.accel_axis.y * dt);
    data.speed.speed_axis.z += _save_n_float(2, data.accel.accel_axis.z * dt);

    data.speed.speed = _save_n_float(2, sqrtf(data.speed.speed_axis.x * data.speed.speed_axis.x + data.speed.speed_axis.y * data.speed.speed_axis.y + data.speed.speed_axis.z * data.speed.speed_axis.z));

    data.speed.direction.x = data.speed.speed_axis.x / data.speed.speed;
    data.speed.direction.y = data.speed.speed_axis.y / data.speed.speed;
    data.speed.direction.z = data.speed.speed_axis.z / data.speed.speed;
}

void MOVEMENT::update_IMU(float ax, float ay, float az, float pitch, float roll, float yaw)
{
    _get_accel_IMU(ax, ay, az, pitch, roll, yaw);
    _get_speed_IMU();
}

void MOVEMENT::update_IMU(float ax, float ay, float az, float q0, float q1, float q2, float q3)
{
    _get_accel_IMU(ax, ay, az, q0, q1, q2, q3);
    _get_speed_IMU();
}



