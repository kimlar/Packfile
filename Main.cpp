#include <iostream>
#include <string>
#include <vector>

#include "PackEngine.h"

/*
Usage:
// Packfile includes
#include <iostream>
#include <string>
#include <vector>
#include "PackEngine.h"

// Loads individual image as texture from memory
SDL_Texture* loadTextureFromMemory(std::string fileName);

// Current displayed texture
SDL_Texture* gTexture = NULL;

// Pack engine
PackEngine* packEngine = nullptr;

// ***************************************

// Create the Pack Engine
packEngine = new PackEngine("Textures.pak");
packEngine->Open();

// Load PNG from memory
gTexture = loadTextureFromMemory("nature-1024x1024.png");
if (gTexture == NULL)
{
printf("Unable to load texture image!\n");
success = false;
}

// Done with the Pack Engine
packEngine->Close();
delete packEngine;
packEngine = nullptr;

// ****************************************

SDL_Texture* loadTextureFromMemory(std::string fileName)
{
std::vector<unsigned char> memFile;
if (!packEngine->LoadFileToMemory(fileName, memFile))
{
printf("Error: Could not load file to memory\n");
}

//The final texture
SDL_Texture* newTexture = NULL;

//Load image at specified path
SDL_RWops* rwOps = SDL_RWFromMem(memFile.data(), (int)memFile.size());
if (rwOps == NULL)
{
printf("RW Ops failed\nError: %s\n", IMG_GetError());
}
SDL_Surface* loadedSurface = IMG_LoadPNG_RW(rwOps);
if (loadedSurface == NULL)
{
printf("Unable to load image %s! IMG_Error: %s\n", fileName.c_str(), IMG_GetError());
}
else
{
//Create texture from surface pixels
newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
if (newTexture == NULL)
{
printf("Unable to create texture from %s! SDL_Error: %s\n", fileName.c_str(), SDL_GetError());
}

//Get rid of old loaded surface
SDL_FreeSurface(loadedSurface);
}

return newTexture;
}
*/

int main(int argc, char* argv[])
{
	bool flagPack = false;
	bool flagUnpack = false;
	std::string packFileName = "";
	std::vector<std::string> packFiles;

	// Show help
	if (argc < 3)
	{
		std::cout << "Packing files:  \tUsage: Packfile.exe -p file.pak file1.txt file2.txt ..." << std::endl;
		std::cout << "Unpacking files:\tUsage: Packfile.txt -u file.pak" << std::endl;
		return 0;
	} else if (argc == 3) // Unpack files
	{
		if(argv[1][0] == '-' && argv[1][1] == 'u')
		{
			flagUnpack = true;
			packFileName = argv[2];
		}
		else
			return 0;
	}
	else if (argc > 3) // Pack files
	{
		if (argv[1][0] == '-' && argv[1][1] == 'p')
		{
			flagPack = true;
			packFileName = argv[2];
			for (int i = 3; i < argc; i++)
				packFiles.push_back(argv[i]);
		}
		else
			return 0;
	}

	// Fire up the pack engine
	PackEngine* packEngine = new PackEngine(packFileName);
	if (flagPack)
	{
		packEngine->Compile(packFiles);
	}
	else if (flagUnpack)
	{
		packEngine->Unpack();
	}
	delete packEngine;
	packEngine = nullptr;

	return 0;
}
