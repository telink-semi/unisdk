# RF Demo Application

## Overview

The sample demonstrates the RF driver capabilities by implementing the ping-pong application.

This requires having two devices for communication:
- **Transmitter** - sends the **ping** packet to the **Receiver**, starting listen to **pong** response. To avoid deadlocking, it has a timeout to resend the **ping** in case of the absence of the **pong** response.
- **Receiver** - infinitely waits for the **ping** packet to be able to synchronize with the **Transmitter**.
Once received, it sends the **pong** response.

So each communication loop can be described by a scheme:
> **Transmitter** ---ping--> **Receiver**  
> **Transmitter** <--pong--- **Receiver**  

The LED1 is used mainly for the PM mode to determine whether the chip is on or off.
The LED2 is flipped each time the **pong** packet is sent by the **Receiver** or once the **Transmitter**'s receive ends, regardless of the result.
