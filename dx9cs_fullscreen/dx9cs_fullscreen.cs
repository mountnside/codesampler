//-----------------------------------------------------------------------------
//           Name: dx9cs_fullscreen.cs
//         Author: Kevin Harris
//  Last Modified: 06/15/05
//    Description: This sample demonstrates how to probe the hardware for 
//                 specific Display or Adaptor Modes and hardware support 
//                 suitable for a full-screen application with Direct3D.
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

        public DX9Form()
        {
            this.ClientSize = new System.Drawing.Size( 640, 480 );
            this.Text = "Direct3D (DX9/C#) - Full Screen";
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
			// For each adapter, examine all of its display modes to see if any 
			// of them can give us the hardware support we desire.
			//

			bool desiredAdapterModeFound = false;

			// For each Adapter...
            foreach( AdapterInformation adapter in Manager.Adapters )
            {
                // Examine each display mode available.
                foreach( DisplayMode display in adapter.SupportedDisplayModes )
                {
					// Does this adapter mode support a mode of 640 x 480?
					if( display.Width != 640 || display.Height != 480 )
						continue;

					// Does this adapter mode support a 32-bit RGB pixel format?
					if( display.Format != Format.X8R8G8B8 )
						continue;

					// Does this adapter mode support a refresh rate of 75 MHz?
					if( display.RefreshRate != 75 )
						continue;

					// We found a match!
					desiredAdapterModeFound = true;
					break;
                }
            }

			if( desiredAdapterModeFound == false )
			{
				// TO DO: Handle lack of support for desired adapter mode...
				return;
			}

            //
            // Here's the manual way of verifying hardware support.
            //

            // Can we get a 32-bit back buffer?
            if( !Manager.CheckDeviceType( Manager.Adapters.Default.Adapter, 
                                          DeviceType.Hardware,
                                          Format.X8R8G8B8,
                                          Format.X8R8G8B8,
                                          false) )
            {
                // TO DO: Handle lack of support for a 32-bit back buffer...
                return;
            }

            // Does the hardware support a 16-bit z-buffer?
            if( !Manager.CheckDeviceFormat( Manager.Adapters.Default.Adapter, 
                                            DeviceType.Hardware,
                                            Manager.Adapters.Default.CurrentDisplayMode.Format, 
                                            Usage.DepthStencil,
                                            ResourceType.Surface,
                                            DepthFormat.D16 ) )
            {
                // POTENTIAL PROBLEM: We need at least a 16-bit z-buffer!
                return;
            }

			//
			// Do we support hardware vertex processing? if so, use it. 
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
			// Everything checks out - create a simple, full-screen device.
			//

			PresentParameters d3dpp = new PresentParameters();

            d3dpp.Windowed               = false;
            d3dpp.EnableAutoDepthStencil = true;
            d3dpp.AutoDepthStencilFormat = DepthFormat.D16;
            d3dpp.SwapEffect             = SwapEffect.Discard;
            d3dpp.BackBufferWidth        = 640;
            d3dpp.BackBufferHeight       = 480;
            d3dpp.BackBufferFormat       = Format.X8R8G8B8;
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
			// This sample doens't create anything that requires recreation 
			// after the DeviceReset event.
		}

        private void Render()
        {
            d3dDevice.Clear( ClearFlags.Target | ClearFlags.ZBuffer, 
                             Color.FromArgb(255, 0, 255, 0), 1.0f, 0 );

            d3dDevice.BeginScene();
            
            // Render geometry here...
    
            d3dDevice.EndScene();

            d3dDevice.Present();
        }
    }
}
