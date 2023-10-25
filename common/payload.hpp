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
#pragma once

#include <list>
#include <string>
#include <switch.h>
#include <cstdint>

namespace Payload {

    enum BootCfg {
        BootCfg_ForceAutoBoot = 1 << 0,
        BootCfg_ShowLaunchLog = 1 << 1,
        BootCfg_BootFromId    = 1 << 2,
        BootCfg_BootToEmuMMC  = 1 << 3,
        BootCfg_SeptRun       = 1 << 7,
    };

    enum ExtraCfg {
        ExtraCfg_Keys      = 1 << 0,
        ExtraCfg_Payload   = 1 << 1,
        ExtraCfg_Module    = 1 << 2,
        ExtraCfg_NyxBis    = 1 << 4,
        ExtraCfg_NyxUms    = 1 << 5,
        ExtraCfg_NyxReload = 1 << 6,
        ExtraCfg_NyxDump   = 1 << 7,
    };

    enum UmsTarget {
        UmsTarget_Sd,
        UmsTarget_NandBoot0,
        UmsTarget_NandBoot1,
        UmsTarget_Nand,
        UmsTarget_EmuMMCBoot0,
        UmsTarget_EmuMMCBoot1,
        UmsTarget_EmuMMC,
    };

    // clang-format off
    struct BootStorage {
        /* 0x94 */  u8 boot_cfg;
        /* 0x95 */  u8 autoboot;
        /* 0x96 */  u8 autoboot_list;
        /* 0x97 */  u8 extra_cfg;
        /* 0x98 */  union {
        /*      */      struct {
        /* 0x98 */          char id[8];
        /* 0xA0 */          char emummc_path[0x78];
        /*      */      };
        /* 0x98 */      u8 ums;
        /* 0x98 */      u8 xt_str[0x80];
        /*      */  };
    };
    // clang-format on

    static_assert(sizeof(BootStorage) == 0x84, "Boot storage size!");

    constexpr inline std::size_t const BootStorageOffset = 0x94;
    constexpr inline std::size_t const MagicOffset       = BootStorageOffset + sizeof(BootStorage);
    constexpr inline std::uint32_t const Magic           = 0x43544349; /* ICTC */

    struct HekateConfig {
        std::string const name;
        std::size_t const index;
    };

    struct PayloadConfig {
        std::string const name;
        std::string const path;
    };

    using HekateConfigList = std::list<HekateConfig>;
    using PayloadConfigList = std::list<PayloadConfig>;

    HekateConfigList LoadHekateConfigList();
    HekateConfigList LoadIniConfigList();
    PayloadConfigList LoadPayloadList();
    bool RebootToHekate();
    bool RebootToHekateConfig(HekateConfig const &config, bool const autoboot_list);
    bool RebootToHekateUMS(UmsTarget const target);
    bool RebootToPayload(PayloadConfig const &config);

}
