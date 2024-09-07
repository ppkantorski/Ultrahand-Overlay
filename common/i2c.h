/*
Functions taken from Switch-OC-Suite source code made by KazushiMe
Original repository link (Deleted, last checked 15.04.2023): https://github.com/KazushiMe/Switch-OC-Suite
*/

Result I2cReadRegHandler(u8 reg, I2cDevice dev, u16 *out)
{
	struct readReg {
        u8 send;
        u8 sendLength;
        u8 sendData;
        u8 receive;
        u8 receiveLength;
    };

	I2cSession _session;

	Result res = i2cOpenSession(&_session, dev);
	if (res)
		return res;

	u16 val;

    struct readReg readRegister = {
        .send = 0 | (I2cTransactionOption_Start << 6),
        .sendLength = sizeof(reg),
        .sendData = reg,
        .receive = 1 | (I2cTransactionOption_All << 6),
        .receiveLength = sizeof(val),
    };

	res = i2csessionExecuteCommandList(&_session, &val, sizeof(val), &readRegister, sizeof(readRegister));
	if (res)
	{
		i2csessionClose(&_session);
		return res;
	}

	*out = val;
	i2csessionClose(&_session);
	return 0;
}


/*
Additional custom temperature functions by ppkantorski (for Ultrahand Overlay)
*/

// CUSTOM SECTION START

#define TMP451_SOC_TEMP_REG 0x01  // Register for SOC temperature integer part
#define TMP451_SOC_TMP_DEC_REG 0x10  // Register for SOC temperature decimal part
#define TMP451_PCB_TEMP_REG 0x00  // Register for PCB temperature integer part
#define TMP451_PCB_TMP_DEC_REG 0x15  // Register for PCB temperature decimal part

// Common helper function to read temperature (integer and fractional parts)
Result ReadTemperature(s32 *temperature, u8 integerReg, u8 fractionalReg, bool integerOnly)
{
    u16 rawValue;
    u8 val;
    s32 integerPart = 0;
    s32 fractionalPart = 0;

    // Read the integer part of the temperature
    Result res = I2cReadRegHandler(integerReg, I2cDevice_Tmp451, &rawValue);
    if (R_FAILED(res)) {
        return res;  // Error during I2C read
    }

    val = (u8)rawValue;  // Cast the value to an 8-bit unsigned integer
    integerPart = val;    // Integer part of temperature in Celsius

    if (integerOnly)
    {
        *temperature = integerPart;
        return 0;  // Return only integer part if requested
    }

    // Read the fractional part of the temperature
    res = I2cReadRegHandler(fractionalReg, I2cDevice_Tmp451, &rawValue);
    if (R_FAILED(res)) {
        return res;  // Error during I2C read
    }

    val = (u8)rawValue;  // Cast the value to an 8-bit unsigned integer
    fractionalPart = (val >> 4) * 0.0625;  // Fractional part: upper 4 bits, scaled by 1/16

    // Combine integer and fractional parts
    *temperature = integerPart + fractionalPart;

    return 0;
}

// Function to get the SOC temperature
Result ReadSocTemperature(s32 *temperature, bool integerOnly = true)
{
    return ReadTemperature(temperature, TMP451_SOC_TEMP_REG, TMP451_SOC_TMP_DEC_REG, integerOnly);
}

// Function to get the PCB temperature
Result ReadPcbTemperature(s32 *temperature, bool integerOnly = true)
{
    return ReadTemperature(temperature, TMP451_PCB_TEMP_REG, TMP451_PCB_TMP_DEC_REG, integerOnly);
}

// CUSTOM SECTION END
