//------------------------------------------------------------------------------
//           Name: dx9cs_multiple_vertex_buffers.cs
//         Author: Kevin Harris
//  Last Modified: 06/15/05
//    Description: This sample demonstrates how to create 3D geometry with 
//                 Direct3D by loading vertex data into a multiple Vertex 
//                 Buffers.
//------------------------------------------------------------------------------

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
        private int spinX;
        private int spinY;
		private VertexBuffer vertexBuffer = null;
        private VertexBuffer colorBuffer = null;
        private VertexBuffer texCoordBuffer = null;
		Texture texture = null;

		static private float elapsedTime;
		static private DateTime currentTime;
		static private DateTime lastTime;

        struct Vertex
        {
            float x, y, z;

            public Vertex( float _x, float _y, float _z )
            {
                x = _x;
                y = _y;
                z = _z;
            }
        };

        Vertex[] cubeVertices =
        {
	        new Vertex(-1.0f, 1.0f,-1.0f ),
	        new Vertex( 1.0f, 1.0f,-1.0f ),
	        new Vertex(-1.0f,-1.0f,-1.0f ),
	        new Vertex( 1.0f,-1.0f,-1.0f ),

	        new Vertex(-1.0f, 1.0f, 1.0f ),
	        new Vertex(-1.0f,-1.0f, 1.0f ),
	        new Vertex( 1.0f, 1.0f, 1.0f ),
	        new Vertex( 1.0f,-1.0f, 1.0f ),

	        new Vertex(-1.0f, 1.0f, 1.0f ),
	        new Vertex( 1.0f, 1.0f, 1.0f ),
	        new Vertex(-1.0f, 1.0f,-1.0f ),
	        new Vertex( 1.0f, 1.0f,-1.0f ),

	        new Vertex(-1.0f,-1.0f, 1.0f ),
	        new Vertex(-1.0f,-1.0f,-1.0f ),
	        new Vertex( 1.0f,-1.0f, 1.0f ),
	        new Vertex( 1.0f,-1.0f,-1.0f ),

	        new Vertex( 1.0f, 1.0f,-1.0f ),
	        new Vertex( 1.0f, 1.0f, 1.0f ),
	        new Vertex( 1.0f,-1.0f,-1.0f ),
	        new Vertex( 1.0f,-1.0f, 1.0f ),

	        new Vertex(-1.0f, 1.0f,-1.0f ),
	        new Vertex(-1.0f,-1.0f,-1.0f ),
	        new Vertex(-1.0f, 1.0f, 1.0f ),
	        new Vertex(-1.0f,-1.0f, 1.0f )
        };

        struct DiffuseColor
        {
	        int color;

            public DiffuseColor( int _color )
            {
               color = _color;
            }
        };

        DiffuseColor[] cubeColors =
        {
	        new DiffuseColor( Color.Red.ToArgb() ),
	        new DiffuseColor( Color.Red.ToArgb() ),
	        new DiffuseColor( Color.Red.ToArgb() ),
	        new DiffuseColor( Color.Red.ToArgb() ),

            new DiffuseColor( Color.Green.ToArgb() ),
            new DiffuseColor( Color.Green.ToArgb() ),
            new DiffuseColor( Color.Green.ToArgb() ),
            new DiffuseColor( Color.Green.ToArgb() ),

            new DiffuseColor( Color.Blue.ToArgb() ),
            new DiffuseColor( Color.Blue.ToArgb() ),
            new DiffuseColor( Color.Blue.ToArgb() ),
            new DiffuseColor( Color.Blue.ToArgb() ),

            new DiffuseColor( Color.Yellow.ToArgb() ),
            new DiffuseColor( Color.Yellow.ToArgb() ),
            new DiffuseColor( Color.Yellow.ToArgb() ),
            new DiffuseColor( Color.Yellow.ToArgb() ),

            new DiffuseColor( Color.Magenta.ToArgb() ),
            new DiffuseColor( Color.Magenta.ToArgb() ),
            new DiffuseColor( Color.Magenta.ToArgb() ),
            new DiffuseColor( Color.Magenta.ToArgb() ),

            new DiffuseColor( Color.Cyan.ToArgb() ),
            new DiffuseColor( Color.Cyan.ToArgb() ),
            new DiffuseColor( Color.Cyan.ToArgb() ),
            new DiffuseColor( Color.Cyan.ToArgb() )
        };

        struct TexCoord
        {
	        float tu, tv;

            public TexCoord( float _tu, float _tv )
            {
                tu = _tu;
                tv = _tv;
            }
        };

        TexCoord[] cubeTexCoords =
        {
	        new TexCoord( 0.0f, 0.0f ),
	        new TexCoord( 1.0f, 0.0f ),
	        new TexCoord( 0.0f, 1.0f ),
	        new TexCoord( 1.0f, 1.0f ),

	        new TexCoord( 1.0f, 0.0f ),
	        new TexCoord( 1.0f, 1.0f ),
	        new TexCoord( 0.0f, 0.0f ),
	        new TexCoord( 0.0f, 1.0f ),

	        new TexCoord( 0.0f, 0.0f ),
	        new TexCoord( 1.0f, 0.0f ),
	        new TexCoord( 0.0f, 1.0f ),
	        new TexCoord( 1.0f, 1.0f ),

            new TexCoord( 0.0f, 0.0f ),
            new TexCoord( 1.0f, 0.0f ),
            new TexCoord( 0.0f, 1.0f ),
            new TexCoord( 1.0f, 1.0f ),

	        new TexCoord( 0.0f, 0.0f ),
	        new TexCoord( 1.0f, 0.0f ),
	        new TexCoord( 0.0f, 1.0f ),
	        new TexCoord( 1.0f, 1.0f ),

	        new TexCoord( 1.0f, 0.0f ),
	        new TexCoord( 1.0f, 1.0f ),
	        new TexCoord( 0.0f, 0.0f ),
	        new TexCoord( 0.0f, 1.0f )
        };

		public DX9Form()
		{
            this.ClientSize = new Size( 640, 480 );
            this.Text = "Direct3D (DX9/C#) - Multiple Vertex Buffers";
			this.SetStyle( ControlStyles.AllPaintingInWmPaint | ControlStyles.Opaque, true );
		}

        protected override void OnPaint(System.Windows.Forms.PaintEventArgs e)
        {
			currentTime = DateTime.Now;
			TimeSpan elapsedTimeSpan = currentTime.Subtract( lastTime );
			elapsedTime = (float)elapsedTimeSpan.Milliseconds * 0.001f;
			lastTime = currentTime;

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

		private void LoadTexture()
		{
			try
			{
				Bitmap testImage = (Bitmap)Bitmap.FromFile( "test.bmp" );
				texture = Texture.FromBitmap( d3dDevice, testImage, 0, Pool.Managed );
			}
			catch
			{
				// We must be running from within Visual Studio. Relocate the 
				// current directory and try again.
				System.IO.Directory.SetCurrentDirectory(
					System.Windows.Forms.Application.StartupPath +  @"\..\..\");

				Bitmap testImage = (Bitmap)Bitmap.FromFile( "test.bmp" );
				texture = Texture.FromBitmap( d3dDevice, testImage, 0, Pool.Managed );
			}

			d3dDevice.SamplerState[0].MinFilter = TextureFilter.Linear;
			d3dDevice.SamplerState[0].MagFilter = TextureFilter.Linear;
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
			// Create a vertex buffer that contains only the cube's vertex data
			//

			vertexBuffer = new VertexBuffer( typeof(Vertex),
				                             cubeVertices.Length, device,
				                             Usage.Dynamic | Usage.WriteOnly,
				                             VertexFormats.Position,
				                             Pool.Default );

			GraphicsStream gStream = vertexBuffer.Lock( 0, 0, LockFlags.None );

			// Now, copy the vertex data into the vertex buffer
			gStream.Write( cubeVertices );

			vertexBuffer.Unlock();

			//
			// Create a vertex buffer that contains only the cube's color data
			//

			colorBuffer = new VertexBuffer( typeof(DiffuseColor),
				                            cubeVertices.Length, device,
				                            Usage.Dynamic | Usage.WriteOnly,
				                            VertexFormats.Diffuse,
				                            Pool.Default );

            gStream = null;
			gStream = colorBuffer.Lock( 0, 0, LockFlags.None );

			// Now, copy the vertex data into the vertex buffer
			gStream.Write( cubeColors );

			colorBuffer.Unlock();

			//
			// Create a vertex buffer that contains only the cube's texture coordinate data
			//

			texCoordBuffer = new VertexBuffer( typeof(TexCoord),
				                               cubeVertices.Length, device,
				                               Usage.Dynamic | Usage.WriteOnly,
				                               VertexFormats.Texture1,
				                               Pool.Default );

            gStream = null;
			gStream = texCoordBuffer.Lock( 0, 0, LockFlags.None );

			// Now, copy the vertex data into the vertex buffer
			gStream.Write( cubeTexCoords );

			texCoordBuffer.Unlock();

			//
			// Create a vertex declaration so we can describe to Direct3D how we'll 
			// be passing our data to it.
			//

            // Create the vertex element array.
            VertexElement[] elements = new VertexElement[]
            {
                //		        Stream  Offset        Type                    Method                 Usage                      Usage Index
                new VertexElement( 0,     0,  DeclarationType.Float3, DeclarationMethod.Default, DeclarationUsage.Position,          0),
                new VertexElement( 1,     0,  DeclarationType.Color,  DeclarationMethod.Default, DeclarationUsage.Color,             0),
                new VertexElement( 2,     0,  DeclarationType.Float2, DeclarationMethod.Default, DeclarationUsage.TextureCoordinate, 0),

                VertexElement.VertexDeclarationEnd 
            };

            // Use the vertex element array to create a vertex declaration.
            d3dDevice.VertexDeclaration = new VertexDeclaration( d3dDevice, elements );

			LoadTexture();
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

            d3dDevice.SetStreamSource( 0, vertexBuffer,   0 );
            d3dDevice.SetStreamSource( 1, colorBuffer,    0 );
            d3dDevice.SetStreamSource( 2, texCoordBuffer, 0 );

            d3dDevice.SetTexture( 0, texture );

            d3dDevice.DrawPrimitives( PrimitiveType.TriangleStrip,  0, 2 );
            d3dDevice.DrawPrimitives( PrimitiveType.TriangleStrip,  4, 2 );
            d3dDevice.DrawPrimitives( PrimitiveType.TriangleStrip,  8, 2 );
            d3dDevice.DrawPrimitives( PrimitiveType.TriangleStrip, 12, 2 );
            d3dDevice.DrawPrimitives( PrimitiveType.TriangleStrip, 16, 2 );
            d3dDevice.DrawPrimitives( PrimitiveType.TriangleStrip, 20, 2 );

			d3dDevice.EndScene();

			d3dDevice.Present();
		}
	}
}
