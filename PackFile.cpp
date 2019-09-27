#include "PackFile.h"

PackFile::PackFile(std::string fileName)
{
	this->fileName;

	// [ROOT] section
	ROOT_Position = 0;
	ROOT_Size = 0;

	// [HEAD] section
	HEAD_Position = 0;
	HEAD_Size = 0;
	HEAD_NumberOfFiles = 0;
	HEAD_TotalDataSize = 0;

	// [FLST] section
	FLST_Position = 0;
	FLST_Size = 0;
	FLST_FileNames.clear();
	FLST_FilePosition.clear();
	FLST_FileSize.clear();

	// [FDAT] section
	FDAT_Position = 0;
	FDAT_Size = 0;
	FDAT_FileData.clear();

}

PackFile::~PackFile()
{

}
