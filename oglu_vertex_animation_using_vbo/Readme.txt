//-----------------------------------------------------------------------------
//           Name: Vertex Animation using VBOs
//         Author: Tomas Dirvanauskas (aka Tactic) 
//  Last Modified: 07/02/03
//    Description: This sample demonstrates vertex animation using VBOs
//		   Three buffers are created for vertices,normals and indices.
//		   Vertex and normal buffers are update every frame,
//		   index buffer is not. The vertex interpolation is processed 
//                 by CPU (that's why the Release version is much much faster 
//                 than Debug)
//
//   Control Keys: Cursor Keys - controls the camera
//		   N - switch to next mesh
//		   Hold U - to do NOT update
//		   Hold V - to render without VBOs
//
//   Additional notes: Sample will run even if your video card doesn's support 
//                     vbo's Some external links - nehe.gamedev.net, 
//                     www.humus.ca
//		       
//-----------------------------------------------------------------------------
//Email - tomdir@gmail.com