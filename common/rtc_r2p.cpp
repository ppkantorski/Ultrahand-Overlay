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
#include "rtc_r2p.hpp"

#include <cstdio>

namespace Max77620Rtc {

	#define RTC_REBOOT_REASON_MAGIC 0x77 // 7-bit reg.

	#define MAX77620_RTC_I2C_ADDR       0x68

	#define MAX77620_RTC_NR_TIME_REGS   7

	#define MAX77620_RTC_RTCINT_REG     0x00
	#define MAX77620_RTC_RTCINTM_REG    0x01
	#define MAX77620_RTC_CONTROLM_REG   0x02
	#define MAX77620_RTC_CONTROL_REG    0x03
	#define  MAX77620_RTC_BIN_FORMAT    BIT(0)
	#define  MAX77620_RTC_24H           BIT(1)

	#define MAX77620_RTC_UPDATE0_REG    0x04
	#define  MAX77620_RTC_WRITE_UPDATE  BIT(0)
	#define  MAX77620_RTC_READ_UPDATE   BIT(4)

	#define MAX77620_RTC_UPDATE1_REG    0x05
	#define MAX77620_RTC_RTCSMPL_REG    0x06

	#define MAX77620_RTC_SEC_REG        0x07
	#define MAX77620_RTC_MIN_REG        0x08
	#define MAX77620_RTC_HOUR_REG       0x09
	#define  MAX77620_RTC_HOUR_PM_MASK  BIT(6)
	#define MAX77620_RTC_WEEKDAY_REG    0x0A
	#define MAX77620_RTC_MONTH_REG      0x0B
	#define MAX77620_RTC_YEAR_REG       0x0C
	#define MAX77620_RTC_DATE_REG       0x0D

	#define MAX77620_ALARM1_SEC_REG     0x0E
	#define MAX77620_ALARM1_MIN_REG     0x0F
	#define MAX77620_ALARM1_HOUR_REG    0x10
	#define MAX77620_ALARM1_WEEKDAY_REG 0x11
	#define MAX77620_ALARM1_MONTH_REG   0x12
	#define MAX77620_ALARM1_YEAR_REG    0x13
	#define MAX77620_ALARM1_DATE_REG    0x14
	#define MAX77620_ALARM2_SEC_REG     0x15
	#define MAX77620_ALARM2_MIN_REG     0x16
	#define MAX77620_ALARM2_HOUR_REG    0x17
	#define MAX77620_ALARM2_WEEKDAY_REG 0x18
	#define MAX77620_ALARM2_MONTH_REG   0x19
	#define MAX77620_ALARM2_YEAR_REG    0x1A
	#define MAX77620_ALARM2_DATE_REG    0x1B
	#define  MAX77620_RTC_ALARM_EN_MASK	BIT(7)

	namespace {

		bool i2c_send_byte(I2cSession &session, u8 reg, u8 val) {
			struct {
				u8 reg;
				u8 val;
			} __attribute__((packed)) cmd;
			static_assert(sizeof(cmd) == 2, "I2C command definition!");

			cmd.reg = reg;
			cmd.val = val;
			Result rc = i2csessionSendAuto(&session, &cmd, sizeof(cmd), I2cTransactionOption_All);

			if (R_FAILED(rc)) {
				std::printf("i2c: Failed to send i2c register (%hhu): 2%03u-%04u\n", reg, R_MODULE(rc), R_DESCRIPTION(rc));
				return false;
			}

			return true;
		}

		bool i2c_recv_byte(I2cSession &session, u8 reg, u8 *out) {
			Result rc = 0;
			struct { u8 reg; } __attribute__((packed)) cmd;
			struct { u8 val; } __attribute__((packed)) rec;
			static_assert(sizeof(cmd) == 1, "I2C command definition!");
			static_assert(sizeof(rec) == 1, "I2C command definition!");

			cmd.reg = reg;
			if (R_FAILED(rc = i2csessionSendAuto(&session, &cmd, sizeof(cmd), I2cTransactionOption_All))) {
				std::printf("i2c: Failed to send i2c register for recv (%hhu): 2%03u-%04u\n", reg, R_MODULE(rc), R_DESCRIPTION(rc));
				return false;
			}

			if (R_FAILED(rc = i2csessionReceiveAuto(&session, &rec, sizeof(rec), I2cTransactionOption_All))) {
				std::printf("i2c: Failed to recv i2c register (%hhu): 2%03u-%04u\n", reg, R_MODULE(rc), R_DESCRIPTION(rc));
				return false;
			}

			*out = rec.val;

			return true;
		}

		bool max77620_rtc_stop_alarm(I2cSession &session) {
			u8 val = 0;

			// Update RTC regs from RTC clock.
			if (!i2c_send_byte(session, MAX77620_RTC_UPDATE0_REG, MAX77620_RTC_READ_UPDATE)) {
				return false;
			}
			svcSleepThread(16'000'000ul);

			// Stop alarm for both ALARM1 and ALARM2. Horizon uses ALARM2.
			for (int i = 0; i < (MAX77620_RTC_NR_TIME_REGS * 2); i++)
			{
				if (!i2c_recv_byte(session, MAX77620_ALARM1_SEC_REG + i, &val)) {
					return false;
				}
				val &= ~MAX77620_RTC_ALARM_EN_MASK;
				if (!i2c_send_byte(session, MAX77620_ALARM1_SEC_REG + i, val)) {
					return false;
				}
			}

			// Update RTC clock from RTC regs.
			auto const ret = i2c_send_byte(session, MAX77620_RTC_UPDATE0_REG, MAX77620_RTC_WRITE_UPDATE);

			svcSleepThread(16'000'000ul);

			return ret;
		}

	}

	bool Reboot(rtc_reboot_reason_t const* rr) {
		Result rc = 0;

		I2cSession session = {};
		if (R_FAILED(rc = i2cOpenSession(&session, I2cDevice_Max77620Rtc))) {
			std::printf("i2c: Failed to open i2c session: 2%03u-%04u\n", R_MODULE(rc), R_DESCRIPTION(rc));
			i2cExit();
			return false;
		}

		bool ret = 
			max77620_rtc_stop_alarm(session) &&

			// Set reboot reason.
			i2c_send_byte(session, MAX77620_ALARM1_YEAR_REG, rr->enc.val1) &&
			i2c_send_byte(session, MAX77620_ALARM2_YEAR_REG, rr->enc.val2) &&

			// Set reboot reason magic.
			i2c_send_byte(session, MAX77620_ALARM1_WEEKDAY_REG, RTC_REBOOT_REASON_MAGIC) &&
			i2c_send_byte(session, MAX77620_ALARM2_WEEKDAY_REG, RTC_REBOOT_REASON_MAGIC) &&

			// Update RTC clock from RTC regs.
			i2c_send_byte(session, MAX77620_RTC_UPDATE0_REG, MAX77620_RTC_WRITE_UPDATE);

		svcSleepThread(16'000'000ul);

		i2csessionClose(&session);

		return ret && R_SUCCEEDED(spsmShutdown(true));
	}
}
