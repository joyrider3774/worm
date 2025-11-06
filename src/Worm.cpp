
#if defined _WIN32 || defined __CYGWIN__
    #include <windows.h>
    #undef TRANSPARENT
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <stdio.h>
#include <math.h>
#include <cstring>
#include <time.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "CInput.h"
#include "Worm.h"
#include "SaveData.h"
#include "GameFuncs.h"

int score = 0, numTunnelSections = 0, numVisibleTunnelSections = 0, selSeed = 0, seed = 1, tunnelPlayableGap = StartTunnelPlayableGap, obstacleCount = 0, collectibleCount = 0, tunnelSpeed = StartTunnelSpeed;
int gameMode = 0, speedTarget = StartSpeedTarget, startDelay=0, MaxObstacles = 4, MaxCollectibles = 3;
float player_y = 250, playerSpeed = 0;
SDL_FRect tunnelParts[ScreenWidth*2 + OffScreenTunnelSections * 2]; // in case spacing is 1
SDL_Point playerTrail[ScreenWidth];
SDL_FRect obstacles[10];
SDL_FRect collectibles[10];
bool playing = false;


SDL_Texture* Screen,*Buffer;
TTF_Font* MonoFont;
SDL_Joystick *Joystick;
Uint32 NextTime=0;
bool disableJoysticks = false;
bool nodelay = false;
int WINDOW_WIDTH = 640;
int WINDOW_HEIGHT = 360;
SDL_Window *SdlWindow = NULL;
SDL_Renderer *Renderer = NULL;
Uint64 frameticks = 0, lastfpstime = 0;
double frameTime = 0, avgfps = 0;
Uint32 framecount = 0;
double fpsSamples[FPS_SAMPLES];
int fpsAvgCount = 0, skipCounter = 10;
char basePath[FILENAME_MAX];
bool fullScreen = false;
bool showfps = false;
bool quit = false;
CInput *Input = NULL;

void drawObstacles()
{
    //set cyan color
    SDL_SetRenderDrawColor(Renderer, 0, 255, 255, 255); 
    for(int i = 0; i < obstacleCount; i++)
        //don't draw not used obstacles
        if((obstacles[i].x > -ObstacleWidth) && (obstacles[i].y > 0))
            SDL_RenderFillRect(Renderer, &obstacles[i]);
}

void moveObstacles()
{
    //for each obstacle
    for (int i = 0; i < obstacleCount; i++)
        //move it at tunnelSpeed
        obstacles[i].x -= tunnelSpeed;
    
    //when have all obstacles on screen
    if (obstacleCount == MaxObstacles)
    {
        //for each obstacle
        for (int i = 0; i < obstacleCount; i++)
        {
            //if obstacle goes of screen to the left
            if(obstacles[i].x + obstacles[i].w < 0 )
            {
                //erase it from the array by moving all other obstalces one position down
                for (int j = 0; j < obstacleCount; j++)
                {
                    obstacles[j].x = obstacles[j+1].x;
                    obstacles[j].y = obstacles[j+1].y;
                }

                //and create a new obstacle at the right side of the screen
                obstacles[obstacleCount-1].x =  ScreenWidth;
                obstacles[obstacleCount-1].y =  tunnelParts[numVisibleTunnelSections*2].h + randint(ObstacleSpaceFromTunnel, tunnelPlayableGap - ObstacleHeight - 2*ObstacleSpaceFromTunnel);
                obstacles[obstacleCount-1].w = ObstacleWidth;
                obstacles[obstacleCount-1].h = ObstacleHeight;
            }
        }
    }

    //when we have no obstacles or the last added obstacle is smaller than the spacing between obstacles from right side of screen
    if ((obstacleCount == 0) || ((obstacleCount > 0) && (obstacleCount < MaxObstacles) && (obstacles[obstacleCount-1].x < ScreenWidth - (ScreenWidth / MaxObstacles))))
    {
        //add a new obstacles
        obstacles[obstacleCount].x =  ScreenWidth;
        obstacles[obstacleCount].y =  tunnelParts[numVisibleTunnelSections*2].h + randint(ObstacleSpaceFromTunnel, tunnelPlayableGap - ObstacleHeight - 2* ObstacleSpaceFromTunnel);
        obstacles[obstacleCount].w = ObstacleWidth;
        obstacles[obstacleCount].h = ObstacleHeight;
        obstacleCount++;
    }
}

void drawCollectibles()
{
    //set yellow color
    SDL_SetRenderDrawColor(Renderer, 255, 255, 0, 255); 
    for(int i = 0; i < collectibleCount; i++)
        //don't draw not used collectible
        if((collectibles[i].x > 0) && (collectibles[i].y > 0))
            SDL_RenderFillRect(Renderer, &collectibles[i]);
  
}

void moveCollectibles()
{
    //for each collectible
    for (int i = 0; i < collectibleCount; i++)
        //move it at tunnelSpeed
        collectibles[i].x -= tunnelSpeed;

    //when we have no collectible or the last added collectible is smaller than the spacing between collectible from right side of screen
    if ((collectibleCount == 0) || ((collectibleCount > 0) && (collectibleCount < MaxCollectibles) && (collectibles[collectibleCount-1].x < ScreenWidth - ((ScreenWidth - player_x) / MaxCollectibles))))
    {
        //add a new collectible
        collectibles[collectibleCount].x =  ScreenWidth;
        collectibles[collectibleCount].y =  tunnelParts[numVisibleTunnelSections*2].h + randint(CollectibleSpaceFromTunnel, tunnelPlayableGap - CollectibleHeight - 2 * CollectibleSpaceFromTunnel);
        collectibles[collectibleCount].w = CollectibleWidth;
        collectibles[collectibleCount].h = CollectibleHeight;
        collectibleCount++;
    }
}


void drawPlayer()
{
    //set yellow color
    SDL_SetRenderDrawColor(Renderer, 255, 0, 255, 255);
    SDL_FRect Rect;
    Rect.x = player_x-2;
    Rect.y = player_y-2;
    Rect.w = 5;
    Rect.h = 5; 
    SDL_RenderFillRect(Renderer, &Rect);
    for (int x = 0; x <=  player_x; x++)
    {
        //don't draw not used array pieces
        if ((playerTrail[x].y > 0) && (playerTrail[x].x > 0))
        {
            Rect.x = playerTrail[x].x-2;
            Rect.y = playerTrail[x].y-2;
            Rect.w = 5;
            Rect.h = 5; 
            SDL_RenderFillRect(Renderer, &Rect);
            if (x > 0)
                if ((playerTrail[x-1].y > 0) && (playerTrail[x-1].x > 0))
                    for (int y = 0; y < 6; y++)
                        SDL_RenderLine(Renderer, playerTrail[x].x, playerTrail[x].y-2+y, (int)playerTrail[x-1].x, (int)playerTrail[x-1].y-2+y);
        }
    }
}

void movePlayer()
{
    if(Input->JoystickHeld(0, 0) || Input->KeyboardHeld(SDLK_SPACE) ||  Input->KeyboardHeld(SDLK_A) ||
        Input->MouseHeld(0, SDL_BUTTON_LEFT))
        playerSpeed += Gravity;
    else
        playerSpeed -= Gravity;

    player_y -= playerSpeed;
    
    //add position to player trail
    for (int x = 0; x <=  player_x; x++)
    {
        playerTrail[x].x = playerTrail[x+1].x-tunnelSpeed;
        playerTrail[x].y = playerTrail[x+1].y;
    }
    playerTrail[player_x].x = player_x;
    playerTrail[player_x].y = player_y;

    SDL_FRect playerRect;
    playerRect.x = player_x -2;
    playerRect.y = player_y -2;
    playerRect.w = PlayerWidthHeight;
    playerRect.h = PlayerWidthHeight;
    

    int playerTunnelSection = player_x / tunnelSectionWidth*2;
    //player is inside tunnel section
    for (int i = playerTunnelSection -4; i <= playerTunnelSection + 4; i++)
    {
        if (checkCollision(&playerRect, &tunnelParts[i]))
            playing = false;
    }

    //player is inside obstacle
    for (int i = 0; i < MaxObstacles; i++)
    {
        if (checkCollision(&playerRect, &obstacles[i]))
            playing = false;
    }

    
    for (int i = 0; i < MaxCollectibles; i++)
    {
        //player is inside collectible (added )
        if (checkCollision(&playerRect, &collectibles[i]))
        //debug
        //if (player_x > collectibles[i].x + collectibles[i].w)
        {
            //erase it from the array by moving all other obstalces one position down
            for (int j = 0; j < collectibleCount; j++)
            {
                collectibles[j].x = collectibles[j+1].x;
                collectibles[j].y = collectibles[j+1].y;
            }

            //and create a new obstacle at the right side of the screen
            if (collectibleCount > 0)
            {
                collectibles[collectibleCount - 1].x = ScreenWidth;
                collectibles[collectibleCount - 1].y = tunnelParts[numVisibleTunnelSections * 2].h + randint(CollectibleSpaceFromTunnel, tunnelPlayableGap - CollectibleHeight - 2 * CollectibleSpaceFromTunnel);
                collectibles[collectibleCount - 1].w = CollectibleWidth;
                collectibles[collectibleCount - 1].h = CollectibleHeight;
            }
        }

        //collectible is futher away than playerx (player missed to pick it up)
        if (player_x - 10 > collectibles[i].x + collectibles[i].w)
            playing = false;
    }

    //player is out of bounds
    if ((player_y < 0) || (player_y > ScreenHeight))
        playing = false;

    //debug
    //playing = true;
}

void createTunnel()
{
    //grab a height
    int top_height =  randint(0, tunnelPlayableGap);
    
    for(int i = 0; i <= numTunnelSections; i++)
    {
        //grab a height based on previous height with tunnelSpacer deviation of height
        top_height = randint(top_height - tunnelSpacer, top_height + tunnelSpacer);        
        
        //make sure it does not exceed our playable gap
        if (top_height < 0)
            top_height = 0;
        else
        {
            if (top_height > tunnelPlayableGap)
                top_height = tunnelPlayableGap;
        }
        
        //set player y position based on tunnel section where player is
        if((i * tunnelSectionWidth <= player_x) && ((i+1) * tunnelSectionWidth >= player_x))
            player_y = top_height + tunnelPlayableGap / 2;

        //top of tunnel
        tunnelParts[i*2].x = i * tunnelSectionWidth;
        tunnelParts[i*2].y = 0;
        tunnelParts[i*2].w = tunnelSectionWidth;
        tunnelParts[i*2].h = top_height;

        //bottom of tunnel
        tunnelParts[i*2+1].x = i * tunnelSectionWidth;
        tunnelParts[i*2+1].y = top_height + tunnelPlayableGap;
        tunnelParts[i*2+1].w = tunnelSectionWidth;
        tunnelParts[i*2+1].h = ScreenHeight - top_height - tunnelPlayableGap;
    }
}

void drawTunnel()
{
    //set green color
    SDL_SetRenderDrawColor(Renderer, 0, 255, 0, 255);
    for(int i = 0; i <= numTunnelSections * 2; i += 2)
    {
        SDL_RenderFillRect(Renderer, &tunnelParts[i]);
        SDL_RenderFillRect(Renderer, &tunnelParts[i+1]);
    }
}

void moveTunnel()
{
    //for every tunnel section
    for(int j = 0; j <= numTunnelSections; j++)
    {
        //move top & bottom tunnel part
        tunnelParts[j*2].x = tunnelParts[j*2].x - tunnelSpeed;
        tunnelParts[j*2+1].x = tunnelParts[j*2+1].x - tunnelSpeed;
    }
    
    bool increaseTunnelSpeed = false;

    for (int j = 0; j < numTunnelSections * 2; j++)
    {
        
        //if tunnel section are back on screen, break out of loop
        //tunnel sections are kept from left to right (lowest x)
        if (tunnelParts[j].x + tunnelSectionWidth >= 0)
            break;
        else
        //if left most tunnel sections is offscreen on the left
        {
            //erase that section from the arrray by moving all other section down in the array
            for (int i = 0; i <= numTunnelSections;i++)
            {
                tunnelParts[i*2].x = tunnelParts[i*2+2].x;
                tunnelParts[i*2].y = tunnelParts[i*2+2].y;
                tunnelParts[i*2].w = tunnelParts[i*2+2].w;
                tunnelParts[i*2].h = tunnelParts[i*2+2].h;
                tunnelParts[i*2+1].x = tunnelParts[i*2+3].x;
                tunnelParts[i*2+1].y = tunnelParts[i*2+3].y;
                tunnelParts[i*2+1].w = tunnelParts[i*2+3].w;
                tunnelParts[i*2+1].h = tunnelParts[i*2+3].h;
            }

            //create new piece at the end of the array
            int lastElement = numTunnelSections * 2;
            
            // place the new section exactly after the current rightmost
            int newX = tunnelParts[lastElement - 2].x + tunnelSectionWidth;

            // --- randomize top height (clamped as before) ---
            int top_height = randint(
                tunnelParts[lastElement - 2].h - tunnelSpacer,
                tunnelParts[lastElement - 2].h + tunnelSpacer
            );

            if (top_height < 0)
                top_height = 0;
            else
            {
                if (top_height > tunnelPlayableGap)
                    top_height = tunnelPlayableGap;
            }

            // --- assign new top & bottom tunnel parts ---
            tunnelParts[lastElement].x = newX;
            tunnelParts[lastElement].y = 0;
            tunnelParts[lastElement].w = tunnelSectionWidth;
            tunnelParts[lastElement].h = top_height;

            //bottom of tunnel
            tunnelParts[lastElement + 1].x = newX;
            tunnelParts[lastElement + 1].y = top_height + tunnelPlayableGap;
            tunnelParts[lastElement + 1].w = tunnelSectionWidth;
            tunnelParts[lastElement + 1].h = ScreenHeight - top_height - tunnelPlayableGap;

            //score increases with every section passed
            score += 1;
            if (seed < maxSeed)
            {
                if (score > save.highScores[gameMode * maxSeed + seed])
                    save.highScores[gameMode * maxSeed + seed] = score;
            }
            else if (score > save.highScores[gameMode * maxSeed])
                save.highScores[gameMode * maxSeed] = score;

            //make tunnel smaller
            if((gameMode == 0) || (gameMode == 3))
                if(tunnelPlayableGap > TunnelMinimumPlayableGap)
                    if(score % 4 == 0)
                        tunnelPlayableGap -= 1;
            
            //need to increase speed ?
            if((gameMode == 1) || (gameMode == 2) || (gameMode == 3))
                //if(tunnelSpeed < MaxTunnelSpeed)
                    if(score % (speedTarget) == 0)
                        increaseTunnelSpeed = true;
        }
    }

    if(increaseTunnelSpeed)
    {                        
        tunnelSpeed += 1;
        speedTarget *=2;
    }  
}

void fixed_SDL_srand(Uint64 aseed)
{
    SDL_srand(aseed);
    SDL_rand(INT_MAX);
}

void startGame(int mode)
{
    if (selSeed == 0)
    {
        fixed_SDL_srand(SDL_GetTicks());
        seed = randint(2, maxSeed);
    }
    else
    {
        if (selSeed == 1)
        {
            fixed_SDL_srand(SDL_GetTicks());
            seed = randint(2, 2000000);
        }
        else
            seed = selSeed;
    }
    fixed_SDL_srand(seed);
    playerSpeed = 0;
    tunnelPlayableGap = StartTunnelPlayableGap;
    score = 0;
    obstacleCount = 0;
    collectibleCount = 0;
    playing = true;
    tunnelSpeed = StartTunnelSpeed;
    speedTarget = StartSpeedTarget;
    gameMode = mode;
    startDelay = 60;
    if (gameMode == 0)
        MaxObstacles = 4;
    if (gameMode == 2)
        MaxObstacles = 2;
    if (gameMode == 4)
        MaxCollectibles = 3;
    //set some defaults in the arrays
    for(int i = 0; i < ScreenWidth + OffScreenTunnelSections; i++)
    {
        if(i < ScreenWidth)
        {
            playerTrail[i].x = 0;
            playerTrail[i].y = 0;
        }
        tunnelParts[i*2].x = 0;
        tunnelParts[i*2+1].x = 0;
        tunnelParts[i*2].w = 0;
        tunnelParts[i*2+1].w = 0;
        tunnelParts[i*2].h = 0;
        tunnelParts[i*2+1].h = 0;
        tunnelParts[i*2].y = 0;
        tunnelParts[i*2+1].y = 0;
    }
    for(int i = 0 ; i < MaxObstacles; i++)
    {
        obstacles[i].x = ScreenWidth;
        obstacles[i].y = 0;
        obstacles[i].w = 0;
        obstacles[i].h = 0;
    }

    for(int i = 0 ; i < MaxCollectibles; i++)
    {
        collectibles[i].x = ScreenWidth;
        collectibles[i].y = 0;
        collectibles[i].w = 0;
        collectibles[i].h = 0;
    }
    createTunnel();
}

void drawBackGround()
{
    SDL_SetRenderDrawColor(Renderer, 0,0,0,255);
    SDL_RenderClear(Renderer);
}

void drawScreenBorder()
{
    //A Darker green
    SDL_SetRenderDrawColor(Renderer, 0,66,0,255);
    SDL_FRect Rect;
    for (int i = 0; i < ScreenBorderWidth; i++)
    {
        Rect.x = i;
        Rect.y = i;
        Rect.w = ScreenWidth -2*i;
        Rect.h = ScreenHeight -2*i;
        SDL_RenderRect(Renderer, &Rect);
    }
}

void gameFrame()
{  
    frameticks = SDL_GetPerformanceCounter();
    SDL_SetRenderTarget(Renderer, Buffer);
    char Text[250];
    char Text2[100];
    char nr[100];
	SDL_Color colorWhite;
    colorWhite.a = 255;
    colorWhite.r = 255;
    colorWhite.g = 92;
    colorWhite.b = 92;
    Input->Update();
    if (Input->Ready() && (((Input->KeyboardHeld(SDLK_LALT) || Input->KeyboardHeld(SDLK_RALT)) && Input->KeyboardHeld(SDLK_RETURN)) ||
        Input->KeyboardHeld(SDLK_F) || Input->JoystickHeld(0,3)))
    {
        fullScreen = !fullScreen;
        SDL_SetWindowFullscreen(SdlWindow, fullScreen);
        Input->Delay();
    }
    drawBackGround();
    drawTunnel();
    if((gameMode == 0) || (gameMode == 2))
        drawObstacles();
    if(gameMode == 4)
        drawCollectibles();
    drawPlayer();
    drawScreenBorder();
    if(playing)
    {
        if(startDelay == 0)
        {
            moveTunnel();
            if((gameMode == 0) || (gameMode == 2))
                moveObstacles();
            if(gameMode == 4)
                moveCollectibles();
            movePlayer();
            if (!playing)
            {
                SaveSavedData();
                //try to prevent accidental new game starts
                Input->Reset();
                Input->Delay();                
            }
        }
        else
        {
            
            startDelay--;
            if(startDelay > 20)
            {
                strcpy(Text2, "Playing GAME A\n\nREADY");
                Text2[13] = (int)'A' + gameMode;
                WriteText(MonoFont, Text2, strlen(Text2), ScreenWidth / 2, ScreenHeight/3, 1, colorWhite, true);
            }
            else
            {
                if (startDelay > 1)
                    WriteText(MonoFont, "GO!", 3, ScreenWidth / 2, ScreenHeight/2, 1, colorWhite, true);
            }
        
        }
    }
    else
    {
        strcpy(Text, "WORM\n\nPress A To Play GAME A\nClick Here to Play GAME A");
        Text[27] = (int)'A' + gameMode;
        Text[53] = (int)'A' + gameMode;
        strcat(Text, "\nPress Direction To Change Mode\nPressing A Repeadetly\nwill keep the worm alive");
        WriteText(MonoFont, Text, strlen(Text), ScreenWidth / 2, ScreenBorderWidth, 1, colorWhite, true);
        float MouseX = 0, MouseY = 0, TargetMouseY, TargetMouseX;
        SDL_GetMouseState(&MouseX, &MouseY);
        SDL_RenderCoordinatesToWindow(Renderer, 320, 320, &TargetMouseX, &TargetMouseY);
        if(Input->Ready() &&  (Input->JoystickHeld(0, 0) || Input->KeyboardHeld(SDLK_SPACE) || Input->KeyboardHeld(SDLK_A) || 
            Input->KeyboardHeld(SDLK_RETURN) || (Input->MouseHeld(0, SDL_BUTTON_LEFT) && (MouseY < TargetMouseY))))
        {
            Input->Delay();
            startGame(gameMode);
        }

        if(Input->Ready() &&  (Input->JoystickHeld(0, JOYSTICK_LEFT) || Input->KeyboardHeld(SDLK_LEFT)))
        {
            Input->Delay();
            gameMode--;
            if (gameMode < 0)
                gameMode = MaxGameModes -1;
        }

        if(Input->Ready() &&  (Input->JoystickHeld(0, JOYSTICK_RIGHT) || Input->KeyboardHeld(SDLK_RIGHT) ||
            (Input->MouseHeld(0, SDL_BUTTON_RIGHT) && (MouseY <  TargetMouseY)) ||
            (Input->MouseHeld(0, SDL_BUTTON_RIGHT) && (MouseY > TargetMouseY) && (MouseX > TargetMouseX)) ||
            (Input->MouseHeld(0, SDL_BUTTON_LEFT) && (MouseY >  TargetMouseY) && (MouseX > TargetMouseX))))
        {
            Input->Delay();
            gameMode++;
            if (gameMode > MaxGameModes -1)
                gameMode = 0;
        }

        if(Input->Ready() &&  (Input->JoystickHeld(0, JOYSTICK_DOWN) || Input->KeyboardHeld(SDLK_DOWN)))
        {
            Input->Delay();
            selSeed -= 1;
            if(selSeed < 0)
                selSeed = maxSeed-1;
        }

        if(Input->Ready() &&  (Input->JoystickHeld(0, JOYSTICK_UP) || Input->KeyboardHeld(SDLK_UP) ||
            (Input->MouseHeld(0, SDL_BUTTON_LEFT) && (MouseY > TargetMouseY) && (MouseX < TargetMouseX))))
        {
            Input->Delay();
            selSeed += 1;
            if(selSeed > maxSeed-1)
            	selSeed = 0;
        }

        if(Input->Ready() &&  (Input->JoystickHeld(0, 4) || Input->KeyboardHeld(SDLK_PAGEDOWN)))
        {
            Input->Delay();
            selSeed -= 10;
            if(selSeed < 0)
            	selSeed =  maxSeed-1;
        }

        if(Input->Ready() &&  (Input->JoystickHeld(0, 5) || Input->KeyboardHeld(SDLK_PAGEUP) ||
            (Input->MouseHeld(0, SDL_BUTTON_RIGHT) && (MouseY > TargetMouseY) && (MouseX < TargetMouseX))))
        {
            Input->Delay();
            selSeed += 10;
            if(selSeed > maxSeed-1)
            	selSeed = 0; 
        }

        if(Input->Ready() &&  (Input->JoystickHeld(0, 7) || Input->KeyboardHeld(SDLK_R)))
        {
            Input->Delay();
            for (int i = 0; i < MaxGameModes*maxSeed; i++)
                save.highScores[i] = 0;
        }

        if(Input->JoystickHeld(0, 6) || Input->KeyboardHeld(SDLK_Q) || 
            Input->KeyboardHeld(SDLK_ESCAPE) || Input->SpecialsHeld(SPECIAL_QUIT_EV))
            quit = true;
            
    }
    strcpy(Text, "LVL:");
       
    if (selSeed == 0)
	{
		SDL_itoa(seed-1, nr, 10);
		strcat(Text, nr);
		strcat(Text, " RND1");
	}
	else
    {
        if (selSeed == 1)
		{
	    	SDL_itoa(seed-1, nr, 10);
    		strcat(Text, nr);
			strcat(Text, " RND2");
		}
        else
        {
	    	SDL_itoa(selSeed-1, nr, 10);
    		strcat(Text, nr);
		}
    }
    int w, h;
    TTF_GetStringSize(MonoFont, Text, 0, &w, &h);
    WriteText(MonoFont, Text, strlen(Text), ScreenBorderWidth + 1, ScreenHeight -h , 1, colorWhite, false);
    strcpy(Text, "S:");
    SDL_itoa(score, nr, 10);
    strcat(Text, nr);
    strcat(Text, " H:");
    if (selSeed <= 1)
    {
        if (seed >= maxSeed)
            SDL_itoa(save.highScores[gameMode * maxSeed], nr, 10);
        else
            SDL_itoa(save.highScores[gameMode * maxSeed + seed], nr, 10);
    }
    else if (selSeed > 1)
   		SDL_itoa(save.highScores[gameMode * maxSeed + selSeed], nr, 10);
    
        strcat(Text, nr);
    TTF_GetStringSize(MonoFont, Text, 0, &w, &h);
    WriteText(MonoFont, Text, strlen(Text), ScreenWidth -2 -ScreenBorderWidth -w, ScreenHeight-h, 1, colorWhite, false);
       
    if(showfps)
    {
        char fpsText[100];
        sprintf(fpsText, "FPS:%.2f", avgfps);
        TTF_GetStringSize(MonoFont, fpsText, strlen(fpsText), &w, &h);
        SDL_FRect Rect;
        Rect.x = 0.0f;
        Rect.y = 0.0f;
        Rect.w = (float)w;
        Rect.h = (float)h;
        SDL_SetRenderDrawColor(Renderer, 255,255,255,255);
        SDL_RenderFillRect(Renderer, &Rect);
        SDL_Color col = {0,0,0,255};
        WriteText(MonoFont, fpsText, strlen(fpsText), 0, 0, 0, col, false);
    }
    SDL_SetRenderTarget(Renderer, NULL);
    SDL_SetRenderDrawColor(Renderer, 0,0,0,255);
    SDL_RenderClear(Renderer);
    SDL_SetRenderLogicalPresentation(Renderer, ScreenWidth, ScreenHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);        
    SDL_RenderTexture(Renderer, Buffer, NULL, NULL);
    SDL_RenderPresent(Renderer);
    Uint64 frameEndTicks = SDL_GetPerformanceCounter();
    Uint64 FramePerf = frameEndTicks - frameticks;
    frameTime = (double)FramePerf / (double)SDL_GetPerformanceFrequency() * 1000.0f;
    double delay = 1000.0f / FPS - frameTime;
    if (!nodelay && (delay > 0.0f))
        SDL_Delay((Uint32)(delay)); 
    if (showfps)
    {
        if(skipCounter > 0)
        {
            skipCounter--;
            lastfpstime = SDL_GetTicks();
        }
        else
        {
            framecount++;
            if(SDL_GetTicks() - lastfpstime >= 1000)
            {
                for (int i = FPS_SAMPLES-1; i > 0; i--)
                    fpsSamples[i] = fpsSamples[i-1];
                fpsSamples[0] = framecount;
                fpsAvgCount++;
                if(fpsAvgCount > FPS_SAMPLES)
                    fpsAvgCount = FPS_SAMPLES;
                int fpsSum = 0;
                for (int i = 0; i < fpsAvgCount; i++)
                    fpsSum += fpsSamples[i];
                avgfps = (double)fpsSum / (double)fpsAvgCount;
                framecount = 0;
                lastfpstime = SDL_GetTicks();
            }
        }
    }
}

static void printHelp(const char* exe)
{
	const char* binaryName = SDL_strrchr(exe, '/');
	if (binaryName == NULL)
	{
		binaryName = SDL_strrchr(exe, '\\');
		if(binaryName == NULL)
			binaryName = exe;
	}
	if(binaryName)
		++binaryName;

	printf("Worm Sdl 3 Version\n");
	printf("Usage: %s [-w <WIDTH>] [-h <HEIGHT>] [-f] [-ns] [-a] [-fps] [-nd] [-nj]\n", binaryName);
	printf("\n");
	printf("Commands:\n");
	printf("  -w <WIDTH>: use <WIDTH> as window width\n");
	printf("  -h <HEIGHT>: use <HEIGHT> as window height\n");
	printf("  -f: Run fullscreen\n");
	printf("  -s: Use Software rendering (default is hardware accelerated)\n");
	printf("  -fps: Show fps\n");
	printf("  -nd: no fps delay (run as fast as possible)\n");
	printf("  -nj: disable joystick input\n");
}

int main(int argc, char **argv)
{
	fixed_SDL_srand(time(NULL));
	//attach to potential console when using -mwindows so we can get output in a cmd / msys prompt 
	//but see no console window when running from explorer start menu or so
	#if defined _WIN32 || defined __CYGWIN__
	if(AttachConsole((DWORD)-1))
	{
		freopen("CON", "w", stderr);
		freopen("CON", "w", stdout);
	}
	#endif
	bool fullScreen = false;
	bool useHWSurface = true;
	bool noAudioInit = false;
	for (int i = 0; i < argc; i++)
	{

		if((SDL_strcmp(argv[i], "-?") == 0) || (SDL_strcmp(argv[i], "--?") == 0) || 
			(SDL_strcmp(argv[i], "/?") == 0) || (SDL_strcmp(argv[i], "-help") == 0) || (SDL_strcmp(argv[i], "--help") == 0))
		{
			printHelp(argv[0]);
			return 0;
		}

		if(SDL_strcmp(argv[i], "-f") == 0)
			fullScreen = true;
		
		if(SDL_strcmp(argv[i], "-s") == 0)
			useHWSurface = false;
		
		if(SDL_strcmp(argv[i], "-fps") == 0)
			showfps = true;
		
		if(SDL_strcmp(argv[i], "-nd") == 0)
			nodelay = true;
		
		if(SDL_strcmp(argv[i], "-nj") == 0)
			disableJoysticks = true;

		if(SDL_strcmp(argv[i], "-w") == 0)
			if(i+1 < argc)
				WINDOW_WIDTH = SDL_atoi(argv[i+1]);
		
		if(SDL_strcmp(argv[i], "-h") == 0)
			if(i+1 < argc)
				WINDOW_HEIGHT = SDL_atoi(argv[i+1]);
	}

	if (SDL_Init(SDL_INIT_VIDEO))
	{
		logMessage("SDL Succesfully initialized\n");
        #ifdef __EMSCRIPTEN__
        SDL_SetHint(SDL_HINT_EMSCRIPTEN_FILL_DOCUMENT, "1");
        #endif
		memset(basePath, 0, FILENAME_MAX);
		const char* SDL_BasePath = SDL_GetBasePath();
		if(SDL_BasePath)
			snprintf(basePath, strlen(SDL_BasePath), "%s", SDL_BasePath);
		else
			snprintf(basePath, FILENAME_MAX, "./");

		logMessage("Using Base Path: %s\n", basePath);

		Uint32 WindowFlags = SDL_WINDOW_RESIZABLE;
		
		SdlWindow = SDL_CreateWindow("Worm (SDL3)", WINDOW_WIDTH, WINDOW_HEIGHT, WindowFlags);

		if (SdlWindow)
		{
			if (fullScreen)
			{
				SDL_SetWindowFullscreen(SdlWindow, fullScreen);
			}
			logMessage("Succesfully Set %dx%d\n",WINDOW_WIDTH, WINDOW_HEIGHT);
			if (useHWSurface == 0)
				Renderer = SDL_CreateRenderer(SdlWindow, SDL_SOFTWARE_RENDERER);
			else
				Renderer = SDL_CreateRenderer(SdlWindow, NULL);
			
			if (Renderer)
			{
			
				logMessage("Using Renderer:%s\n", SDL_GetRendererName(Renderer));
				char RenderDriverNames[1000];
				memset(RenderDriverNames, 0, 1000);
				for (int i = 0; i < SDL_GetNumRenderDrivers(); i++)
				{
					if(i > 0)
						strcat(RenderDriverNames, ",");
					strcat(RenderDriverNames, SDL_GetRenderDriver(i));
				}
				logMessage("Available Renders: %s\n",RenderDriverNames);
				logMessage("Succesfully Created Buffer\n");					
                SDL_SetRenderLogicalPresentation(Renderer, ScreenWidth, ScreenHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);
				Buffer = SDL_CreateTexture(Renderer, PIXELFORMAT, SDL_TEXTUREACCESS_TARGET, ScreenWidth, ScreenHeight);
				if (Buffer)
				{
				
					SDL_SetTextureBlendMode(Buffer, SDL_BLENDMODE_BLEND);

					if (TTF_Init())
					{
						logMessage("Succesfully initialized TTF\n");
						char *TmpPath = assetPath("fonts/SpaceMono-Regular.ttf");
						MonoFont = TTF_OpenFont(TmpPath,28);
						SDL_free(TmpPath);
						if (MonoFont)
						{
							logMessage("Succesfully Loaded fonts\n");
							TTF_SetFontStyle(MonoFont,TTF_STYLE_BOLD);
                            LoadSavedData();
	                        fixed_SDL_srand(time(NULL));
                            //these never change and are used a lot in above functions
                            numVisibleTunnelSections = ceil(ScreenWidth / tunnelSectionWidth);
                            numTunnelSections = numVisibleTunnelSections + OffScreenTunnelSections;
                            createTunnel();
							Input = new CInput(InputDelay, disableJoysticks);
							#ifdef __EMSCRIPTEN__
                                emscripten_set_main_loop(gameFrame, 0, 1);
                            #else
                                while (!quit)
                                {
                                    gameFrame();
                                }
                            #endif
							delete Input;
							SaveSavedData();
							TTF_CloseFont(MonoFont);
							MonoFont=NULL;
						}
						else
						{
							logMessage("Failed to Load fonts\n");
						}
						TTF_Quit();
					}
					else
					{
						logMessage("Failed to initialize TTF\n");
					}

	                SDL_DestroyTexture(Buffer);
				}
				else
				{
					logMessage("Failed to create Buffer\n");
				}
				SDL_DestroyRenderer(Renderer);			
			}
			else
               	logMessage("Failed To Create Renderer\n");
            //SDL_DestroyTexture(Icon);
			SDL_DestroyWindow(SdlWindow);
		}
		else
		{
			logMessage("Failed to create window %dx%d\n",WINDOW_WIDTH, WINDOW_HEIGHT);
		}
		
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);
		SDL_Quit();
	}
	else
	{
		logMessage("Couldn't initialise SDL!: %s\n", SDL_GetError());
	}
	return 0;
}