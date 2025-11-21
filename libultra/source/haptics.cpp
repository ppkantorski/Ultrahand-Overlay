/********************************************************************************
 * File: haptics.cpp
 * Author: ppkantorski
 * Description:
 *   This source file provides implementations for the functions declared in
 *   haptics.hpp. These functions manage haptic feedback for the Ultrahand Overlay
 *   using libnx’s vibration interfaces. It includes routines for initializing
 *   rumble devices, sending vibration patterns, and handling single or double
 *   click feedback with timing control. Thread safety is maintained through
 *   atomic operations and synchronization mechanisms.
 *
 *   For the latest updates and contributions, visit the project's GitHub repository.
 *   (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)
 *
 *   Note: Please be aware that this notice cannot be altered or removed. It is a part
 *   of the project's documentation and must remain intact.
 * 
 *  Licensed under both GPLv2 and CC-BY-4.0
 *  Copyright (c) 2025 ppkantorski
 ********************************************************************************/

#include "haptics.hpp"

namespace ult {
    
    // ===== Internal state (private to this file) =====
    //bool rumbleInitialized = false;
    static HidVibrationDeviceHandle vibHandheld;
    static HidVibrationDeviceHandle vibPlayer1Left;
    static HidVibrationDeviceHandle vibPlayer1Right;
    static u64 rumbleStartTick = 0;
    static u64 doubleClickTick = 0;
    static u8 doubleClickPulse = 0;
    
    // ===== Shared flags (accessible globally) =====
    std::atomic<bool> rumbleActive{false};
    std::atomic<bool> doubleClickActive{false};
    
    // ===== Constants =====
    static constexpr u64 RUMBLE_DURATION_NS = 30'000'000ULL;
    static constexpr u64 DOUBLE_CLICK_PULSE_DURATION_NS = 30'000'000ULL;
    static constexpr u64 DOUBLE_CLICK_GAP_NS = 80'000'000ULL;
    
    static constexpr HidVibrationValue clickDocked = {
        .amp_low  = 0.20f,
        .freq_low = 100.0f,
        .amp_high = 0.80f,
        .freq_high = 300.0f
    };
    
    static constexpr HidVibrationValue clickHandheld = {
        .amp_low  = 0.25f,
        .freq_low = 100.0f,
        .amp_high = 1.0f,
        .freq_high = 300.0f
    };
    
    static constexpr HidVibrationValue vibrationStop{0};
    
    // ===== Internal helpers =====
    static void initController(HidNpadIdType npad, HidVibrationDeviceHandle* handles, int count) {
        const u32 styleMask = hidGetNpadStyleSet(npad);
        if (styleMask)
            hidInitializeVibrationDevices(handles, count, npad, static_cast<HidNpadStyleTag>(styleMask));
    }
    
    static void sendVibration(const HidVibrationValue* value) {
        if (hidGetNpadStyleSet(HidNpadIdType_Handheld))
            hidSendVibrationValue(vibHandheld, value);

        if (hidGetNpadStyleSet(HidNpadIdType_No1)) {
            hidSendVibrationValue(vibPlayer1Left, value);
            hidSendVibrationValue(vibPlayer1Right, value);
        }
    }
    
    // ===== Public API =====
    void initRumble() {
        //if (rumbleInitialized) return;
    
        // Try to initialize whatever is available
        // Don't check if controllers exist - let initController handle it
        initController(HidNpadIdType_Handheld, &vibHandheld, 1);
    
        HidVibrationDeviceHandle handles[2];
        initController(HidNpadIdType_No1, handles, 2);
        vibPlayer1Left = handles[0];
        vibPlayer1Right = handles[1];
    
        // Only mark as initialized if at least one controller was found
        hidGetNpadStyleSet(HidNpadIdType_Handheld);
        hidGetNpadStyleSet(HidNpadIdType_No1);
        
        //rumbleInitialized = (handheldStyle || player1Style);
        
        // If neither exist, stay uninitialized so we retry later
    }
    
    //void deinitRumble() {
    //    rumbleInitialized = false;
    //}
    
    void checkAndReinitRumble() {
        static u32 lastHandheldStyle = 0;
        static u32 lastPlayer1Style = 0;
        static bool everWorked = false;
    
        const u32 currentHandheldStyle = hidGetNpadStyleSet(HidNpadIdType_Handheld);
        const u32 currentPlayer1Style = hidGetNpadStyleSet(HidNpadIdType_No1);
    
        // If it's never worked and controllers exist, keep trying to init
        bool shouldInit = !everWorked && (currentHandheldStyle || currentPlayer1Style);
        
        // Or if controller config changed
        shouldInit |= (currentHandheldStyle != lastHandheldStyle || currentPlayer1Style != lastPlayer1Style);
    
        if (shouldInit) {
            initRumble();
            // Mark as working if we have any active controller
            if (currentHandheldStyle || currentPlayer1Style) {
                everWorked = true;
            }
        }
        
        lastHandheldStyle = currentHandheldStyle;
        lastPlayer1Style = currentPlayer1Style;
    }
    
    void rumbleClick() {
        //if (!rumbleInitialized) {
        //    initRumble();
        //    if (!rumbleInitialized) return;
        //}
    
        sendVibration(hidGetNpadStyleSet(HidNpadIdType_Handheld) ? &clickHandheld : &clickDocked);
        rumbleActive.store(true, std::memory_order_release);
        rumbleStartTick = armGetSystemTick();
    }
    
    void rumbleDoubleClick() {
        //if (!rumbleInitialized) {
        //    initRumble();
        //    if (!rumbleInitialized) return;
        //}
    
        sendVibration(hidGetNpadStyleSet(HidNpadIdType_Handheld) ? &clickHandheld : &clickDocked);
        doubleClickActive.store(true, std::memory_order_release);
        doubleClickPulse = 1;
        doubleClickTick = armGetSystemTick();
    }
    
    void processRumbleStop(u64 nowNs) {
        if (rumbleActive.load(std::memory_order_acquire) &&
            nowNs - armTicksToNs(rumbleStartTick) >= RUMBLE_DURATION_NS) {
            sendVibration(&vibrationStop);
            rumbleActive.store(false, std::memory_order_release);
        }
    }
    
    void processRumbleDoubleClick(u64 nowNs) {
        if (!doubleClickActive.load(std::memory_order_acquire)) return;
    
        const u64 elapsed = nowNs - armTicksToNs(doubleClickTick);
    
        switch (doubleClickPulse) {
            case 1:
                if (elapsed >= DOUBLE_CLICK_PULSE_DURATION_NS) {
                    sendVibration(&vibrationStop);
                    doubleClickPulse = 2;
                    doubleClickTick = armGetSystemTick();
                }
                break;
    
            case 2:
                if (elapsed >= DOUBLE_CLICK_GAP_NS) {
                    sendVibration(hidGetNpadStyleSet(HidNpadIdType_Handheld) ? &clickHandheld : &clickDocked);
                    doubleClickPulse = 3;
                    doubleClickTick = armGetSystemTick();
                }
                break;
    
            case 3:
                if (elapsed >= DOUBLE_CLICK_PULSE_DURATION_NS) {
                    sendVibration(&vibrationStop);
                    doubleClickActive.store(false, std::memory_order_release);
                    doubleClickPulse = 0;
                }
                break;
        }
    }

    void rumbleDoubleClickStandalone() {
        sendVibration(hidGetNpadStyleSet(HidNpadIdType_Handheld) ? &clickHandheld : &clickDocked);
        svcSleepThread(DOUBLE_CLICK_PULSE_DURATION_NS);
        
        sendVibration(&vibrationStop);
        svcSleepThread(DOUBLE_CLICK_GAP_NS);
        
        sendVibration(hidGetNpadStyleSet(HidNpadIdType_Handheld) ? &clickHandheld : &clickDocked);
        svcSleepThread(DOUBLE_CLICK_PULSE_DURATION_NS);
        
        sendVibration(&vibrationStop);
    }
}