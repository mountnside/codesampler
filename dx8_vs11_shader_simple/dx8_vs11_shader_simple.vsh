vs.1.1                       // Shader version 1.0

//-----------------------------------------------------------------------------
// Simple vertex shader
//
// Constants Registers:
//
//    c0-c3 = combined model-view-projection matrices
//    c4    = constant color (defined by the application)
//
// Input Registers: (Stream 0)
//
//    v0    = per-vertex position ( 4x1 vector )
//    v5    = per-vertex diffuse color
//
// Ouput Registers:
//    oPos  = homogeneous position ( 4x1 vector )
//    oD0   = diffuse color
//
//-----------------------------------------------------------------------------

dp4   oPos.x , v0 , c0       // Transform the x component of the per-vertex position into clip-space
dp4   oPos.y , v0 , c1       // Transform the y component of the per-vertex position into clip-space
dp4   oPos.z , v0 , c2       // Transform the z component of the per-vertex position into clip-space
dp4   oPos.w , v0 , c3       // Transform the w component of the per-vertex position into clip-space

mov   oD0  , v5              // Use the original per-vertex color specified
//mov   oD0  , c4             // Uncomment this to use the constant color stored at c4

//-----------------------------------------------------------------------------
// Note: Instead of calling the dp4 instruction four times to transform the
// vertex's position into clip-space, we could use the m4x4 instruction,
// which basically does the same thing. Just replace the four calls to dp4
// with this:
//
// m4x4  oPos , v0 , c0
//
//-----------------------------------------------------------------------------

