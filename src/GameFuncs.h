#ifndef GAMEFUNCS_H
#define GAMEFUNCS_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

char* assetPath(const char* assetSubPath);
void logMessage(SDL_PRINTF_FORMAT_STRING const char *fmt, ...);
void WriteText(TTF_Font* FontIn,const char* Tekst,int NrOfChars,int X,int Y,int YSpacing,SDL_Color ColorIn,bool Centered);
char chr(int ascii);
int ord(char chr);
int randint(int min, int max);
#endif