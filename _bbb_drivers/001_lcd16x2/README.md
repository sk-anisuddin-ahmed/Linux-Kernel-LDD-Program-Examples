# LCD16x2 Project

This is a simple setup for controlling a 16x2 LCD display on your BeagleBone board 
using a custom kernel driver and user application.

## Building

1. **Kernel Driver:**

   ```bash
   cd kernel_driver
   make
   ```

2. **Device Tree Overlay:**

   ```bash
   dtc -O dtb -o device_tree/lcd16x2.dtbo device_tree/lcd16x2.dts
   ```

3. **User Application:**

   ```bash
   cd user_app
   gcc lcd_test.c -o lcd_test
   ```

## Usage

1. Copy the compiled .dtbo file to `/lib/firmware/` on your BeagleBone. 
   This will trigger automatic loading of the overlay and probe the driver.
2. Run the test application:

   ```bash
   ./user_app/lcd_test
   ```

## Project Structure

```
001_lcd16x2_project/
├── device_tree/
│   ├── lcd16x2.dts
│   ├── lcd16x2.dtsi
│   └── lcd16x2.yaml
├── kernel_driver/
│   ├── lcd16x2_driver.c
│   └── Makefile
├── user_app/
│   └── lcd_test.c
│   └── Makefile
├── build.sh
└── README.md
```
