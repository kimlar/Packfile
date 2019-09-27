#include "PackEngine.h"
#include "UtilDataType.h"

PackEngine::PackEngine(std::string packFileName)
{
	this->packFileName = packFileName;
	gotError = false;
}

PackEngine::~PackEngine()
{
	ClosePackFile();
}

void PackEngine::Open()
{
	Close();
	gotError = false;

	packFile = new PackFile(packFileName);

	std::ifstream infile(packFileName, std::ios::binary | std::ios::ate);
	unsigned int size = (unsigned int)infile.tellg();
	infile.seekg(0, std::ios::beg);

	// Check Format version: PACK1000
	if (!CheckFormatVersion(infile))
		return;

	// Read ROOT section
	if (!ReadRootSection(infile))
		return;

	// Read HEAD section
	if (!ReadHeadSection(infile))
		return;

	// Read FLST section
	if (!ReadFlstSection(infile))
		return;

	// Read FDAT section
	if (!ReadFdatSection(infile))
		return;

	//
	// Fill data into the pack file structure
	//

	// Fill HEAD section
	if (!FillHeadSection(infile))
		return;

	// Fill FLST section
	if (!FillFlstSection(infile))
		return;

	//infile.close();
}

void PackEngine::Close()
{
	sourceFiles.clear();
	ClosePackFile();
}

bool PackEngine::LoadFileToMemory(std::string file, std::vector<unsigned char>& buffer)
{
	bool foundFile = false;
	unsigned int fileIndex = 0;
	for (unsigned int i = 0; i < packFile->HEAD_NumberOfFiles; i++)
	{
		if (file == packFile->FLST_FileNames[i])
		{
			fileIndex = i;
			foundFile = true;
			break;
		}
	}
	if (!foundFile)
	{
		std::cout << "Error: Could not find file in the packfile" << std::endl;
		std::cout << "Packfile: " << packFileName << std::endl;
		std::cout << "File: " << file << std::endl;
		gotError = true;
		return false;
	}

	unsigned int bufferSize = packFile->FLST_FileSize[fileIndex];
	char* bufferData = new char[bufferSize];

	std::ifstream infile(packFileName, std::ios::binary);
	infile.seekg(packFile->FLST_FilePosition[fileIndex], std::ios::beg);
	infile.read(bufferData, bufferSize);

	buffer.resize(bufferSize);
	for (unsigned int i = 0; i < bufferSize; i++)
		buffer[i] = bufferData[i];

	delete[] bufferData;
	return true;
}

void PackEngine::Compile(std::vector<std::string> sourceFiles)
{
	this->sourceFiles = sourceFiles;

	ClosePackFile();

	packFile = new PackFile(packFileName);

	// Get the ISO date and time
	packFile->HEAD_DataVersion = utilDateTime.GetISODateTime();

	// We already know the amount of files
	unsigned int numFiles = (unsigned int)sourceFiles.size();
	packFile->HEAD_NumberOfFiles = numFiles;

	// And all the files.
	packFile->FLST_FileNames.reserve(numFiles);
	packFile->FLST_FilePosition.reserve(numFiles);
	packFile->FLST_FileSize.reserve(numFiles);
	for (unsigned int i = 0; i < numFiles; i++)
	{
		packFile->FLST_FileNames.push_back(sourceFiles[i]);
		packFile->FLST_FilePosition.push_back(0);
		packFile->FLST_FileSize.push_back(0);
	}

	// Let us find all file sizes and total file size (data container).
	unsigned int totSize = 0;
	for (unsigned int i = 0; i < numFiles; i++)
	{
		unsigned int tempFileSize = GetFileSize(packFile->FLST_FileNames[i]);
		if (gotError)
			return;
		packFile->FLST_FileSize[i] = tempFileSize;
		totSize += tempFileSize;
	}
	packFile->HEAD_TotalDataSize = totSize;

	// Calculating section positions and sizes
	packFile->ROOT_Position = 8; // const
	packFile->ROOT_Size = 48; // const
	packFile->HEAD_Position = 56; // const
	packFile->HEAD_Size = 20; // const
	packFile->FLST_Position = 76; // const
	packFile->FLST_Size = GetFileListSize();
	packFile->FDAT_Position = GetFileDataPosition();
	packFile->FDAT_Size = packFile->HEAD_TotalDataSize;

	// Calculating file positions
	unsigned int relPosition = packFile->FDAT_Position;
	for (unsigned int i = 0; i < numFiles; i++)
	{
		packFile->FLST_FilePosition[i] = relPosition;
		relPosition += packFile->FLST_FileSize[i];
	}

	// Generate the pack structure
	GeneratePackStructure();

	// Write pack file
	WritePackFile();

	std::cout << "Done" << std::endl;
}

void PackEngine::Unpack()
{
	ReadPackFile();
	if (gotError)
		return;

	// Unpack each file to disk
	for (unsigned int i = 0; i < packFile->HEAD_NumberOfFiles; i++)
	{
		UnpackFile(i);
		if (gotError)
			return;
	}

	std::cout << "Done" << std::endl;
}

void PackEngine::ReadPackFile()
{
	ClosePackFile();
	gotError = false;

	packFile = new PackFile(packFileName);

	std::ifstream infile(packFileName, std::ios::binary | std::ios::ate);
	unsigned int size = (unsigned int)infile.tellg();
	infile.seekg(0, std::ios::beg);

	// Check Format version: PACK1000
	if (!CheckFormatVersion(infile))
		return;

	// Read ROOT section
	if (!ReadRootSection(infile))
		return;

	// Read HEAD section
	if (!ReadHeadSection(infile))
		return;

	// Read FLST section
	if (!ReadFlstSection(infile))
		return;

	// Read FDAT section
	if (!ReadFdatSection(infile))
		return;

	//
	// Fill data into the pack file structure
	//

	// Fill HEAD section
	if (!FillHeadSection(infile))
		return;

	// Fill FLST section
	if (!FillFlstSection(infile))
		return;

	infile.close();
}

void PackEngine::WritePackFile()
{
	std::ofstream outfile(packFileName, std::ofstream::binary | std::ofstream::trunc);

	// Get the total file size of the pack file
	unsigned int packFileSize = 0;
	packFileSize += 8; // PACK1000
	packFileSize += packFile->ROOT_Size;
	packFileSize += packFile->HEAD_Size;
	packFileSize += packFile->FLST_Size;
	packFileSize += packFile->FDAT_Size;

	// allocate memory for the entire pack file
	char* buffer = new char[packFileSize];

	//
	// fill buffer
	//
	unsigned int bufferIndex = 0;

	// filling the pack structure
	for (unsigned int i = 0; i < (unsigned int)packStructure.size(); i++)
		buffer[i] = packStructure[i];
	bufferIndex += (unsigned int)packStructure.size();

	// add data from files (still filling the buffer)
	for (unsigned int i = 0; i < packFile->HEAD_NumberOfFiles; i++)
	{
		std::vector<char> bufferData = ReadFile(packFile->FLST_FileNames[i]);
		for (unsigned int j = 0; j < (unsigned int)bufferData.size(); j++)
		{
			buffer[bufferIndex] = bufferData[j];
			bufferIndex++;
		}
	}

	// write to file
	outfile.write(buffer, packFileSize);

	// release memory
	delete[] buffer;

	outfile.close();
}

void PackEngine::ClosePackFile()
{
	if (packFile)
	{
		delete packFile;
		packFile = nullptr;
	}
}

unsigned int PackEngine::GetFileSize(std::string path)
{
	unsigned int size = 0;
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
	{
		printf("Error: Could not open file: %s\n", path.c_str());
		gotError = true;
		return 0;
	}
	size = (unsigned int)file.tellg();
	file.close();
	return size;
}

unsigned int PackEngine::GetFileListSize()
{
	unsigned int tempSize = 0;
	for (unsigned int i = 0; i < packFile->HEAD_NumberOfFiles; i++)
	{
		tempSize += (unsigned int)packFile->FLST_FileNames[i].size();
		tempSize += 1; // for '\0' terminator
		tempSize += 8; // for position and size
	}
	return tempSize;
}

unsigned int PackEngine::GetFileDataPosition()
{
	unsigned int tempPos = 0;
	tempPos += 8; // "PACK1000"
	tempPos += packFile->ROOT_Size; // ROOT size
	tempPos += packFile->HEAD_Size; // HEAD size
	tempPos += packFile->FLST_Size; // FLST size
	return tempPos;
}

void PackEngine::GeneratePackStructure()
{
	packStructure.clear();
	packStructure.reserve(packFile->FDAT_Position);

	// Format version: PACK1000
	AddStringToPackStructure("PACK1000");
	// *** ROOT ***
	// ROOT section
	AddStringToPackStructure("ROOT");
	AddUnsignedIntToPackStructure(packFile->ROOT_Position);
	AddUnsignedIntToPackStructure(packFile->ROOT_Size);
	// HEAD section
	AddStringToPackStructure("HEAD");
	AddUnsignedIntToPackStructure(packFile->HEAD_Position);
	AddUnsignedIntToPackStructure(packFile->HEAD_Size);
	// FLST section
	AddStringToPackStructure("FLST");
	AddUnsignedIntToPackStructure(packFile->FLST_Position);
	AddUnsignedIntToPackStructure(packFile->FLST_Size);
	// FDAT section
	AddStringToPackStructure("FDAT");
	AddUnsignedIntToPackStructure(packFile->FDAT_Position);
	AddUnsignedIntToPackStructure(packFile->FDAT_Size);
	// *** HEAD ***
	AddUnsignedIntToPackStructure(packFile->HEAD_NumberOfFiles);
	AddUnsignedIntToPackStructure(packFile->HEAD_TotalDataSize);
	AddStringToPackStructure(packFile->HEAD_DataVersion);
	// *** FLST ***
	for (unsigned int i = 0; i < packFile->HEAD_NumberOfFiles; i++)
	{
		AddStringToPackStructure(packFile->FLST_FileNames[i]);
		AddNullCharToPackStructure();
		AddUnsignedIntToPackStructure(packFile->FLST_FilePosition[i]);
		AddUnsignedIntToPackStructure(packFile->FLST_FileSize[i]);
	}
	//for (unsigned int i = 0; i < packStructure.size(); i++)
	//	printf("%X ", packStructure[i]);
}

std::vector<unsigned char> PackEngine::ConvertToChars(std::string text)
{
	std::vector<unsigned char> chars;
	chars.reserve(text.size());
	for (unsigned int i = 0; i < text.size(); i++)
		chars.push_back(text[i]);
	return chars;
}

void PackEngine::AddStringToPackStructure(std::string text)
{
	for (unsigned int i = 0; i < text.size(); i++)
		packStructure.push_back(text[i]);
}

void PackEngine::AddUnsignedIntToPackStructure(unsigned int value)
{
	std::string bytes = ConvertUnsignedIntToChar4(value);

	// Writing unsigned int value as bytes
	packStructure.push_back(bytes[0]);
	packStructure.push_back(bytes[1]);
	packStructure.push_back(bytes[2]);
	packStructure.push_back(bytes[3]);
}

void PackEngine::AddNullCharToPackStructure()
{
	packStructure.push_back('\0');
}

std::vector<char> PackEngine::ReadFile(std::string path)
{
	unsigned fileSize = 0;
	std::ifstream infile(path, std::ios::binary | std::ios::ate);
	fileSize = (unsigned int)infile.tellg();
	infile.seekg(0, std::ios::beg);

	std::vector<char> buffer(fileSize);
	if (infile.read(buffer.data(), fileSize))
	{
		return buffer;
	}

	std::cout << "Error: Could not read: " << path << std::endl;
	return std::vector<char>();
}

bool PackEngine::CheckFormatVersion(std::ifstream& file)
{
	unsigned int formatVersionBufferSize = (unsigned int)packFormatVersion.size();
	std::vector<char> formatVersionBuffer(formatVersionBufferSize);
	if (file.read(formatVersionBuffer.data(), formatVersionBufferSize))
	{
		std::string tempStr = formatVersionBuffer.data();
		if (tempStr.substr(0, formatVersionBufferSize) != packFormatVersion)
		{
			printf("Error: Wrong format version.");
			gotError = true;
			return false;
		}
	}
	file.seekg(formatVersionBufferSize);
	return true;
}

bool PackEngine::ReadRootSection(std::ifstream& file)
{
	PackSection packSection = ReadSection(file, 8);
	if (gotError)
		return false;

	std::string sectionName = "";
	sectionName.append(packSection.name, 0, 4);
	if (sectionName != "ROOT")
	{
		gotError = true;
		std::cout << "ROOT section not found" << std::endl;
		return false;
	}
	packFile->ROOT_Position = packSection.position;
	packFile->ROOT_Size = packSection.size;
	return true;
}


bool PackEngine::ReadHeadSection(std::ifstream& file)
{
	PackSection packSection = ReadSection(file, 20);
	if (gotError)
		return false;

	std::string sectionName = "";
	sectionName.append(packSection.name, 0, 4);
	if (sectionName != "HEAD")
	{
		gotError = true;
		std::cout << "HEAD section not found" << std::endl;
		return false;
	}
	packFile->HEAD_Position = packSection.position;
	packFile->HEAD_Size = packSection.size;
	return true;
}

bool PackEngine::ReadFlstSection(std::ifstream& file)
{
	PackSection packSection = ReadSection(file, 32);
	if (gotError)
		return false;

	std::string sectionName = "";
	sectionName.append(packSection.name, 0, 4);
	if (sectionName != "FLST")
	{
		gotError = true;
		std::cout << "FLST section not found" << std::endl;
		return false;
	}
	packFile->FLST_Position = packSection.position;
	packFile->FLST_Size = packSection.size;
	return true;
}

bool PackEngine::ReadFdatSection(std::ifstream& file)
{
	PackSection packSection = ReadSection(file, 44);
	if (gotError)
		return false;

	std::string sectionName = "";
	sectionName.append(packSection.name, 0, 4);
	if (sectionName != "FDAT")
	{
		gotError = true;
		std::cout << "FDAT section not found" << std::endl;
		return false;
	}
	packFile->FDAT_Position = packSection.position;
	packFile->FDAT_Size = packSection.size;
	return true;
}

PackSection PackEngine::ReadSection(std::ifstream& file, unsigned int location)
{
	PackSection packSection;
	packSection.name[0] = '\0';
	packSection.name[1] = '\0';
	packSection.name[2] = '\0';
	packSection.name[3] = '\0';
	packSection.position = 0;
	packSection.size = 0;

	// Read 12 bytes
	unsigned int bufferSize = 12;
	std::vector<char> buffer(bufferSize);
	if (!file.read(buffer.data(), bufferSize))
	{
		printf("Error: Could not read section.");
		gotError = true;
		return packSection;
	}

	// Fill in section name into the struct
	for (unsigned int i = 0; i < 4; i++)
		packSection.name[i] = buffer[i];

	// Fill in position of the section
	packSection.position = ConvertChar4ToUnsignedInt(&buffer[4]);

	// Fill in size of the section
	packSection.size = ConvertChar4ToUnsignedInt(&buffer[8]);

	return packSection;
}

bool PackEngine::FillHeadSection(std::ifstream& file)
{
	// Read 20 bytes
	unsigned int bufferSize = packFile->HEAD_Size;
	std::vector<char> buffer(bufferSize);
	if (!file.read(buffer.data(), bufferSize))
	{
		printf("Error: Could not read section.");
		gotError = true;
		return false;
	}

	// Number of files
	packFile->HEAD_NumberOfFiles = ConvertChar4ToUnsignedInt(&buffer[0]);

	// Total data size
	packFile->HEAD_TotalDataSize = ConvertChar4ToUnsignedInt(&buffer[4]);

	// Data version
	std::string dataVersion = "";
	for (unsigned int i = 8; i < 20; i++)
		dataVersion.append(&buffer[i]);
	packFile->HEAD_DataVersion = dataVersion.substr(0, 12);;

	return true;
}

bool PackEngine::FillFlstSection(std::ifstream& file)
{
	// Read X bytes
	unsigned int bufferSize = packFile->FLST_Size;
	std::vector<char> buffer(bufferSize);
	if (!file.read(buffer.data(), bufferSize))
	{
		printf("Error: Could not read section.");
		gotError = true;
		return false;
	}

	unsigned int last = 0;
	std::string tempStr = "";
	for (unsigned int i = 0; i < bufferSize; i++)
	{
		if (buffer[i] == '\0')
		{
			packFile->FLST_FileNames.push_back(tempStr);
			tempStr.clear();
			char tempC[4];
			tempC[0] = buffer[i + 1];
			tempC[1] = buffer[i + 2];
			tempC[2] = buffer[i + 3];
			tempC[3] = buffer[i + 4];
			packFile->FLST_FilePosition.push_back(ConvertChar4ToUnsignedInt(tempC));
			tempC[0] = buffer[i + 5];
			tempC[1] = buffer[i + 6];
			tempC[2] = buffer[i + 7];
			tempC[3] = buffer[i + 8];
			packFile->FLST_FileSize.push_back(ConvertChar4ToUnsignedInt(tempC));
			i += 8;
			last = i;
			continue;
		}
		tempStr.append(&buffer[i], 1);
	}

	return true;
}

void PackEngine::UnpackFile(unsigned int fileIndex)
{
	unsigned int bufferSize = packFile->FLST_FileSize[fileIndex];
	char* buffer = new char[bufferSize];

	std::ifstream infile(packFileName, std::ios::binary);
	infile.seekg(packFile->FLST_FilePosition[fileIndex], std::ios::beg);
	infile.read(buffer, bufferSize);

	std::ofstream outfile(packFile->FLST_FileNames[fileIndex], std::ofstream::binary | std::ofstream::trunc);
	outfile.write(buffer, bufferSize);
	outfile.close();

	delete[] buffer;
}
