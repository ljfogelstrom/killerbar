# killerbar

A status-bar based off of [slstatus](https://suckless.org/tools/sltatus).

This program works by setting the root name in X, and is intended for use with the [dwm](https://suckless.org/dwm) window manager.

## Features
The program runs in an event loop which synchronously handles incoming signals. In **killerbar**, each 'block' is assigned an unique real-time signal (numbered 34-64 on modern Linux kernels). Assignment is done automatically during compilation, the blocks receiving their respective signal in ascending order.

Block execution occurs either continuously or intermittently (manually):
### Continuous execution
Blocks with a specified timer interval will execute at each timer expiration, whereby the initial timer value can be set to an arbitrary value.
### Intermittent execution
Blocks for which the interval is set to 0 will only execute when their respectively assigned signal is received and handled by the program. Specifying an initial value will be interpreted as the screen _lifetime_ of the block; after the specified time has passed, the block will be unrendered. For these blocks, the initial time can be set to 0, which will cause the block to remain rendered indefinitely.

## Configuration
Block configuration is done by editing the `block` array in `config.h`. Time is specified in seconds as floating-point values for each block using the `TIMER(interval, initial)` macro.

This status-bar is designed to be compatible with the helper-functions included in slstatus. Using the `run_command` function you can execute a shell command and print its output to the statusline.

## Install
```
git clone https://github.com/ljfogelstrom/killerbar
cd killerbar
sudo make install
```


## Usage
Add
`killerbar &`
to your `.xinitrc` to start the program alongside `dwm` (or another compatible window manager)
