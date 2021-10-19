GCC=~/arm-eabi-4.6/bin/arm-eabi-

make distclean
make ARCH=arm CROSS_COMPILE=$GCC mx6sl_heisenberg_config
make ARCH=arm CROSS_COMPILE=$GCC

