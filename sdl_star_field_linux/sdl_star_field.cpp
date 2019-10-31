//-----------------------------------------------------------------------------
//           Name: sdl_star_field.cpp
//         Author: Kevin Harris
//  Last Modified: 08/09/04
//    Description: This sample demonstrates how to create a star field 
//                 simulation using SDL by directly writing pixels to a 
//                 SDL_Surface.
//
//   Control Keys: Left  - Stars scroll left
//                 Right - Stars scroll right
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

//-----------------------------------------------------------------------------
// SYMBOLIC CONSTANTS
//-----------------------------------------------------------------------------
const int SCREEN_WIDTH    = 640;
const int SCREEN_HEIGHT   = 480;
const int COLOR_DEPTH     = 8;
const int TOTAL_STARS     = 250;
const int UPDATE_INTERVAL = 30;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
SDL_Surface *g_screenSurface = NULL;
bool g_bScrollLeft = false;

struct STAR
{
    int x;        // Star posit x
    int y;        // Star posit y
    int velocity; // Star velocity
};

STAR stars[TOTAL_STARS]; // Star field array

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int main(int argc, char *argv[]);
void init(void);
void shutDown(void);
void scrollStars(void);
void renderPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B);
Uint32 timeLeft(void);
void render(void);

//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
int main( int argc, char *argv[] )
{
    init();

    bool bDone = false;

    while( bDone == false )
    {
        SDL_Event event;

        while( SDL_PollEvent( &event ) )
        {
            if( event.type == SDL_QUIT )  
                bDone = true;

            if( event.type == SDL_KEYDOWN )
            {
                if( event.key.keysym.sym == SDLK_ESCAPE ) 
                    bDone = true;

                if( event.key.keysym.sym == SDLK_LEFT ) 
                    g_bScrollLeft = true;

                if( event.key.keysym.sym == SDLK_RIGHT ) 
                    g_bScrollLeft = false;
            }
        }

        scrollStars();
        
        render();
    }

    shutDown();

    return 0;
}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
void init( void )
{
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        exit(1);
    }

    atexit( SDL_Quit );

    g_screenSurface = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_DEPTH, 
                                        SDL_HWSURFACE | SDL_DOUBLEBUF );

    if( g_screenSurface == NULL )
    {
        printf( "Unable to set video: %s\n", SDL_GetError() );
        exit(1);
    }

    //
    // Initialize stars...
    //

    for( int i = 0; i < TOTAL_STARS; i++ )
    {
        stars[i].x = rand()%SCREEN_WIDTH;
        stars[i].y = rand()%SCREEN_HEIGHT;
        stars[i].velocity = 1 + rand()%16;
    }
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    SDL_FreeSurface( g_screenSurface );
}

//-----------------------------------------------------------------------------
// Name: scrollStars()
// Desc: 
//-----------------------------------------------------------------------------
void scrollStars( void )
{
    if( g_bScrollLeft == true )
    {
        // Scroll stars to the left
        for( int i = 0; i< TOTAL_STARS; i++ )
        {
            // Move the star
            stars[i].x -= stars[i].velocity;

            // If the star falls off the screen's edge, wrap it around
            if( stars[i].x <= 0 )
                stars[i].x = SCREEN_WIDTH;
        }
    }
    else
    {
        // Scroll stars to the right
        for( int i = 0; i < TOTAL_STARS; i++ )
        {
            // Move the star
            stars[i].x += stars[i].velocity;

            // If the star falls off the screen's edge, wrap it around
            if( stars[i].x >= SCREEN_WIDTH )
                stars[i].x -= SCREEN_WIDTH;
        }
    }
}

//-----------------------------------------------------------------------------
// Name: renderPixel()
// Desc: 
//-----------------------------------------------------------------------------
void renderPixel( int x, int y, Uint8 R, Uint8 G, Uint8 B )
{
    Uint32 color = SDL_MapRGB( g_screenSurface->format, R, G, B );

    switch( g_screenSurface->format->BytesPerPixel )
    {
        case 1: // Assuming 8-bpp
        {
            Uint8 *bufp;
            bufp = (Uint8 *)g_screenSurface->pixels + y*g_screenSurface->pitch + x;
            *bufp = color;
        }
        break;

        case 2: // Probably 15-bpp or 16-bpp
        {
            Uint16 *bufp;
            bufp = (Uint16 *)g_screenSurface->pixels + y*g_screenSurface->pitch/2 + x;
            *bufp = color;
        }
        break;

        case 3: // Slow 24-bpp mode, usually not used
        {
            Uint8 *bufp;
            bufp = (Uint8 *)g_screenSurface->pixels + y*g_screenSurface->pitch + x * 3;

            if( SDL_BYTEORDER == SDL_LIL_ENDIAN )
            {
                bufp[0] = color;
                bufp[1] = color >> 8;
                bufp[2] = color >> 16;
            } 
            else 
            {
                bufp[2] = color;
                bufp[1] = color >> 8;
                bufp[0] = color >> 16;
            }
        }
        break;

        case 4: // Probably 32-bpp
        {
            Uint32 *bufp;
            bufp = (Uint32 *)g_screenSurface->pixels + y*g_screenSurface->pitch/4 + x;
            *bufp = color;
        }
        break;
    }
}

//-----------------------------------------------------------------------------
// Name: timeLeft()
// Desc: 
//-----------------------------------------------------------------------------
Uint32 timeLeft( void )
{
    static Uint32 timeToNextUpdate = 0;
    Uint32 currentTime;

    currentTime = SDL_GetTicks();

    if( timeToNextUpdate <= currentTime ) 
    {
        timeToNextUpdate = currentTime + UPDATE_INTERVAL;
        return 0;
    }

    return( timeToNextUpdate - currentTime );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    SDL_Delay( timeLeft() );

    SDL_FillRect( g_screenSurface, NULL, SDL_MapRGB( g_screenSurface->format, 0, 0, 0));

    //
    // Lock the screen's surface...
    //

    if( SDL_MUSTLOCK( g_screenSurface ) )
    {
        if( SDL_LockSurface(g_screenSurface) < 0 )
            return;
    }

    //
    // Plot each star as a single white pixel...
    //

    for( int i = 0; i < TOTAL_STARS; i++ )
        renderPixel( stars[i].x, stars[i].y, 255, 255, 255 );

    //
    // Unlock the screen's surface...
    //

    if( SDL_MUSTLOCK( g_screenSurface ) )
        SDL_UnlockSurface( g_screenSurface );

    SDL_Flip( g_screenSurface );
}
