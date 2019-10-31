//------------------------------------------------------------------------------
//           Name: dx9cs_view_ports.cs
//         Author: Kevin Harris
//  Last Modified: 06/15/05
//    Description: This sample demonstrates how to set multiple view ports
//                 usingDirect3D.
//
//   Control Keys: Left Mouse Button - Spin the teapot
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
		private Mesh teapotMesh;
        private Material teapotMtrl;

		public DX9Form()
		{
            this.ClientSize = new System.Drawing.Size( 640, 480 );
            this.Text = "Direct3D (DX9/C#) - Setting Multiple View Ports";
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
			device.RenderState.Lighting = true;
			device.RenderState.SpecularEnable = true;

			//
			// Setup a simple directional light and some ambient...
			//

			device.Lights[0].Type = LightType.Directional;
			device.Lights[0].Direction = new Vector3( 1.0f, 0.0f, 1.0f );
			device.Lights[0].Diffuse = Color.White;
			device.Lights[0].Specular = Color.White;
			device.Lights[0].Enabled = true;

			device.RenderState.Ambient = Color.Gray;

			// Setup a material for the teapot
			teapotMtrl = new Material();
			teapotMtrl.Diffuse = Color.White;

			// Load the teapot mesh...
			try
			{
				teapotMesh = Mesh.FromFile( "teapot.x", MeshFlags.Managed, device );
			}
			catch
			{
				// We must be running from within Visual Studio. Relocate the 
				// current directory and try again.
				System.IO.Directory.SetCurrentDirectory(
					System.Windows.Forms.Application.StartupPath +  @"\..\..\");

				teapotMesh = Mesh.FromFile( "teapot.x", MeshFlags.Managed, device );
			}
		}

		/// <summary>
		/// This method is dedicated completely to rendering our 3D scene and is
		/// is called by the OnPaint() event-handler.
		/// </summary>
		private void Render()
		{
			//
			// Render to the left view-port
			//

			Microsoft.DirectX.Direct3D.Viewport leftViewPort = new Viewport(); 
			leftViewPort.X      = 0;
			leftViewPort.Y      = 0;
			leftViewPort.Width  = this.ClientSize.Width / 2;
			leftViewPort.Height = this.ClientSize.Height;
			leftViewPort.MinZ   = 0.0f;
			leftViewPort.MaxZ   = 1.0f;

			d3dDevice.Viewport = leftViewPort;

            // Now we can clear just view-port's portion of the buffer to red...
			d3dDevice.Clear( ClearFlags.Target | ClearFlags.ZBuffer, 
                             Color.FromArgb(255, 255, 0, 0), 1.0f, 0 );

			d3dDevice.BeginScene();

            // For the left view-port, leave the view at the origin...
            d3dDevice.Transform.View = Matrix.Identity;

            // ... and use the world matrix to spin and translate the teapot  
            // out where we can see it...
            d3dDevice.Transform.World = 
                Matrix.RotationYawPitchRoll( Geometry.DegreeToRadian(spinX),
                Geometry.DegreeToRadian(spinY), 0.0f) * 
                Matrix.Translation(0.0f, 0.0f, 5.0f);

            d3dDevice.Material = teapotMtrl;
            teapotMesh.DrawSubset(0);

			d3dDevice.EndScene();

			//
			// Render to the right view-port
			//

			// Set up view-port properties
			Microsoft.DirectX.Direct3D.Viewport rightViewPort = new Viewport(); 
			rightViewPort.X      = this.ClientSize.Width / 2;
			rightViewPort.Y      = 0;
			rightViewPort.Width  = this.ClientSize.Width / 2;
			rightViewPort.Height = this.ClientSize.Height;
			rightViewPort.MinZ   = 0.0f;
			rightViewPort.MaxZ   = 1.0f;

			d3dDevice.Viewport = rightViewPort;

            // Now we can clear just view-port's portion of the buffer to green...
			d3dDevice.Clear( ClearFlags.Target | ClearFlags.ZBuffer, 
				             Color.FromArgb(255, 0, 255, 0), 1.0f, 0 );

			d3dDevice.BeginScene();

            // For the right view-port, translate and rotate the view around 
            // the teapot so we can see it...
            d3dDevice.Transform.View = 
				Matrix.RotationYawPitchRoll( Geometry.DegreeToRadian(spinX),
				                             Geometry.DegreeToRadian(spinY), 0.0f) * 
				                             Matrix.Translation(0.0f, 0.0f, 5.0f);

            // ... and don't bother with the world matrix at all.
            d3dDevice.Transform.World = Matrix.Identity;

            d3dDevice.Material = teapotMtrl;
			teapotMesh.DrawSubset(0);
    
			d3dDevice.EndScene();

			d3dDevice.Present();
		}
	}
}
