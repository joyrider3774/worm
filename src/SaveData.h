#ifndef SAVEDATA_H
#define SAVEDATA_H

#include "Worm.h"

struct SaveData {
	int highScores[MaxGameModes];
};

SaveData save;

void LoadSavedData()
{
	for (int i = 0; i < MaxGameModes; i++)
		save.highScores[i] = 0;
	FILE *Fp;
	char Filename[FILENAME_MAX];
	sprintf(Filename,"%s/worm_sdl3.dat",SDL_getenv("HOME") == NULL ? ".": SDL_getenv("HOME"));
	Fp = fopen(Filename,"rb");
	if (Fp)
	{
		fread(&save,sizeof(SaveData),1,Fp);
		fclose(Fp);
	}
}

void SaveSavedData()
{
	FILE *Fp;
	char Filename[FILENAME_MAX];
	sprintf(Filename,"%s/worm_sdl3.dat",SDL_getenv("HOME") == NULL ? ".": SDL_getenv("HOME"));
	Fp = fopen(Filename,"wb");
	if (Fp)
	{
		fwrite(&save,sizeof(SaveData),1,Fp);
		fclose(Fp);
	}
}

#endif