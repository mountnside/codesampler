Imports Microsoft.DirectX
Imports Microsoft.DirectX.Direct3D

Module mdx9a
    Public device As device
    Dim deviceSetting As New PresentParameters
    Structure oggX
        'contain mesh and material for an object
        Public mesh As Mesh
        Public numX As Integer
        Public tex() As Texture
        Public mat() As Material
    End Structure

    Structure oggPX
        'contain mesh and material for a progressiveMesh
        Public mesh As ProgressiveMesh
        Public numX As Integer
        Public tex() As Texture
        Public mat() As Material
        Public numFaces As Integer
    End Structure

    Public Const GradToRad = Math.PI / 180

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

        device.SamplerState.SamplerState(0).MagFilter = TextureFilter.Linear
        device.SamplerState.SamplerState(0).MinFilter = TextureFilter.Linear
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

    'create progressive mesh
    Function createProgMesh(ByVal fileSrc As String, ByVal materialOn As Boolean, ByVal textureOn As Boolean, ByVal TexPath As String) As oggPX
        'create an oggPX structure from file defining file path, if use material and texture and texture path 
        With createProgMesh
            Dim mesh As Mesh

            Dim materials() As ExtendedMaterial
            'create a mesh
            Dim adiacenza As GraphicsStream = Nothing
            mesh = mesh.FromFile(fileSrc, MeshFlags.Managed, device, adiacenza, materials)

            'from mesh create the progressive mesh
            .mesh = New ProgressiveMesh(mesh, adiacenza, Nothing, 1, MeshFlags.SimplifyFace)
            'set the number of faces to maximum
            .mesh.NumberFaces = .mesh.MaxFaces
            .numFaces = .mesh.MaxFaces
            'create texture and material array
            .numX = UBound(materials)
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

    'create font
    Function createFont(ByVal f As Drawing.Font) As Font
        Return New Font(device, f)
    End Function

    'this function create a world matrix with a manual setting of the world matrix
    'improve performance and works for simple mesh transformation
    Public Sub moveObj(ByVal scalaX As Single, ByVal scalaY As Single, ByVal scalaZ As Single, ByVal angleX As Single, ByVal angleY As Single, ByVal angleZ As Single, ByVal posX As Single, ByVal posY As Single, ByVal posZ As Single)

        Dim CosRx As Single, CosRy As Single, CosRz As Single
        Dim SinRx As Single, SinRy As Single, SinRz As Single
        Dim mat1 As Matrix
        '
        CosRx = Math.Cos(angleX)  'Used 6x
        CosRy = Math.Cos(angleY)  'Used 4x
        CosRz = Math.Cos(angleZ)   'Used 4x
        SinRx = Math.Sin(angleX)   'Used 5x
        SinRy = Math.Sin(angleY)   'Used 5x
        SinRz = Math.Sin(angleZ)   'Used 5x
        With mat1
            .M11 = (scalaX * CosRy * CosRz)
            .M12 = (scalaX * CosRy * SinRz)
            .M13 = -(scalaX * SinRy)
            '
            .M21 = -(scalaY * CosRx * SinRz) + (scalaY * SinRx * SinRy * CosRz)
            .M22 = (scalaY * CosRx * CosRz) + (scalaY * SinRx * SinRy * SinRz)
            .M23 = (scalaY * SinRx * CosRy)
            '
            .M31 = (scalaZ * SinRx * SinRz) + (scalaZ * CosRx * SinRy * CosRz)
            .M32 = -(scalaZ * SinRx * CosRz) + (scalaZ * CosRx * SinRy * SinRz)
            .M33 = (scalaZ * CosRx * CosRy)
            '
            .M41 = posX
            .M42 = posY
            .M43 = posZ
            .M44 = 1.0#
        End With
        device.SetTransform(TransformType.World, mat1)
    End Sub

    Function standardVertexD() As VertexDeclaration
        'create vertex declaration for position, normal texture vertex type
        Dim elements() As VertexElement = {New VertexElement(0, 0, DeclarationType.Float3, DeclarationMethod.Default, DeclarationUsage.Position, 0), New VertexElement(0, 12, DeclarationType.Float3, DeclarationMethod.Default, DeclarationUsage.Normal, 0), New VertexElement(0, 24, DeclarationType.Float2, DeclarationMethod.Default, DeclarationUsage.TextureCoordinate, 0), VertexElement.VertexDeclarationEnd}
        Return New VertexDeclaration(device, elements)
    End Function

    Function CreateVertexShader(ByVal strFilename As String) As VertexShader
        Dim code As GraphicsStream = Nothing
        ' Assemble the vertex shader file
        code = ShaderLoader.FromFile(strFilename, Nothing, 0)
        ' Create the vertex shader
        Return New VertexShader(device, code)
    End Function

End Module