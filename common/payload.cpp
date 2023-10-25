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
#include "payload.hpp"

#include "util.hpp"
#include "rtc_r2p.hpp"
#include "reboot_to_payload.h"
#include "ams_bpc.h"
#include "ini.h"

#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <algorithm>
#include <span>

namespace Payload {

    namespace {

        void RebootToPayload() {
            /* Try reboot with safe ams bpc api. */
            Result rc = amsBpcInitialize();
            if (R_SUCCEEDED(rc)) {
                rc = amsBpcSetRebootPayload(g_reboot_payload, IRAM_PAYLOAD_MAX_SIZE);
                if (R_SUCCEEDED(rc)) {
                    spsmShutdown(true);
                }
                amsBpcExit();
            }
            
            /* Fallback to old smc reboot to payload. */
            if (R_FAILED(rc))
                smc_reboot_to_payload();
        }

        int HekateConfigHandler(void *user, char const *section, char const *name, char const *value) {
            auto const list = reinterpret_cast<HekateConfigList *>(user);

            /* Ignore pre-config and global config entries. */
            if (section[0] == '\0' || std::strcmp(section, "config") == 0) {
                return 1;
            }

            /* Find existing entry. */
            auto it = std::find_if(list->begin(), list->end(), [section](HekateConfig &cfg) {
                return cfg.name == section;
            });

            /* Create config entry if not existant. */
            HekateConfig &config = (it != list->end()) ? *it : list->emplace_back(section, list->size() + 1);

            /* TODO: parse more information and display that. */
            (void)config;

            (void)name;
            (void)value;

            return 1;
        }

        constexpr char const *const HekatePaths[] = {
            "sdmc:/atmosphere/reboot_payload.bin",
            "sdmc:/bootloader/update.bin",
            "sdmc:/bootloader/payloads/hekate.bin",
            "sdmc:/sept/payload.bin",
        };

        constexpr char const *const PayloadDirs[] = {
            "sdmc:/",
            "sdmc:/bootloader/payloads/",
            "sdmc:/payloads/",
        };

        bool LoadPayload(const char* path, bool hekate) {
            /* Clear payload buffer. */
            std::memset(g_reboot_payload, 0xFF, sizeof(g_reboot_payload));

            /* Open payload. */
            auto const file = fopen(path, "r");
            if (file == nullptr)
                return false;

            /* Read payload to buffer. */
            auto const ret = fread(g_reboot_payload, 1, sizeof(g_reboot_payload), file);

            /* Close file. */
            fclose(file);

            /* Verify payload loaded successfully. */
            if (ret == 0)
                return false;

            /* Check if payload has hekate magic. */
            if (hekate && *(u32 *)(g_reboot_payload + Payload::MagicOffset) != Payload::Magic)
                return false;

            return true;
        }

        bool LoadHekatePayload() {
            /* Iterate through the payload dirs */
            for (auto const path : HekatePaths) {
                /* Try loading the payload */
                if (LoadPayload(path, true))
                    return true;
            }

            return false;
        }

    }

    HekateConfigList LoadHekateConfigList() {
        HekateConfigList configs;
        ini_parse("sdmc:/bootloader/hekate_ipl.ini", HekateConfigHandler, &configs);
        return configs;
    }

    HekateConfigList LoadIniConfigList() {
        HekateConfigList configs;

        if (chdir("sdmc:/bootloader/ini") != 0)
            return configs;

        /* Open ini folder */
        auto const dirp = opendir(".");
        if (dirp == nullptr)
            return configs;

        u32 count=0;
        char dir_entries[8][0x100];

        /* Get entries */
        while (auto dent = readdir(dirp)) {
            if (dent->d_type != DT_REG)
                continue;

            std::strcpy(dir_entries[count++], dent->d_name);

            if (count == std::size(dir_entries))
                break;
        }

        if (count > 1) {
            /* Reorder ini files by ASCII ordering. */
            char temp[0x100];
            for (size_t i = 0; i < count - 1 ; i++) {
                for (size_t j = i + 1; j < count; j++) {
                    if (std::strcmp(dir_entries[i], dir_entries[j]) > 0) {
                        std::strcpy(temp, dir_entries[i]);
                        std::strcpy(dir_entries[i], dir_entries[j]);
                        std::strcpy(dir_entries[j], temp);
                    }
                }
            }
        }

        /* parse config */
        for (auto const &entry : std::span(dir_entries, count))
            ini_parse(entry, HekateConfigHandler, &configs);

        closedir(dirp);

        chdir("sdmc:/");

        return configs;
    }

    PayloadConfigList LoadPayloadList() {
        PayloadConfigList res;

        /* Iterate through all the payload folders */
        for (const auto& path : PayloadDirs) {

            if (chdir(path) != 0)
                continue;

            /* Open `path` folder */
            auto const dirp = opendir(".");
            if (dirp == nullptr)
                continue;

            /* Get entries */
            while (auto const dent = readdir(dirp)) {
                if (dent->d_type != DT_REG)
                    continue;

                /* Get payloads */
                std::string const name(dent->d_name);
                if (name.substr(name.size() - 4) == ".bin")
                    res.emplace_back(name.substr(0, name.size() - 4), (path + name));
            }

            closedir(dirp);
        }

        chdir("sdmc:/");

        return res;
    }

    template<typename ConfigureFunction>
    bool Reboot(ConfigureFunction func) {
        /* Load payload. */
        if (!LoadHekatePayload())
            return false;

        /* Get boot storage pointer. */
        auto const storage = reinterpret_cast<BootStorage *>(g_reboot_payload + BootStorageOffset);

        /* Clear boot storage. */
        std::memset(storage, 0, sizeof(BootStorage));

        /* Configure boot storage */
        func(storage);

        /* Reboot */
        RebootToPayload();

        return true;
    }

    bool RebootToHekate() {
        if (util::IsErista()) {
            return Reboot([&] (BootStorage *storage) {
                /* No-Op */
            });
        } else {
            Max77620Rtc::rtc_reboot_reason_t rr {.dec = {
                .reason = Max77620Rtc::REBOOT_REASON_NOP,
            }};
            return Max77620Rtc::Reboot(&rr);
        }
    }

    bool RebootToHekateConfig(HekateConfig const &config, bool const autoboot_list) {
        if (util::IsErista()) {
            return Reboot([&] (BootStorage *storage) {
                /* Force autoboot and set boot id. */
                storage->boot_cfg      = BootCfg_ForceAutoBoot;
                storage->autoboot      = config.index;
                storage->autoboot_list = autoboot_list;
            });
        } else {
            Max77620Rtc::rtc_reboot_reason_t rr {.dec = {
                .reason = Max77620Rtc::REBOOT_REASON_SELF,
                .autoboot_idx = static_cast<u16>(config.index & 0xf),
                .autoboot_list = autoboot_list,
            }};
            return Max77620Rtc::Reboot(&rr);
        }
    }

    bool RebootToHekateUMS(UmsTarget const target) {
        if (util::IsErista()) {
            return Reboot([&] (BootStorage *storage) {
                /* Force boot to menu, target UMS and select target. */
                storage->boot_cfg  = BootCfg_ForceAutoBoot;
                storage->extra_cfg = ExtraCfg_NyxUms;
                storage->autoboot  = 0;
                storage->ums       = target;
            });
        } else {
            Max77620Rtc::rtc_reboot_reason_t rr {.dec = {
                .reason = Max77620Rtc::REBOOT_REASON_UMS,
                .ums_idx = target,
            }};
            return Max77620Rtc::Reboot(&rr);
        }
    }

    bool RebootToPayload(PayloadConfig const &config) {
        if (util::IsErista()) {
            /* Load payload. */
            if (!LoadPayload(config.path.c_str(), false))
                return false;

            /* Reboot */
            RebootToPayload();

            return true;
        } else {
            return false;
        }
    }

}
