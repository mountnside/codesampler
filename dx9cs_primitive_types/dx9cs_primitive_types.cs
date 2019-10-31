//------------------------------------------------------------------------------
//           Name: dx9cs_primitive_types.cs
//         Author: Kevin Harris
//  Last Modified: 06/15/05
//    Description: This sample demonstrates how to properly use all the 
//                 primitive types available under Direct3D with C#.
//
//                 The primitive types are:
//
//                 PrimitiveType.PointList
//                 PrimitiveType.LineList
//                 PrimitiveType.LineStrip
//                 PrimitiveType.TriangleList
//                 PrimitiveType.TriangleStrip
//                 PrimitiveType.TriangleFan
//
//   Control Keys: F1 - Switch the primitive type to be rendered.
//                 F2 - Toggle wire-frame mode.
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

		private VertexBuffer pointList_VB     = null;
		private VertexBuffer lineStrip_VB     = null;
		private VertexBuffer lineList_VB      = null;
		private VertexBuffer triangleList_VB  = null;
		private VertexBuffer triangleStrip_VB = null;
		private VertexBuffer triangleFan_VB   = null;

		bool renderInWireFrame = false;
		PrimitiveType currentPrimitive = PrimitiveType.TriangleFan;

		public DX9Form()
		{
            this.ClientSize = new Size( 640, 480 );
            this.Text = "Direct3D (DX9/C#) - Primitive Types";
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
				{
					if( currentPrimitive == PrimitiveType.PointList )
						currentPrimitive = PrimitiveType.LineList;
					else if( currentPrimitive == PrimitiveType.LineList )
						currentPrimitive = PrimitiveType.LineStrip;
					else if( currentPrimitive == PrimitiveType.LineStrip )
						currentPrimitive = PrimitiveType.TriangleList;
					else if( currentPrimitive == PrimitiveType.TriangleList )
						currentPrimitive = PrimitiveType.TriangleStrip;
					else if( currentPrimitive == PrimitiveType.TriangleStrip )
						currentPrimitive = PrimitiveType.TriangleFan;
					else if( currentPrimitive == PrimitiveType.TriangleFan )
						currentPrimitive = PrimitiveType.PointList;
				}
				break;

                case System.Windows.Forms.Keys.F2:
                {
                    renderInWireFrame = !renderInWireFrame;
                    if( renderInWireFrame == true )
                        d3dDevice.RenderState.FillMode = FillMode.WireFrame;
                    else
                        d3dDevice.RenderState.FillMode = FillMode.Solid;
                }
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

			device.RenderState.ZBufferEnable = true;
			device.RenderState.Lighting = false;
			device.RenderState.CullMode = Cull.None;
            device.RenderState.PointSize = 2.0f;

			//
			// Point List
			//

			pointList_VB = new VertexBuffer( typeof(CustomVertex.PositionColored),
				                             5, device,
				                             Usage.Dynamic | Usage.WriteOnly,
				                             CustomVertex.PositionColored.Format,
				                             Pool.Default );

			CustomVertex.PositionColored[] verts = new CustomVertex.PositionColored[5];

			verts[0].Position = new Vector3( 0.0f, 0.0f, 0.0f );
			verts[0].Color = Color.Red.ToArgb();

			verts[1].Position = new Vector3( 0.5f, 0.0f, 0.0f );
			verts[1].Color = Color.Green.ToArgb();

			verts[2].Position = new Vector3( -0.5f, 0.0f, 0.0f );
			verts[2].Color = Color.Blue.ToArgb();

			verts[3].Position = new Vector3( 0.0f,-0.5f, 0.0f );
			verts[3].Color = Color.Cyan.ToArgb();

			verts[4].Position = new Vector3( 0.0f, 0.5f, 0.0f );
			verts[4].Color = Color.Magenta.ToArgb();

			pointList_VB.SetData( verts, 0, LockFlags.None );

			//
			// Line List
			//

            lineList_VB = new VertexBuffer( typeof(CustomVertex.PositionColored),
				                             6, device,
				                             Usage.Dynamic | Usage.WriteOnly,
				                             CustomVertex.PositionColored.Format,
				                             Pool.Default );

			verts = new CustomVertex.PositionColored[6];

            // Line #1
            verts[0].Position = new Vector3( -1.0f,  0.0f, 0.0f );
            verts[0].Color = Color.Red.ToArgb();

            verts[1].Position = new Vector3( 0.0f,  1.0f, 0.0f );
            verts[1].Color = Color.Red.ToArgb();

            // Line #2
            verts[2].Position = new Vector3( 0.5f,  1.0f, 0.0f );
            verts[2].Color = Color.Green.ToArgb();

            verts[3].Position = new Vector3( 0.5f, -1.0f, 0.0f );
            verts[3].Color = Color.Green.ToArgb();

            // Line #3
            verts[4].Position = new Vector3( 1.0f, -0.5f, 0.0f);
            verts[4].Color = Color.Blue.ToArgb();

            verts[5].Position = new Vector3( -1.0f, -0.5f, 0.0f );
            verts[5].Color = Color.Blue.ToArgb();

            lineList_VB.SetData( verts, 0, LockFlags.None );

			//
			// Line Strip
			//

            lineStrip_VB = new VertexBuffer( typeof(CustomVertex.PositionColored),
				                             6, device,
				                             Usage.Dynamic | Usage.WriteOnly,
				                             CustomVertex.PositionColored.Format,
				                             Pool.Default );

			verts = new CustomVertex.PositionColored[6];

            verts[0].Position = new Vector3( 0.5f, 0.5f, 0.0f );
            verts[0].Color = Color.Red.ToArgb();

            verts[1].Position = new Vector3( 1.0f, 0.0f, 0.0f );
            verts[1].Color = Color.Green.ToArgb();

            verts[2].Position = new Vector3( 0.0f,-1.0f, 0.0f );
            verts[2].Color = Color.Blue.ToArgb();

            verts[3].Position = new Vector3( -1.0f, 0.0f, 0.0f );
            verts[3].Color = Color.Cyan.ToArgb();

            verts[4].Position = new Vector3( 0.0f, 0.0f, 0.0f );
            verts[4].Color = Color.Magenta.ToArgb();

            verts[5].Position = new Vector3( 0.0f, 1.0f, 0.0f );
            verts[5].Color = Color.Red.ToArgb();

			lineStrip_VB.SetData( verts, 0, LockFlags.None );

			//
			// Triangle List
			//

            triangleList_VB = new VertexBuffer( typeof(CustomVertex.PositionColored),
				                                6, device,
				                                Usage.Dynamic | Usage.WriteOnly,
				                                CustomVertex.PositionColored.Format,
				                                Pool.Default );

			verts = new CustomVertex.PositionColored[6];

            // Triangle #1
            verts[0].Position = new Vector3( -1.0f, 0.0f, 0.0f );
            verts[0].Color = Color.Red.ToArgb();

            verts[1].Position = new Vector3( 0.0f, 1.0f, 0.0f );
            verts[1].Color = Color.Green.ToArgb();

            verts[2].Position = new Vector3( 1.0f, 0.0f, 0.0f );
            verts[2].Color = Color.Blue.ToArgb();

            // Triangle #2
            verts[3].Position = new Vector3( -0.5f,-1.0f, 0.0f );
            verts[3].Color = Color.Cyan.ToArgb();

            verts[4].Position = new Vector3( 0.0f,-0.5f, 0.0f );
            verts[4].Color = Color.Magenta.ToArgb();

            verts[5].Position = new Vector3( 0.5f,-1.0f, 0.0f );
            verts[5].Color = Color.Yellow.ToArgb();

            triangleList_VB.SetData( verts, 0, LockFlags.None );

			//
			// Triangle Strip
			//

            triangleStrip_VB = new VertexBuffer( typeof(CustomVertex.PositionColored),
				                                8, device,
				                                Usage.Dynamic | Usage.WriteOnly,
				                                CustomVertex.PositionColored.Format,
				                                Pool.Default );

			verts = new CustomVertex.PositionColored[8];

            verts[0].Position = new Vector3( -2.0f, 0.0f, 0.0f );
            verts[0].Color = Color.Red.ToArgb();

            verts[1].Position = new Vector3( -1.0f, 1.0f, 0.0f );
            verts[1].Color = Color.Green.ToArgb();

            verts[2].Position = new Vector3( -1.0f, 0.0f, 0.0f );
            verts[2].Color = Color.Blue.ToArgb();

            verts[3].Position = new Vector3( 0.0f, 1.0f, 0.0f );
            verts[3].Color = Color.Cyan.ToArgb();

            verts[4].Position = new Vector3( 0.0f, 0.0f, 0.0f );  
            verts[4].Color = Color.Magenta.ToArgb();

            verts[5].Position = new Vector3( 1.0f, 1.0f, 0.0f ); 
            verts[5].Color = Color.Yellow.ToArgb();

            verts[6].Position = new Vector3( 1.0f, 0.0f, 0.0f );
            verts[6].Color = Color.Red.ToArgb();

            verts[7].Position = new Vector3( 2.0f, 1.0f, 0.0f );
            verts[7].Color = Color.Green.ToArgb();

            triangleStrip_VB.SetData( verts, 0, LockFlags.None );

			//
			// Triangle Fan
			//

            triangleFan_VB = new VertexBuffer( typeof(CustomVertex.PositionColored),
				                                6, device,
				                                Usage.Dynamic | Usage.WriteOnly,
				                                CustomVertex.PositionColored.Format,
				                                Pool.Default );

			verts = new CustomVertex.PositionColored[6];

            verts[0].Position = new Vector3( 0.0f,-1.0f, 0.0f );
            verts[0].Color = Color.Red.ToArgb();

            verts[1].Position = new Vector3( -1.0f, 0.0f, 0.0f );
            verts[1].Color = Color.Green.ToArgb();

            verts[2].Position = new Vector3( -0.5f, 0.5f, 0.0f );
            verts[2].Color = Color.Blue.ToArgb();

            verts[3].Position = new Vector3( 0.0f, 1.0f, 0.0f );
            verts[3].Color = Color.Cyan.ToArgb();

            verts[4].Position = new Vector3( 0.5f, 0.5f, 0.0f );
            verts[4].Color = Color.Yellow.ToArgb();

            verts[5].Position = new Vector3( 1.0f, 0.0f, 0.0f );
            verts[5].Color = Color.Magenta.ToArgb();

            triangleFan_VB.SetData( verts, 0, LockFlags.None );
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

            d3dDevice.Transform.World = Matrix.Translation(0.0f, 0.0f, 5.0f);

            switch( currentPrimitive )
            {
                case PrimitiveType.PointList:
			        d3dDevice.VertexFormat = CustomVertex.PositionColored.Format;
			        d3dDevice.SetStreamSource(0, pointList_VB, 0);
			        d3dDevice.DrawPrimitives( PrimitiveType.PointList, 0, 5 );
                    break;

                case PrimitiveType.LineList:
                    d3dDevice.VertexFormat = CustomVertex.PositionColored.Format;
                    d3dDevice.SetStreamSource(0, lineList_VB, 0);
                    d3dDevice.DrawPrimitives( PrimitiveType.LineList, 0, 6 );
                    break;

                case PrimitiveType.LineStrip:
                    d3dDevice.VertexFormat = CustomVertex.PositionColored.Format;
                    d3dDevice.SetStreamSource(0, lineStrip_VB, 0);
                    d3dDevice.DrawPrimitives( PrimitiveType.LineStrip, 0, 5 );
                    break;

                case PrimitiveType.TriangleList:
                    d3dDevice.VertexFormat = CustomVertex.PositionColored.Format;
                    d3dDevice.SetStreamSource(0, triangleList_VB, 0);
                    d3dDevice.DrawPrimitives( PrimitiveType.TriangleList, 0, 2 );
                    break;

                case PrimitiveType.TriangleStrip:
                    d3dDevice.VertexFormat = CustomVertex.PositionColored.Format;
                    d3dDevice.SetStreamSource(0, triangleStrip_VB, 0);
                    d3dDevice.DrawPrimitives( PrimitiveType.TriangleStrip, 0, 6 );
                    break;

                case PrimitiveType.TriangleFan:
                    d3dDevice.VertexFormat = CustomVertex.PositionColored.Format;
                    d3dDevice.SetStreamSource(0, triangleFan_VB, 0);
                    d3dDevice.DrawPrimitives( PrimitiveType.TriangleFan, 0, 4 );
                    break;

                default: 
                    break;
            }

			d3dDevice.EndScene();

			d3dDevice.Present();
		}
	}
}
