!!ARBvp1.0
# cgc version 1.4.0000, build date Jun  9 2005 12:09:02
# command line args: -profile arbvp1
# source file: c:\Documents and Settings\Kevin\Desktop\samples\OpenGL\ogl_cg_simple\ogl_cg_simple.cg
#vendor NVIDIA Corporation
#version 1.0.02
#profile arbvp1
#program main
#semantic main.constColor
#semantic modelViewProj : STATE.MATRIX.MVP
#var float4 IN.position : $vin.POSITION : POSITION : 0 : 1
#var float4 IN.color0 : $vin.COLOR0 : COLOR0 : 0 : 0
#var float4 constColor :  : c[5] : 1 : 1
#var float4x4 modelViewProj : STATE.MATRIX.MVP : c[1], 4 : -1 : 1
#var float4 main.position : $vout.POSITION : HPOS : -1 : 1
#var float4 main.color0 : $vout.COLOR0 : COL0 : -1 : 1
PARAM c[6] = { program.local[0],
		state.matrix.mvp,
		program.local[5] };
MOV result.color, c[5];
DP4 result.position.w, vertex.position, c[4];
DP4 result.position.z, vertex.position, c[3];
DP4 result.position.y, vertex.position, c[2];
DP4 result.position.x, vertex.position, c[1];
END
# 5 instructions, 0 R-regs
