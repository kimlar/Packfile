#pragma once
#pragma warning(disable:4996) // Suppress deprecation error C4996

#include <string>
#include <ctime> // time
#include <iomanip> // setfill
#include <sstream>

class UtilDateTime
{
public:
	UtilDateTime()
	{
		useGMT = false;
		update = true;
	}
	~UtilDateTime() {}

	std::string GetISODateTime()
	{
		bool oldUpdate = this->update;
		this->update = update;
		std::string tempStr = "";

		UpdateDateTimeStruct();

		// YYYYMMDDhhmm
		tempStr += ZeroPadding(GetYear());
		tempStr += ZeroPadding(GetMonth());
		tempStr += ZeroPadding(GetDay());
		tempStr += ZeroPadding(GetHour());
		tempStr += ZeroPadding(GetMinute());

		this->update = oldUpdate;
		return tempStr;
	}

	// Operators
	void UseGMT() { useGMT = true; }
	void UseLocalTime() { useGMT = false; }
	void SetUpdate(bool update) { this->update = update; }

	// Getters
	int GetYear()
	{
		if(update)
			UpdateDateTimeStruct();
		return timeStruct->tm_year + 1900;
	}
	int GetMonth()
	{
		if (update)
			UpdateDateTimeStruct();
		return timeStruct->tm_mon + 1;
	}
	int GetDay()
	{
		if (update)
			UpdateDateTimeStruct();
		return timeStruct->tm_mday;
	}
	int GetHour()
	{
		if (update)
			UpdateDateTimeStruct();
		return timeStruct->tm_hour;
	}
	int GetMinute()
	{
		if (update)
			UpdateDateTimeStruct();
		return timeStruct->tm_min;
	}
	int GetSecond()
	{
		if (update)
			UpdateDateTimeStruct();
		return timeStruct->tm_sec;
	}
private:
	time_t currentTime;
	tm* timeStruct;
	bool useGMT = false;
	bool update = true;

	void UpdateDateTimeStruct()
	{
		currentTime = time(0);

		if (useGMT)
			timeStruct = gmtime(&currentTime);
		else
			timeStruct = localtime(&currentTime);
	}

	std::string ZeroPadding(int number) // converts 3 to "03"
	{
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(2) << number;
		return ss.str();
	}
};
