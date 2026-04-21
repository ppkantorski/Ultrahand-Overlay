# Ultrahand Overlay (HOS 16.0.0+)

[![platform](https://img.shields.io/badge/platform-Switch-898c8c?logo=C++.svg)](https://gbatemp.net/forums/nintendo-switch.283/?prefix_id=44)
[![language](https://img.shields.io/badge/language-C++-ba1632?logo=C++.svg)](https://github.com/topics/cpp)
[![GPLv2 License](https://img.shields.io/badge/license-GPLv2-189c11.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![Latest Version](https://img.shields.io/github/v/release/ppkantorski/Ultrahand-Overlay?label=latest&color=blue)](https://github.com/ppkantorski/Ultrahand-Overlay/releases/latest)
[![GitHub Downloads](https://img.shields.io/github/downloads/ppkantorski/Ultrahand-Overlay/total?color=6f42c1)](https://somsubhra.github.io/github-release-stats/?username=ppkantorski&repository=Ultrahand-Overlay&page=1&per_page=300)
[![HB App Store](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/ppkantorski/Ultrahand-Overlay/main/.github/hbappstore.json&label=hb%20app%20store&color=6f42c1)](https://hb-app.store/switch/UltrahandOverlay)
[![GitHub issues](https://img.shields.io/github/issues/ppkantorski/Ultrahand-Overlay?color=222222)](https://github.com/ppkantorski/Ultrahand-Overlay/issues)
[![GitHub stars](https://img.shields.io/github/stars/ppkantorski/Ultrahand-Overlay)](https://github.com/ppkantorski/Ultrahand-Overlay/stargazers)

**Ultrahand Overlay** is a fully scriptable overlay menu ecosystem for the Nintendo Switch. Accessible instantly via hotkey from any game or application, it provides a powerful custom command language for managing files, configurations, and system settings, all without leaving your game.

[![Ultrahand Logo](.pics/banner.gif)](https://gbatemp.net/threads/ultrahand-overlay-the-fully-craft-able-overlay-executor.633560/)

Built on [libultrahand](https://github.com/ppkantorski/libultrahand) (an expanded fork of [libtesla](https://github.com/WerWolv/libtesla)), Ultrahand is a full drop-in replacement for Tesla Menu, every existing Tesla overlay (`.ovl`) works without modification.

---

## Screenshots

![Slideshow](.pics/slideshow.gif)

---

## Features

### For Users
- Instantly accessible from any game via hotkey or swipe gesture — no game suspension required
- Launch and manage other overlays with per-overlay key combos
- Install and run community packages from [Ultrahand Packages](https://github.com/ppkantorski/Ultrahand-Packages)
- Control volume, backlight, and system settings on the fly
- Real-time progress feedback for downloads, copies, and installs
- Full touch support alongside controller input
- Customizable themes, wallpapers, sound packs, and UI layout

### For Package Devs
A rich INI-based scripting environment with:
- **File operations** — copy, move, delete, download, unzip, mirror, touch
- **Config patching** — INI, JSON, and binary hex editing
- **Mod conversion** — `.pchtxt` to `.ips` or Atmosphere cheat format
- **System control** — reboot targets, shutdown, volume, backlight
- **Dynamic UI** — toggles, sliders, dropdowns, tables, notifications
- **Placeholders & expressions** — math, string manipulation, timestamps, hardware info
- **Conditional logic** — `try:` blocks, `path_exists`, hardware/version guards

See the [Wiki](https://github.com/ppkantorski/Ultrahand-Overlay/wiki) for full documentation.

---

## Installation

### Quick Install (Recommended)

Download the latest [`sdout.zip`](https://github.com/ppkantorski/Ultrahand-Overlay/releases/latest/download/sdout.zip) and extract it to the root of your SD card. It includes everything needed: `nx-ovlloader`, `ovlmenu.ovl`, language files, themes, sound packs, and the required folder structure.

### Manual Install

1. Download and install the latest [nx-ovlloader](https://github.com/ppkantorski/nx-ovlloader).
2. Place [`ovlmenu.ovl`](https://github.com/ppkantorski/Ultrahand-Overlay/releases/latest/download/ovlmenu.ovl) at `/switch/.overlays/ovlmenu.ovl`.
   > **Warning:** This will replace Tesla Menu if it is installed.
3. Launch Ultrahand with the default hotkey (`ZL+ZR+DDOWN`) or any Tesla-compatible combo.
4. On first launch, Ultrahand creates `/config/ultrahand/` and generates a starter `package.ini` at `/switch/.packages/`.

> If a package does not appear in the menu, try running *Fix Bit Archive* in Hekate.

For available community packages, see [Ultrahand Packages](https://github.com/ppkantorski/Ultrahand-Packages).

---

## Navigation

| Input | Action |
|---|---|
| `A` | Execute the selected command |
| `MINUS` | View and run individual command lines (Script Overlay) |
| Long-tap (touch) | Open command line view or overlay/package settings |
| `X` | Star/favorite an overlay or package |
| `Y` | Open overlay/package settings |
| `PLUS` | Open Ultrahand Settings from the main menu |
| `L` / `R` | Jump to top / bottom of the current list |
| `ZL` / `ZR` | Page up / down (hold for rapid scrolling) |
| `R` during command | Abort the running operation |
| `B` during command | Dismiss the overlay without canceling |
| Swipe inward from edge | Open Ultrahand (alternative to key combo) |

---

## Settings Overview

Access the Settings menu by pressing `PLUS` from the main screen.

- **Key Combo** — Configure the hotkey used to open Ultrahand
- **Language** — Select UI language (loaded from `/config/ultrahand/lang/`)
- **Notifications** — Configure toast notification behavior and slot limits
- **System** — View device info and adjust overlay memory heap size (4 / 6 / 8 MB)
- **Software Update** — Check for and install updates from within the overlay
- **Theme** — Select a theme from `/config/ultrahand/themes/`
- **Sounds** — Select a sound-effect pack from `/config/ultrahand/.sounds/`
- **Wallpaper** — Set a background wallpaper (requires 6 MB+ heap; `.rgba` format, 448×720 px)
- **Widget** — Toggle status bar elements (clock, temps, battery, backdrop, border)
- **Miscellaneous** — Granular toggles for swipe-to-open, haptics, NTP sync, page layout, and more

Per-overlay and per-package launch combos can be assigned independently via the overlay/package settings menu (`Y`).

---

## Writing Packages

Packages live in `/switch/.packages/<YOUR_PACKAGE_NAME>/` and are configured with a `package.ini` file. A minimal example:

```ini
;title='My Package'
;version=1.0.0
;creator=YourName

[Copy Config]
copy /switch/.packages/package.ini /config/mypackage/

[Reboot to Hekate]
;hold=true
reboot hekate
```

For complete documentation on the package format, all available commands, placeholder variables, and command modes, see the [Wiki](https://github.com/ppkantorski/Ultrahand-Overlay/wiki):

- **[Package Reference](https://github.com/ppkantorski/Ultrahand-Overlay/wiki/Package-Reference)** — Package structure, headers, pages, boot/exit hooks, and configuration
- **[Command Reference](https://github.com/ppkantorski/Ultrahand-Overlay/wiki/Command-Reference)** — All commands, modes, source functions, and placeholder variables

For real-world package examples, see the [`examples/`](https://github.com/ppkantorski/Ultrahand-Overlay/tree/main/examples) directory.

---

## Building from Source

**Prerequisites:**

- [devkitPro](https://devkitpro.org) with `devkitA64` and `libnx`
- [libultrahand](https://github.com/ppkantorski/libultrahand)
- `switch-curl`, `switch-zlib`, `switch-minizip`, `switch-mbedtls`

```sh
export DEVKITPRO=/opt/devkitpro
make
```

The Makefile auto-detects available CPU cores for parallel LTO compilation. Targets C++26 and ARMv8-A. Output is `ovlmenu.ovl`.

---

## Contributing

Contributions are welcome. Please open an [issue](https://github.com/ppkantorski/Ultrahand-Overlay/issues/new/choose) or submit a [pull request](https://github.com/ppkantorski/Ultrahand-Overlay/compare). You can also reach out on [GBATemp](https://gbatemp.net/threads/ultrahand-overlay-the-fully-craft-able-overlay-executor.633560/).

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/X8X3VR194)

---

## License

Licensed under [GPLv2](LICENSE) with a [custom library](https://github.com/ppkantorski/libultrahand/tree/main/libultra) under [CC-BY-4.0](SUB_LICENSE).

Copyright © 2023–2026 ppkantorski
