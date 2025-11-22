# killerbar

A status-bar inspired by [slstatus](https://suckless.org/tools/sltatus) and [dwmblocks](https://github.com/torrinfail/dwmblocks).

This program works by setting the root name in X, and is intended for use with the [dwm](https://suckless.org/dwm) window manager.

## Features
The program runs in an event loop which synchronously handles incoming signals. In **killerbar**, each 'block' is assigned an unique real-time signal (numbered 34-64 on modern Linux kernels). Assignment is done automatically during compilation in top-to-bottom fashion; blocks first in order receive the smallest numbered signals (which are consequently prioritized by the program).

Block execution occurs either continuously or intermittently (manually):
### Continuous execution
Blocks with a specified timer interval will execute at each timer expiration. The initial timer value can be set to an arbitrary value in this scenario (giving tardy functions some runway could be a good idea, however).
### Intermittent execution
Blocks for which the interval is set to 0 will only execute when their respectively assigned signal is received and handled by the program. Specifying an initial value will be interpreted as the screen _lifetime_ of the block; after the specified time has passed, the block will be unrendered. For these blocks, the initial time can be set to 0, which will cause the block to remain rendered between executions.

## Configuration
Block configuration is done by editing the `block` array in the `main.c` source file (where you will find examples). Time is specified in seconds as floating-point values (with nanosecond precision) for each block using the `TIME(interval, initial)` macro.

This status-bar is designed to be compatible with the helper-functions included in slstatus. Using the `run_command` function you can execute a shell command and print its output to the statusline.

## Install
`git clone https://github.com/ljfogelstrom/killerbar`\
`cd killerbar`\
`sudo make install`

## Usage
Add
`killerbar &`
to your `.xinitrc` to start the program alongside `dwm` (or another compatible window manager)
