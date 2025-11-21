/********************************************************************************
 * File: haptics.hpp
 * Author: ppkantorski
 * Description:
 *   This header declares functions and shared flags for managing haptic feedback
 *   in the Ultrahand Overlay. It includes routines for initializing vibration
 *   devices, sending rumble and double-click patterns, and controlling timing
 *   for single and double pulse haptics. Atomic flags ensure safe access across
 *   threads.
 *
 *   For the latest updates and contributions, visit the project's GitHub repository.
 *   (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)
 *
 *   Note: Please be aware that this notice cannot be altered or removed. It is a part
 *   of the project's documentation and must remain intact.
 * 
 *   Licensed under both GPLv2 and CC-BY-4.0
 *   Copyright (c) 2025 ppkantorski
 ********************************************************************************/

#pragma once
#include <switch.h>
#include <atomic>
#include <cstdint>

namespace ult {
	
	//extern bool rumbleInitialized;
	extern std::atomic<bool> rumbleActive;
	extern std::atomic<bool> doubleClickActive;
	
	//void initRumble();
	void deinitRumble();
	void checkAndReinitRumble();
	
	void rumbleClick();
	void rumbleDoubleClick();
	void processRumbleStop(u64 nowNs);
	void processRumbleDoubleClick(u64 nowNs);
	void rumbleDoubleClickStandalone();
}