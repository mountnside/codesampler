//-----------------------------------------------------------------------------
//           Name: dx9cs_indexed_geometry.cs
//         Author: Kevin Harris
//  Last Modified: 06/15/05
//    Description: This sample demonstrates how to optimize performance by 
//                 using indexed geometry. As a demonstration, the sample 
//                 reduces the vertex count of a simple cube from 24 to 8 by 
//                 redefining the cube’s geometry using an indices array.
//
//   Control Keys: F1 - Toggle between indexed and non-indexed geoemtry.
//                      Shouldn't produce any noticeable change since they
//                      render the same cube.
//-----------------------------------------------------------------------------


using System;
using System.Drawing;
using System.Windows.Forms;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;

namespace DX9Sample
{
	public class DX9Form : System.Windows.Forms.Form
	{
		private Device d3dDevice = null;
        private bool mousing = false;
        private Point ptLastMousePosit;
        private Point ptCurrentMousePosit;
		bool useIndexedGeometry = true;
        private int spinX;
        private int spinY;

		private VertexBuffer vertexBuffer = null;

		private VertexBuffer vertexBuffer_indexed = null;
        private IndexBuffer indexBuffer = null;

		struct Vertex
		{
			float x, y, z;
			int color;

			public Vertex( float _x, float _y, float _z, int _color )
			{
				x = _x; y = _y; z = _z;
				color = _color;
			}

			public static readonly VertexFormats FVF_Flags = VertexFormats.Position | VertexFormats.Diffuse;
		};

		//
		// To understand how indexed geometry works, we must first build something
		// which can be optimized through the use of indices.
		//
		// Below, is the vertex data for a simple multi-colored cube, which is defined
		// as 6 individual quads, one quad for each of the cube's six sides. At first,
		// this doesn’t seem too wasteful, but trust me it is.
		//
		// You see, we really only need 8 vertices to define a simple cube, but since 
		// we're using a quad list, we actually have to repeat the usage of our 8 
		// vertices 3 times each. To make this more understandable, I've actually 
		// numbered the vertices below so you can see how the vertices get repeated 
		// during the cube's definition.
		//
		// Note how the first 8 vertices are unique. Everything else after that is just
		// a repeat of the first 8.
		//

		Vertex[] cubeVertices =
		{
			new Vertex(-1.0f, 1.0f,-1.0f,  Color.Red.ToArgb() ),    // 0 (unique)
			new Vertex( 1.0f, 1.0f,-1.0f,  Color.Green.ToArgb() ),  // 1 (unique)
			new Vertex(-1.0f,-1.0f,-1.0f,  Color.Blue.ToArgb() ),   // 2 (unique)
			new Vertex( 1.0f,-1.0f,-1.0f,  Color.Yellow.ToArgb() ), // 3 (unique)

			new Vertex(-1.0f, 1.0f, 1.0f,  Color.Magenta.ToArgb() ),// 4 (unique)
			new Vertex(-1.0f,-1.0f, 1.0f,  Color.Cyan.ToArgb() ),   // 5 (unique)
			new Vertex( 1.0f, 1.0f, 1.0f,  Color.White.ToArgb() ),  // 6 (unique)
			new Vertex( 1.0f,-1.0f, 1.0f,  Color.Red.ToArgb() ),    // 7 (unique)

			new Vertex(-1.0f, 1.0f, 1.0f,  Color.Magenta.ToArgb() ),// 4 (start repeating here)
			new Vertex( 1.0f, 1.0f, 1.0f,  Color.White.ToArgb() ),  // 6 (repeat of vertex 6)
			new Vertex(-1.0f, 1.0f,-1.0f,  Color.Red.ToArgb() ),    // 0 (repeat of vertex 0... etc.)
			new Vertex( 1.0f, 1.0f,-1.0f,  Color.Green.ToArgb() ),  // 1

			new Vertex(-1.0f,-1.0f, 1.0f,  Color.Red.ToArgb() ),    // 5
			new Vertex(-1.0f,-1.0f,-1.0f,  Color.Green.ToArgb() ),  // 2
			new Vertex( 1.0f,-1.0f, 1.0f,  Color.Red.ToArgb() ),    // 7
			new Vertex( 1.0f,-1.0f,-1.0f,  Color.Yellow.ToArgb() ), // 3

			new Vertex( 1.0f, 1.0f,-1.0f,  Color.Green.ToArgb() ),  // 1
			new Vertex( 1.0f, 1.0f, 1.0f,  Color.White.ToArgb() ),  // 6
			new Vertex( 1.0f,-1.0f,-1.0f,  Color.Yellow.ToArgb() ), // 3
			new Vertex( 1.0f,-1.0f, 1.0f,  Color.Red.ToArgb() ),    // 7

			new Vertex(-1.0f, 1.0f,-1.0f,  Color.Red.ToArgb() ),    // 0
			new Vertex(-1.0f,-1.0f,-1.0f,  Color.Blue.ToArgb() ),   // 2
			new Vertex(-1.0f, 1.0f, 1.0f,  Color.Magenta.ToArgb() ),// 4
			new Vertex(-1.0f,-1.0f, 1.0f,  Color.Cyan.ToArgb() )    // 5
		};

		//
		// Now, to save ourselves the bandwidth of passing a bunch or redundant vertices
		// down the graphics pipeline, we shorten our vertex list and pass only the 
		// unique vertices. We then create a indices array, which contains index values
		// that reference vertices in our vertex array. 
		//
		// In other words, the vertex array doens't actually define our cube anymore, 
		// it only holds the unique vertices; it's the indices array that now defines  
		// the cube's geometry.
		//

		Vertex[] g_cubeVertices_indexed =
		{
			new Vertex(-1.0f, 1.0f,-1.0f,  Color.Red.ToArgb() ),    // 0 (unique)
			new Vertex( 1.0f, 1.0f,-1.0f,  Color.Green.ToArgb() ),  // 1 (unique)
			new Vertex(-1.0f,-1.0f,-1.0f,  Color.Blue.ToArgb() ),   // 2 (unique)
			new Vertex( 1.0f,-1.0f,-1.0f,  Color.Yellow.ToArgb() ), // 3 (unique)

			new Vertex(-1.0f, 1.0f, 1.0f,  Color.Magenta.ToArgb() ),// 4 (unique)
			new Vertex(-1.0f,-1.0f, 1.0f,  Color.Cyan.ToArgb() ),   // 5 (unique)
			new Vertex( 1.0f, 1.0f, 1.0f,  Color.White.ToArgb() ),  // 6 (unique)
			new Vertex( 1.0f,-1.0f, 1.0f,  Color.Red.ToArgb() ),    // 7 (unique)
		};

		int[] g_cubeIndices =
		{
			0, 1, 2, 3, // Quad 0
			4, 5, 6, 7, // Quad 1
			4, 6, 0, 1, // Quad 2
			5, 2, 7, 3, // Quad 3
			1, 6, 3, 7, // Quad 4
			0, 2, 4, 5  // Quad 5
		};

		public DX9Form()
		{
            this.ClientSize = new Size( 640, 480 );
            this.Text = "Direct3D (DX9/C#) - Indexed Geometry";
			this.SetStyle( ControlStyles.AllPaintingInWmPaint | ControlStyles.Opaque, true );
		}

        protected override void OnPaint(System.Windows.Forms.PaintEventArgs e)
        {
			this.Render();
			this.Invalidate();
        }

        protected override void OnKeyDown(System.Windows.Forms.KeyEventArgs e)
        {
            switch( e.KeyCode )
            {
                case System.Windows.Forms.Keys.Escape:
                    this.Dispose();
                    break;

				case System.Windows.Forms.Keys.F1:
					useIndexedGeometry = !useIndexedGeometry;
					break;
            }
        }

        protected override void OnMouseDown(System.Windows.Forms.MouseEventArgs e)
        {
            ptLastMousePosit = ptCurrentMousePosit = PointToScreen( new Point(e.X, e.Y) );
            mousing = true;
        }

        protected override void OnMouseUp(System.Windows.Forms.MouseEventArgs e)
        {
            mousing = false;
        }

        protected override void OnMouseMove(System.Windows.Forms.MouseEventArgs e)
        {
            ptCurrentMousePosit = PointToScreen( new Point(e.X, e.Y) );

            if( mousing )
            {
                spinX -= (ptCurrentMousePosit.X - ptLastMousePosit.X);
                spinY -= (ptCurrentMousePosit.Y - ptLastMousePosit.Y);
            }
			
            ptLastMousePosit = ptCurrentMousePosit;
        }

		protected override void Dispose(bool disposing)
		{
			base.Dispose( disposing );
		}

		static void Main() 
		{
			using( DX9Form frm = new DX9Form() )
			{
				frm.Show();
				frm.Init();
				Application.Run( frm );
			}
		}

		/// <summary>
		/// This method basically creates and initialize the Direct3D device and
		/// anything else that doens't need to be recreated after a device 
		/// reset.
		/// </summary>
		private void Init()
		{
			//
			// Do we support hardware vertex processing? If so, use it. 
			// If not, downgrade to software.
			//

			Caps caps = Manager.GetDeviceCaps( Manager.Adapters.Default.Adapter, 
				                               DeviceType.Hardware );
			CreateFlags flags;

			if( caps.DeviceCaps.SupportsHardwareTransformAndLight )
				flags = CreateFlags.HardwareVertexProcessing;
			else
				flags = CreateFlags.SoftwareVertexProcessing;

			//
			// Everything checks out - create a simple, windowed device.
			//

			PresentParameters d3dpp = new PresentParameters();

			d3dpp.BackBufferFormat       = Format.Unknown;
			d3dpp.SwapEffect             = SwapEffect.Discard;
			d3dpp.Windowed               = true;
			d3dpp.EnableAutoDepthStencil = true;
			d3dpp.AutoDepthStencilFormat = DepthFormat.D16;
			d3dpp.PresentationInterval   = PresentInterval.Immediate; 

			d3dDevice = new Device( 0, DeviceType.Hardware, this, flags, d3dpp );

			// Register an event-handler for DeviceReset and call it to continue
			// our setup.
			d3dDevice.DeviceReset += new System.EventHandler( this.OnResetDevice );
			OnResetDevice( d3dDevice, null );
		}

		/// <summary>
		/// This event-handler is a good place to create and initialize any 
		/// Direct3D related objects, which may become invalid during a 
		/// device reset.
		/// </summary>
		public void OnResetDevice(object sender, EventArgs e)
		{
			Device device = (Device)sender;
			device.Transform.Projection = 
				Matrix.PerspectiveFovLH( Geometry.DegreeToRadian( 45.0f ),
				(float)this.ClientSize.Width / this.ClientSize.Height,
				0.1f, 100.0f );

			device.RenderState.ZBufferEnable = true;
			device.RenderState.Lighting = false;

			//
			// Create a vertex buffer...
			//

			vertexBuffer = new VertexBuffer( typeof(Vertex),
				                             cubeVertices.Length, device,
				                             Usage.Dynamic | Usage.WriteOnly,
				                             Vertex.FVF_Flags,
				                             Pool.Default );

			GraphicsStream gStream = vertexBuffer.Lock( 0, 0, LockFlags.None );

			// Now, copy the vertex data into the vertex buffer
			gStream.Write( cubeVertices );

			vertexBuffer.Unlock();

			//
			// Create a vertex buffer, which contains indexed geometry...
			//

			vertexBuffer_indexed = new VertexBuffer( typeof(Vertex),
				                             cubeVertices.Length, device,
				                             Usage.Dynamic | Usage.WriteOnly,
				                             Vertex.FVF_Flags,
				                             Pool.Default );

			gStream = vertexBuffer_indexed.Lock( 0, 0, LockFlags.None );

			// Now, copy the vertex data into the vertex buffer
			gStream.Write( g_cubeVertices_indexed );

			vertexBuffer_indexed.Unlock();

			//
			// Create an index buffer to use with our indexed vertex buffer...
			//

			indexBuffer = new IndexBuffer( typeof(int), 24, device, 
				                           Usage.WriteOnly, Pool.Default );

			gStream = indexBuffer.Lock( 0, 0, LockFlags.None );

			// Now, copy the indices data into the index buffer
			gStream.Write( g_cubeIndices );

			indexBuffer.Unlock();

		}

		/// <summary>
		/// This method is dedicated completely to rendering our 3D scene and is
		/// is called by the OnPaint() event-handler.
		/// </summary>
		private void Render()
		{
			d3dDevice.Clear( ClearFlags.Target | ClearFlags.ZBuffer, 
                             Color.FromArgb(255, 0, 0, 0), 1.0f, 0 );

			d3dDevice.BeginScene();

            d3dDevice.Transform.World = 
                Matrix.RotationYawPitchRoll( Geometry.DegreeToRadian(spinX),
                Geometry.DegreeToRadian(spinY), 0.0f) * 
                Matrix.Translation(0.0f, 0.0f, 5.0f);

			if( useIndexedGeometry == true )
			{
				d3dDevice.VertexFormat = Vertex.FVF_Flags;
				d3dDevice.SetStreamSource( 0, vertexBuffer_indexed, 0 );
				d3dDevice.Indices = indexBuffer;

				d3dDevice.DrawIndexedPrimitives( PrimitiveType.TriangleStrip, 0, 0, 8,  0, 2 );
				d3dDevice.DrawIndexedPrimitives( PrimitiveType.TriangleStrip, 0, 0, 8,  4, 2 );
				d3dDevice.DrawIndexedPrimitives( PrimitiveType.TriangleStrip, 0, 0, 8,  8, 2 );
				d3dDevice.DrawIndexedPrimitives( PrimitiveType.TriangleStrip, 0, 0, 8, 12, 2 );
				d3dDevice.DrawIndexedPrimitives( PrimitiveType.TriangleStrip, 0, 0, 8, 16, 2 );
				d3dDevice.DrawIndexedPrimitives( PrimitiveType.TriangleStrip, 0, 0, 8, 20, 2 );
			}
			else
			{
				d3dDevice.VertexFormat = Vertex.FVF_Flags;
				d3dDevice.SetStreamSource( 0, vertexBuffer, 0 );

				d3dDevice.DrawPrimitives( PrimitiveType.TriangleStrip,  0, 2 );
				d3dDevice.DrawPrimitives( PrimitiveType.TriangleStrip,  4, 2 );
				d3dDevice.DrawPrimitives( PrimitiveType.TriangleStrip,  8, 2 );
				d3dDevice.DrawPrimitives( PrimitiveType.TriangleStrip, 12, 2 );
				d3dDevice.DrawPrimitives( PrimitiveType.TriangleStrip, 16, 2 );
				d3dDevice.DrawPrimitives( PrimitiveType.TriangleStrip, 20, 2 );
			}

			d3dDevice.EndScene();

			d3dDevice.Present();
		}
	}
}
