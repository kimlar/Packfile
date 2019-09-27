#pragma once

#include <string>
#include <vector>

class PackFile
{
public:
	PackFile(std::string fileName);
	~PackFile();
private:
	std::string fileName;


	////////////////////////////////////////////////////////////////////////////
public:

	// TODO: Structify this

	// [ROOT] section
	const char* ROOT_ID = "ROOT";
	unsigned int ROOT_Position;
	unsigned int ROOT_Size;

	// [HEAD] section
	const char* HEAD_ID = "HEAD";
	unsigned int HEAD_Position;
	unsigned int HEAD_Size;
	unsigned int HEAD_NumberOfFiles;
	unsigned int HEAD_TotalDataSize;
	std::string HEAD_DataVersion;
	//const char* HEAD_DataVersion = "201803201638";

	// [FLST] section
	const char* FLST_ID = "FLST";
	unsigned int FLST_Position;
	unsigned int FLST_Size;
	std::vector<std::string> FLST_FileNames;
	std::vector<unsigned int> FLST_FilePosition;
	std::vector<unsigned int> FLST_FileSize;

	// [FDAT] section
	const char* FDAT_ID = "FDAT";
	unsigned int FDAT_Position;
	unsigned int FDAT_Size;
	std::vector<unsigned char> FDAT_FileData;

	////////////////////////////////////////////////////////////////////////////
};
