# Spaceport-America-HPR

<img src="https://rocketry.okstate.edu/sites/default/files/OSU%20AIAA%20ROCKETRY%20logo.png" width="250">

## About
This project will be used to create a working airbrake system for the Spaceport America Competition in June. The goal of this competition is to get as close to 10,000 feet as possible with a High Powered Rocket.

Our airbrake will be controlled by stepper motors with a gear increase for torque, which will turn a lead screw after the rocket motor has burnt out until apogee. The STM32 Board will receive telemetry data over SPI from the Telemega, it will access a lookup table on our flash chip to get the corresponding drag coefficients. It will then drive the stepper motor using a stepper motor controller to step the airbrake pads out, thus slowing down the rocket. This could be further enhanced using more accurate data and a PID controller.

## Details
The code will be written in two main sections
Section 1: STM32 code written in STM32 CubeIDE programmed using SWD pins
Section 2: Code written and built in Linux using Altus Metrum's Telemega code as a base

## References
For more information on the Altus Metrum Telemega visit https://altusmetrum.org/

Thank you to Bdale Garbee for his help in the development process and for designing the Telemega https://gag.com/bdale/
