Imports Microsoft.DirectX
Imports Microsoft.DirectX.Direct3D
Imports Microsoft.DirectX.DirectInput
Imports Microsoft.DirectX.DirectSound
Imports Microsoft.DirectX.AudioVideoPlayback


#Region "grafica 3D"
Module mdx9a

    Public device As Direct3D.Device
    Public Const rad = Math.PI / 180
    Public sprite As sprite
    Public settaggio As PresentParameters  'oggetto per il settaggio parametri

    Public Structure Vertex
        Public p As Vector3
        Public n As Vector3
        Public tu, tv As Single
        Public Const Format As VertexFormats = VertexFormats.Position Or VertexFormats.Normal Or VertexFormats.Texture1
    End Structure

    Structure oggX
        'struttura per la gestione semplice delle mesh
        Public mesh As mesh
        Public numX As Integer
        Public tex() As Texture
        Public mat() As Material
    End Structure


    Structure oggPX
        'struttura per la gestione semplice delle progressiveMesh
        Public mesh As ProgressiveMesh
        Public numX As Integer
        Public tex() As Texture
        Public mat() As Material
        Public numF As Integer
    End Structure

    Sub creaSchermo(ByVal width As Integer, ByVal height As Integer, ByVal bpp As Integer, ByVal fhWnd As System.IntPtr, ByVal finestra As Boolean, Optional ByVal debugMode As Boolean = False, Optional ByVal Hz As Integer = 0)
        settaggio = New PresentParameters()
        settaggio.BackBufferCount = 1 'numero backbuffer
        settaggio.AutoDepthStencilFormat = DepthFormat.D16 'formato Z/Stencil buffer
        settaggio.EnableAutoDepthStencil = True 'Z/Stencil buffer attivato
        settaggio.DeviceWindowHandle = fhWnd 'handle del form
        settaggio.SwapEffect = SwapEffect.Discard 'tipo di renderizzazione



        If Hz = 0 Then
        Else
            settaggio.FullScreenRefreshRateInHz = Hz
        End If

        If finestra Then
            settaggio.Windowed = True 'settaggio per finestra
        Else
            settaggio.Windowed = False 'settaggio per fullscreen
            settaggio.BackBufferWidth = width 'risoluzione schermo
            settaggio.BackBufferHeight = height 'risoluzione schermo
            If bpp = 16 Then
                settaggio.BackBufferFormat = Format.R5G6B5 'formato colore a 16
            Else
                settaggio.BackBufferFormat = Format.X8R8G8B8 'formato colore a 32
            End If

        End If
        'intervallo di presentazione
        If debugMode Then settaggio.PresentationInterval = PresentInterval.Immediate
        'crea oggetto device
        device = New Direct3D.Device(0, Direct3D.DeviceType.Hardware, fhWnd, CreateFlags.HardwareVertexProcessing, settaggio)

        'crea sprite
        sprite = New sprite(device)

    End Sub

    Sub iniziatutto()
        'caricamento personale di default
        device.RenderState.ZBufferEnable = True 'Z buffer on
        device.RenderState.Lighting = False 'luci spente
        device.RenderState.ShadeMode = ShadeMode.Gouraud 'gouraud mode

        'texture
        'qualità texture
        device.SamplerState.SamplerState(0).MagFilter = TextureFilter.Linear
        device.SamplerState.SamplerState(0).MinFilter = TextureFilter.Linear

        device.Transform.World = Matrix.Identity
        device.Transform.View = Matrix.LookAtLH(New Vector3(0, 0, -30), New Vector3(0, 0, 0), New Vector3(0, 1, 0))
        device.Transform.Projection = Matrix.PerspectiveFovLH(CSng(Math.PI / 3), CSng(4 / 3), 1, 2000)

    End Sub

    Public Sub muoviObj(ByVal scalaX As Single, ByVal scalaY As Single, ByVal scalaZ As Single, ByVal angleX As Long, ByVal angleY As Long, ByVal angleZ As Long, ByVal posX As Single, ByVal posY As Single, ByVal posZ As Single)
        'creazione rapida matworld
        Dim CosRx As Single, CosRy As Single, CosRz As Single
        Dim SinRx As Single, SinRy As Single, SinRz As Single
        Dim mat1 As Matrix
        '
        CosRx = Math.Cos(angleX * rad) 'Used 6x
        CosRy = Math.Cos(angleY * rad) 'Used 4x
        CosRz = Math.Cos(angleZ * rad) 'Used 4x
        SinRx = Math.Sin(angleX * rad) 'Used 5x
        SinRy = Math.Sin(angleY * rad) 'Used 5x
        SinRz = Math.Sin(angleZ * rad) 'Used 5x
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

    Public Function muoviRet(ByVal scalaX As Single, ByVal scalaY As Single, ByVal scalaZ As Single, ByVal angleX As Long, ByVal angleY As Long, ByVal angleZ As Long, ByVal posX As Single, ByVal posY As Single, ByVal posZ As Single) As Matrix
        'creazione rapida matworld
        Dim CosRx As Single, CosRy As Single, CosRz As Single
        Dim SinRx As Single, SinRy As Single, SinRz As Single
        Dim mat1 As Matrix
        '
        CosRx = Math.Cos(angleX * rad) 'Used 6x
        CosRy = Math.Cos(angleY * rad) 'Used 4x
        CosRz = Math.Cos(angleZ * rad) 'Used 4x
        SinRx = Math.Sin(angleX * rad) 'Used 5x
        SinRy = Math.Sin(angleY * rad) 'Used 5x
        SinRz = Math.Sin(angleZ * rad) 'Used 5x
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
        Return mat1
    End Function

    Function creaTexture(ByVal filesrc As String, Optional ByVal colore As Integer = 0) As Texture
        creaTexture = TextureLoader.FromFile(device, filesrc, 0, 0, 1, 0, Format.Unknown, Pool.Default, Filter.Linear, Filter.Linear, colore)
    End Function



    Function creaMesh(ByVal fileSrc As String, ByVal materialiOn As Boolean, ByVal textureOn As Boolean, ByVal TexPath As String) As oggX
        'crea un oggetto oggX che gestisce le mesh e i suoi materiali.
        'filesrc è il nome del file con path, materialiOn e textureOn indicano se si vuole caricare quel componente
        'texPath è il path della cartella senza / finale dove si trovano le texture
        With creaMesh
            'crea una mesh dichiarando i materiali in un extendMaterial
            Dim materiali() As ExtendedMaterial
            .mesh = Mesh.FromFile(fileSrc, MeshFlags.Dynamic, device, materiali)
            'setta il numero di materiali e ridimensiona gli array
            .numX = UBound(materiali)
            ReDim .tex(.numX)
            ReDim .mat(.numX)
            Dim i As Integer
            'con un loop carica i materiali da file
            For i = 0 To .numX
                If textureOn Then
                    'solo se il nome della è texture non è vuoto eseguo il caricamento
                    If materiali(i).TextureFilename <> "" Then
                        .tex(i) = TextureLoader.FromFile(device, TexPath & "\" & materiali(i).TextureFilename)
                    End If
                End If
                If materialiOn Then
                    .mat(i) = materiali(i).Material3D
                    .mat(i).Ambient = .mat(i).Diffuse 'pongo l'ambiente uguale alla diffuse
                End If
            Next
        End With
    End Function


    Function creaProgMesh(ByVal fileSrc As String, ByVal materialiOn As Boolean, ByVal textureOn As Boolean, ByVal TexPath As String) As oggPX
        'crea un oggetto oggX che gestisce le mesh e i suoi materiali.
        'filesrc è il nome del file con path, materialiOn e textureOn indicano se si vuole caricare quel componente
        'texPath è il path della cartella senza / finale dove si trovano le texture
        With creaProgMesh
            Dim mesh As Mesh
            'crea una mesh dichiarando i materiali in un extendMaterial
            Dim materiali() As ExtendedMaterial

            Dim adiacenza As GraphicsStream = Nothing
            mesh = mesh.FromFile(fileSrc, MeshFlags.Managed, device, adiacenza, materiali)


            'numero iniziale uguale al massimo

            .mesh = New ProgressiveMesh(mesh, adiacenza, Nothing, 1, MeshFlags.SimplifyFace)
            .mesh.NumberFaces = .mesh.MaxFaces
            .numF = .mesh.MaxFaces
            'setta il numero di materiali e ridimensiona gli array
            .numX = UBound(materiali)
            ReDim .tex(.numX)
            ReDim .mat(.numX)
            Dim i As Integer
            'con un loop carica i materiali da file
            For i = 0 To .numX
                If textureOn Then
                    'solo se il nome della è texture non è vuoto eseguo il caricamento
                    If materiali(i).TextureFilename <> "" Then
                        .tex(i) = TextureLoader.FromFile(device, TexPath & "\" & materiali(i).TextureFilename)
                    End If
                End If
                If materialiOn Then
                    .mat(i) = materiali(i).Material3D
                    .mat(i).Ambient = .mat(i).Diffuse 'pongo l'ambiente uguale alla diffuse
                End If
            Next
        End With
    End Function


    Function AppPath() As String
        Dim s As String = Application.ExecutablePath
        Dim i As Integer
        For i = s.Length - 1 To 0 Step -1
            If s.Chars(i) = "\" Then
                Return s.Substring(0, i)
            End If
        Next
    End Function

    Function creaFont(ByVal f As Drawing.Font) As Font
        Return New Font(device, f)
    End Function

    Function getVertexArray(ByVal m As Mesh) As System.Array
        'restituisce un array di vertici
        Dim vertexBuffer As VertexBuffer = m.VertexBuffer
        getVertexArray = vertexBuffer.Lock(0, GetType(Vertex), 0, m.NumberVertices)
        vertexBuffer.Unlock()
        'esempioDim Dim vertices As CustomVertex.PositionNormalTextured() = Nothing
        'vertici = getVertexArray(cubo.mesh)
    End Function

    Function getIndexArray(ByVal m As Mesh) As System.Array
        'restituisce un array di vertici
        Dim indexBuffer As IndexBuffer = m.IndexBuffer
        getIndexArray = indexBuffer.Lock(0, GetType(Short), 0, m.NumberFaces * 3)
        indexBuffer.Unlock()
        'esempioDim Dim vertices As CustomVertex.PositionNormalTextured() = Nothing
        'vertici = getIndexArray(cubo.mesh)
    End Function

    Sub setVertexArray(ByVal m As Mesh, ByVal v As Vertex())
        'imposta la mesh con un array di D3DVertex
        Dim vertici As Vertex()
        Dim vertexBuffer As VertexBuffer = m.VertexBuffer
        vertici = vertexBuffer.Lock(0, GetType(Vertex), 0, m.NumberVertices)
        Dim i As Integer
        For i = 0 To vertici.Length - 1
            vertici(i) = v(i)
        Next
        vertexBuffer.Unlock()
    End Sub

    Sub creaNebbia(ByVal attiva As Boolean, ByVal colore As Color, ByVal tipoV As FogMode, Optional ByVal tipoT As FogMode = FogMode.None, Optional ByVal inizio As Single = 0, Optional ByVal fine As Single = 0, Optional ByVal densità As Single = 0)
        'impostazione nebbia 
        With device.RenderState
            .FogEnable = attiva
            .FogStart = inizio
            .FogEnd = fine
            .FogDensity = densità
            .FogVertexMode = tipoV
            .FogColor = colore
            .FogTableMode = tipoT
        End With
    End Sub


    'Pixel shader
    Function CreaPixelShader(ByVal strFilename As String) As PixelShader
        Dim code As GraphicsStream = Nothing
        ' Assemble the vertex shader file
        code = ShaderLoader.FromFile(strFilename, Nothing, 0)
        ' Create the vertex shader
        Return New PixelShader(device, code)
        'esempio
        'device.PixelShader = variabile
        'device.SetPixelShaderConstant registro,new Vector4{vettore}
    End Function 'CreateVertexShader

    Function standardVertexD() As VertexDeclaration
        'genera una dichiarazione standard
        Dim elementi() As VertexElement = {New VertexElement(0, 0, DeclarationType.Float3, DeclarationMethod.Default, DeclarationUsage.Position, 0), New VertexElement(0, 12, DeclarationType.Float3, DeclarationMethod.Default, DeclarationUsage.Normal, 0), New VertexElement(0, 24, DeclarationType.Float2, DeclarationMethod.Default, DeclarationUsage.TextureCoordinate, 0), VertexElement.VertexDeclarationEnd}
        Return New VertexDeclaration(device, elementi)
    End Function

    Function CreateVertexShader(ByVal strFilename As String) As VertexShader
        Dim code As GraphicsStream = Nothing
        ' Assembla il vertex shader da file
        code = ShaderLoader.FromFile(strFilename, Nothing, 0)
        ' Create the vertex shader
        Return New VertexShader(device, code)
    End Function

    Function createVB(ByVal m As Mesh) As VertexBuffer
        Dim src As Vertex() 'memorizza temporaneamente l'array di vertici
        Dim dst As GraphicsStream 'memorizza il puntatore alla destinazione
        Dim vb As VertexBuffer 'vertexbuffer sorgente
        vb = m.VertexBuffer 'preso da mesh
        'generiamo il ricevente
        Dim vb2 As New VertexBuffer(GetType(Vertex), m.NumberVertices, device, Usage.WriteOnly, 0, Pool.Managed)
        'trasferiamo da uno ad altro
        dst = vb2.Lock(0, DXHelp.GetTypeSize(GetType(Vertex)) * m.NumberVertices, 0)
        src = CType(vb.Lock(0, GetType(Vertex), 0, m.NumberVertices), Vertex())
        dst.Write(src)
        vb2.Unlock()
        vb.Unlock()
        vb.Dispose()
        'restituisci
        Return vb2
    End Function

    Sub destroyMesh(ByVal m As oggX)
        Dim i As Integer
        For i = 0 To m.numX
            m.tex(i).Dispose()
        Next
        m.mesh.Dispose()
    End Sub
End Module
#End Region

#Region "controlli"
Module mdx9_DI
    'tutto direct input
    Public tastiera As DirectInput.Device 'controllo tastiera
    Public mouse As DirectInput.Device 'controllo mouse
    Public joy1 As DirectInput.Device 'controllo joy1
    Public J1Attivo As Boolean 'se il joy1 è presente
    Public joy2 As DirectInput.Device 'controllo joy2
    Public J2Attivo As Boolean 'se il joy2 è presente

    Sub creaTastiera(ByVal fhWnd As System.Windows.Forms.Control, Optional ByVal esclusiva As Boolean = False)
        tastiera = New DirectInput.Device(SystemGuid.Keyboard)
        If esclusiva Then
            tastiera.SetCooperativeLevel(fhWnd, CooperativeLevelFlags.Foreground Or CooperativeLevelFlags.Exclusive)
        Else
            tastiera.SetCooperativeLevel(fhWnd, CooperativeLevelFlags.Background Or CooperativeLevelFlags.NonExclusive)
        End If
        tastiera.SetDataFormat(DeviceDataFormat.Keyboard)
        tastiera.Acquire()
    End Sub

    Function TastieraData() As KeyboardState
        TastieraData = tastiera.GetCurrentKeyboardState
    End Function

    Sub creaMouse(ByVal fhWnd As System.Windows.Forms.Control, Optional ByVal esclusiva As Boolean = False)
        mouse = New DirectInput.Device(SystemGuid.Mouse)
        If esclusiva Then
            mouse.SetCooperativeLevel(fhWnd, CooperativeLevelFlags.Foreground Or CooperativeLevelFlags.Exclusive)
        Else
            mouse.SetCooperativeLevel(fhWnd, CooperativeLevelFlags.Background Or CooperativeLevelFlags.NonExclusive)
        End If
        mouse.SetDataFormat(DeviceDataFormat.Mouse)
        mouse.Acquire()
    End Sub

    Function MouseData() As MouseState
        'mouse.Poll()
        MouseData = mouse.CurrentMouseState
    End Function

    Sub creaJoy1(ByVal fhWnd As System.Windows.Forms.Control, Optional ByVal esclusiva As Boolean = False)
        Dim periferica As DeviceInstance

        For Each periferica In DirectInput.Manager.GetDevices(DeviceClass.GameControl, EnumDevicesFlags.AttachedOnly)
            joy1 = New DirectInput.Device(periferica.InstanceGuid)
            joy1.SetDataFormat(DeviceDataFormat.Joystick)
            joy1.SetCooperativeLevel(fhWnd, CooperativeLevelFlags.Background Or CooperativeLevelFlags.NonExclusive)
            Dim d As DeviceObjectInstance
            For Each d In joy1.Objects
                If (d.ObjectId And CInt(DeviceObjectTypeFlags.Axis)) <> 0 Then
                    joy1.Properties.SetRange(ParameterHow.ById, d.ObjectId, New InputRange(-5000, 5000))
                End If
            Next
            joy1.Acquire()
            'ci interessa il primo disponibile
            Exit For
        Next
        If joy1 Is Nothing Then J1Attivo = False Else J1Attivo = True
    End Sub

    Function Joy1Data() As JoystickState
        joy1.Poll()
        Joy1Data = joy1.CurrentJoystickState
    End Function
End Module
#End Region

#Region "suono"
Module mdx9_Sound
    Public DirectSound As DirectSound.Device

    Sub creaAudio(ByVal fhWnd As System.Windows.Forms.Control)
        DirectSound = New DirectSound.Device()
        DirectSound.SetCooperativeLevel(fhWnd, CooperativeLevel.Priority)
    End Sub
    Function caricaSuono(ByVal fileSrc As String) As SecondaryBuffer
        caricaSuono = New SecondaryBuffer(fileSrc, DirectSound)
    End Function

End Module
#End Region

#Region "audio video"
Module mdx9_AudioVideo
    Function caricaAudio(ByVal filesrc As String) As Audio
        caricaAudio = New Audio(filesrc)
    End Function

    Function caricaVideo(ByVal filesrc As String) As Video
        caricaVideo = New Video(filesrc)
    End Function
End Module
#End Region


