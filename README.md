# ShiftPWM-Redux
This is an Arduino Library for software PWM with shift registers based on the popular ShiftPWM library from Elco Jacobs.

DO NOT USE THIS LIBRARY, I CAN NOT SUPPORT IT. |
----|
If you are looking for a code compatible update to ShiftPWM that features 30% faster SPI performce, check out [this fork](https://github.com/Rothens/ShiftPWM-Redux2) |

This fork features following benefits over the original...

* Uses much less code space.
* Works on other Arduinos including ATTINY2/4/85 based boards.
* Leaves SPI, Serial, and Timer hardware free for other purposes.
* Fails at compile time rather than run-time if there is not enough memory for the buffer.
* General cleanup and simplifications over the codebase.


##Changes to existing code

*Old Code...*

```
// You can choose the latch pin yourself.
const int ShiftPWM_latchPin=8;

// ** uncomment this part to NOT use the SPI port and change the pin numbers. This is 2.5x slower **
// #define SHIFTPWM_NOSPI
// const int ShiftPWM_dataPin = 11;
// const int ShiftPWM_clockPin = 13;


// If your LED's turn on if the pin is low, set this to true, otherwise set it to false.
const bool ShiftPWM_invertOutputs = false;

// You can enable the option below to shift the PWM phase of each shift register by 8 compared to the previous.
// This will slightly increase the interrupt load, but will prevent all PWM signals from becoming high at the same time.
// This will be a bit easier on your power supply, because the current peaks are distributed.
const bool ShiftPWM_balanceLoad = false;

#include <ShiftPWM.h>   // include ShiftPWM.h after setting the pins!

...

  // Sets the number of 8-bit registers that are used.
  ShiftPWM.SetAmountOfRegisters(numRegisters);
```

*New Code...*

```
// These are the Arduino pins connected to your shift register string
const int ShiftPWM_latchPin = 8;
const int ShiftPWM_dataPin  = 11;
const int ShiftPWM_clockPin = 13;

// The number of 8-bit shift registers connected 
const unsigned int ShiftPWM_numRegisters=5;       // The number of 8-bit shift registers connected

// If your LED's turn on if the pin is low, set this to true, otherwise set it to fals
const bool ShiftPWM_invertOutputs = false; 

// You can enable the option below to shift the PWM phase of each shift register by 8 compared to the previous.
// This will slightly increase the interrupt load, but will prevent all PWM signals from becoming high at the same time.
// This will be a bit easier on your power supply, because the current peaks are distributed.
const bool ShiftPWM_balanceLoad = false;

#include <ShiftPWM.h>   // include ShiftPWM.h after setting the pins!
```

So, in other words...

1. There are no more optinal defines for pins. You always just set the latch, data, and clock pin variables before the inlcude.
2. You now set the number of shift registers in ShiftPWM_numRegisters before the inlcude rather than with SetAmountOfRegisters() afterwards.


##Technical Details

To enable the new functionality, this version had to...

1. Eliminate alldependancies on SPI and Serial hardware.
2. Use the existing Timer0 interrupt for refresh triggers.
3. Statically allocate the buffer.
4. Define pin and device mapping for non-ATMEGA328 Arduinos.


### New Interrupt Scheme

The old version used a dedicate Timer to generate periodic interrupts to refresh the display.

The new version chains off the existing Timer0 that is already setup by Arduino to trigger at 1000Hz.

By getting rid of dependancies on the various timers, the code is simplified and no longer dependant on specific timer hardware being present or unused.

### New Bit Banging Scheme

The old version could either use the dedicate SPI hardware or could fall back on a much slower bit banging strategy.

The new version has an updated bit binging strategy that is muich faster and is almost as fast as the oringal running on the SPI hardware.

## FAQ

Q: Can I attach multipule strings of shift registers to a single controller using different clock and data lines?

A: Not in the current version of the code, but it would be possible to add something like that pretty easily.

Q: Can the bitbanging be any faster?

A: Yes, I think I could probably make it at least 2x faster with some careful coding. If you need it to be faster, open an issue and I'll see what I can do.
