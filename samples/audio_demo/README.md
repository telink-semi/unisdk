# Audio driver sample

## Overview
This demo allows you to record some voice using the onboard microphone or line input and play it via line out.
> **Note:** You need speakers or headphones to be able to hear the recorded sample.

## How it works

1. Pressing **button0** erases reserved flash storage to be ready to save the audio sample and then starts the actual recording. Each time the RAM buffer is filled, its content is dumped into flash. Once you press **button0** again or the reserved space is full, the recording is stopped.
2. Pressing **button1** fills the RAM buffer and starts the playback. Each time the RAM buffer is played, it is filled with new data from flash. Once you press **button1** again or the recording ends, the playback is stopped.

## LED indication

|LED0|LED1|State|
|---|---|---|
|off|off|idle|
|on|off|recording|
|off|on|playing|
|on|on|erasing|

> **Note:** More detailed info about the current state is provided via logs.
