# Board Support Package

- Board code handles the physical platform (pin muxing, clock init, onboard peripherals)
- Uses arch/ and drivers/ and defines a platform
- Chooses HAL + driver for each supported device
- Linker scripts
