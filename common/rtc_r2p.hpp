/*
 * Copyright (c) 2020-2023 Studious Pancake
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <switch.h>

namespace Max77620Rtc {

    enum {
        REBOOT_REASON_NOP   = 0, // Use [config].
        REBOOT_REASON_SELF  = 1, // Use autoboot_idx/autoboot_list.
        REBOOT_REASON_MENU  = 2, // Force menu.
        REBOOT_REASON_UMS   = 3, // Force selected UMS partition.
        REBOOT_REASON_REC   = 4, // Set PMC_SCRATCH0_MODE_RECOVERY and reboot to self.
        REBOOT_REASON_PANIC = 5  // Inform bootloader that panic occured if T210B01.
    };

    typedef struct _rtc_rr_decoded_t
    {
        u16 reason:4;
        u16 autoboot_idx:4;
        u16 autoboot_list:1;
        u16 ums_idx:3;
    } rtc_rr_decoded_t;

    typedef struct _rtc_rr_encoded_t
    {
        u16 val1:6; // 6-bit reg.
        u16 val2:6; // 6-bit reg.
    } rtc_rr_encoded_t;

    typedef struct _rtc_reboot_reason_t
    {
        union {
            rtc_rr_decoded_t dec;
            rtc_rr_encoded_t enc;
        };
    } rtc_reboot_reason_t;
    
    bool Reboot(rtc_reboot_reason_t const* rr);

}
