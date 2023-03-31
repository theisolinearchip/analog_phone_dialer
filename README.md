# Analog Phone Dialer Firmware

Few months ago I got one of those old analog dial-based phones. Since the pulse-dialing technique is kinda deprecated nowadays
I made an __atmega48p-based board__ to capture the input numbers from the rotary dial and translate them into "modern" DTMF
pulses using a DAC converter made with a R-2R resistor ladder.

More info about this project available on [my website](http://albertgonzalez.coffee/projects/analog_phone_dialer/) and [hackaday.io](https://hackaday.io/project/189787-analog-phone-dialer).

## Building the firmware

- __make all__ to compile
- __make flash__ to upload
- __make fuses__ to set the fuses (for an __atmega48p__)

## Links

- [Project page on my website](http://albertgonzalez.coffee/projects/analog_phone_dialer/)
- [Project page on hackaday.io](https://hackaday.io/project/189787-analog-phone-dialer)

## 
![Analog Phone Dialer, phone](http://albertgonzalez.coffee/projects/analog_phone_dialer/img/phone_1.jpg)
