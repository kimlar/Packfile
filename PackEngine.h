#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "PackFile.h"
#include "UtilDateTime.h"

struct PackSection
{
	char name[4];
	unsigned int position;
	unsigned int size;
};

class PackEngine
{
public:
	PackEngine(std::string packFileName);
	~PackEngine();

	void Open();
	void Close();
	bool LoadFileToMemory(std::string file, std::vector<unsigned char>& buffer);


	// Asset pipe line functions
	void Compile(std::vector<std::string> sourceFiles);
	void Unpack();
private:
	std::string packFileName;
	std::vector<std::string> sourceFiles;
	bool gotError;

	PackFile* packFile;

	void ReadPackFile();
	void WritePackFile();
	void ClosePackFile();

	unsigned int GetFileSize(std::string path);
	unsigned int GetFileListSize();
	unsigned int GetFileDataPosition();

	void GeneratePackStructure();
	std::vector<unsigned char> packStructure; // It is all except the file data
	std::vector<unsigned char> ConvertToChars(std::string text);
	void AddStringToPackStructure(std::string text);
	void AddUnsignedIntToPackStructure(unsigned int value);
	void AddNullCharToPackStructure();

	std::vector<char> ReadFile(std::string path);

	UtilDateTime utilDateTime;

	//
	// Read pack file
	//
	bool CheckFormatVersion(std::ifstream& file);
	const std::string packFormatVersion = "PACK1000";

	bool ReadRootSection(std::ifstream& file);
	bool ReadHeadSection(std::ifstream& file);
	bool ReadFlstSection(std::ifstream& file);
	bool ReadFdatSection(std::ifstream& file);
	PackSection ReadSection(std::ifstream& file, unsigned int location);

	bool FillHeadSection(std::ifstream& file);
	bool FillFlstSection(std::ifstream& file);

	//
	// Unpack pack file
	//
	void UnpackFile(unsigned int fileIndex);
};
