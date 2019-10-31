//------------------------------------------------------------------------------
//           Name: dx9cs_transforms.cs
//         Author: Kevin Harris
//  Last Modified: 06/28/05
//    Description: Demonstrates how to use translation, rotation, and scaling 
//                 matrices to create a simulated solar system.
//
//   Control Keys: F1    - Speed up rotations
//                 F2    - Slow down rotations
//                 Space - Toggle orbiting on/off
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
		private Mesh sunMesh;
		private Mesh earthMesh;
		private Mesh moonMesh;
		private MatrixStack matrixStack;

		static private float elapsedTime;
		static private DateTime currentTime;
		static private DateTime lastTime;

		private float speedmodifier = 1.0f;
		private bool  orbitOn = true;

		static private float fSunSpin    = 0.0f;    
		static private float fEarthSpin  = 0.0f;    
		static private float fEarthOrbit = 0.0f;
		static private float fMoonSpin   = 0.0f;
		static private float fMoonOrbit  = 0.0f;

		struct Vertex
		{
			public float x, y, z; // Position of vertex in 3D space
			public int color;     // Diffuse color of vertex

			public Vertex( float _x, float _y, float _z, int _color )
			{
				x = _x; y = _y; z = _z;
				color = _color;
			}

			public static readonly VertexFormats FVF_Flags = VertexFormats.Position | VertexFormats.Diffuse;
		};

		public DX9Form()
		{
            this.ClientSize = new System.Drawing.Size( 640, 480 );
            this.Text = "Direct3D (DX9/C#) - Transforms";
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

                case System.Windows.Forms.Keys.Space:
                    orbitOn = !orbitOn;
                    break;

                case System.Windows.Forms.Keys.F1:
                    ++speedmodifier;
                    break;

                case System.Windows.Forms.Keys.F2:
                    --speedmodifier;
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

			device.RenderState.Lighting = false;
			device.RenderState.CullMode = Cull.None;
			device.RenderState.FillMode = FillMode.WireFrame;

			//
			// We'll use the Mesh.Sphere method to create three simple sphere 
			// meshes to experiment with.
			//

			sunMesh   = Mesh.Sphere( device, 1.0f, 20, 20 );
			earthMesh = Mesh.Sphere( device, 1.0f, 10, 10 );
			moonMesh  = Mesh.Sphere( device, 0.5f, 8, 8 );

            //
            // Unfortunately, the Mesh.Sphere utility function creates a mesh 
            // with no color, so we'll need to make a clone of the original 
            // meshes using a FVF code that does include color so we can set up 
            // the Earth and Sun with color.
            //
            // Once that's been done, we'll need to set the color values to  
            // something appropriate for our solar system model.
            //

			//
			// Clone the original Earth mesh and make it blue...
			//

			Mesh tempMesh = earthMesh.Clone( earthMesh.Options.Value, Vertex.FVF_Flags, device );

			Vertex[] vertData = 
				(Vertex[]) tempMesh.VertexBuffer.Lock( 0, typeof(Vertex), 
				                                       LockFlags.None, 
				                                       tempMesh.NumberVertices );

			for( int i = 0; i < vertData.Length; ++i )
				vertData[i].color = Color.Blue.ToArgb();

			tempMesh.VertexBuffer.Unlock();

			earthMesh.Dispose();
			earthMesh = tempMesh;

			//
			// Clone the original Sun mesh and make it yellow...
			//

			tempMesh = sunMesh.Clone( sunMesh.Options.Value, Vertex.FVF_Flags, device );

			vertData = 
				(Vertex[]) tempMesh.VertexBuffer.Lock( 0, typeof(Vertex), 
				                                       LockFlags.None, 
				                                       tempMesh.NumberVertices );

			for( int i = 0; i < vertData.Length; ++i )
				vertData[i].color = Color.Yellow.ToArgb();

			tempMesh.VertexBuffer.Unlock();

			sunMesh.Dispose();
			sunMesh = tempMesh;

            //
            // Create a matrix stack...
            //

            matrixStack = new MatrixStack();
		}

//*
		/// <summary>
		/// This method is dedicated completely to rendering our 3D scene and is
		/// is called by the OnPaint() event-handler.
		/// 
		/// Render a solar system using the D3DXMATRIX utility class.
		/// </summary>
		private void Render()
		{
            // Now we can clear just view-port's portion of the buffer to red...
			d3dDevice.Clear( ClearFlags.Target | ClearFlags.ZBuffer, 
                             Color.FromArgb(255, 0, 0, 0), 1.0f, 0 );

			d3dDevice.BeginScene();

			//
			// Have the view matrix move the view move us to a good vantage point so 
			// we can see the Sun sitting at the origin while the Earth orbits it.
			//

			d3dDevice.Transform.View = Matrix.LookAtLH( new Vector3(0.0f, 2.0f, -25.0f), // Camera position
				                                        new Vector3(0.0f, 0.0f, 0.0f),   // Look-at point
				                                        new Vector3(0.0f, 1.0f, 0.0f));  // Up vector

			if( orbitOn == true )
			{
				fSunSpin += speedmodifier * (elapsedTime * 10.0f);

				fEarthSpin  += speedmodifier * (elapsedTime * 100.0f);
				fEarthOrbit += speedmodifier * (elapsedTime * 20.0f);

				fMoonSpin  += speedmodifier * (elapsedTime * 50.0f);
				fMoonOrbit += speedmodifier * (elapsedTime * 200.0f);
			}

			//
			// The Sun is easy because the mesh for it is initially created centered  
			// at origin. All we have to do is spin it by rotating it about the Y axis
			// and scale it by 5.0f.
			//

			Matrix mSunScale = Matrix.Identity;
			Matrix mSunSpinRotation = Matrix.Identity;
			Matrix mSunMatrix = Matrix.Identity;
    
			mSunSpinRotation.RotateY( Geometry.DegreeToRadian( fSunSpin ) );
			mSunScale.Scale( 5.0f, 5.0f, 5.0f );

			// Now, concatenate them together...
			mSunMatrix = mSunScale *       // 1. Uniformly scale the Sun up in size
				         mSunSpinRotation; // 2. and then spin it on its axis.

			d3dDevice.Transform.World = mSunMatrix;
			sunMesh.DrawSubset(0);

			//
			// The Earth is a little more complicated since it needs to spin as well 
			// as orbit the Sun. This can be done by combining three transformations 
			// together.
			//
		    
			Matrix mEarthTranslationToOrbit = Matrix.Identity;
			Matrix mEarthSpinRotation = Matrix.Identity;
			Matrix mEarthOrbitRotation = Matrix.Identity;
			Matrix mEarthMatrix = Matrix.Identity;

			mEarthSpinRotation.RotateY( Geometry.DegreeToRadian( fEarthSpin ) );
			mEarthTranslationToOrbit.Translate( 0.0f, 0.0f, 12.0f );
			mEarthOrbitRotation.RotateY( Geometry.DegreeToRadian( fEarthOrbit ) );

			// Now, concatenate them together...
			mEarthMatrix = mEarthSpinRotation *       // 1. Spin the Earth on its own axis.
						   mEarthTranslationToOrbit * // 2. Then translate it away from the origin (where the Sun's at)
						   mEarthOrbitRotation;       // 3. and rotate it again to make it orbit the origin (or the Sun).

			d3dDevice.Transform.World = mEarthMatrix;
			earthMesh.DrawSubset(0);

			//
			// The Moon is the hardest to understand since it needs to not only spin on
			// its own axis and orbit the Earth, but needs to follow the Earth, 
			// which is orbiting the Sun.
			//
			// This can be done by combining five transformations together with the last
			// two being borrowed from the Earth's transformation.
			//

			Matrix mMoonTranslationToOrbit = Matrix.Identity;
			Matrix mMoonSpinRotation = Matrix.Identity;
			Matrix mMoonOrbitRotation = Matrix.Identity;
			Matrix mMoonMatrix = Matrix.Identity;

			mMoonSpinRotation.RotateY( Geometry.DegreeToRadian( fMoonSpin ) );
			mMoonOrbitRotation.RotateY( Geometry.DegreeToRadian( fMoonOrbit ) );
			mMoonTranslationToOrbit.Translate( 0.0f, 0.0f, 2.0f );

			//
			// The key to understanding the first three transforms is to pretend that 
			// the Earth is located at the origin. We know it's not, but if we pretend 
			// that it is, we can set up the Moon just like the we did the Earth since 
			// the Moon orbits the Earth just like the Earth orbits the Sun.
			//
			// Once the Moon's transforms are set up we simply reuse the Earth's 
			// translation and rotation matrix, which placed it in orbit, to offset
			// the Moon out to where it should be... following the Earth.
			// 

			// Now, concatenate them together...
		    
			mMoonMatrix = mMoonSpinRotation *        // 1. Spin the Moon on its own axis.
						  mMoonTranslationToOrbit *  // 2. Then translate it away from the origin (pretending that the Earth is there)
						  mMoonOrbitRotation *       // 3. and rotate it again to make it orbit the origin (or the pretend Earth).
		                  
						  mEarthTranslationToOrbit * // 4. Now, translate out to where the Earth is really at
						  mEarthOrbitRotation;       // 5. and move with it by matching its orbit of the Earth.

			d3dDevice.Transform.World = mMoonMatrix;
			moonMesh.DrawSubset(0);

			d3dDevice.EndScene();

			d3dDevice.Present();
		}
/*/

/*
		/// <summary>
		/// This method is dedicated completely to rendering our 3D scene and
		/// is called by the OnPaint() event-handler.
		///
		/// Render a solar system using the D3DXMATRIX utility class and a  
		/// matrix stack similar to OpenGL's. See the note below for details.
		///
		/// Note:
		///
		/// Direct3D uses the world and view matrices that we set to configure 
		/// several internal data structures. Each time we set a new world or 
		/// view matrix, the system is forced to recalculate these internal 
		/// structures. Therefore, setting these matrices frequently, which is 
		/// the case for applications that require a high frame-rate, is 
		/// computationally expensive. We can minimize the number of required 
		/// calculations by concatenating our world and view matrices into a 
		/// combined world-view matrix that we set as the world matrix. 
		///
		/// With the view matrix combined in with each world matrix that we set, 
		/// we no longer have to set the view matrix separately and incur its 
		/// overhead. Instead, we simply set the view matrix to the identity 
		/// once and leave it untouched during all calculations.
		///
		/// For clarity, Direct3D samples rarely employ this optimization since 
		/// it confuses beginners.
		/// </summary>
		private void Render()
		{
            // Now we can clear just view-port's portion of the buffer to red...
			d3dDevice.Clear( ClearFlags.Target | ClearFlags.ZBuffer, 
                             Color.FromArgb(255, 0, 0, 0), 1.0f, 0 );

			d3dDevice.BeginScene();

			//
			// Have the view matrix move the view move us to a good vantage 
            // point so we can see the Sun sitting at the origin while the 
            // Earth orbits it.
			//

			d3dDevice.Transform.View = Matrix.LookAtLH( new Vector3(0.0f, 2.0f, -25.0f), // Camera position
				                                        new Vector3(0.0f, 0.0f, 0.0f),   // Look-at point
				                                        new Vector3(0.0f, 1.0f, 0.0f));  // Up vector

            matrixStack.LoadMatrix( d3dDevice.Transform.View );

			if( orbitOn == true )
			{
				fSunSpin += speedmodifier * (elapsedTime * 10.0f);

				fEarthSpin  += speedmodifier * (elapsedTime * 100.0f);
				fEarthOrbit += speedmodifier * (elapsedTime * 20.0f);

				fMoonSpin  += speedmodifier * (elapsedTime * 50.0f);
				fMoonOrbit += speedmodifier * (elapsedTime * 200.0f);
			}

			//
			// The Sun is easy because the mesh for it is initially created 
            // centered  at origin. All we have to do is spin it by rotating it 
            // about the Y axis and scale it by 5.0f.
			//

			Matrix mSunScale = Matrix.Identity;
			Matrix mSunSpinRotation = Matrix.Identity;
			Matrix mSunMatrix = Matrix.Identity;
    
			mSunSpinRotation.RotateY( Geometry.DegreeToRadian( fSunSpin ) );
			mSunScale.Scale( 5.0f, 5.0f, 5.0f );

			// Now, concatenate them together...
			mSunMatrix = mSunScale *       // 1. Uniformly scale the Sun up in size
				         mSunSpinRotation; // 2. and then spin it on its axis.

			matrixStack.Push();
			{
				matrixStack.MultiplyMatrixLocal( mSunMatrix );

				d3dDevice.Transform.World = matrixStack.Top;
				sunMesh.DrawSubset(0);
			}
			matrixStack.Pop();

			//
			// The Earth is a little more complicated since it needs to spin as 
            // well as orbit the Sun. This can be done by combining three 
            // transformations together.
			//
		    
			Matrix mEarthTranslationToOrbit = Matrix.Identity;
			Matrix mEarthSpinRotation = Matrix.Identity;
			Matrix mEarthOrbitRotation = Matrix.Identity;
			Matrix mEarthMatrix = Matrix.Identity;

			mEarthSpinRotation.RotateY( Geometry.DegreeToRadian( fEarthSpin ) );
			mEarthTranslationToOrbit.Translate( 0.0f, 0.0f, 12.0f );
			mEarthOrbitRotation.RotateY( Geometry.DegreeToRadian( fEarthOrbit ) );

			// Now, concatenate them together...
			mEarthMatrix = mEarthSpinRotation *       // 1. Spin the Earth on its own axis.
						   mEarthTranslationToOrbit * // 2. Then translate it away from the origin (where the Sun's at)
						   mEarthOrbitRotation;       // 3. and rotate it again to make it orbit the origin (or the Sun).

            matrixStack.Push();
            {
                matrixStack.MultiplyMatrixLocal( mEarthMatrix );

                d3dDevice.Transform.World = matrixStack.Top;
                earthMesh.DrawSubset(0);
            }
            matrixStack.Pop();

			//
			// The Moon is the hardest to understand since it needs to not only 
            // spin on its own axis and orbit the Earth, but needs to follow 
            // the Earth, which is orbiting the Sun.
			//
			// This can be done by combining five transformations together 
            // with the last two being borrowed from the Earth's transformation.
			//

			Matrix mMoonTranslationToOrbit = Matrix.Identity;
			Matrix mMoonSpinRotation = Matrix.Identity;
			Matrix mMoonOrbitRotation = Matrix.Identity;
			Matrix mMoonMatrix = Matrix.Identity;

			mMoonSpinRotation.RotateY( Geometry.DegreeToRadian( fMoonSpin ) );
			mMoonOrbitRotation.RotateY( Geometry.DegreeToRadian( fMoonOrbit ) );
			mMoonTranslationToOrbit.Translate( 0.0f, 0.0f, 2.0f );

			//
			// The key to understanding the first three transforms is to 
            // pretend that the Earth is located at the origin. We know it's 
            // not, but if we pretend that it is, we can set up the Moon just 
            // like the we did the Earth since the Moon orbits the Earth just 
            // like the Earth orbits the Sun.
			//
			// Once the Moon's transforms are set up we simply reuse the Earth's 
			// translation and rotation matrix, which placed it in orbit, to 
            // offset the Moon out to where it should be... following the Earth.
			// 

			// Now, concatenate them together...
		    
			mMoonMatrix = mMoonSpinRotation *        // 1. Spin the Moon on its own axis.
						  mMoonTranslationToOrbit *  // 2. Then translate it away from the origin (pretending that the Earth is there)
						  mMoonOrbitRotation *       // 3. and rotate it again to make it orbit the origin (or the pretend Earth).
		                  
						  mEarthTranslationToOrbit * // 4. Now, translate out to where the Earth is really at
						  mEarthOrbitRotation;       // 5. and move with it by matching its orbit of the Earth.

            matrixStack.Push();
            {
                matrixStack.MultiplyMatrixLocal( mMoonMatrix );

                d3dDevice.Transform.World = matrixStack.Top;
                moonMesh.DrawSubset(0);
            }
            matrixStack.Pop();

			d3dDevice.EndScene();

			d3dDevice.Present();
		}
//*/

	}
}
