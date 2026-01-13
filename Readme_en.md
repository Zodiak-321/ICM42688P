# English Version

[中文](README.md) | [English](README_EN.md)

# Introduction

This repository contains a C++ library for ICM42688P, which is fully encapsulated and easy to use.

It also includes a standalone Madgwick algorithm for Euler angle calculation as a reference. If you have a QMC5883P magnetometer, there is also a corresponding library available for direct use to achieve 9-axis Euler angle calculation.

This repository is primarily designed for STM32, but it is easy to port. The main functions that need modification are placed at the front. For example, the Code section includes specializations made for the magnetometer and gyroscope code during the preparation for the Rensas Cup, for reference only.

# Features

A C++ driver library designed specifically for the ICM42688P 6-axis IMU (3-axis accelerometer + 3-axis gyroscope), suitable for STM32 platforms, providing flexible configuration and powerful functionality.

## ✨ Main Features
### 🔧 Flexible Filter System
Configurable multi-stage filters: Supports UI filters (1st/2nd/3rd order), M2 filter, AAF (Anti-Aliasing Filter), NF (Notch Filter)

Automatic parameter calculation: Automatically calculates optimal parameters for AAF and NF based on ODR

Preset filter levels: Provides multiple preset modes from VERY_HIGH to VERY_LOW

### 🚀 Intelligent Interrupt Management System
Event callback mechanism: Supports 18 interrupt events, allowing registration of custom callback functions

Priority scheduling: Interrupt events are processed by priority to ensure timely response to critical events

Flexible registration: Supports independent or simultaneous configuration of different callbacks for INT1 and INT2

### 🔌 Dual Communication Interface Support
Plug and play: Supports both I2C and SPI, automatically selected via conditional compilation

Error handling: Comprehensive communication error detection and debug output

### 🎯 Professional-Level Features
Software/Hardware dual offset calibration: Automatic compensation for sensor offsets

Installation direction compensation: Supports 6 different installation directions

Timestamp support: Two resolutions of 1μs/16μs

Real-time data reading: Optimized data reading mechanism

### 🚀 Quick Start
```cpp
// Initialize IMU
ICM42688 imu(&hi2c1);  // I2C method
// ICm42688 imu(&hspi1, GPIOA, GPIO_PIN_4);  // SPI method

// Initialize device
imu.begin();

// Enable accelerometer and gyroscope (preset filter level)
imu.general.enable_acc(ICM42688::General::ACCEL_ODR_200HZ, 
                       ICM42688::General::ACCEL_FS_2G,
                       ICM42688::General::FILTER_LEVEL::MEDIUM);
imu.general.enable_gyro(ICM42688::General::GYRO_ODR_200HZ,
                        ICM42688::General::GYRO_FS_500DPS,
                        ICM42688::General::FILTER_LEVEL::MEDIUM);

// Register data ready interrupt
imu.Int.register_event(ICM42688::INT::IntEvent::DATA_RDY_INT, 
                      ICM42688::INT::INT_NUM::INT1, 
                      my_data_callback);

// Read data
imu.general.read_data();
float ax = imu.general.get_ax();
```

### 📁 Code Structure
```text
ICM42688.h/cpp          # Main class, communication interface
├── General            # General functions: sensor configuration, data reading
├── INT                # Interrupt management: event registration, priority handling
├── FIFO              # FIFO buffer management (under development)
└── APEX              # Advanced features: pedometer, tap detection, etc. (under development)
```

### 🔧 System Requirements
STM32 HAL Library

C++11 or higher

I2C or SPI peripheral

# Repository Structure

Code: Main code storage

Document: Reference documents

# Future Directions

[26/01/13] May consider completing the FIFO and APEX parts, as this project is currently for drones, and the requirements for FIFO and APEX are not very high.

# Usage

Please refer to the Readme.md in specific sections for details.
