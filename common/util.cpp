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
#include "util.hpp"

#include <switch.h>

namespace util {
    
    bool IsErista() { // CUSTOM MODIFICATION
        SetSysProductModel model = SetSysProductModel_Invalid;
        setsysGetProductModel(&model);
        
        if (model == SetSysProductModel_Nx || \
            model == SetSysProductModel_Copper)
            return true;
        
        return false;
    }
    
    
    bool IsMariko() { // CUSTOM MODIFICATION
        SetSysProductModel model = SetSysProductModel_Invalid;
        setsysGetProductModel(&model);
        
        if (model == SetSysProductModel_Iowa || \
            model == SetSysProductModel_Hoag || \
            model == SetSysProductModel_Calcio || \
            model == SetSysProductModel_Aula)
            return true;
        
        return false;
    }


    /**
     * Since 1.6.0, Atmosphère bpc-mitm overwrites the reboot on mariko to prevent clearing
     * Timers. We are using those timing registers to communicate with hekate.
     */

    bool SupportsMarikoRebootToConfig() {
        static bool impl_computed = false;
        static bool impl_value = false;
    
        if (impl_computed)
            return impl_value;
    
        u64 version = 0;
    
        if (R_FAILED(splGetConfig(static_cast<SplConfigItem>(65000), &version)))
            return false;
    
        const u32 version_minor = (version >> 48) & 0xff;
        const u32 version_major = (version >> 56) & 0xff;
    
        impl_value = (version_major >= 1 && version_minor >= 6);
        impl_computed = true; // Set to true to indicate the value has been computed
    
        return impl_value;
    }

}
