Imports Microsoft.DirectX
Imports Microsoft.DirectX.Direct3D

Module mdx9c
    Public device As device
    Dim deviceSetting As New PresentParameters

    Sub createDevice(ByVal width As Integer, ByVal heigth As Integer, ByVal bpp As Integer, ByVal fhWnd As System.IntPtr, ByVal windowed As Boolean)
        'screen description
        deviceSetting.BackBufferCount = 1 'backbuffer number
        deviceSetting.AutoDepthStencilFormat = DepthFormat.D16 'Z/Stencil buffer formats
        deviceSetting.EnableAutoDepthStencil = True 'active Z/Stencil buffer 
        deviceSetting.DeviceWindowHandle = fhWnd 'handle del form
        deviceSetting.SwapEffect = SwapEffect.Flip 'rendering type
        If windowed Then
            deviceSetting.Windowed = True 'setting for windowed mode 
        Else
            deviceSetting.Windowed = False 'setting for fullscreen
            deviceSetting.BackBufferWidth = width 'screen resolution 
            deviceSetting.BackBufferHeight = heigth 'screen resolution
            If bpp = 16 Then
                deviceSetting.BackBufferFormat = Format.R5G6B5 'backbuffer format at 16Bit
            Else
                deviceSetting.BackBufferFormat = Format.X8R8G8B8 'backbuffer format at 32Bit
            End If

        End If
        'presentation type
        deviceSetting.PresentationInterval = PresentInterval.Immediate
        'create device
        device = New device(0, DeviceType.Hardware, fhWnd, CreateFlags.HardwareVertexProcessing, deviceSetting)



    End Sub

    'must be executed when form is resized
    Sub resetDevice()
        'you must putting them to zero to permit directX to change backbuffer size
        deviceSetting.BackBufferHeight = 0
        deviceSetting.BackBufferWidth = 0
        device.Reset(deviceSetting)
    End Sub

    Sub defaultSetting()

        device.RenderState.ZBufferEnable = True 'Z buffer on
        device.RenderState.Lighting = False 'lights off
        device.RenderState.ShadeMode = ShadeMode.Gouraud 'gouraud mode


        device.Transform.World = Matrix.Identity
        device.Transform.View = Matrix.LookAtLH(New Vector3(0, 0, -30), New Vector3(0, 0, 0), New Vector3(0, 1, 0))
        device.Transform.Projection = Matrix.PerspectiveFovLH(CSng(Math.PI / 3), CSng(4 / 3), 1, 2000)

    End Sub

End Module
