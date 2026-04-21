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

Ultrahand Overlay is a fully scriptable overlay menu ecosystem for the Nintendo Switch, built from the ground up on [libultrahand](https://github.com/ppkantorski/libultrahand) (an expanded fork of [libtesla](https://github.com/WerWolv/libtesla)). It runs as a system overlay — accessible instantly via hotkey from **any game or application without closing it** — and provides a powerful custom interpretive [command language](https://github.com/ppkantorski/Ultrahand-Overlay/wiki/Command-Reference) (similar to Shell/BASH) for managing files, configurations, and system settings directly on your Nintendo Switch.

**For users**, Ultrahand is a feature-rich overlay hub: launch and manage other overlays, install and run community packages, control volume and backlight, trigger reboots, and get real-time feedback on downloads and file operations — all without leaving your game. It is a full drop-in replacement for Tesla Menu, so every existing Tesla overlay (`.ovl`) works without modification.

**For package authors**, Ultrahand exposes a rich scripting environment: copy, move, delete, download, and unzip files; patch INI, JSON, and binary files; convert mods; trigger notifications; run hardware-conditional commands; and control virtually every aspect of the UI with a simple INI-based syntax. Packages can be shared and installed from the community library at [Ultrahand Packages](https://github.com/ppkantorski/Ultrahand-Packages).

## Screenshots
![Slideshow](.pics/slideshow.gif)

## Features

### File and Directory Management

- **Create Directories** — Effortlessly create directories on your SD card by specifying the directory path.

- **Copy Files or Directories** — Easily copy files or directories from one location to another on your SD card.

- **Delete Files or Directories** — Simplify file and directory deletion. Specify the path of the file or directory you want to remove and Ultrahand handles it cleanly.

- **Move Files or Directories** — Seamlessly move files or directories between locations on your SD card.

- **Mirror Copy / Mirror Delete** — `mirror_copy <SOURCE> <DESTINATION>` mirrors the structure of a source directory into a destination; `mirror_delete <SOURCE> <DESTINATION>` removes from the destination any files that exist in the source. Both commands support wildcard patterns.

- **Download Files** — Efficiently retrieve files from repositories or URLs directly to your SD card. Supports both standard (3-retry) and no-retry download modes, with live progress feedback in the UI. Download commands automatically perform a one-time NTP sync to `pool.ntp.org` on first use to prevent SSL handshake failures caused by clock desynchronization (can be disabled via `Auto NTP Sync` in Settings).

- **Unzip Files** — Extract compressed zip archives on your SD card, preserving their original directory structure.

- **Modify INI Files** — Edit INI files with full control: update existing key-value pairs, add new entries, or create new sections.

- **Hex Edit Files** — Perform binary-level hexadecimal editing of files for precise data manipulation.

- **Convert Mods** — `pchtxt2ips <PATH_TO_PCHTXT_FILE> <OUTPUT_FOLDER>` converts `.pchtxt` patch files into `.ips` binaries. `pchtxt2cheat <PATH_TO_PCHTXT_FILE> [CHEAT_NAME]` converts `.pchtxt` files into cheat format, automatically placing them in the correct directory for the running game (the game's title ID must be present in the pchtxt).

- **Touch File** — `touch <PATH>` creates an empty file at the specified path if it does not already exist.

- **Dot-Clear** — `dot-clear <PATH>` recursively removes all macOS metadata files (those beginning with `._`) from the specified directory and all its subdirectories. Useful for SD cards populated on macOS where these files can interfere with mods.

- **Logging** — `logging` writes commands and errors to `log.txt` within the package's folder, bypassing the per-package `Error Logging` toggle. Useful for debugging during development.

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

Command argument strings support `{value}` and `{index}` placeholders, which are substituted at runtime with the current footer/selection value and its list index respectively.

For a full reference of all available commands, see the [Command Reference wiki](https://github.com/ppkantorski/Ultrahand-Overlay/wiki/Command-Reference).

#### Refresh and Navigation Commands

- `refresh` — re-renders the current page. Called as `refresh <ITEM_NAME> [ITEM_VALUE]` to also jump the cursor to the matching item after redraw.
- `refresh-to <ITEM_NAME> [ITEM_VALUE] [EXACT_MATCH]` — like `refresh` with cursor jump, but with an optional `true`/`false` exact-match flag (defaults to `true`; set to `false` for substring matching).

### Placeholder Variables

Package commands support a rich set of runtime placeholder variables enclosed in `{…}`. Supported placeholders include:

**Value & state**
- `{value}` — the current footer/selection value of the enclosing command entry
- `{index}` — the zero-based index of the current selection
- `{title_id}` — the title ID of the foreground game/application; returns `null` if none is running
- `{local_ip}` — the device's current local IP address
- `{backlight}` — the current handheld backlight level
- `{volume}` — the current master volume level
- `{package_version}` — the version of the package the command belongs to

**Hardware info**
- `{cpu_speedo}`, `{cpu_iddq}`, `{gpu_speedo}`, `{gpu_iddq}`, `{soc_speedo}`, `{soc_iddq}` — CPU/GPU/SoC fuse data values

**Time**
- `{timestamp(<STRFTIME_FORMAT>)}` — current date/time formatted with a `strftime`-style string, e.g. `{timestamp(%Y-%m-%d)}`. Can also be called as `{timestamp}` with no arguments for a default format.

**Math & string functions**
- `{math(<EXPRESSION>)}` — evaluates a mathematical expression (`+`, `-`, `*`, `/`, `%`, parentheses). Append `, true` to force integer output: `{math(5/2, true)}` → `2`.
- `{random(<START>, <END>)}` — generates a random integer in the given inclusive range
- `{length(<STRING>)}` — returns the character count of the given string (leading/trailing whitespace trimmed)
- `{slice(<STRING>, <START>, <END>)}` — returns a substring by start and end index

**Hex conversion**
- `{decimal_to_hex(<n>)}` — converts a decimal string to hex
- `{hex_to_decimal(<n>)}` — converts a hex string to decimal
- `{ascii_to_hex(<s>)}` — converts an ASCII string to its hex representation
- `{hex_to_rhex(<h>)}` — reverses the endianness of a hex string

**Button symbols** — render the corresponding glyph in the overlay font:
- `{A}`, `{B}`, `{X}`, `{Y}`, `{L}`, `{R}`, `{ZL}`, `{ZR}`
- `{DUP}`, `{DDOWN}`, `{DLEFT}`, `{DRIGHT}`
- `{LS}`, `{RS}`, `{PLUS}`, `{MINUS}`
- `{UP_ARROW}`, `{DOWN_ARROW}`, `{LEFT_ARROW}`, `{RIGHT_ARROW}`, diagonal variants
- `{POWER}`, `{HOME}`, `{CAPTURE}`, `{REFRESH_SYMBOL}`, `{WARNING_SYMBOL}`, `{INFO_SYMBOL}`

Placeholders are resolved in order and support nesting, so you can compose them freely (e.g. `{math({index}+1)}`).

### Conditional Execution

Package commands support several mechanisms for conditional and hardware-specific execution:

**Inline annotations** (apply to the single command they accompany):
- `;system=erista` / `;system=mariko` — restrict a command to a specific hardware revision
- `;state=handheld` / `;state=docked` — draw the command only in handheld or docked mode respectively
- `;state=on` / `;state=off` — conditional on a toggle's current state
- `;hos_version=>=16.0.0` — version operator conditions on HOS (supports `==`, `>=`, `<=`, `>`, `<`)
- `;ams_version=` — same operators applied to the Atmosphère version

All of the above annotations can be combined on a single command.

**Block labels** (flip all following commands in the section to that hardware variant):
- `erista:` — all commands after this label run only on Erista hardware
- `mariko:` — all commands after this label run only on Mariko hardware

**Runtime conditionals** (gate subsequent execution on file system state):
- `path_exists <PATH>` — marks success only if the file or directory exists; subsequent commands in a `try:` block are skipped if this fails
- `!path_exists <PATH>` — marks success only if the file or directory does not exist

**Error handling**:
- `try:` — begins a conditional block. `commandSuccess` is reset to `true` at the `try:` label, and any command in the block that fails causes all remaining commands in that block to be skipped. A new `try:` label starts a fresh block. The overall command sequence continues regardless of whether the block succeeded or failed.

### System Commands

A variety of system-level operations are available from within packages:

- **Shutdown** — `shutdown` performs a clean system shutdown. `shutdown controllers` disconnects all Bluetooth controllers without shutting down.
- **Reboot** — On Erista and supported Mariko hardware, a bare `reboot` with no arguments reboots to the Hekate bootloader. On unsupported hardware it performs a plain firmware reboot. Additional reboot targets:
  - `reboot HEKATE` — reboot directly into the Hekate bootloader menu
  - `reboot UMS` — reboot into Hekate UMS (SD card USB mass storage) mode
  - `reboot boot <NAME or INDEX>` — reboot to a named or indexed Hekate boot config entry
  - `reboot ini <NAME or INDEX>` — reboot to a named or indexed Hekate INI config entry
  - `reboot <PAYLOAD_PATH>` — reboot directly to a specific payload file on the SD card
- **Backlight** — Controls the screen backlight:
  - `backlight <0–100>` — sets brightness as a percentage
  - `backlight on` / `backlight off` — turns the backlight fully on or off
  - `backlight auto on` / `backlight auto off` — enables or disables automatic brightness control
- **Volume** — `volume <0–150>` sets the system master volume as a percentage. Values above 100 amplify beyond the normal maximum (requires the bundled `audio_mastervolume` Atmosphere patch, included in `sdout.zip`).
- **Open Overlay** — `open <OVERLAY_PATH> [ARGS]` launches another overlay programmatically from within a package command, with optional launch arguments.
- **Notify** — Displays an in-overlay toast notification from within a package command:
  - `notify <MESSAGE> [FONT_SIZE] [ALIGNMENT] [SPLIT_TYPE] [DURATION_MS] [TITLE] [SHOW_TIME] [APP_NAME]`
  - `notify-now` follows the same signature but always displays in slot 0 with immediate priority, bypassing the queue.
  - Parameters in `[…]` are optional and positional. `ALIGNMENT` is `left`, `center`, or `right`; `SPLIT_TYPE` is `word` or `char`; `DURATION_MS` of `0` keeps the notification visible until dismissed.
- **Exec** — `exec <SECTION>` runs the named section from the current package's `boot_package.ini`. `exec <SECTION> <PACKAGE_INI_FILE_PATH>` runs the named section from an explicitly specified INI file. Useful for reusing shared command blocks across multiple package entries.
- **Exit** — `exit` closes the overlay cleanly. `exit overlays` closes the overlay and navigates back to the overlays tab when next opened; `exit packages` does the same for the packages tab.

#### Safe Atmosphere Updates via Reboot

When a `reboot` command is issued and Ultrahand detects that protected Atmosphere files (`atmosphere/package3`, `atmosphere/stratosphere.romfs`) have been staged for an update (marked with a `.ultra` extension by a package), Ultrahand will boot into `ultrahand_updater.bin` instead of performing a plain reboot. This payload safely applies the pending update before the system comes back up. The payload is stored at `/config/ultrahand/payloads/ultrahand_updater.bin` and is automatically downloaded if not already present.

### Boot and Exit Hooks

- **On-boot commands**: Place a `boot_package.ini` file in `/switch/.packages/` with a `[on-boot]` section to run commands once on each device boot. Individual packages support their own `boot_package.ini` with a `[boot]` section that runs each time the package is launched (this can be suppressed per-package by enabling *Quick Launch* in that package's settings).
- **On-exit commands**: An `exit_package.ini` in `/switch/.packages/` with an `[exit]` section runs when Ultrahand is closed cleanly. Individual packages can also include their own `exit_package.ini` with an `[exit]` section that runs each time the package is closed.

### Package Language Translations

Packages can include their own language translation JSON files in a `lang/` subfolder within the package directory. Ultrahand loads translations from the file matching the currently selected overlay language and applies them to `drawString` calls (keys are original text strings, values are translated strings). This mirrors the language system used by `libultrahand` overlays.

---

## Getting Started

### Usage

**Quick Start (recommended):** Download the latest [`sdout.zip`](https://github.com/ppkantorski/Ultrahand-Overlay/releases/latest/download/sdout.zip) and extract it directly to the root of your SD card. It contains everything you need: nx-ovlloader, `ovlmenu.ovl`, language files, themes, sound packs, assets, audio mastervolume patches, and the full required folder structure — ready to use immediately. Then skip to step 3 below.

**Manual Installation:**

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
- Long-tap a command (touch) to open the same individual command line view.
- Press `X` on an overlay or package to star it (moves it to a hidden favorites section).
- Press `Y` on an overlay or package to open its configuration settings.
- Long-tap an overlay or package (touch) to open its configuration settings.
- Press `PLUS` from the main menu to enter the Ultrahand Settings menu.
- Press `L` to jump to the top of the current menu; press `R` to jump to the bottom.
- Press `ZL` / `ZR` to skip the current list one viewport's distance up or down. Hold for rapid page skipping. Also works for table scrolling.
- Press `R` during a running command to abort the operation.
- Press `B` during a running command to dismiss the overlay without canceling.
- **Swipe to Open** — as an alternative to the key combo, swipe inward ~1.5 cm from the side of the screen where the menu is drawn (left by default, or right in Right-side Mode) in under 150 ms. Can be toggled off in `Settings > Miscellaneous`.

---

## Settings

The Ultrahand Settings menu (accessible via `PLUS` on the main screen) exposes the following options:

### General
- **Key Combo** — Choose the button combination used to open Ultrahand from any of the supported default combos.
- **Language** — Select the UI language. Languages are loaded from JSON files in `/config/ultrahand/lang/`. English is always available.
- **Notifications** — Configure notification behavior, including silence, startup notification, API-triggered notifications, API toggle hotkeys (hold `MINUS` for 4 s to disable API notifications on demand), and the maximum number of simultaneous notification toasts (up to 4 on 4 MB heap, up to 8 on 6 MB+).
- **System** — Displays device info (Erista/Mariko, memory vendor/model, CPU/GPU fuse data, firmware version, storage) and exposes an **Overlay Memory** slider (4 MB / 6 MB / 8 MB heap) to tune the memory available to the overlay. 4 MB supports sound effects; 6 MB+ additionally supports wallpapers and higher transfer buffer sizes (faster copy, download, and unzip).
- **Software Update** — Check for and install the latest Ultrahand release directly from within the overlay.

### UI Settings
- **Theme** — Select a UI theme from `.ini` files in `/config/ultrahand/themes/`.
- **Sounds** — Select a sound-effect pack from zip archives in `/config/ultrahand/.sounds/`.
- **Wallpaper** — Select a background wallpaper from `/config/ultrahand/wallpapers/` (requires 6 MB+ heap). Wallpapers must be in raw 448x720 `.rgba` format.
- **Widget** — Toggle individual status bar elements (clock, SoC temperature, PCB temperature, battery indicator, backdrop, border) and adjust widget display settings (dynamic colors, center alignment, extended backdrop).
- **Miscellaneous** — Granular feature toggles including:
  - Launch combos, haptic feedback, auto NTP sync, opaque screenshots
  - Swipe to Open, Right-side Mode
  - Page swap, page recall, launch recall
  - Packages menu visibility
  - User guide visibility, hidden items visibility, delete action visibility
  - Overlay/package version label display and formatting
  - Dynamic logo, selection background/text/value highlighting
  - libultrahand and package title/version display modes

---

## Per-Overlay and Per-Package Key Combos

Each overlay and package can be assigned its own launch combo independently of the global Ultrahand hotkey. Combos are deconflicted automatically: assigning a combo to one overlay or package removes it from any other that was using it. Mode-specific combos (`mode_combos`) are also supported for multi-mode overlays, allowing a single overlay to expose multiple entry points (e.g. `-mini`, `-micro` modes), each with its own combo and label. These can be configured via `mode_args` and `mode_labels` in `/config/ultrahand/overlays.ini`.

---

## Compilation Prerequisites

To compile and run the software, you need:

- [devkitPro](https://devkitpro.org) with `devkitA64` and `libnx`
- [libultrahand](https://github.com/ppkantorski/libultrahand)
- `switch-curl`
- `switch-zlib`
- `switch-minizip`
- `switch-mbedtls`

```sh
export DEVKITPRO=/opt/devkitpro
make
```

The Makefile auto-detects available CPU cores for parallel LTO compilation. The build targets C++26 and ARMv8-A. Output is `ovlmenu.ovl`.

---

## Contributing

Contributions are welcome! If you have any ideas, suggestions, or bug reports, please raise an [issue](https://github.com/ppkantorski/Ultrahand-Overlay/issues/new/choose), submit a [pull request](https://github.com/ppkantorski/Ultrahand-Overlay/compare) or reach out to me directly on [GBATemp](https://gbatemp.net/threads/ultrahand-overlay-the-fully-craft-able-overlay-executor.633560/).

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/X8X3VR194)

## License

This project is licensed and distributed under [GPLv2](LICENSE) with a [custom library](https://github.com/ppkantorski/libultrahand/tree/main/libultra) utilizing [CC-BY-4.0](SUB_LICENSE).

Copyright (c) 2023-2026 ppkantorski
