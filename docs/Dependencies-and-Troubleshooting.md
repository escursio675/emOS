
# Dependencies

## ST-Link tools
- `sudo apt install --reinstall stlink-tools`
- VSCode with extensions:
  - C/C++
  - Cortex-Debug

### Verify
```bash
which st-info st-flash st-util
ls /etc/udev/rules.d/ | grep -i stlink
```
- board actually detected
```bash
st-info --probe
```
- GDB server
```bash
st-util
```

## Other dependencies
- `arm-none-eabi-gcc`, `arm-none-eabi-gdb`, `arm-none-eabi-objcopy` — from the Arm GNU Toolchain tarball
- `make`
- `openocd`
- `picocom`
- `dialout` group membership (for `/dev/ttyACM0` access without sudo)

## ARM Toolchain Setup

Install the official Arm GNU Toolchain (do NOT use `apt install gcc-arm-none-eabi` — Ubuntu 22.04's repo version is outdated and can misbehave with newer libopencm3/CMSIS headers):

```bash
cd ~/Downloads
wget https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz
sudo tar xf arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz -C /opt
```

**Check the extracted folder name exactly before setting PATH** — Arm's tarball may unpack with a capitalized `Rel1` (e.g. `arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi`) rather than lowercase `rel1`. Linux paths are case-sensitive, so a mismatch here will silently break `which arm-none-eabi-gcc` later.

```bash
ls /opt/ | grep -i arm
```

Add the **exact** matching path to `~/.bashrc` (adjust casing to match what `ls` showed):

```bash
echo 'export PATH="/opt/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

### Verify
```bash
which arm-none-eabi-gcc
which arm-none-eabi-gdb
which arm-none-eabi-objcopy
which arm-none-eabi-size
arm-none-eabi-gcc --version
```

All four `which` commands should return a path under `/opt/arm-gnu-toolchain-.../bin/`.

## Fixing `arm-none-eabi-gdb` (required for Cortex-Debug)

The prebuilt GDB binary in the Arm GNU Toolchain has two dependency issues on a stock Ubuntu 22.04 install. Both must be fixed for Cortex-Debug (and manual GDB debugging) to work.

## Issue 1 — missing `libncursesw.so.5`

Ubuntu 22.04 only ships ncurses version 6; the toolchain's GDB was built against version 5.

**Symptom:**
```
arm-none-eabi-gdb: error while loading shared libraries: libncursesw.so.5: cannot open shared object file: No such file or directory
```

**Fix** — confirm you only have `.so.6` on disk, then symlink it to satisfy the version-5 name:
```bash
find / -name "libncursesw.so*" 2>/dev/null
sudo ln -s /usr/lib/x86_64-linux-gnu/libncursesw.so.6 /usr/lib/x86_64-linux-gnu/libncursesw.so.5
```

If GDB still errors afterward, also symlink `libtinfo`:
```bash
find / -name "libtinfo.so*" 2>/dev/null
sudo ln -s /usr/lib/x86_64-linux-gnu/libtinfo.so.6 /usr/lib/x86_64-linux-gnu/libtinfo.so.5
```

## Issue 2 — GDB's embedded Python looks for Python 3.8

The prebuilt GDB has Python 3.8 support baked in at build time (expects the standard library at `/usr/lib/python3.8`). Ubuntu 22.04 ships Python 3.10 by default, so GDB's internal Python interpreter fails to initialize.

**Symptom:**
```
Fatal Python error: init_fs_encoding: failed to get the Python codec of the filesystem encoding
ModuleNotFoundError: No module named 'encodings'
```

**Fix** — install real Python 3.8 alongside your system Python (a snap-packaged `python38` will NOT work, since snaps are sandboxed and GDB can't see inside them):
```bash
sudo add-apt-repository ppa:deadsnakes/ppa
sudo apt update
sudo apt install python3.8
```

Then set `PYTHONHOME` so GDB reliably points at the right prefix, and make it persistent:
```bash
echo 'export PYTHONHOME=/usr' >> ~/.bashrc
source ~/.bashrc
```

### Verify both fixes
```bash
arm-none-eabi-gdb --version
```
Should print normal version info with no shared-library or Python errors.

**Important:** fully close and reopen VSCode after these fixes so its integrated terminal and the Cortex-Debug extension pick up the updated `PATH`/`PYTHONHOME` — VSCode does not always inherit a freshly-edited `~/.bashrc` from an already-running session.

## Cortex-Debug VSCode configuration

`cortex-debug.armToolchainPath` (user or workspace settings.json)

Cortex-Debug does not inherit your shell's `PATH`, so it needs the toolchain's `bin/` directory set explicitly:

```json
"cortex-debug.armToolchainPath": "/opt/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi/bin"
```

Add this as a top-level key in your global `settings.json` (Ctrl+Shift+P → "Preferences: Open User Settings (JSON)"), or scope it per-project in `.vscode/settings.json`.

 `c_cpp_properties.json` — fix `compilerPath`

If IntelliSense complains it can't find the compiler, the repo's default config usually points at `/usr/bin/arm-none-eabi-gcc`, which doesn't exist on this setup. Update it:

```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/bootloader/inc"
            ],
            "defines": [
                "STM32F4",
                "__ARM_ARCH_7EM__"
            ],
            "compilerPath": "/opt/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc",
            "cStandard": "c99",
            "cppStandard": "c++14",
            "intelliSenseMode": "gcc-arm"
        }
    ],
    "version": 4
}
```

This only affects IntelliSense (editor squiggles/autocomplete) — it does not affect whether `make` or Cortex-Debug actually build/flash correctly, since those use their own configured paths independently.

## Verifying the full chain manually (before trusting Cortex-Debug)

Useful as a standalone sanity check if Cortex-Debug ever misbehaves — isolates whether an issue is in VSCode's integration or in the underlying toolchain/board.

**Terminal 1:**
```bash
st-util
```
Should print the detected chip/memory info and `Listening at *:4242...`.

**Terminal 2:**
```bash
cd app
arm-none-eabi-gdb firmware.elf
```
Inside GDB:
```
target extended-remote localhost:4242
load
continue
```

**To exit cleanly:**
- `Ctrl-C` in the GDB terminal to halt execution and return to the `(gdb)` prompt
- `quit` (confirm with `y` if prompted) to exit GDB
- `Ctrl-C` in the `st-util` terminal to stop the GDB server

The flashed firmware keeps running on the board independently after you disconnect — quitting GDB/st-util does not stop or erase it.
