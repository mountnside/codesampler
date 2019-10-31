//------------------------------------------------------------------------------
//           Name: dx9cs_vertex_data.cs
//         Author: Kevin Harris
//  Last Modified: 06/15/05
//    Description: This sample demonstrates how to create 3D geometry with 
//                 Direct3D by loading vertex data into a Vertex Buffer.
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
		private VertexBuffer vertexBuffer = null;
		Texture texture = null;

		static private float elapsedTime;
		static private DateTime currentTime;
		static private DateTime lastTime;

		private float xrot = 0.0f;
		private float yrot = 0.0f;
		private float zrot = 0.0f;

		struct Vertex
		{
			float x, y, z;
			float tu, tv;

			public Vertex( float _x, float _y, float _z, float _tu, float _tv )
			{
				x = _x; y = _y; z = _z;
				tu = _tu; tv = _tv;
			}

			public static readonly VertexFormats FVF_Flags = VertexFormats.Position | VertexFormats.Texture1;
		};

		Vertex[] cubeVertices =
		{
			new Vertex(-1.0f, 1.0f,-1.0f,  0.0f,0.0f ),
			new Vertex( 1.0f, 1.0f,-1.0f,  1.0f,0.0f ),
			new Vertex(-1.0f,-1.0f,-1.0f,  0.0f,1.0f ),
			new Vertex( 1.0f,-1.0f,-1.0f,  1.0f,1.0f ),

			new Vertex(-1.0f, 1.0f, 1.0f,  1.0f,0.0f ),
			new Vertex(-1.0f,-1.0f, 1.0f,  1.0f,1.0f ),
			new Vertex( 1.0f, 1.0f, 1.0f,  0.0f,0.0f ),
			new Vertex( 1.0f,-1.0f, 1.0f,  0.0f,1.0f ),

			new Vertex(-1.0f, 1.0f, 1.0f,  0.0f,0.0f ),
			new Vertex( 1.0f, 1.0f, 1.0f,  1.0f,0.0f ),
			new Vertex(-1.0f, 1.0f,-1.0f,  0.0f,1.0f ),
			new Vertex( 1.0f, 1.0f,-1.0f,  1.0f,1.0f ),

			new Vertex(-1.0f,-1.0f, 1.0f,  0.0f,0.0f ),
			new Vertex(-1.0f,-1.0f,-1.0f,  1.0f,0.0f ),
			new Vertex( 1.0f,-1.0f, 1.0f,  0.0f,1.0f ),
			new Vertex( 1.0f,-1.0f,-1.0f,  1.0f,1.0f ),

			new Vertex( 1.0f, 1.0f,-1.0f,  0.0f,0.0f ),
			new Vertex( 1.0f, 1.0f, 1.0f,  1.0f,0.0f ),
			new Vertex( 1.0f,-1.0f,-1.0f,  0.0f,1.0f ),
			new Vertex( 1.0f,-1.0f, 1.0f,  1.0f,1.0f ),

			new Vertex(-1.0f, 1.0f,-1.0f,  1.0f,0.0f ),
			new Vertex(-1.0f,-1.0f,-1.0f,  1.0f,1.0f ),
			new Vertex(-1.0f, 1.0f, 1.0f,  0.0f,0.0f ),
			new Vertex(-1.0f,-1.0f, 1.0f,  0.0f,1.0f )
		};

		public DX9Form()
		{
            this.ClientSize = new Size( 640, 480 );
            this.Text = "Direct3D (DX9/C#) - Vertex Data";
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

			vertexBuffer = new VertexBuffer( typeof(Vertex),
				                             cubeVertices.Length, device,
				                             Usage.Dynamic | Usage.WriteOnly,
				                             Vertex.FVF_Flags,
				                             Pool.Default );

			GraphicsStream gStream = vertexBuffer.Lock( 0, 0, LockFlags.None );

			// Now, copy the vertex data into the vertex buffer
			gStream.Write( cubeVertices );

			vertexBuffer.Unlock();

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

			xrot += 10.1f * elapsedTime;
			yrot += 10.2f * elapsedTime;
			zrot += 10.3f * elapsedTime;

			d3dDevice.BeginScene();

			d3dDevice.Transform.World = 
				Matrix.RotationYawPitchRoll( Geometry.DegreeToRadian(xrot),
				                             Geometry.DegreeToRadian(yrot), 
				                             Geometry.DegreeToRadian(zrot)) * 
				Matrix.Translation(0.0f, 0.0f, 5.0f);



			d3dDevice.VertexFormat = Vertex.FVF_Flags;
			d3dDevice.SetStreamSource( 0, vertexBuffer, 0 );

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
