.. _lis3dh:

LIS3DH Accelerometer/Temperature Sensor
##########################################

Overview
********

A sensor application that demonstrates how to read the accelation and temperature
of the lis3dh sensor.


Building and Running
********************

This project outputs sensor data to the console. It requires an lis3dh
sensor.

.. zephyr-app-commands::
   :zephyr-app: samples/sensors/lis3dh
   :board: akita_gen2_devboard
   :goals: build
   :compact:

Sample Output
=============

.. code-block:: console

    AX=  0.229846 AY=  0.076615 AZ=  9.500328, TEMP=13.000000
    AX=  0.153231 AY=  0.306462 AZ=  9.576943, TEMP=12.000000
    AX=  0.229846 AY=  0.153231 AZ=  9.653559, TEMP=12.000000
    AX=  0.229846 AY=  0.076615 AZ=  9.576943, TEMP=12.000000
    AX=  0.229846 AY=  0.306462 AZ=  9.576943, TEMP=12.000000
    AX=  0.229846 AY=  0.229846 AZ=  9.576943, TEMP=12.000000

<repeats endlessly>
