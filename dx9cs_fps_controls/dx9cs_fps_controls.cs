//------------------------------------------------------------------------------
//           Name: dx9cs_fps_controls.cs
//         Author: Kevin Harris
//  Last Modified: 07/08/05
//    Description: This sample demonstrates how to collect user input and 
//                 build a custom view matrix for First Person Shooter style 
//                 controls.
//
//   Control Keys: Up         - View moves forward
//                 Down       - View moves backward
//                 Left       - View strafes left
//                 Right      - View strafes Right
//                 Left Mouse - Perform looking
//                 Mouse      - Look about the scene
//                 Home       - View moves up
//                 End        - View moves down
//------------------------------------------------------------------------------

using System;
using System.Drawing;
using System.Windows.Forms;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using Microsoft.DirectX.DirectInput;

// These both define a "Device" objects so let's create shorter names for them.
// If we don't do this, we'll end up with code that is really hard to read.
using Direct3D = Microsoft.DirectX.Direct3D;
using DirectInput = Microsoft.DirectX.DirectInput;

namespace DX9Sample
{
    

	public class DX9Form : System.Windows.Forms.Form
	{
		private Direct3D.Device d3dDevice = null;
		private DirectInput.Device dinputDevice = null;
        private VertexBuffer vertexBuffer = null;
        private Mesh teapotMesh;
        private Material teapotMtrl;

        private Point ptLastMousePosit = new Point();
        private Point ptCurrentMousePosit = new Point();
        private bool mousing = false;
        private float moveSpeed = 25.0f;

        static private float elapsedTime;
        static private DateTime currentTime;
        static private DateTime lastTime;

        private Vector3	vEye   = new Vector3(5.0f, 5.0f, -5.0f);  // Eye Position
        private Vector3	vLook  = new Vector3(-0.5f, -0.5f, 0.5f); // Look Vector
        private Vector3	vUp    = new Vector3(0.0f, 1.0f, 0.0f);   // Up Vector
        private Vector3	vRight = new Vector3(1.0f, 0.0f, 0.0f);   // Right Vector

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

        Vertex[] lineVertices =
        {
            new Vertex( 0.0f, 0.0f, 0.0f,  Color.Red.ToArgb() ),   // red = +x Axis
            new Vertex( 5.0f, 0.0f, 0.0f,  Color.Red.ToArgb() ),
            new Vertex( 0.0f, 0.0f, 0.0f,  Color.Green.ToArgb() ), // green = +y Axis
            new Vertex( 0.0f, 5.0f, 0.0f,  Color.Green.ToArgb() ),
            new Vertex( 0.0f, 0.0f, 5.0f,  Color.Blue.ToArgb() ),  // blue = +z Axis
            new Vertex( 0.0f, 0.0f, 0.0f,  Color.Blue.ToArgb() )
        };

		public DX9Form()
		{
            this.ClientSize = new Size( 640, 480 );
            this.Text = "Direct3D (DX9/C#) - First Person Shooter style controls";
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
            mousing = true;
        }

        protected override void OnMouseUp(System.Windows.Forms.MouseEventArgs e)
        {
            mousing = false;
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

			Caps caps = 
				Direct3D.Manager.GetDeviceCaps( Direct3D.Manager.Adapters.Default.Adapter, 
				                                Direct3D.DeviceType.Hardware );
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

			d3dDevice = new Direct3D.Device( 0, Direct3D.DeviceType.Hardware, this, flags, d3dpp );

			// Register an event-handler for DeviceReset and call it to continue
			// our setup.
			d3dDevice.DeviceReset += new System.EventHandler( this.OnResetDevice );
			OnResetDevice( d3dDevice, null );

			// Setup DirectInput so we can get real-time user input from the
			// keyboard.
			dinputDevice = new DirectInput.Device( SystemGuid.Keyboard );
			dinputDevice.SetCooperativeLevel( this, CooperativeLevelFlags.Background | CooperativeLevelFlags.NonExclusive );
			dinputDevice.Acquire();
		}

		/// <summary>
		/// This event-handler is a good place to create and initialize any 
		/// Direct3D related objects, which may become invalid during a 
		/// device reset.
		/// </summary>
		public void OnResetDevice(object sender, EventArgs e)
		{
			Direct3D.Device device = (Direct3D.Device)sender;
			device.Transform.Projection = 
				Matrix.PerspectiveFovLH( Geometry.DegreeToRadian( 45.0f ),
				(float)this.ClientSize.Width / this.ClientSize.Height,
				0.1f, 100.0f );

			device.RenderState.Lighting = false;
            device.RenderState.FillMode = FillMode.WireFrame;
			device.RenderState.CullMode = Cull.None;

            //
            // Create three colored lines to represent the axis...
            //

            vertexBuffer = new VertexBuffer( typeof(Vertex),
				                             lineVertices.Length, device,
				                             Usage.WriteOnly,
				                             Vertex.FVF_Flags,
				                             Pool.Default );

            GraphicsStream gStream = vertexBuffer.Lock( 0, 0, LockFlags.None );
            gStream.Write( lineVertices );
            vertexBuffer.Unlock();

            //
            // Load a teapot so we'll have something to look at...
            //

            teapotMtrl = new Material();
            teapotMtrl.Diffuse = Color.White;

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
		/// Uses DirectInput to get real-time user input from the keyboard.
		/// </summary>
        private void GetRealTimeUserInput()
        {
            //
            // Get mouse input...
            //

            ptCurrentMousePosit.X = Cursor.Position.X;
            ptCurrentMousePosit.Y = Cursor.Position.Y;

            Matrix matRotation;

            if( mousing )
            {
                int nXDiff = (ptCurrentMousePosit.X - ptLastMousePosit.X);
                int nYDiff = (ptCurrentMousePosit.Y - ptLastMousePosit.Y);

                if( nYDiff != 0 )
                {
                    matRotation = Matrix.RotationAxis( vRight, Geometry.DegreeToRadian((float)nYDiff / 3.0f));
                    vLook = Vector3.TransformCoordinate( vLook, matRotation );
                    vUp = Vector3.TransformCoordinate( vUp, matRotation );

                }

                if( nXDiff != 0 )
                {
                    Vector3	vTemp = new Vector3(0.0f, 1.0f, 0.0f);
                    matRotation = Matrix.RotationAxis( vTemp, Geometry.DegreeToRadian((float)nXDiff / 3.0f));
                    vLook = Vector3.TransformCoordinate( vLook, matRotation );
                    vUp = Vector3.TransformCoordinate( vUp, matRotation );
                }
            }

            ptLastMousePosit.X = ptCurrentMousePosit.X;
            ptLastMousePosit.Y = ptCurrentMousePosit.Y;

            //
            // Get keyboard input...
            //

            Vector3	tmpLook = new Vector3();
            Vector3	tmpRight = new Vector3();
            tmpLook = vLook;
            tmpRight = vRight;

			KeyboardState keys = dinputDevice.GetCurrentKeyboardState();

			// Up Arrow Key - View moves forward
			if( keys[Key.Up] )
				vEye -= tmpLook*-moveSpeed*elapsedTime;

			// Down Arrow Key - View moves backward
			if( keys[Key.Down] )
				vEye += (tmpLook*-moveSpeed)*elapsedTime;

			// Left Arrow Key - View side-steps or strafes to the left
			if( keys[Key.Left] )
				vEye -= (tmpRight*moveSpeed)*elapsedTime;

			// Right Arrow Key - View side-steps or strafes to the right
			if( keys[Key.Right] )
				vEye += (tmpRight*moveSpeed)*elapsedTime;

			// Home Key - View elevates up
			if( keys[Key.Home] )
				vEye.Y += moveSpeed*elapsedTime;

			// End Key - View elevates down
			if( keys[Key.End] )
				vEye.Y -= moveSpeed*elapsedTime;
        }

        /// <summary>
        /// Name : updateViewMatrix()
        /// Desc : Builds a view matrix suitable for Direct3D.
        ///
        /// Here's what the final matrix should look like:
        ///
        ///  |   rx     ux     lx    0 |
        ///  |   ry     uy     ly    0 |
        ///  |   rz     uz     lz    0 |
        ///  | -(r.e) -(u.e) -(l.e)  1 |
        ///
        /// Where r = Right vector
        ///       u = Up vector
        ///       l = Look vector
        ///       e = Eye position in world space
        ///       . = Dot-product operation
        /// </summary>
        private void UpdateViewMatrix()
        {
            Matrix view = Matrix.Identity;

            vLook = Vector3.Normalize( vLook );
            vRight = Vector3.Cross( vUp, vLook );
            vRight = Vector3.Normalize( vRight );
            vUp = Vector3.Cross( vLook, vRight );
            Vector3.Normalize( vUp );

            view.M11 = vRight.X;
            view.M12 = vUp.X;
            view.M13 = vLook.X;
            view.M14 = 0.0f;

            view.M21 = vRight.Y;
            view.M22 = vUp.Y;
            view.M23 = vLook.Y;
            view.M24 = 0.0f;

            view.M31 = vRight.Z;
            view.M32 = vUp.Z;
            view.M33 = vLook.Z;
            view.M34 = 0.0f;

            view.M41 = -Vector3.Dot( vEye, vRight );
            view.M42 = -Vector3.Dot( vEye, vUp );
            view.M43 = -Vector3.Dot( vEye, vLook );
            view.M44 =  1.0f;

            d3dDevice.Transform.View = view;
        }

		/// <summary>
		/// This method is dedicated completely to rendering our 3D scene and is
		/// is called by the OnPaint() event-handler.
		/// </summary>
		private void Render()
		{
			d3dDevice.Clear( ClearFlags.Target | ClearFlags.ZBuffer, 
                             Color.FromArgb(255, 78, 129, 179), 1.0f, 0 );

            GetRealTimeUserInput();
            UpdateViewMatrix();

			d3dDevice.BeginScene();

            // Render a wire-frame teapot.
            d3dDevice.Transform.World = Matrix.Scaling(2.0f, 2.0f, 2.0f);
            d3dDevice.Material = teapotMtrl;
            teapotMesh.DrawSubset(0);

            // Render our axis lines for reference.
            d3dDevice.Transform.World = Matrix.Scaling(1.0f, 1.0f, 1.0f);
            d3dDevice.VertexFormat = Vertex.FVF_Flags;
            d3dDevice.SetStreamSource( 0, vertexBuffer, 0 );
            d3dDevice.DrawPrimitives( PrimitiveType.LineList, 0, 3 );

			d3dDevice.EndScene();

			d3dDevice.Present();
		}
	}
}
