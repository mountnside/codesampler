//-----------------------------------------------------------------------------
//           Name: sprite.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: Implementation of Sprite class for gathering together all 
//                 properties and methods related to managing 2D sprite based 
//                 game objects.
//-----------------------------------------------------------------------------

#include "sprite.h"

//-----------------------------------------------------------------------------
// Name: CSprite()
// Desc: Constructor of the CSprite class
//-----------------------------------------------------------------------------
CSprite::CSprite() :
m_nID( 0 ),
m_nState( 0 ),
m_dPosition_x( 0 ),
m_dPosition_y( 0 ),
m_dVelocity_x( 0 ),
m_dVelocity_y( 0 ),
m_bVisible( true ),
m_bCollide( true ),
m_bAutoAnimate( true ),
m_bActive( true ),
m_bScripting( false ),
m_bModifyCollision( false ),
m_bSingleFrame( false ),
m_bDestroy( false ),
m_nFrameRateModifier( 0 ),
m_nFrameSkipCount( 0 ),
m_nFrameWidth( 0 ),
m_nFrameHeight( 0 ),
m_nFramesAcross( 1 ),
m_nFrameOffset_x( 0 ),
m_nFrameOffset_y( 0 ),
m_nWidthScaling( 0 ),
m_nHeightScaling( 0 ),
m_nCurrentAnimation( 0 ),
m_nCurrentFrame( 0 ),
m_nCurrentScript( 0 ),
m_nCollisionTop( 0 ),
m_nCollisionBottom( 0 ),
m_nCollisionLeft( 0 ),
m_nCollisionRight( 0 )
{
	for( int i = 0; i < TOTAL_ANIMATIONS; i++ )
		m_nAnimations[i] = NULL;

	for( i = 0; i < MAX_SIZE; i++ )
	{
		m_chType[i]   = NULL;
		m_chName[i]   = NULL;
		m_chBitmap[i] = NULL;
	}
}

//-----------------------------------------------------------------------------
// Name: ~CSprite()
// Desc: Destructor of the CSprite class
//-----------------------------------------------------------------------------
CSprite::~CSprite()
{

}

//-----------------------------------------------------------------------------
// Name: zeroSpriteValues()
// Desc: Resets all member variables to their default values
//-----------------------------------------------------------------------------
void CSprite::zeroSpriteValues()
{
	m_nID                = 0;
	m_nState             = 0;
	m_dPosition_x        = 0;
	m_dPosition_y        = 0;
	m_dVelocity_x        = 0;
	m_dVelocity_y        = 0;
	m_bVisible           = true;
	m_bCollide           = true;
	m_bAutoAnimate       = true;
	m_bActive            = true;
	m_bScripting         = false;
	m_bModifyCollision   = false;
	m_bSingleFrame       = false;
	m_bDestroy           = false;
	m_nFrameRateModifier = 0;
	m_nFrameSkipCount    = 0;
	m_nFrameWidth        = 0;
	m_nFrameHeight       = 0;
	m_nFramesAcross      = 1;
	m_nFrameOffset_x     = 0;
	m_nFrameOffset_y     = 0;
	m_nWidthScaling      = 0;
	m_nHeightScaling     = 0;
	m_nCurrentAnimation  = 0;
	m_nCurrentFrame      = 0;
	m_nCurrentScript     = 0;
	m_nCollisionTop      = 0;
	m_nCollisionBottom   = 0;
	m_nCollisionLeft     = 0;
	m_nCollisionRight    = 0;

	for( int i = 0; i < TOTAL_ANIMATIONS; i++ )
		m_nAnimations[i] = NULL;

	for( i = 0; i < MAX_SIZE; i++ )
	{
		m_chType[i]   = NULL;
		m_chName[i]   = NULL;
		m_chBitmap[i] = NULL;
	}
}


//-----------------------------------------------------------------------------
// Name: releaseMemory()
// Desc: Releases memory allocated for the sprite's animation arrays
//-----------------------------------------------------------------------------
void CSprite::releaseMemory()
{
	// This function releases any memory that was allocated 
	// from the heap for this sprite object.

	for( int i = 0; i < TOTAL_ANIMATIONS; i++ )
	{
	    // Deallocate the memory that was previously 
		// reserved for each animation sequence.
		if( m_nAnimations[i] != NULL )
		{
			delete [] m_nAnimations[i];
			m_nAnimations[i] = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: loadAnimation()
// Desc: Creates an array of frame numbers to display so animation effects 
//       can be created.
//-----------------------------------------------------------------------------
void CSprite::loadAnimation( int nAnimeNumber, int nStartFrame, int nEndFrame, 
							 AnimeEndOption nOption, int nNextAnimation )
{
	// This function loads the frame numbers that make up an animation 
	// sequence into an array by calculating the frame number values 
	// that exist between the nStartFrame and nEndFrame values passed in. 
	// After it determines how big the array will need to be, it then 
	// creates the array  dynamically on the heap. The pointer to the new 
	// array is then stored in the Animations[] array where it can be 
	// accessed later by specifying the proper index value. The frame 
	// numbers are then pulled from the number range between nStartFrame 
	// and nEndFrame and loaded into the new array.

	bool bOverFlow = false;
	int  nFrameNumber = 0;
	int  nTotalFrames = 0;
	int  i = 0;

	if( nAnimeNumber >= 0 && 
		nAnimeNumber < TOTAL_ANIMATIONS && 
		m_nAnimations[nAnimeNumber] == NULL )
	{
		if( nStartFrame < nEndFrame )
			nTotalFrames = (nEndFrame - nStartFrame);

		if( nStartFrame > nEndFrame  )
			nTotalFrames = (nStartFrame - nEndFrame);

		// Make room for the control codes that 
		// will be added later
		nTotalFrames += 3; 

		m_nAnimations[nAnimeNumber] = new int[nTotalFrames];
		m_nFrameCount[nAnimeNumber] = nTotalFrames;

		nFrameNumber = nStartFrame;

		if( nAnimeNumber >= 0 && nAnimeNumber < TOTAL_ANIMATIONS )
		{
			while( nFrameNumber != (nEndFrame + 1) )
			{
				if( i <= (nTotalFrames - 1) )
					m_nAnimations[nAnimeNumber][i] = nFrameNumber;
				else
					bOverFlow = true;

				if( nStartFrame < nEndFrame )
					{++nFrameNumber;}
				else if( nStartFrame > nEndFrame )
					{--nFrameNumber;}

				++i;
			}
		
			// We now need to attach the control code stored in nOptions
			// so the sprite object will know how to react when it reaches 
			// the last frame in thwe animation sequence.
			if( nOption == LOOP_ANIMATION )
				m_nAnimations[nAnimeNumber][i] = LOOP_ANIMATION;

			if( nOption == MAINTAIN_LAST_FRAME )
				m_nAnimations[nAnimeNumber][i] = MAINTAIN_LAST_FRAME;

			if( nOption == GOTO_NEXT_ANIMATION )
			{
				m_nAnimations[nAnimeNumber][i] = GOTO_NEXT_ANIMATION;
				m_nAnimations[nAnimeNumber][i+1] = nNextAnimation;
			}

			if( nOption == GO_INACTIVE )
				m_nAnimations[nAnimeNumber][i] = GO_INACTIVE;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: loadAnimationString()
// Desc: Creates an array of frame numbers to display so animation effects 
//       can be created.
//-----------------------------------------------------------------------------
void CSprite::loadAnimationString( int nAnimeNumber, char *chString, 
								   AnimeEndOption nOption, int nNextAnimation )
{
	// This function loads the frame numbers that make up an animation 
	// sequence into an array by parsing a string that's passed in.
	// After it determines how big the array will need to be, it then 
	// creates it dynamically on the heap. The pointer to the new array 
	// is then stored in the Animations[] array where it can be accessed 
	// later by specifying the proper index value. The frame numbers are 
	// then parsed out and loaded into the new array.

	char *token = NULL;
	char chDelimiters[] = " ";
	char chBuffer[200] = { NULL };
	bool bOverFlow = false;
	int  nFrameNumber = 0;
	int  nTotalFrames = 0;
	int  i = 0;
	
	if( nAnimeNumber >= 0 && 
		nAnimeNumber < TOTAL_ANIMATIONS && 
		m_nAnimations[nAnimeNumber] == NULL )
	{
		// Count the avaible tokens to find out how 
		// many frames have been listed.
		strcpy( chBuffer, chString );
		token = strtok( chBuffer, chDelimiters );

		while( token != NULL )
		{
			++nTotalFrames;
			token = strtok( NULL, chDelimiters );
		}

		// Make room for the control codes that 
		// will be added later
		nTotalFrames += 3; 

		m_nAnimations[nAnimeNumber] = new int[nTotalFrames];
		m_nFrameCount[nAnimeNumber] = nTotalFrames;

		// Now, tokenize the string and convert the 
		// substrings into integer values.
		strcpy( chBuffer, chString );
		token = strtok( chBuffer, chDelimiters );

		while( token != NULL )
		{
			nFrameNumber = atoi( token );

			if( i <= (nTotalFrames - 1) )
				m_nAnimations[nAnimeNumber][i] = nFrameNumber;
			else
				bOverFlow = true;

			++i;
			token = strtok( NULL, chDelimiters );
		}
	
		// We now need to attach the control code stored in nOptions
		// so the sprite object will know how to react when it reaches 
		// the last frame in the animation sequence.
		if( nOption == LOOP_ANIMATION )
			m_nAnimations[nAnimeNumber][i] = LOOP_ANIMATION;

		if( nOption == MAINTAIN_LAST_FRAME )
			m_nAnimations[nAnimeNumber][i] = MAINTAIN_LAST_FRAME;

		if( nOption == GOTO_NEXT_ANIMATION )
		{
			m_nAnimations[nAnimeNumber][i] = GOTO_NEXT_ANIMATION;
			m_nAnimations[nAnimeNumber][i+1] = nNextAnimation;
		}

		if( nOption == GO_INACTIVE )
			m_nAnimations[nAnimeNumber][i] = GO_INACTIVE;
	}
}

//-----------------------------------------------------------------------------
// Name: incFrame()
// Desc: Increments the current animation to the next frame
//-----------------------------------------------------------------------------
void CSprite::incFrame( bool bUseModifier )
{
	// This function allows the programmer to manually animate the sprite
	// through an animationn sequence by incrementing the sprite's 
	// nCurrentFrame member variable in a safe manner.

	DWORD dwNextNumber;
	int   nTempHolder = 0;

	if( bUseModifier == false )
	{
		nTempHolder = m_nFrameRateModifier;
		m_nFrameRateModifier = 0;
	}

	if( m_nFrameRateModifier > 0 )
	{
		// The frame rate for animations has been speeded up!
		// Skip ahead the number of frames given 
		// by nFrameRateModifier
		for( int i = 0; i < m_nFrameRateModifier; ++i )
		{
			// Check and see if the current frame number is a special control code!
			dwNextNumber = m_nAnimations[m_nCurrentAnimation][m_nCurrentFrame + 1];

			if( dwNextNumber == LOOP_ANIMATION )
				m_nCurrentFrame = -1;

			if( dwNextNumber == GOTO_NEXT_ANIMATION )
				return;

			if( dwNextNumber == MAINTAIN_LAST_FRAME )
				return;

			if( dwNextNumber == GO_INACTIVE )
				return;

			++m_nCurrentFrame;
		}
	}
	else if( m_nFrameRateModifier < 0 )
	{
		// The frame rate for animations has been slowed!
		// Keep skipping cycles until we can move on to
		// the next frame
		--m_nFrameSkipCount;

		if( m_nFrameRateModifier == m_nFrameSkipCount )
		{
			// Check and see if the current frame number is a special control code!
			dwNextNumber = m_nAnimations[m_nCurrentAnimation][m_nCurrentFrame + 1];

			if( dwNextNumber == LOOP_ANIMATION )
				m_nCurrentFrame = -1;

			if( dwNextNumber == GOTO_NEXT_ANIMATION )
				return;

			if( dwNextNumber == MAINTAIN_LAST_FRAME )
				return;

			if( dwNextNumber == GO_INACTIVE )
				return;

			++m_nCurrentFrame;
			m_nFrameSkipCount = 0;
		}
	}
	else if( m_nFrameRateModifier == 0 )
	{
		// Check and see if the current frame number is a special control code!
		dwNextNumber = m_nAnimations[m_nCurrentAnimation][m_nCurrentFrame + 1];

		if( dwNextNumber == LOOP_ANIMATION )
			m_nCurrentFrame = -1;

		if( dwNextNumber == GOTO_NEXT_ANIMATION )
			return;

		if( dwNextNumber == MAINTAIN_LAST_FRAME )
			return;

		if( dwNextNumber == GO_INACTIVE )
			return;

		++m_nCurrentFrame;
	}

	if( bUseModifier == false )
		m_nFrameRateModifier = nTempHolder;
}

//-----------------------------------------------------------------------------
// Name: decFrame()
// Desc: Decrements the current animation to the frame that comes before the 
//       current frame number.
//-----------------------------------------------------------------------------
void CSprite::decFrame( bool bUseModifier )
{
	// This function allows the programmer to manually animate the sprite
	// through an animationn sequence by decrementing the sprite's 
	// nCurrentFrame member variable in a safe manner.

	DWORD dwNextNumber;
	int   nTempHolder = 0;

	if( bUseModifier == false )
	{
		nTempHolder = m_nFrameRateModifier;
		m_nFrameRateModifier = 0;
	}

	if( m_nFrameRateModifier > 0 )
	{
		// The frame rate for animations has been speeded up!
		// Skip ahead the number of frames given 
		// by nFrameRateModifier
		for( int i = 0; i < m_nFrameRateModifier; ++i )
		{
			if( m_nCurrentFrame <= 0 )
			{
				for( int i = 0; i < m_nFrameCount[m_nCurrentAnimation]; i++ )
				{
					dwNextNumber = m_nAnimations[m_nCurrentAnimation][i];

					if( dwNextNumber == LOOP_ANIMATION )
						m_nCurrentFrame = i - 1;

					if( dwNextNumber == GOTO_NEXT_ANIMATION )
						return;

					if( dwNextNumber == MAINTAIN_LAST_FRAME )
						return;

					if( dwNextNumber == GO_INACTIVE )
						return;
				}
			}
			else
			{
				--m_nCurrentFrame;
			}
		}
	}
	else if( m_nFrameRateModifier < 0 )
	{
		// The frame rate for animations has been slowed!
		// Keep skipping cycles until we can move on to
		// the next frame
		--m_nFrameSkipCount;

		if( m_nFrameRateModifier == m_nFrameSkipCount )
		{
			if( m_nCurrentFrame <= 0 )
			{
				for( int i = 0; i < m_nFrameCount[m_nCurrentAnimation]; i++ )
				{
					dwNextNumber = m_nAnimations[m_nCurrentAnimation][i];

					if( dwNextNumber == LOOP_ANIMATION )
						m_nCurrentFrame = i - 1;

					if( dwNextNumber == GOTO_NEXT_ANIMATION )
						return;

					if( dwNextNumber == MAINTAIN_LAST_FRAME )
						return;

					if( dwNextNumber == GO_INACTIVE )
						return;
				}
			}
			else
			{
				--m_nCurrentFrame;
			}

			m_nFrameSkipCount = 0;
		}
	}
	else if( m_nFrameRateModifier == 0 )
	{
		if( m_nCurrentFrame <= 0 )
		{
			for( int i = 0; i < m_nFrameCount[m_nCurrentAnimation]; i++ )
			{
				dwNextNumber = m_nAnimations[m_nCurrentAnimation][i];

				if( dwNextNumber == LOOP_ANIMATION )
					m_nCurrentFrame = i - 1;

				if( dwNextNumber == GOTO_NEXT_ANIMATION )
					return;

				if( dwNextNumber == MAINTAIN_LAST_FRAME )
					return;

				if( dwNextNumber == GO_INACTIVE )
					return;
			}
		}
		else
		{
			--m_nCurrentFrame;
		}
	}

	if( bUseModifier == false )
		m_nFrameRateModifier = nTempHolder;
}

//-----------------------------------------------------------------------------
// Name: drawSprite()
// Desc: Uses the sprite's properties and current animation values to select
//       the correct frame from the sprit's bitmap, and then copies the
//       bitmap data from the surface given, onto the display given.
//-----------------------------------------------------------------------------
HRESULT CSprite::drawSprite( CDisplay *pDisplay, CSurface *pSurface )
{
	// This function draws the sprite's next frame of animation by 
	// copying a rectangular region of bitmap data from pSurface 
	// onto a rectangular region of the back buffer held by pDisplay. 
	// The source surface will be a bitmap that contains the frames of 
	// animation necessary to produce an animated sprite and the 
	// destination surface will most likely be the back buffer surface 
	// where all the sprites will be copied to before being flipped with 
	// the primary surface. Information concerning the frame's width and 
	// height, number of frames per row and possible frame offsets will 
	// be used to create a RECT structure that will specify the exact 
	// position of the next frame of animation within the bitmap. 
	// The sprite's current x/y position on the screen will then be used 
	// to create a second RECT which will tell the Blt() function where 
	// to copy the frame to on the destination surface when the data 
	// is blitted. 

	HRESULT hr;

	if( m_bActive == true )
	{
		RECT  rcSource;
		RECT  rcDest;
		DWORD dwFrameNumber;
		DWORD dwNextNumber;

		if( m_bSingleFrame == false )
		{
			dwFrameNumber = m_nAnimations[m_nCurrentAnimation][m_nCurrentFrame];
		}
		else
		{
			dwFrameNumber = 0;
			m_nFramesAcross = 1;
		}


		// Create a source RECT that represents the section of the bitmap 
		// where the next frame of animation will be copied from during the blit.
		rcSource.top    = ( ( dwFrameNumber / m_nFramesAcross ) * m_nFrameHeight );
		rcSource.left   = ( ( dwFrameNumber % m_nFramesAcross ) * m_nFrameWidth );
		rcSource.bottom = rcSource.top  + m_nFrameHeight;
		rcSource.right  = rcSource.left + m_nFrameWidth;

		// If an offset was used, (i.e. frame 0 is not be located in 
		// the upper left corner of the bitmap), the offset values
		// must be applied or the frames will not be copied correctly. 
		if( m_nFrameOffset_x != 0 || m_nFrameOffset_y != 0 )
		{
			rcSource.top    += m_nFrameOffset_y;
			rcSource.left   += m_nFrameOffset_x;
			rcSource.bottom += m_nFrameOffset_y;
			rcSource.right  += m_nFrameOffset_x;
		}

		// Now, create a destination RECT that represents where the 
		// source RECT will be copied to on the destination surface.
		rcDest.top    = (long)m_dPosition_y;
		rcDest.left   = (long)m_dPosition_x;
		rcDest.bottom = rcDest.top  + m_nFrameHeight;
		rcDest.right  = rcDest.left + m_nFrameWidth;
		rcDest.bottom += m_nHeightScaling;
		rcDest.right  += m_nWidthScaling;

		hr = pDisplay->GetBackBuffer()->Blt( &rcDest,
			 pSurface->GetDDrawSurface(), &rcSource,
			 DDBLT_WAIT | DDBLT_KEYSRC, NULL );

		if( m_bAutoAnimate == true && m_bSingleFrame == false)
		{
			// Check and see if the next frame number is a special control code!
			dwNextNumber = m_nAnimations[m_nCurrentAnimation][m_nCurrentFrame+1];

			if( dwNextNumber == LOOP_ANIMATION )
				m_nCurrentFrame = -1;

			if( dwNextNumber == GOTO_NEXT_ANIMATION )
			{
				m_nCurrentAnimation = m_nAnimations[m_nCurrentAnimation][m_nCurrentFrame+2];
				m_nCurrentFrame = -1;
			}

			if( dwNextNumber == GO_INACTIVE )
			{
				m_bActive = false;
				m_nCurrentFrame = -1;
			}

			if( dwNextNumber == MAINTAIN_LAST_FRAME )
				return hr;

			if( m_nFrameRateModifier == 0 )
			{
				++m_nCurrentFrame;
			}
			else
			{
				// Has the frame rate has been modified!
				if( m_nFrameRateModifier < 0 )
				{
					// The frame rate for animations has been slowed!
					// Keep skipping cycles until we can move on to
					// the next frame
					--m_nFrameSkipCount;

					if( m_nFrameRateModifier == m_nFrameSkipCount )
					{
						++m_nCurrentFrame;
						m_nFrameSkipCount = 0;
					}
				}
				else
				{
					// The frame rate for animations has been speeded up!
					// Skip ahead the number of frames given 
					// by nFrameRateModifier
					for( int i = 0; i < m_nFrameRateModifier; ++i )
					{
						incFrame(false);
					}
				}
			}
		}
	}

	return hr;
}