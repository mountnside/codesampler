'=======================
' ROBYDX demo for the CodeSampler.com website
' www.robydx.135.it
' 2005 Italy
'=======================

Imports Microsoft.DirectX
Imports Microsoft.DirectX.Direct3D

Public Class Form1
    Inherits System.Windows.Forms.Form

#Region " Codice generato da Progettazione Windows Form "

    Public Sub New()
        MyBase.New()

        'Chiamata richiesta da Progettazione Windows Form.
        InitializeComponent()

        'Aggiungere le eventuali istruzioni di inizializzazione dopo la chiamata a InitializeComponent()

    End Sub

    'Form esegue l'override del metodo Dispose per pulire l'elenco dei componenti.
    Protected Overloads Overrides Sub Dispose(ByVal disposing As Boolean)
        If disposing Then
            If Not (components Is Nothing) Then
                components.Dispose()
            End If
        End If
        MyBase.Dispose(disposing)
    End Sub

    'Richiesto da Progettazione Windows Form
    Private components As System.ComponentModel.IContainer

    'NOTA: la procedura che segue è richiesta da Progettazione Windows Form.
    'Può essere modificata in Progettazione Windows Form.  
    'Non modificarla nell'editor del codice.
    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Dim resources As System.Resources.ResourceManager = New System.Resources.ResourceManager(GetType(Form1))
        '
        'Form1
        '
        Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
        Me.ClientSize = New System.Drawing.Size(632, 446)
        Me.Icon = CType(resources.GetObject("$this.Icon"), System.Drawing.Icon)
        Me.Name = "Form1"
        Me.Text = "Cell Shading ASM"

    End Sub

#End Region

    Public text1 As Font 'directX text

    Dim angle As Single 'rotation angle

    Dim m As oggX 'mesh

    Dim vShader As VertexShader 'vertex shader
    Dim vDecla As VertexDeclaration 'vertex declaration

    Dim tex1 As Texture 'texture for diffuse
    Dim tex2 As Texture 'texture for specular
    Dim camPos As Vector4 'camera position
    Dim LightPos As Vector4 'light position
   

    Private Sub Form1_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        Dim cap As Caps
        cap = Manager.GetDeviceCaps(0, DeviceType.Hardware)
        If (cap.VertexShaderVersion.Major < 1) Then
            MsgBox("Vertex Shader 1.1 not supported, application will be closed")
        End If

        'this function create directX for fullscreen of windowed mode
        'with true you'll use windowed mode
        createDevice(0, 0, 32, Me.Handle, True)
        defaultSetting()
        loadObject()
        'this instruction avoid artefact
        Me.SetStyle(ControlStyles.AllPaintingInWmPaint Or ControlStyles.Opaque, True)
        Me.Invalidate() 'invalidate rendering area
    End Sub

    Sub loadObject()
        'see createFont function in moduleX
        'this function create directX font from .net font
        text1 = createFont(New Drawing.Font("Arial", 12))

        'set lights
        device.Lights(0).Type = LightType.Directional
        device.Lights(0).Diffuse = Color.White
        device.Lights(0).Direction = New Vector3(1, 0, 1)
        device.Lights(0).Enabled = True
        device.Lights(0).Update()
        device.RenderState.Lighting = True

        'set a diffuse material for object
        Dim materialDesc As Material
        materialDesc.Diffuse = Color.White
        device.Material = materialDesc

        'create mesh
        m = createMesh(Application.StartupPath & "\head.x", True, True, Application.StartupPath)
        vDecla = standardVertexD()
        'load vertex shader from file (see function)
        vShader = CreateVertexShader(Application.StartupPath & "\cellS.txt")
        'create texture for diffuse value and border
        tex1 = createTexture(Application.StartupPath & "\cell.bmp")
        tex2 = createTexture(Application.StartupPath & "\sharp.bmp")

    End Sub

    Private Sub Form1_Closed(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Closed
        text1.Dispose()
        'end application
        End
    End Sub

    Private Sub Form1_Paint(ByVal sender As Object, ByVal e As System.Windows.Forms.PaintEventArgs) Handles MyBase.Paint

        'camera setting
        device.Transform.View = Matrix.LookAtLH(New Vector3(0, 0, -150), New Vector3(0, 0, 0), New Vector3(0, 1, 0))

        'rotation angle for the mesh
        angle = Environment.TickCount / 1000.0F

        device.Clear(ClearFlags.Target Or ClearFlags.ZBuffer, Color.Blue, 1, 0) 'clear the screen
        device.BeginScene() 'begin 3D scene

        'this function create a quick transformation matrix
        moveObj(1, 1, 1, 0, angle, 0, 0, 0, 0)
        Dim mat As Matrix

        'set vertex shader in directX 
        device.VertexShader = vShader
        device.VertexDeclaration = vDecla
        'set transformation matrix
        mat = Matrix.Multiply(device.Transform.World, device.Transform.View)
        mat = Matrix.MultiplyTranspose(mat, device.Transform.Projection)
        device.SetVertexShaderConstant(0, mat)

        'set camera and light vector
        mat = Matrix.Invert(device.Transform.World)
        camPos = Vector3.Transform(New Vector3(0, 0, -150), mat)
        LightPos = Vector3.Transform(New Vector3(100, 0, -100), mat)

        device.SetVertexShaderConstant(5, LightPos)
        device.SetVertexShaderConstant(6, camPos)

        'setting for device

        device.SetTexture(0, tex1)
        device.SamplerState(0).MagFilter = TextureFilter.Point
        device.SamplerState(0).MinFilter = TextureFilter.Point

        device.SamplerState(0).AddressU = TextureAddress.Clamp
        device.SamplerState(0).AddressV = TextureAddress.Wrap

        device.SetTexture(1, tex2)
        device.SamplerState(1).MagFilter = TextureFilter.Point
        device.SamplerState(1).MinFilter = TextureFilter.Point
        device.SamplerState(1).AddressU = TextureAddress.Clamp
        device.SamplerState(1).AddressV = TextureAddress.Wrap

        device.TextureState(0).ColorOperation = TextureOperation.Modulate
        device.TextureState(0).ColorArgument1 = TextureArgument.TextureColor
        device.TextureState(0).ColorArgument2 = TextureArgument.Diffuse

        device.TextureState(1).ColorOperation = TextureOperation.Modulate
        device.TextureState(1).ColorArgument1 = TextureArgument.TextureColor
        device.TextureState(1).ColorArgument2 = TextureArgument.Current

        Dim i As Integer
        For i = 0 To m.numX
            With m.mat(i).Diffuse
                'colore oggetto
                device.SetVertexShaderConstant(4, New Single() {.R / 256, .G / 256, .B / 256, .A / 256})
            End With
            m.mesh.DrawSubset(i)
        Next

        device.EndScene() 'end 3D scene
        device.Present() 'send graphics to screen
        Me.Invalidate()
    End Sub

    Private Sub Form1_Resize(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Resize

        If Not (device Is Nothing Or Me.WindowState = FormWindowState.Minimized) Then
            'device must be reset
            resetDevice()
            'all setting must be updated
            defaultSetting()

            'reload device setting
            'set lights
            device.Lights(0).Type = LightType.Directional
            device.Lights(0).Diffuse = Color.White
            device.Lights(0).Direction = New Vector3(1, 0, 1)
            device.Lights(0).Update()
            device.Lights(0).Enabled = True
            device.RenderState.Lighting = True

            'set a diffuse material for object
            Dim materialDesc As Material
            materialDesc.Diffuse = Color.White
            device.Material = materialDesc
        End If
    End Sub

End Class
