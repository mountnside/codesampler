Imports Microsoft.DirectX
Imports Microsoft.DirectX.Direct3D

Module mdx9a
    Public device As Device
    Dim deviceSetting As New PresentParameters
    Structure oggX
        'contain mesh and material for an object
        Public mesh As mesh
        Public numX As Integer
        Public tex() As Texture
        Public mat() As Material
    End Structure




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
        device = New Device(0, DeviceType.Hardware, fhWnd, CreateFlags.HardwareVertexProcessing, deviceSetting)



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

    'create texture from file
    Function createTexture(ByVal filesrc As String, Optional ByVal colorKey As Integer = 0) As Texture
        Return TextureLoader.FromFile(device, filesrc, 0, 0, 0, 0, Format.Unknown, Pool.Managed, Filter.Linear, Filter.Linear, colorKey)
    End Function
    Function createMesh(ByVal fileSrc As String, ByVal materialOn As Boolean, ByVal textureOn As Boolean, ByVal TexPath As String) As oggX
        'create an oggX structure from file defining file path, if use material and texture and texture path 
        With createMesh
            'create a mesh
            Dim materials() As ExtendedMaterial
            .mesh = Mesh.FromFile(fileSrc, MeshFlags.Managed, device, materials)
            'memorize material number
            .numX = UBound(materials)
            'create texture and material array
            ReDim .tex(.numX)
            ReDim .mat(.numX)
            Dim i As Integer
            'load all material
            For i = 0 To .numX
                If textureOn Then
                    'load texture if presente in file X
                    If materials(i).TextureFilename <> "" Then
                        .tex(i) = TextureLoader.FromFile(device, TexPath & "\" & materials(i).TextureFilename)
                    End If
                End If
                If materialOn Then
                    .mat(i) = materials(i).Material3D
                End If
            Next
        End With
    End Function

End Module
