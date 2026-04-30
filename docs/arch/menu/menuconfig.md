# Menuconfig Interface Documentation

## Overview
The Menuconfig Interface is a terminal-based tool used to configure the entire system. Unlike the Pinmux tool (which handles physical pins), Menuconfig handles logic, drivers, and build settings.
It allows to enable or disable specific hardware drivers, choose the target chip, and configure core settings.

### Key Features:
- Two-Stage Configuration: Separates hardware settings (Chip) from software settings (Build) for better organization.
- Visual Navigation: Uses a classic terminal interface with arrow keys and menus.
- Search Capability: Allows users to search for specific configuration options using the `/` key.
- Help System: Provides detailed descriptions for every option using the `?` key or Help button.
- Dependency Management: Automatically hides options that are not available based on your current selection (e.g., you can't configure UART settings if UART is disabled).
- Merged Output: Automatically combines hardware and software settings into a single system `.config` file.



## Usage

### 1. Launching the Interface
To open the system configuration tool, run the following command in your terminal:

```bash
make config
```

**Important Note:** The interface will open twice (for chip and peripheral configurations).


### 2. Navigation & Controls
Once opened, you will see a menu structure.

- **Arrow Keys (↑ ↓):** Navigate through the list.
- **Enter:** Enter a submenu (marked with `--->`).
- **Space:** Toggle an option (Enable `[*]`, Disable `[ ]`, or Module `[M]`).
- **Esc (x2):** Go back to the previous menu or exit.
- **/ (Forward Slash):** Search for a specific symbol or setting.
- **? (Question Mark):** View help/documentation for the selected item.

![alt text](pics/menuconfig_1.png)
*Figure 1. Chip configuration screen*

![alt text](pics/menuconfig_2.png)
*Figure 2. Build configuration screen*


### 3. The Configuration Process (Step-by-Step)

#### Step A: Chip Configuration
When the tool opens first, you are configuring the Hardware.

- **What you see:** Options related to the specific SoC Series, SoC, and Board.
- **Action:** Select your target SoC and Board.
- **Exit:** When you select `<Save>` and `<Exit>`, the window closes.

#### Step B: Build Configuration
Immediately after the first window closes, the tool opens again. This time you are configuring the Software.

- **What you see:** Options for the Api configurations, Drivers selections, and others.
- **Action:** Configure how the software should run on the chip.
- **Exit:** When you select `<Save>` and `<Exit>`, the process finishes.


### 4. Saving
You do not need to manually merge files. Simply use the `<Save>` button in the interface. When you exit the second window, the system automatically saves everything to the final `.config` file.



## Internal Execution Flow
This section explains how the system handles the two configuration stages and merges them into one final result.

### 1. Stage 1: Chip Configuration
- **Input:** The system reads `Kconfig.chip`. This file defines all hardware-related options.
- **Execution:** The user selects the settings in the UI.
- **Output:** The system saves these choices to a temporary file named `chip.config`.


### 2. Stage 2: Pre-processing
Between the two windows, a script (`preprocess_kconfig.py`) runs automatically.


### 3. Stage 3: Build Configuration
- **Input:** The system reads `Kconfig.build`. This file defines software and project settings.
- **Execution:** The user selects the settings in the UI.
- **Output:** The system saves these choices to a temporary file named `build.config`.


### 4. Merging (Final Step)
After both stages are complete, the system performs a merge operation.

- **Process:** It combines the contents of `chip.config` and `build.config`.
- **Command:**

```bash
cat chip.config build.config > .config
```

- **Result:** A single `.config` file is created. This file contains the complete system configuration (both Hardware and Software) and is used by the compiler and the Pinmux tool.