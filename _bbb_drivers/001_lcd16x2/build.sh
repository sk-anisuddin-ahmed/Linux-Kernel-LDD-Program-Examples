# Usage:
#   chmod +x build.sh
#   ./build.sh

# Build Device Tree Overlay
dtc -@ -O dtb -o device_tree/lcd16x2_anis.dtbo device_tree/lcd16x2.dts

# Copy overlay to firmware
sudo cp device_tree/lcd16x2_anis.dtbo /lib/firmware/

# Enable overlay (BeagleBone)
#echo lcd16x2_anis | sudo tee /sys/devices/platform/bone_capemgr/slots
# NOTE: Overlay is now enabled via /boot/uEnv.txt, not bone_capemgr 
echo "Add the following line to /boot/uEnv.txt:" 
echo "dtb_overlay=/lib/firmware/lcd16x2_anis.dtbo"

# Build kernel driver
make -C kernel_driver

# Load driver
sudo insmod kernel_driver/lcd16x2_driver.ko

# Build user app
make -C user_app
