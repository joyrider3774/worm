#ifndef WORM_H
#define WORM_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>

#define PIXELFORMAT SDL_PIXELFORMAT_RGBA32
#define FPS_SAMPLES 5

#define ScreenWidth 640
#define ScreenHeight 360
#define FPS 60

#define ObstacleWidth 12
#define ObstacleHeight 35
#define ObstacleSpaceFromTunnel 10

#define PlayerWidthHeight 5

#define CollectibleWidth 20
#define CollectibleHeight 20
#define CollectibleSpaceFromTunnel 30

#define StartTunnelSpeed 2
#define StartTunnelPlayableGap 180
#define TunnelMinimumPlayableGap 120
#define MaxTunnelSpeed 7
#define OffScreenTunnelSections 3

#define tunnelSectionWidth 8
#define tunnelSpacer 16
#define StartSpeedTarget 50

#define ScreenBorderWidth 7

#define Gravity 0.20

#define player_x 250

#define MaxGameModes 5

#define maxSeed 101

// names for textures
#define TextureFullFont 0

// texture regions for full font texture
#define FirstRegionFullFont 0

#define InputDelay 16

extern char basePath[FILENAME_MAX];
extern SDL_Window *SdlWindow;
extern SDL_Renderer *Renderer;
#endif