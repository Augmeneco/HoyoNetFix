# libhoyonetfix - Network Fix for Hoyoverse Games

A library that fixes launch/cpu-related issues in Hoyoverse games (Genshin Impact and Zenless Zone Zero) by being preloaded with LD_PRELOAD. 

## Problem Description

Currently, Genshin Impact and Zenless Zone Zero have a bug where:
- The game won't launch unless you temporarily disable internet during launch
- You must wait about 10 seconds before re-enabling internet
- Otherwise, the game will unnecessarily load the CPU without offline launch

This library fixes these issues by temporarily blocking network access during game launch.

## Installation

Compile the library with:
```bash
gcc -shared -fPIC -o libhoyonetfix.so hoyonetfix.c -ldl
```

## Usage

**Important**: The library file should be placed in any directory EXCEPT the game's installation folder to avoid problems with the game's anti-cheat system.

Preload the library when launching the game:
```bash
LD_PRELOAD=/path/to/libhoyonetfix.so wine game.exe
```

## Compatibility

**Note on Proton compatibility**:  
This fix does not work with Steam Proton and GE-Proton because `steam.exe` inside the prefix requires internet access during launch. Instead, use either:
- Direct launch with Wine (with Lutris, .sh and etc)
- [UMU Proton](https://github.com/Open-Wine-Components/umu-launcher) inside steam

**Steam Deck verification**:  
The fix has been tested and confirmed working on Steam Deck using UMU Proton.

### Configuration

You can set the network blocking duration (in seconds) using the `HOYO_TIMEOUT` environment variable. The optimal timeout value may vary depending on your system configuration - you may need to experiment with different values (try 10-15 seconds range). If not specified, it defaults to 10 seconds:

```bash
# With custom timeout (15 seconds):
HOYO_TIMEOUT=15 LD_PRELOAD=/path/to/libhoyonetfix.so wine game.exe

# With default 10 second timeout:
LD_PRELOAD=/path/to/libhoyonetfix.so wine game.exe
```

## How It Works

The library:
1. Temporarily blocks network connections during game launch
2. Intercepts network-related system calls (connect, send, recv, gethostbyname)
3. Returns network errors during the blocking period
4. Restores normal network operation after the timeout period

### Anti-Cheat Compatibility

This solution is safe to use with game anti-cheat systems because:
- It only modifies network calls at the Linux/Wine level
- The game's anti-cheat runs at a higher level and cannot detect these modifications
- No game files are modified - the changes happen in memory during runtime
