# Larson Scanner using 595 Shift Registers

This project uses 595 Shift Registers to create the sweeping light pattern known as a Larson Scanner (Think Cylon Eye or the front of the car in Knight Rider).

The master branch uses an ATmega168 as an example, driving the shift registers with the hardware SPI. This should be easily adapted to any chip that has SPI capabilities.

There is a second branch in which I've ported the code to an ATtiny13. This chip doesn't really have hardware SPI so I'm driving the shift registers in software (bit banging). I have also added some code that keeps the scanning function stopped until one of the external pins is pulled low. That's becuase I'm triggering the hardware with one pin from the parallel port on my computer.
For more information on that branch, check out the code repository, and my blog post about it.

*	[ATtiny13 code branch](https://github.com/szczys/595-Larson-Scanner/tree/t13-port)
*	[Blog post about ATtiny13 branch](http://jumptuck.com/2011/11/17/cylon-eye-conclusion/)

