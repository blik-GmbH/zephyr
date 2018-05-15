.. _lis3dh:

LIS3DH Accelerometer - Manual triggering
##########################################

Overview
********

A sensor application that demonstrates the manual trigger mode.

There are two states:
- Sleep: Sleep for some time. In this time the interrupt won't get processed, since we don't want to wake up
- Awake: Process any interrupt that happened since the last wake-phase

The states are changed via setting a trigger function with the sensor API.
When the trigger function is a NULL-pointer, the GPIO pin will get reconfigured to actually be a input pin. As soon as the trigger function is non NULL, the GPIO pin will get configured to act on any interrupt that has happened since last time.


Building and Running
********************

This project outputs sensor data to the console. It requires an lis3dh
sensor.
Make sure that the pinmux is right or simply use the akita_gen2_devboard.

.. zephyr-app-commands::
   :zephyr-app: samples/sensors/lis3dh
   :board: akita_gen2_devboard
   :goals: build
   :compact:
