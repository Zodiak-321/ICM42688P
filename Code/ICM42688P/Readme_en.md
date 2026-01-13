# ICM42688P Driver Library - Detailed Documentation

[中文](Readme.md) | [English](Readme_en.md)

## 📖 Table of Contents
1. [Project Overview](#project-overview)
2. [Core Architecture Design](#core-architecture-design)
3. [Filter System Details](#filter-system-details)
4. [Interrupt Management System Details](#interrupt-management-system-details)
5. [Data Reading and Calibration](#data-reading-and-calibration)
6. [API Reference](#api-reference)
7. [Usage Examples](#usage-examples)
8. [Debugging Support](#debugging-support)

## Project Overview

This library is a complete driver implementation for the TDK InvenSense ICM42688P 6-axis IMU, developed based on the STM32 HAL library. The code is strictly written according to the ICM42688P technical manual, providing rich configuration options and professional signal processing functions.

## Core Architecture Design

### 🏗️ Class Hierarchy Structure
```
ICM42688 (Main Class)
├── General: General function management
│   ├── Sensor enable/disable
│   ├── Filter configuration
│   ├── Data reading
│   └── Offset calibration
├── INT: Interrupt management
│   ├── Interrupt source configuration
│   ├── Event callback registration
│   ├── Priority scheduling
│   └── Interrupt status management
├── FIFO: Buffer management (under development)
└── APEX: Advanced algorithms (under development)
```

### 🔄 Design Philosophy
1. **Modular Design**: Each functional module is independent, easy to maintain and extend
2. **Type Safety**: Use strong type enumeration to reduce configuration errors
3. **Resource Efficient**: Avoid dynamic memory allocation, suitable for embedded systems
4. **Debugging Friendly**: Provide detailed debug output and error information

## Filter System Details

### 📊 Accelerometer Filter
Supports two operating modes and multiple filter combinations:

**LN Mode (Low Noise)**:
- Supports ODR: 12.5Hz - 32kHz
- Filters: UI filter + M2 filter + AAF
- UI filter order: 1/2/3 order optional
- Bandwidth configuration: ODR/2 to ODR/400

**LP Mode (Low Power)**:
- Supports ODR: 1.5625Hz - 200Hz
- Filter: Average filter (1/16 times average)

### 🔄 Gyroscope Filter
**LN Mode**:
- Supports ODR: 12.5Hz - 32kHz
- Filter combination:
  - UI filter (1/2/3 order)
  - M2 filter (optional)
  - NF notch filter (configurable center frequency and bandwidth)
  - AAF anti-aliasing filter

### 🎛️ Preset Filter Levels
Provides 6 preset configurations to meet different application requirements:

| Level | UI Order | M2 Filter | AAF | Application Scenario |
|-------|----------|-----------|-----|----------------------|
| VERY_HIGH | 3rd order | Enabled | Enabled | High-precision measurement |
| HIGH | 3rd order | Enabled | Enabled | General IMU applications |
| MEDIUM | 2nd order | Enabled | Enabled | Balance performance and power consumption |
| LOW | 1st order | Disabled | Enabled | Low latency applications |
| VERY_LOW | 1st order | Disabled | Enabled | Low power mode |
| VERT_VERY_LOW | 1st order | Disabled | Disabled | Minimum power consumption |

### ⚙️ Automatic Parameter Calculation
The library provides intelligent parameter calculation functions:
```cpp
// Automatically calculate AAF parameters based on ODR
uint8_t aaf_delt;
uint16_t aaf_deltsqr;
uint8_t aaf_bitshift;
imu.general.find_AFF(ICM42688::General::ACCEL_ODR_1000HZ, 
                     &aaf_delt, &aaf_deltsqr, &aaf_bitshift);

// Automatically calculate NF parameters
uint8_t nf_coswz_sel;
uint16_t nf_coswz;
imu.general.find_NF(2000, &nf_coswz_sel, &nf_coswz);
```

## Interrupt Management System Details

### 🎯 Event-Driven Architecture
Supports 18 interrupt events, each can be configured independently:

```cpp
// Interrupt event enumeration
enum class IntEvent : uint8_t {
    UI_FSYNC_INT,       // UI frame synchronization
    PLL_RDY_INT,        // PLL ready
    RESET_DONE_INT,     // Reset complete
    DATA_RDY_INT,       // Data ready (most commonly used)
    FIFO_THS_INT,       // FIFO threshold
    FIFO_FULL_INT,      // FIFO full
    // ... total 18 events
};
```

### 📝 Callback Function Registration
Provides flexible registration mechanism, supports custom callbacks:

```cpp
// Register data ready interrupt (using default callback)
imu.Int.register_event(ICM42688::INT::IntEvent::DATA_RDY_INT,
                      ICM42688::INT::INT_NUM::INT1);

// Register custom callback
imu.Int.register_event(ICM42688::INT::IntEvent::TAP_DET_INT,
                      ICM42688::INT::INT_NUM::INT2,
                      my_tap_callback,
                      &my_context,
                      ICM42688::INT::EVENT_PRIORITY::PRIORITY_HIGH);
```

### 🏆 Priority Scheduling System
Interrupt events are processed by priority to ensure timely response to critical events:

```cpp
// Set event priority
imu.Int.set_event_priority(ICM42688::INT::IntEvent::DATA_RDY_INT,
                          ICM42688::INT::INT_NUM::INT1,
                          ICM42688::INT::EVENT_PRIORITY::PRIORITY_HIGHEST);
```

### 🔧 Interrupt Pin Configuration
Supports flexible pin configuration:

```cpp
// Configure INT1 pin
imu.Int.set_int1_pin_cfg(GPIOA, GPIO_PIN_0,
                        ICM42688::INT::LEVEL::LEVEL_ACTIVE_HIGH,
                        ICM42688::INT::DRIVE::DRIVE_PUSH_PULL,
                        ICM42688::INT::MODE::LATCHED);

// Configure INT2 pin
imu.Int.set_int2_pin_cfg(GPIOA, GPIO_PIN_1,
                        ICM42688::INT::LEVEL::LEVEL_ACTIVE_LOW,
                        ICM42688::INT::DRIVE::DRIVE_OPEN_DRAIN,
                        ICM42688::INT::MODE::PULSE);
```

## Data Reading and Calibration

### 📈 Data Reading Optimization
- **Smart Axis Detection**: Automatically skips disabled axes, improves reading efficiency
- **Bulk Reading**: Reads all data at once, reduces communication overhead
- **Type Conversion**: Automatically converts raw data to physical quantities

### ⚖️ Dual Offset Calibration
Provides hardware and software dual offset calibration:

```cpp
// Automatic calibration (recommended)
imu.general.set_acc_gyro_offset(1000);  // Sample 1000 times

// Manually set software offset
imu.general._acc_offset_x = -0.1f;
imu.general._gyro_offset_y = 0.05f;
```

### 🧭 Mounting Direction Compensation
Supports 6 mounting directions, automatically compensates for gravity effects:

```cpp
// Specify mounting direction during initialization
ICM42688 imu(&hi2c1, ICM42688_I2C_ADDR_LOW, 
             ICM42688::ICM42688_MountingOrientation::MOUNT_Z_UP);
```

## API Reference

### General Class Main Methods

```cpp
// Sensor control
StatusTypeDef enable_acc(...);      // Enable accelerometer
StatusTypeDef disable_acc();        // Disable accelerometer
StatusTypeDef enable_gyro(...);     // Enable gyroscope
StatusTypeDef disable_gyro();       // Disable gyroscope
StatusTypeDef enable_temp();        // Enable thermometer
StatusTypeDef enable_tmst();        // Enable timestamp

// Data operations
StatusTypeDef read_data();          // Read sensor data
float get_ax();                     // Get X-axis acceleration
float get_gy();                     // Get Y-axis angular velocity
float get_temp();                   // Get temperature

// Calibration
StatusTypeDef set_acc_gyro_offset(uint16_t samples);
```

### INT Class Main Methods

```cpp
// Interrupt configuration
StatusTypeDef set_int1_pin_cfg(...);    // Configure INT1 pin
StatusTypeDef set_int2_pin_cfg(...);    // Configure INT2 pin

// Event management
StatusTypeDef register_event(...);      // Register interrupt event
StatusTypeDef unregister_event(...);    // Unregister interrupt event
StatusTypeDef set_event_priority(...);  // Set event priority
StatusTypeDef set_event_enable(...);    // Enable/disable event

// Interrupt handling
StatusTypeDef process_interrupts(...);  // Process interrupts
bool is_event_registered(...);          // Check if event is registered
```

## Usage Examples

### Example 1: Basic Data Reading
```cpp
#include "ICM42688.h"

ICM42688 imu(&hi2c1);

void setup() {
    imu.begin();
    
    // Enable sensors
    imu.general.enable_acc();
    imu.general.enable_gyro();
}

void loop() {
    imu.general.read_data();
    
    printf("Acc: %.2f, %.2f, %.2f g\n", 
           imu.general.get_ax(),
           imu.general.get_ay(),
           imu.general.get_az());
    
    printf("Gyro: %.2f, %.2f, %.2f dps\n",
           imu.general.get_gx(),
           imu.general.get_gy(),
           imu.general.get_gz());
    
    HAL_Delay(100);
}
```

### Example 2: Interrupt-Driven Application
```cpp
#include "ICM42688.h"

ICM42688 imu(&hi2c1);
volatile bool data_ready = false;

void data_ready_callback(void* context) {
    data_ready = true;
}

void setup() {
    imu.begin();
    
    // Configure sensors
    imu.general.enable_acc(ICM42688::General::ACCEL_ODR_1000HZ,
                          ICM42688::General::ACCEL_FS_16G);
    
    // Register interrupt
    imu.Int.set_int1_pin_cfg(GPIOA, GPIO_PIN_0);
    imu.Int.register_event(ICM42688::INT::IntEvent::DATA_RDY_INT,
                          ICM42688::INT::INT_NUM::INT1,
                          data_ready_callback);
}

// Call in EXTI interrupt service routine
void EXTI0_IRQHandler() {
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0)) {
        imu.Int.process_interrupts(ICM42688::INT::INT_NUM::INT1);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
    }
}

void loop() {
    if(data_ready) {
        imu.general.read_data();
        // Process data...
        data_ready = false;
    }
}
```

## Debugging Support

### 🔍 Debug Output
Enable detailed debug information by defining `ENABLE_DEBUG`:

```cpp
#define ENABLE_DEBUG
#include "debug.h"
```

Debug information includes:
- ✅ Operation success prompts (green)
- ⚠️ Warning information (yellow)
- ❌ Error information (red)
- 🔧 Processing information (blue)

### 📊 Register Access Tracking
All register read/write operations are recorded for easy debugging of hardware communication issues.

### 🎨 Color Terminal Output
Supports ANSI color codes to make debug information easier to read.

---

## 🎯 Application Scenarios

- **Drone Flight Control**: High frequency, low latency attitude data
- **Robot Navigation**: Precise inertial measurement
- **Motion Capture**: High-precision motion tracking
- **Health Monitoring**: Step detection, activity recognition
- **Industrial Control**: Vibration monitoring, attitude stabilization

## 📝 Precautions

1. **Power Stability**: Ensure stable power supply for IMU, recommend using LDO
2. **Communication Speed**: Choose appropriate I2C/SPI clock frequency based on ODR
3. **Interrupt Handling**: Interrupt service routines should be as brief as possible
4. **Calibration Environment**: Keep IMU stationary during calibration
5. **Temperature Effects**: Temperature changes affect sensor accuracy, recommend regular calibration

## 🤝 Contribution Guidelines

Welcome to submit Issues and Pull Requests! For new feature requests, please discuss the implementation plan first.

## 📄 License

MIT License

---

*This library is under continuous development, welcome feedback and suggestions!*
