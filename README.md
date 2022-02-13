**Alnwlsn's custom version**

This is mostly standard [grbl-Mega-5X](https://github.com/fra589/grbl-Mega-5X) with some extra stuff hacked on for my own purposes. I use this on my converted Chinese 3040CNC.

**K40 Laser Branch** - I am now trying to use this GRBL on my new K40 laser. The CO2 laser driver it uses takes an analog voltage in, not PWM, so I hooked up a I2C DAC to provide the signal. This is kind of hacky, hence why it's in the K40 branch

* **A B and C axes** - Provided by grbl-Mega-5X, three extra axes to use with 3 more steppers, in addition to the normal X Y and Z. Used in G commands like normal axes. I use A for the rotary axis that came with my mill, and I want to use one of the extras to drive a 3D printer extruder.

* **Laser Mode** - I added one of those cheap 5.5W laser diode modules to my mill. It's driven by PWM, and most laser machines would normally use the spindle PWM output, but I already have a spindle attached to that. 
  * Timer5 is unused in normal grbl-Mega-5X, so I used the OC5B pin to run PWM for the laser (9-bit at ~4Khz).
  * There's already a Laser Mode function in grbl, so I use that to change between spindle PWM and laser PWM ($32=0 - spindle or 1 - laser).
    *  For safety, I never want to power my laser by accident, so I have grbl ignore the $32 setting in the EEPROM, and always overwrite $32 = 0 when the EEPROM is read. That way, at power up the CNC will always be in spindle mode. You can then change to laser mode with $32 = 1
    *  Laser power is adjusted with S setting; S0 for minimum power (off) to S512 for maximum power (full on). M3/M4 for laser on (at selected power) and M5 for laser off.
    *  Per a standard Grbl feature, the laser will not turn on with M3 until the first G1 move, so that you don't burn a hole in your stock by turning on the laser before the laser head is moving.

* **Interrupt Monitored Endstop Switches** - Immediately makes all steppers stop moving when an endstop is hit. Apparently, this was a feature in standard Grbl, but I didn't find any interrupt code in grbl-Mega-5X. 
  * grbl-Mega-5X does come with a solution for this, ramps_hard_limit(), which doesn't use interrupts on the endstop limit pins, but works (I think) by checking the limit pins during the Stepper interrupts. I've found this to have a detrimental effect on the stepper performance; seems that the AVR just doesn't have enough juice to do both. 
  * All the standard limit pins (at least as set up by default for a RAMPS 1.4 pinout) have hardware interrupt capability of some kind. Some have regular interrupts, while a couple only have pin change interrupts. 
    * I configured all pins for a pin change interrupt. When any are tripped, we measure the state of all the pins and see if any of the endstop switches are pressed. If they are, we trigger the stuff that ramps_hard_limit() did (which is to stop the steppers, raise an alarm, and go into a loop). 
    * This addition DOES NOT respect the pin settings in cpu_map.h or any of the invert settings. It only works with the pins in the pinout below, and only works with Low Side Switching limit switches (or equivalent). This could probably be improved if I could be bothered. 
  * These interrupts are disabled during a Homing cycle (since that uses the endstops to home the machine). 
    * also, homing cycle now only recognizes the Home position at the Max limit endstops for each axis. Previously, it didn't, which made the homing cycle break when trying to home with an axis stuck on the Min position, thinking that it was actually at the Max one.  

* **Control of 8 Servos** - I might have a need to run a couple servos (lifting a pen, opening a clamp, tool changer, whatever), so I added the ability to control servos. This is merged from my first attempt [grbl-Mega-5X-servos](https://github.com/fra589/grbl-Mega-5X-servos)
  * This works by using a timer and interrupts to toggle some pins, and create the standard hobby servo control pulse signal. This is similar to what the Servo library does on Arduino to drive a bunch of servos from one timer.
    * TIMER3 is used here, which conflicts with Grbl's Sleep function (which is disabled by default, and I commented out the Init function for sleep in my version so I could use TIMER3). But, if you really need the Sleep functions, you could count cycles of the Servo pulse generator timer. 
  * You also get 3 new M commands:
    * M96 - sets the servo position immediately when issued. Works like G0.
      * Usage: **M96 An Bn Cn Dn Wn Xn Yn Zn** 
      * where A B C D E W X Y Z refer to the corresponding servo (do not include any servo you don't want to change), and n is the servo position (from 0 to 499 - actual angle this refers to on your servo depends on the type of servo)  
    * M97 - Starts servo moving smoothly from current position to new position. Works like G1. Non-blocking.
      * Usage: **M97 An Bn Cn Dn Wn Xn Yn Zn Ts** 
      * where A B C D E W X Y Z refer to the corresponding servo, n is the servo position, and T is the time of the move, in seconds. This timing uses the time base of the servo signal period, and is therefore completely isolated from the rest of Grbl's stepper control functions. It more or less does the servo move in the background.
    * M98 - works exactly the same as M97, but instead of starting when the command is issued, it starts with the next G command issued. This makes it possible to (roughly) cordinate motion with a move in the stepper axes (but you need to figure out the correct timings yourself).
    * The servos don't remember their positions like steppers do, so at boot, all servos will return to position 0.  
  * I wanted to use the Servo pin headers on the RAMPS 1.4 board, but 2 of them are used for the Spindle Enable and Direction pins, so I moved those functions to other pins.

and here is the pinout that I use:
![Pinout](https://github.com/alnwlsn/grbl-Mega-5X-alnwlsn/blob/edge/doc/mega-pinouts.png)

***

![GitHub Logo](https://github.com/fra589/grbl-Mega-5X/blob/edge/doc/images/Mega-5X-logo.svg)

***

Grbl is a no-compromise, high performance, low cost alternative to parallel-port-based motion control for CNC milling. This version of Grbl runs on an Arduino Mega2560 only.

The controller is written in highly optimized C utilizing every clever feature of the AVR-chips to achieve precise timing and asynchronous operation. It is able to maintain up to 30kHz of stable, jitter free control pulses.

It accepts standards-compliant g-code and has been tested with the output of several CAM tools with no problems. Arcs, circles and helical motion are fully supported, as well as, all other primary g-code commands. Macro functions, variables, and most canned cycles are not supported, but we think GUIs can do a much better job at translating them into straight g-code anyhow.

Grbl includes full acceleration management with look ahead. That means the controller will look up to 24 motions into the future and plan its velocities ahead to deliver smooth acceleration and jerk-free cornering.

* [Licensing](https://github.com/fra589/grbl-Mega-5X/blob/edge/COPYING): Grbl is free software, released under the GPLv3 license.

* For more information and help, check out our **[Wiki pages!](https://github.com/gnea/grbl/wiki)** If you find that the information is out-dated, please to help us keep it updated by editing it or notifying our community! Thanks!

* Lead Developer: Gauthier Brière (France) aka @fra589

* Built on the wonderful Grbl v1.1f (2017) firmware originally written by Simen Svale Skogsrud (Norway) and maintained by Sungeun "Sonny" Jeon, Ph.D. (USA) aka @chamnit

***

##Update Summary for v1.1
- **IMPORTANT:** Your EEPROM will be wiped and restored with new settings. This is due to the addition of two new spindle speed '$' settings.

- **Real-time Overrides** : Alters the machine running state immediately with feed, rapid, spindle speed, spindle stop, and coolant toggle controls. This awesome new feature is common only on industrial machines, often used to optimize speeds and feeds while a job is running. Most hobby CNC's try to mimic this behavior, but usually have large amounts of lag. Grbl executes overrides in realtime and within tens of milliseconds.

- **Jogging Mode** : The new jogging commands are independent of the g-code parser, so that the parser state doesn't get altered and cause a potential crash if not restored properly. Documentation is included on how this works and how it can be used to control your machine via a joystick or rotary dial with a low-latency, satisfying response.

- **Laser Mode** : The new "laser" mode will cause Grbl to move continuously through consecutive G1, G2, and G3 commands with spindle speed changes. When "laser" mode is disabled, Grbl will instead come to a stop to ensure a spindle comes up to speed properly. Spindle speed overrides also work with laser mode so you can tweak the laser power, if you need to during the job. Switch between "laser" mode and "normal" mode via a `$` setting.

	- **Dynamic Laser Power Scaling with Speed** : If your machine has low accelerations, Grbl will automagically scale the laser power based on how fast Grbl is traveling, so you won't have burnt corners when your CNC has to make a turn! Enabled by the `M4` spindle CCW command when laser mode is enabled!

- **Sleep Mode** : Grbl may now be put to "sleep" via a `$SLP` command. This will disable everything, including the stepper drivers. Nice to have when you are leaving your machine unattended and want to power down everything automatically. Only a reset exits the sleep state.

- **Significant Interface Improvements**: Tweaked to increase overall performance, include lots more real-time data, and to simplify maintaining and writing GUIs. Based on direct feedback from multiple GUI developers and bench performance testing. _NOTE: GUIs need to specifically update their code to be compatible with v1.1 and later._

	- **New Status Reports**: To account for the additional override data, status reports have been tweaked to cram more data into it, while still being smaller than before. Documentation is included, outlining how it has been changed. 
	- **Improved Error/Alarm Feedback** : All Grbl error and alarm messages have been changed to providing a code. Each code is associated with a specific problem, so users will know exactly what is wrong without having to guess. Documentation and an easy to parse CSV is included in the repo.
	- **Extended-ASCII realtime commands** : All overrides and future real-time commands are defined in the extended-ASCII character space. Unfortunately not easily type-able on a keyboard, but helps prevent accidental commands from a g-code file having these characters and gives lots of space for future expansion.
	- **Message Prefixes** : Every message type from Grbl has a unique prefix to help GUIs immediately determine what the message is and parse it accordingly without having to know context. The prior interface had several instances of GUIs having to figure out the meaning of a message, which made everything more complicated than it needed to be.

- New OEM specific features, such as safety door parking, single configuration file build option, EEPROM restrictions and restoring controls, and storing product data information.
 
- New safety door parking motion as a compile-option. Grbl will retract, disable the spindle/coolant, and park near Z max. When resumed, it will perform these task in reverse order and continue the program. Highly configurable, even to add more than one parking motion. See config.h for details.

- New '$' Grbl settings for max and min spindle rpm. Allows for tweaking the PWM output to more closely match true spindle rpm. When max rpm is set to zero or less than min rpm, the PWM pin D11 will act like a simple enable on/off output.

- Updated G28 and G30 behavior from NIST to LinuxCNC g-code description. In short, if a intermediate motion is specified, only the axes specified will move to the stored coordinates, not all axes as before.

- Lots of minor bug fixes and refactoring to make the code more efficient and flexible.


```
List of Supported G-Codes in Grbl v1.1:
  - Non-Modal Commands: G4, G10L2, G10L20, G28, G30, G28.1, G30.1, G53, G92, G92.1
  - Motion Modes: G0, G1, G2, G3, G38.2, G38.3, G38.4, G38.5, G80
  - Feed Rate Modes: G93, G94
  - Unit Modes: G20, G21
  - Distance Modes: G90, G91
  - Arc IJK Distance Modes: G91.1
  - Plane Select Modes: G17, G18, G19
  - Tool Length Offset Modes: G43.1, G49
  - Cutter Compensation Modes: G40
  - Coordinate System Modes: G54, G55, G56, G57, G58, G59
  - Control Modes: G61
  - Program Flow: M0, M1, M2, M30*
  - Coolant Control: M7*, M8, M9
  - Spindle Control: M3, M4, M5
  - Valid Non-Command Words: F, I, J, K, L, N, P, R, S, T, X, Y, Z, A, B, C
```

-------------
Grbl-Mega-5X is an open-source project and fueled by the free-time of our intrepid administrators and altruistic users. If you'd like to donate, all proceeds will be used to help fund supporting hardware and testing equipment. Thank you!

[![Donate](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](https://paypal.me/pools/c/842hNSm2It)

