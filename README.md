# Ultrahand Overlay (HOS 16.0.0+)
[![platform](https://img.shields.io/badge/platform-Switch-898c8c?logo=C++.svg)](https://gbatemp.net/forums/nintendo-switch.283/?prefix_id=44)
[![language](https://img.shields.io/badge/language-C++-ba1632?logo=C++.svg)](https://github.com/topics/cpp)
[![GPLv2 License](https://img.shields.io/badge/license-GPLv2-189c11.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![Latest Version](https://img.shields.io/github/v/release/ppkantorski/Ultrahand-Overlay?label=latest&color=blue)](https://github.com/ppkantorski/Ultrahand-Overlay/releases/latest)
[![GitHub Downloads](https://img.shields.io/github/downloads/ppkantorski/Ultrahand-Overlay/total?color=6f42c1)](https://somsubhra.github.io/github-release-stats/?username=ppkantorski&repository=Ultrahand-Overlay&page=1&per_page=300)
[![HB App Store](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/ppkantorski/Ultrahand-Overlay/main/.github/hbappstore.json&label=hb%20app%20store&color=6f42c1)](https://hb-app.store/switch/UltrahandOverlay)
[![GitHub issues](https://img.shields.io/github/issues/ppkantorski/Ultrahand-Overlay?color=222222)](https://github.com/ppkantorski/Ultrahand-Overlay/issues)
[![GitHub stars](https://img.shields.io/github/stars/ppkantorski/Ultrahand-Overlay)](https://github.com/ppkantorski/Ultrahand-Overlay/stargazers)

Create directories, manage files, and customize configurations effortlessly using simple ini files.

[![Ultrahand Logo](.pics/banner.gif)](https://gbatemp.net/threads/ultrahand-overlay-the-fully-craft-able-overlay-executor.633560/)

Ultrahand Overlay is an overlay menu ecosystem built from the ground up on [libultrahand](https://github.com/ppkantorski/libultrahand) (an expanded fork of [libtesla](https://github.com/WerWolv/libtesla)) that provides powerful C/C++ commands through its own custom interpretive [programming language](https://github.com/ppkantorski/Ultrahand-Overlay/wiki/Command-Reference) (similar to Shell/BASH). It is a versatile tool that enables users to create and share custom command-based packages, providing enhanced functionality for managing settings, files and directories on your Nintendo Switch.

With Ultrahand, you have the flexibility to customize and shape your file management system according to your needs, empowering you with greater control over your system configurations.

## Screenshots
![Slideshow](.pics/slideshow.gif)

## Features

### File and Directory Management

- **Create Directories** — Effortlessly create directories on your SD card by specifying the directory path.

- **Copy Files or Directories** — Easily copy files or directories from one location to another on your SD card.

- **Delete Files or Directories** — Simplify file and directory deletion. Specify the path of the file or directory you want to remove and Ultrahand handles it cleanly.

- **Move Files or Directories** — Seamlessly move files or directories between locations on your SD card.

- **Download Files** — Efficiently retrieve files from repositories or URLs directly to your SD card. Supports both standard and no-retry download modes, with live progress feedback in the UI.

- **Unzip Files** — Extract compressed zip archives on your SD card, preserving their original directory structure.

- **Modify INI Files** — Edit INI files with full control: update existing key-value pairs, add new entries, or create new sections.

- **Hex Edit Files** — Perform binary-level hexadecimal editing of files for precise data manipulation.

- **Convert Mods** — Convert `pchtxt` mods into `ips` or `cheats` format.

### Command System

Ultrahand's interpretive language supports a rich set of command modes, giving package authors fine-grained control over how each item behaves in the UI:

| Mode | Description |
|---|---|
| `default` | Standard one-shot command execution |
| `toggle` | Two-state ON/OFF toggle, state persisted to config |
| `hold` | Requires a button hold before executing (3-second progress bar with haptic pulses) |
| `slot` | Option picker backed by a config footer value |
| `option` | Selection list that updates a config footer value on confirm |
| `forwarder` | Launches another overlay or package directly |
| `text` | Displays static or dynamic text content |
| `table` | Renders a scrollable, pollable data table with configurable alignment and colors |
| `trackbar` | Continuous numeric slider with configurable min/max/units |
| `step_trackbar` | Stepped discrete slider |
| `named_step_trackbar` | Stepped slider with a named label for each step |

Commands also support **grouping** options (`split`, `split2` through `split5`) for multi-column layouts within a single command row.

### Conditional Execution

Package commands can be gated by system context at parse time using inline annotations:

- `;system=erista` / `;system=mariko` — restrict a command to a specific hardware revision
- `;hos_version=>=16.0.0` — version operator conditions on HOS (supports `==`, `>=`, `<=`, `>`, `<`)
- `;ams_version=` — same operators applied to the Atmosphère version
- `;state=on` / `;state=off` — conditional on a toggle's current state

### System Commands

A variety of system-level operations are available from within packages:

- Shutdown and reboot (including direct boot into Hekate entries and modes)
- Screen backlight control
- System volume adjustment
- Disconnect all Bluetooth controllers
- NTP time synchronization

### Boot and Exit Hooks

- **On-boot commands**: Place a `boot_package.ini` file in `/switch/.packages/` with a `[on-boot]` section to run commands once on each device boot. Individual packages support their own `boot_package.ini` with a `[boot]` section that runs when the package is first launched.
- **On-exit commands**: An `exit_package.ini` in `/switch/.packages/` with an `[exit]` section runs when Ultrahand is closed cleanly.

---

## Getting Started

### Usage

1. Download and install the latest [nx-ovlloader](https://github.com/ppkantorski/nx-ovlloader).
    - **Note:** Two variants are available. The standard `nx-ovlloader` has a 4MB overlay heap. You can switch between overlay heap sizes from within the Ultrahand Settings menu for expanded features.
2. Download the latest [`ovlmenu.ovl`](https://github.com/ppkantorski/Ultrahand-Overlay/releases/latest/download/ovlmenu.ovl) and place it at `/switch/.overlays/ovlmenu.ovl`.
    - **Warning:** This will overwrite `Tesla Menu` if it is already installed.
3. After first launch, Ultrahand will create `/config/ultrahand/` on your SD card along with a `config.ini` file containing global settings.
4. Launch Ultrahand using its default hotkey combo (`ZL+ZR+DDOWN`) or any of the Tesla-compatible combos. A `/switch/.packages/` directory will be created with a preset `package.ini` for your base menu commands.
5. Place your custom `package.ini` in `/switch/.packages/<PACKAGE_NAME>/`. This file contains the commands for your package.
    - **Note:** If your package does not appear, you may need to run *Fix Bit Archive* in Hekate.
    - See [Ultrahand Packages](https://github.com/ppkantorski/Ultrahand-Packages) for a list of known packages.
6. Your commands will now appear in the packages menu inside Ultrahand.

---

## UI and Navigation

- Press `A` to execute any command.
- Press `MINUS` on a command to view and execute its individual command lines.
- Press `X` on an overlay or package to star it (moves it to a hidden favorites section).
- Press `Y` on an overlay or package to open its configuration settings.
- Press `PLUS` from the main menu to enter the Ultrahand Settings menu.
- Press `R` during a running command to abort the operation.
- Press `B` during a running command to dismiss the overlay without canceling.

---

## Settings

The Ultrahand Settings menu (accessible via `PLUS` on the main screen) exposes the following options:

### General
- **Key Combo** — Choose the button combination used to open Ultrahand from any of the supported default combos.
- **Language** — Select the UI language. Languages are loaded from JSON files in `/config/ultrahand/lang/`. English is always available.
- **Notifications** — Configure notification behavior, including silence, startup notification, API-triggered notifications, API toggle hotkeys, and the maximum number of simultaneous notification toasts.
- **System** — Displays device info (Erista/Mariko, memory vendor/model, CPU/GPU fuse data) and exposes an **Overlay Memory** slider (4 MB / 6 MB / 8 MB heap) to tune the memory available to the overlay.
- **Software Update** — Check for and install the latest Ultrahand release directly from within the overlay.

### UI Settings
- **Theme** — Select a UI theme from `.ini` files in `/config/ultrahand/themes/`. The built-in `ultra` and `ultra-blue` themes are downloadable from within the menu.
- **Sounds** — Select a sound-effect pack from `/config/ultrahand/.sounds/`.
- **Wallpaper** — Select a background wallpaper from `/config/ultrahand/wallpapers/` (requires 6 MB+ heap).
- **Widget** — Toggle individual status bar elements (clock, SoC temperature, PCB temperature, battery indicator, backdrop, border) and adjust widget display settings (dynamic colors, center alignment, extended backdrop).
- **Miscellaneous** — Granular feature toggles including:
  - Launch combos, haptic feedback, auto NTP sync, opaque screenshots
  - Page swap, page recall, launch recall
  - User guide visibility, hidden items visibility, delete action visibility
  - Overlay/package version label display and formatting
  - Dynamic logo, selection background/text/value highlighting
  - libultrahand and package title/version display modes

---

## Per-Overlay and Per-Package Key Combos

Each overlay and package can be assigned its own launch combo independently of the global Ultrahand hotkey. Combos are deconflicted automatically: assigning a combo to one overlay or package removes it from any other that was using it. Mode-specific combos (`mode_combos`) are also supported for multi-mode overlays.

---

## Compilation Prerequisites

To compile and run the software, you need:

- [devkitPro](https://devkitpro.org) with `devkitA64` and `libnx`
- [libultrahand](https://github.com/ppkantorski/libultrahand)
- `switch-curl`
- `switch-zlib`
- `switch-mbedtls`

```sh
export DEVKITPRO=/opt/devkitpro
make -j6
```

The build targets C++26 and ARMv8-A. Output is `ovlmenu.ovl`.

---

## Contributing

Contributions are welcome! If you have any ideas, suggestions, or bug reports, please raise an [issue](https://github.com/ppkantorski/Ultrahand-Overlay/issues/new/choose), submit a [pull request](https://github.com/ppkantorski/Ultrahand-Overlay/compare) or reach out to me directly on [GBATemp](https://gbatemp.net/threads/ultrahand-overlay-the-fully-craft-able-overlay-executor.633560/).

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/X8X3VR194)

## License

This project is licensed and distributed under [GPLv2](LICENSE) with a [custom library](https://github.com/ppkantorski/libultrahand/tree/main/libultra) utilizing [CC-BY-4.0](SUB_LICENSE).

Copyright (c) 2023-2026 ppkantorski
