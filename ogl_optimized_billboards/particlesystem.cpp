//-----------------------------------------------------------------------------
//           Name: particlesystem.h
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: Implementation file for the CParticleSystem Class
//-----------------------------------------------------------------------------

#include <tchar.h>
#include <windows.h>
#include <mmsystem.h>
#include <gl\gl.h>
#include <gl\glaux.h>
#include "particlesystem.h"

//-----------------------------------------------------------------------------
// FUNCTION POINTERS FOR OPENGL EXTENSIONS
//-----------------------------------------------------------------------------

// For convenience, this project ships with its own "glext.h" extension header 
// file. If you have trouble running this sample, it may be that this "glext.h" 
// file is defining something that your hardware doesn�t actually support. 
// Try recompiling the sample using your own local, vendor-specific "glext.h" 
// header file.

#include "glext.h"      // Sample's header file
//#include <GL/glext.h> // Your local header file

//#ifndef GL_ARB_point_parameters
PFNGLPOINTPARAMETERFARBPROC  glPointParameterfARB  = NULL;
PFNGLPOINTPARAMETERFVARBPROC glPointParameterfvARB = NULL;

//-----------------------------------------------------------------------------
// Name: getRandomMinMax()
// Desc: Gets a random number between min/max boundaries
//-----------------------------------------------------------------------------
float getRandomMinMax( float fMin, float fMax )
{
    float fRandNum = (float)rand () / RAND_MAX;
    return fMin + (fMax - fMin) * fRandNum;
}

//-----------------------------------------------------------------------------
// Name: getRandomVector()
// Desc: Generates a random vector where X,Y, and Z components are between
//       -1.0 and 1.0
//-----------------------------------------------------------------------------
vector3f getRandomVector( void )
{
    vector3f vVector;

    // Pick a random Z between -1.0f and 1.0f.
    vVector.z = getRandomMinMax( -1.0f, 1.0f );
    
    // Get radius of this circle
    float radius = (float)sqrt(1 - vVector.z * vVector.z);
    
    // Pick a random point on a circle.
    float t = getRandomMinMax( -OGL_PI, OGL_PI );

    // Compute matching X and Y for our Z.
    vVector.x = (float)cosf(t) * radius;
    vVector.y = (float)sinf(t) * radius;

    return vVector;
}

//-----------------------------------------------------------------------------
// Name : classifyPoint()
// Desc : Classifies a point against the plane passed
//-----------------------------------------------------------------------------
int classifyPoint( vector3f *vPoint, Plane *pPlane )
{
    vector3f vDirection = pPlane->m_vPoint - *vPoint;

    float fResult = vector3f::dotProduct( vDirection,  pPlane->m_vNormal );

    if( fResult < -0.001f )
        return CP_FRONT;

    if( fResult >  0.001f )
        return CP_BACK;

    return CP_ONPLANE;
}

//-----------------------------------------------------------------------------
// Name: CParticleSystem()
// Desc:
//-----------------------------------------------------------------------------
CParticleSystem::CParticleSystem()
{
    m_pActiveList      = NULL; // Head node of particles that are active
    m_pFreeList        = NULL; // Head node of particles that are inactive and waiting to be recycled.
    m_pPlanes          = NULL;
    m_dwActiveCount    = 0;
    m_fCurrentTime     = 0.0f;
    m_fLastUpdate      = 0.0f;
    m_chTexFile        = NULL;
    m_textureID        = NULL;
    m_renderMethod     = RENDER_METHOD_MANUAL_CPU_BILLBOARDS;
    m_dwMaxParticles   = 1;
    m_dwNumToRelease   = 1;
    m_fReleaseInterval = 1.0f;
    m_fLifeCycle       = 1.0f;
    m_fSize            = 1.0f;
    m_clrColor         = vector3f(1.0f,1.0f,1.0f);
    m_vPosition        = vector3f(0.0f,0.0f,0.0f);
    m_vVelocity        = vector3f(0.0f,0.0f,0.0f);
    m_vGravity         = vector3f(0.0f,0.0f,0.0f);
    m_vWind            = vector3f(0.0f,0.0f,0.0f);
    m_bAirResistence   = true;
    m_fVelocityVar     = 1.0f;
    
    SetTexture("particle.bmp");
}

//-----------------------------------------------------------------------------
// Name: ~CParticleSystem()
// Desc:
//-----------------------------------------------------------------------------
CParticleSystem::~CParticleSystem()
{
    glDeleteTextures( 1, &m_textureID );

    while( m_pPlanes ) // Repeat till null...
    {
        Plane *pPlane = m_pPlanes;   // Hold onto the first one
        m_pPlanes = pPlane->m_pNext; // Move down to the next one
        delete pPlane;               // Delete the one we're holding
    }

    while( m_pActiveList )
    {
        Particle* pParticle = m_pActiveList;
        m_pActiveList = pParticle->m_pNext;
        delete pParticle;
    }
    m_pActiveList = NULL;

    while( m_pFreeList )
    {
        Particle *pParticle = m_pFreeList;
        m_pFreeList = pParticle->m_pNext;
        delete pParticle;
    }
    m_pFreeList = NULL;

    if( m_chTexFile != NULL )
    {
        delete [] m_chTexFile;
        m_chTexFile = NULL;
    }
}

//-----------------------------------------------------------------------------
// Name: SetTexture()
// Desc: 
//-----------------------------------------------------------------------------
void CParticleSystem::SetTexture( char *chTexFile )
{
    // Deallocate the memory that was previously reserved for this string.
    if( m_chTexFile != NULL )
    {
        delete [] m_chTexFile;
        m_chTexFile = NULL;
    }
    
    // Dynamically allocate the correct amount of memory.
    m_chTexFile = new char[strlen( chTexFile ) + 1];

    // If the allocation succeeds, copy the initialization string.
    if( m_chTexFile != NULL )
        strcpy( m_chTexFile, chTexFile );
}

//-----------------------------------------------------------------------------
// Name: GetTextureObject()
// Desc: 
//-----------------------------------------------------------------------------
GLuint CParticleSystem::GetTextureID()
{
    return m_textureID;
}

//-----------------------------------------------------------------------------
// Name: SetCollisionPlane()
// Desc: 
//-----------------------------------------------------------------------------
void CParticleSystem::SetCollisionPlane( vector3f vPlaneNormal, vector3f vPoint, 
                                         float fBounceFactor, int nCollisionResult )
{
    Plane *pPlane = new Plane;

    pPlane->m_vNormal          = vPlaneNormal;
    pPlane->m_vPoint           = vPoint;
    pPlane->m_fBounceFactor    = fBounceFactor;
    pPlane->m_nCollisionResult = nCollisionResult;

    pPlane->m_pNext = m_pPlanes; // Attach the current list to it...
    m_pPlanes = pPlane;          // ... and make it the new head.
}

//-----------------------------------------------------------------------------
// Name: Init()
// Desc: 
//-----------------------------------------------------------------------------
int CParticleSystem::Init( void )
{
    //
    // If the required extensions are present, get the addresses of thier 
    // functions that we wish to use...
    //

    char *ext = (char*)glGetString( GL_EXTENSIONS );

    if( strstr( ext, "GL_ARB_point_parameters" ) == NULL )
    {
        MessageBox(NULL,"GL_ARB_point_parameters extension was not found",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return 0;
    }
    else
    {
        glPointParameterfARB  = (PFNGLPOINTPARAMETERFARBPROC)wglGetProcAddress("glPointParameterfARB");
        glPointParameterfvARB = (PFNGLPOINTPARAMETERFVARBPROC)wglGetProcAddress("glPointParameterfvARB");

        if( !glPointParameterfARB || !glPointParameterfvARB )
        {
            MessageBox(NULL,"One or more GL_ARB_point_parameters functions were not found",
                "ERROR",MB_OK|MB_ICONEXCLAMATION);
            return 0;
        }
    }

    //
    // Load up the point sprite's texture...
    //

    AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( m_chTexFile );

    if( pTextureImage != NULL )
    {
        glGenTextures( 1, &m_textureID );

        glBindTexture(GL_TEXTURE_2D, m_textureID);

        glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        glTexImage2D( GL_TEXTURE_2D, 0, 3, pTextureImage->sizeX, pTextureImage->sizeY, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, pTextureImage->data );
    }

    if( pTextureImage )
    {
        if( pTextureImage->data )
            free( pTextureImage->data );

        free( pTextureImage );
    }

    //
    // If you want to know the max size that a point sprite can be set 
    // to, do this.
    //

    glGetFloatv( GL_POINT_SIZE_MAX_ARB, &m_fMaxPointSize );
    glPointSize( m_fMaxPointSize );

    //
    // Init the Cg shader
    //

    //
    // Search for a valid vertex shader profile in this order:
    //
    // CG_PROFILE_ARBVP1 - GL_ARB_vertex_program
    // CG_PROFILE_VP30   - GL_NV_vertex_program2
    // CG_PROFILE_VP20   - GL_NV_vertex_program
    //

    if( cgGLIsProfileSupported(CG_PROFILE_ARBVP1) )
        m_CGprofile = CG_PROFILE_ARBVP1;
    else if( cgGLIsProfileSupported(CG_PROFILE_VP30) )
        m_CGprofile = CG_PROFILE_VP30;
    else if( cgGLIsProfileSupported(CG_PROFILE_VP20) )
        m_CGprofile = CG_PROFILE_VP20;
    else
    {
        MessageBox( NULL,"Failed to initialize vertex shader! Hardware doesn't "
                    "support any of the vertex shading extensions!",
                    "ERROR",MB_OK|MB_ICONEXCLAMATION );
        return -1;
    }

    // Create the context...
    m_CGcontext = cgCreateContext();

    //
    // Create the vertex shader...
    //
    
    m_CGprogram = cgCreateProgramFromFile( m_CGcontext,
                                           CG_SOURCE,
                                           "billboard.cg",
                                           m_CGprofile,
                                           NULL, 
                                           NULL );

    //
    // Load the program using Cg's expanded interface...
    //

    cgGLLoadProgram( m_CGprogram );

    //
    // Bind some parameters by name so we can set them later...
    //

    m_CGparam_modelViewProj  = cgGetNamedParameter(m_CGprogram, "modelViewProj");
    //m_CGparam_size           = cgGetNamedParameter(m_CGprogram, "size");
    m_CGparam_preRotatedQuad = cgGetNamedParameter(m_CGprogram, "preRotatedQuad");

    return 1;
}

//-----------------------------------------------------------------------------
// Name: Update()
// Desc:
//-----------------------------------------------------------------------------
int CParticleSystem::Update( FLOAT fElpasedTime )
{
    Particle  *pParticle;
    Particle **ppParticle;
    Plane     *pPlane;
    Plane    **ppPlane;
    vector3f vOldPosition;

    m_fCurrentTime += fElpasedTime;     // Update our particle system timer...

    ppParticle = &m_pActiveList; // Start at the head of the active list

    while( *ppParticle )
    {
        pParticle = *ppParticle; // Set a pointer to the head

        // Calculate new position
        float fTimePassed  = m_fCurrentTime - pParticle->m_fInitTime;

        if( fTimePassed >= m_fLifeCycle )
        {
            // Time is up, put the particle back on the free list...
            *ppParticle = pParticle->m_pNext;
            pParticle->m_pNext = m_pFreeList;
            m_pFreeList = pParticle;

            --m_dwActiveCount;
        }
        else
        {
            // Update particle position and velocity

            // Update velocity with respect to Gravity (Constant Acceleration)
            pParticle->m_vCurVel += m_vGravity * fElpasedTime;

            // Update velocity with respect to Wind (Acceleration based on 
            // difference of vectors)
            if( m_bAirResistence == true )
                pParticle->m_vCurVel += (m_vWind - pParticle->m_vCurVel) * fElpasedTime;

            // Finally, update position with respect to velocity
            vOldPosition = pParticle->m_vCurPos;
            pParticle->m_vCurPos += pParticle->m_vCurVel * fElpasedTime;

            //-----------------------------------------------------------------
            // BEGIN Checking the particle against each plane that was set up

            ppPlane = &m_pPlanes; // Set a pointer to the head

            while( *ppPlane )
            {
                pPlane = *ppPlane;
                int result = classifyPoint( &pParticle->m_vCurPos, pPlane );

                if( result == CP_BACK /*|| result == CP_ONPLANE */ )
                {
                    if( pPlane->m_nCollisionResult == CR_BOUNCE )
                    {
                        pParticle->m_vCurPos = vOldPosition;

            //-----------------------------------------------------------------
            //
            // The new velocity vector of a particle that is bouncing off
            // a plane is computed as follows:
            //
            // Vn = (N.V) * N
            // Vt = V - Vn
            // Vp = Vt - Kr * Vn
            //
            // Where:
            // 
            // .  = Dot product operation
            // N  = The normal of the plane from which we bounced
            // V  = Velocity vector prior to bounce
            // Vn = Normal force
            // Kr = The coefficient of restitution ( Ex. 1 = Full Bounce, 
            //      0 = Particle Sticks )
            // Vp = New velocity vector after bounce
            //
            //-----------------------------------------------------------------

                        float Kr = pPlane->m_fBounceFactor;

                        vector3f Vn = vector3f::dotProduct( pPlane->m_vNormal, 
                                                            pParticle->m_vCurVel ) * pPlane->m_vNormal;

                        vector3f Vt = pParticle->m_vCurVel - Vn;
                        vector3f Vp = Vt - Kr * Vn;

                        pParticle->m_vCurVel = Vp;
                    }
                    else if( pPlane->m_nCollisionResult == CR_RECYCLE )
                    {
                        pParticle->m_fInitTime -= m_fLifeCycle;
                    }

                    else if( pPlane->m_nCollisionResult == CR_STICK )
                    {
                        pParticle->m_vCurPos = vOldPosition;
                        pParticle->m_vCurVel = vector3f(0.0f,0.0f,0.0f);
                    }
                }

                ppPlane = &pPlane->m_pNext;
            }

            // END Plane Checking
            //-----------------------------------------------------------------

            ppParticle = &pParticle->m_pNext;
        }
    }

    //-------------------------------------------------------------------------
    // Emit new particles in accordance to the flow rate...
    // 
    // NOTE: The system operates with a finite number of particles.
    //       New particles will be created until the max amount has
    //       been reached, after that, only old particles that have
    //       been recycled can be reinitialized and used again.
    //-------------------------------------------------------------------------

    if( m_fCurrentTime - m_fLastUpdate > m_fReleaseInterval )
    {
        // Reset update timing...
        m_fLastUpdate = m_fCurrentTime;
    
        // Emit new particles at specified flow rate...
        for( DWORD i = 0; i < m_dwNumToRelease; ++i )
        {
            // Do we have any free particles to put back to work?
            if( m_pFreeList )
            {
                // If so, hand over the first free one to be reused.
                pParticle = m_pFreeList;
                // Then make the next free particle in the list next to go!
                m_pFreeList = pParticle->m_pNext;
            }
            else
            {
                // There are no free particles to recycle...
                // We'll have to create a new one from scratch!
                if( m_dwActiveCount < m_dwMaxParticles )
                {
                    if( NULL == ( pParticle = new Particle ) )
                        return E_OUTOFMEMORY;
                }
            }

            if( m_dwActiveCount < m_dwMaxParticles )
            {
                pParticle->m_pNext = m_pActiveList; // Make it the new head...
                m_pActiveList = pParticle;
                
                // Set the attributes for our new particle...
                pParticle->m_vCurVel = m_vVelocity;

                if( m_fVelocityVar != 0.0f )
                {
                    vector3f vRandomVec = getRandomVector();
                    pParticle->m_vCurVel += vRandomVec * m_fVelocityVar;
                }

                pParticle->m_fInitTime  = m_fCurrentTime;
                pParticle->m_vCurPos    = m_vPosition;
                
                ++m_dwActiveCount;
            }
        }
    }
    
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: RestartParticleSystem()
// Desc: 
//-----------------------------------------------------------------------------
void CParticleSystem::RestartParticleSystem( void )
{
    Particle  *pParticle;
    Particle **ppParticle;

    ppParticle = &m_pActiveList; // Start at the head of the active list

    while( *ppParticle )
    {
        pParticle = *ppParticle; // Set a pointer to the head

        // Put the particle back on the free list...
        *ppParticle = pParticle->m_pNext;
        pParticle->m_pNext = m_pFreeList;
        m_pFreeList = pParticle;

        --m_dwActiveCount;
    }
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: 
//-----------------------------------------------------------------------------
void CParticleSystem::Render( void )
{
    //
    // Compute billboard vertices on the CPU using the model-view matrix...
    //

    if( m_renderMethod == RENDER_METHOD_MANUAL_CPU_BILLBOARDS )
    {
        Particle  *pParticle = m_pActiveList;

        //
        // Now, recompute the quad points with respect to the view matrix 
        // and the quad's center point. For this demo, the quad's center 
        // is always at the origin...
        //

        float mat[16];
        glGetFloatv( GL_MODELVIEW_MATRIX, mat );

        vector3f vRight( mat[0], mat[4], mat[8] );
        vector3f vUp( mat[1], mat[5], mat[9] );
        vector3f vPoint0;
        vector3f vPoint1;
        vector3f vPoint2;
        vector3f vPoint3;
        vector3f vCenter;
    
        float fAdjustedSize = m_fSize / 300.0f;

        glBindTexture( GL_TEXTURE_2D, m_textureID );
        glColor3f( m_clrColor.x, m_clrColor.y, m_clrColor.z );

        // Render each particle...
            
        glBegin( GL_QUADS );
        {
            while( pParticle )
            {
                vCenter.x = pParticle->m_vCurPos.x;
                vCenter.y = pParticle->m_vCurPos.y;
                vCenter.z = pParticle->m_vCurPos.z;

                //-------------------------------------------------------------
                //
                // vPoint3                vPoint2
                //         +------------+
                //         |            |
                //         |     +      |
                //         |  vCenter   |
                //         |            |
                //         +------------+
                // vPoint0                vPoint1
                //
                // Now, build a quad around the center point based on the vRight 
                // and vUp vectors. This will guarantee that the quad will be 
                // orthogonal to the view.
                //
                //-------------------------------------------------------------

                vPoint0 = vCenter + ((-vRight - vUp) * fAdjustedSize );
                vPoint1 = vCenter + (( vRight - vUp) * fAdjustedSize );
                vPoint2 = vCenter + (( vRight + vUp) * fAdjustedSize );
                vPoint3 = vCenter + ((-vRight + vUp) * fAdjustedSize );

                glTexCoord2f( 0.0f, 0.0f );
                glVertex3f( vPoint0.x, vPoint0.y, vPoint0.z );

                glTexCoord2f( 1.0f, 0.0f );
                glVertex3f( vPoint1.x, vPoint1.y, vPoint1.z );

                glTexCoord2f( 1.0f, 1.0f);
                glVertex3f( vPoint2.x, vPoint2.y, vPoint2.z );

                glTexCoord2f( 0.0f, 1.0f );
                glVertex3f( vPoint3.x, vPoint3.y, vPoint3.z );

                pParticle = pParticle->m_pNext;
            }
        }
        glEnd();
    }

    //
    // Compute billboard vertices on the GPU using ARB_point_sprites...
    //

    if( m_renderMethod == RENDER_METHOD_ARB_POINT_SPRITES )
    {
        Particle *pParticle = m_pActiveList;

        // This is how will our point sprite's size will be modified by 
        // distance from the viewer.
        float quadratic[] =  { 1.0f, 0.0f, 0.01f };
        glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );

        // The alpha of a point is calculated to allow the fading of points 
        // instead of shrinking them past a defined threshold size. The threshold 
        // is defined by GL_POINT_FADE_THRESHOLD_SIZE_ARB and is not clamped to 
        // the minimum and maximum point sizes.
        glPointParameterfARB( GL_POINT_FADE_THRESHOLD_SIZE_ARB, 60.0f );

        float fAdjustedSize = m_fSize / 4.0f;

        glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, 1.0f );
        glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, fAdjustedSize );

        // Specify point sprite texture coordinate replacement mode for each texture unit
        glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );

        //
        // Render point sprites...
        //

        glEnable( GL_POINT_SPRITE_ARB );

        glPointSize( m_fSize );

        glBindTexture( GL_TEXTURE_2D, m_textureID );
        glColor3f( m_clrColor.x, m_clrColor.y, m_clrColor.z );

        glBegin( GL_POINTS );
        {
            // Render each particle...
            while( pParticle )
            {
                glVertex3f( pParticle->m_vCurPos.x,
                            pParticle->m_vCurPos.y,
                            pParticle->m_vCurPos.z );

                pParticle = pParticle->m_pNext;
            }
        }
        glEnd();

		glDisable( GL_POINT_SPRITE_ARB );
    }

    //
    // Compute billboard vertices on the GPU using a Cg based shader...
    //

    if( m_renderMethod == RENDER_METHOD_CG_VERTEX_SHADER )
    {
        Particle  *pParticle = m_pActiveList;

        //
        // Compute billboard vertices on the GPU...
        //

        // Track the combined model-view-projection matrix
        cgGLSetStateMatrixParameter( m_CGparam_modelViewProj,
                                     CG_GL_MODELVIEW_PROJECTION_MATRIX,
                                     CG_GL_MATRIX_IDENTITY );

        // Load the billboard size into a constant register
        //float fSize[] = { 1.0f, 0.0f, 0.0f, 0.0f };
        //cgGLSetParameter4fv( m_CGparam_size, fSize );

        //
        // Create a pre-rotated quad for use by the shader...
        //

        float mat[16];
        glGetFloatv( GL_MODELVIEW_MATRIX, mat );

        vector3f vRight( mat[0], mat[4], mat[8] );
        vector3f vUp( mat[1], mat[5], mat[9] );
        vector3f vCenter( 0.0f, 0.0f, 0.0f );

        vector3f vPoint0 = vCenter + (-vRight - vUp);
        vector3f vPoint1 = vCenter + ( vRight - vUp);
        vector3f vPoint2 = vCenter + ( vRight + vUp);
        vector3f vPoint3 = vCenter + (-vRight + vUp);

        float preRotatedQuad[16] = 
        {
             vPoint0.x, vPoint0.y, vPoint0.z, 0.0f,
             vPoint1.x, vPoint1.y, vPoint1.z, 0.0f,
             vPoint2.x, vPoint2.y, vPoint2.z, 0.0f,
             vPoint3.x, vPoint3.y, vPoint3.z, 0.0f
        };

        cgGLSetParameterArray4f( m_CGparam_preRotatedQuad, 0, 4, preRotatedQuad );

        //
        // Bind the shader and render the particles...
        //

        cgGLBindProgram( m_CGprogram );
        cgGLEnableProfile( m_CGprofile );

        glBindTexture( GL_TEXTURE_2D, m_textureID );
        glColor3f( m_clrColor.x, m_clrColor.y, m_clrColor.z );

        float fAdjustedSize = m_fSize / 300.0f;

        glBegin( GL_QUADS );
        {
            while( pParticle )
            {
                vCenter.x = pParticle->m_vCurPos.x;
                vCenter.y = pParticle->m_vCurPos.y;
                vCenter.z = pParticle->m_vCurPos.z;
                
                glTexCoord4f( 0.0f, 0.0f, 0.0f, fAdjustedSize );
                glVertex3f( vCenter.x, vCenter.y, vCenter.z );

                glTexCoord4f( 1.0f, 0.0f, 1.0f, fAdjustedSize );
                glVertex3f( vCenter.x, vCenter.y, vCenter.z );

                glTexCoord4f( 1.0f, 1.0f, 2.0f, fAdjustedSize );
                glVertex3f( vCenter.x, vCenter.y, vCenter.z );

                glTexCoord4f( 0.0f, 1.0f, 3.0f, fAdjustedSize );
                glVertex3f( vCenter.x, vCenter.y, vCenter.z );

                pParticle = pParticle->m_pNext;
            }
        }
        glEnd();

        cgGLDisableProfile( m_CGprofile );
    }
}

