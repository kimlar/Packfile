#pragma once

#include <string>

inline std::string ConvertUnsignedIntToChar4(unsigned int value)
{
	// Convert unsigned int to char[4].Note that it must be printed backwards.
	unsigned char bytes[4];
	bytes[0] = (value >> 24) & 0xFF;
	bytes[1] = (value >> 16) & 0xFF;
	bytes[2] = (value >> 8) & 0xFF;
	bytes[3] = value & 0xFF;

	//printf("bytes[4]: %d %d %d %d\n", bytes[3], bytes[2], bytes[1], bytes[0]);
	//printf("bytes[4]: %X %X %X %X\n", bytes[3], bytes[2], bytes[1], bytes[0]);

	std::string temp = "";
	temp += bytes[3];
	temp += bytes[2];
	temp += bytes[1];
	temp += bytes[0];	
	return temp;
}

inline unsigned int ConvertChar4ToUnsignedInt(char value[4])
{
	return unsigned int((unsigned char)(value[3]) << 24 | (unsigned char)(value[2]) << 16 | (unsigned char)(value[1]) << 8 | (unsigned char)(value[0]));
}
