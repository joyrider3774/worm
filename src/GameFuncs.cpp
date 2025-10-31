#include <stdlib.h>
#include <SDL3/SDL.h>
#include "GameFuncs.h"
#include "CInput.h"
#include "Worm.h"

char* assetPath(const char* assetSubPath)
{
	char* Result = (char*) SDL_malloc(FILENAME_MAX);

	snprintf(Result, FILENAME_MAX, "%s/%s", basePath, assetSubPath);
	
	return Result;
}

void logMessage(SDL_PRINTF_FORMAT_STRING const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
#if defined _WIN32 || defined __CYGWIN__
    vprintf(fmt, ap);
#else
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, fmt, ap);
#endif    
    va_end(ap);   
}


void WriteText(TTF_Font* FontIn,const char* Tekst,int NrOfChars,int X,int Y,int YSpacing,SDL_Color ColorIn,bool Centered)
{
	char List[100][255];
	int Lines,Teller,Chars;
	SDL_FRect DstRect;
	SDL_Surface* TmpSurface1;
	SDL_Texture *TmpTexture;
	Lines = 0;
	Chars = 0;
	for (Teller=0;Teller<NrOfChars;Teller++)
	{
		if(Lines > 100)
			break;
		if((Tekst[Teller] == '\n') || (Chars==255))
		{
			List[Lines][Chars]='\0';
			Lines++;
			Chars = 0;
		}
		else
		{
		 	List[Lines][Chars]=Tekst[Teller];
		 	Chars++;
		}
	}
	List[Lines][Chars] = '\0';
	for (Teller=0;Teller <= Lines;Teller++)
	{
		if(strlen(List[Teller]) > 0)
		{
			TmpSurface1 = TTF_RenderText_Blended(FontIn,List[Teller],strlen(List[Teller]) * sizeof(char), ColorIn);
			if(Centered)
			{
				int w;
				SDL_GetCurrentRenderOutputSize(Renderer, &w, NULL);
                DstRect.x = (w /2) - (TmpSurface1->w / 2);
			}
			else
                DstRect.x = X;
			DstRect.y = Y + (Teller) * TTF_GetFontLineSkip(FontIn) + (Teller*YSpacing);
			DstRect.w = TmpSurface1->w;
			DstRect.h = TmpSurface1->h;
			TmpTexture = SDL_CreateTextureFromSurface(Renderer, TmpSurface1);
			SDL_RenderTexture(Renderer, TmpTexture,NULL,&DstRect);
			SDL_DestroyTexture(TmpTexture);
			SDL_DestroySurface(TmpSurface1);
		}
	}
}


char chr(int ascii)
{
	return((char)ascii);
}

int ord(char chr)
{
	return((int)chr);
}

int randint(int min, int max)
{
    return SDL_rand(max - min) + min;
}