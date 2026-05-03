# Skittles Sorter V1

This repository contains the Arduino code for the original Skittles Sorter.

The Skittles Sorter is a 3D printed candy sorting machine that detects the color of a Skittle and sorts it into the correct container. It was developed as an educational project to introduce primary school children to technology, electronics, coding and mechanical design.

The goal is simple: make technology visible, fun and hands-on.
Link to design: https://makerworld.com/en/models/1481101-skittlers-sorter#profileId-1546502

## What the code does

The Arduino controls the main functions of the machine:

* Rotates the turntable using a stepper motor
* Detects the color of each Skittle using a TCS3200 color sensor
* Keeps track of the detected color positions
* Activates the correct servo-controlled trapdoor
* Uses a PCA9685 servo driver to control multiple servos
* Supports basic serial commands for testing and validation

## Hardware used

* Arduino Uno R3
* PCA9685 servo driver board
* TCS3200 color sensor
* Stepper motor with driver
* Servo motors
* 3D printed mechanical parts

## Arduino libraries

Install the following library in the Arduino IDE:

* Adafruit PWM Servo Driver Library

## Serial commands

The code includes a few simple serial commands for testing:

```text
test [servo number] [repeat count]
validate
turn
