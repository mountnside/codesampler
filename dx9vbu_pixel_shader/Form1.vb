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
        Me.Text = "ASM Pixel Shader"

    End Sub

#End Region

    Public text1 As Font 'directX text

    Dim angle As Single 'rotation angle

    'the pixel shader
    Dim shad1 As PixelShader
    Dim shad2 As PixelShader
    Dim shad3 As PixelShader
    Dim shad4 As PixelShader
    Dim shad5 As PixelShader
    Dim n As Integer = 0 'used to change shader

    Dim ogg As oggX 'model
    Dim t As Texture 'second texture

    Private Sub Form1_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        'check pixel shader support
        Dim cap As Caps
        cap = Manager.GetDeviceCaps(0, DeviceType.Hardware)
        If (cap.PixelShaderVersion.Major < 1) Then
            MsgBox("Hardware doesn't not support pixel shader, application will be closed")
            End
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

        'set a diffuse material for object
        Dim materialDesc As Material
        materialDesc.Diffuse = Color.White
        device.Material = materialDesc

        ogg = createMesh(Application.StartupPath & "\cubo.x", True, True, Application.StartupPath)
        'set pixel shader definition
        shad1 = CreatePixelShader(Application.StartupPath & "\tex1.txt")
        shad2 = CreatePixelShader(Application.StartupPath & "\tex2.txt")
        shad3 = CreatePixelShader(Application.StartupPath & "\tex3.txt")
        shad4 = CreatePixelShader(Application.StartupPath & "\tex4.txt")
        shad5 = CreatePixelShader(Application.StartupPath & "\tex5.txt")
        t = createTexture(Application.StartupPath & "\texture1.bmp")
        device.TextureState(1).TextureCoordinateIndex = 0
        device.PixelShader = shad1
    End Sub

    Private Sub Form1_Closed(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Closed
        text1.Dispose()
        'end application
        End
    End Sub

    Private Sub Form1_Paint(ByVal sender As Object, ByVal e As System.Windows.Forms.PaintEventArgs) Handles MyBase.Paint

        'camera setting
        device.Transform.View = Matrix.LookAtLH(New Vector3(0, 8, -8), New Vector3(0, 2, 0), New Vector3(0, 1, 0))

        'rotation angle for the mesh
        angle = Environment.TickCount / 5000.0F

        device.Clear(ClearFlags.Target Or ClearFlags.ZBuffer, Color.Blue, 1, 0) 'clear the screen
        device.BeginScene() 'begin 3D scene

        'this function create a quick transformation matrix
        moveObj(1, 1, 1, 0, angle, 0, 0, 0, 0)
        'set pixel shader into the device
        Select Case n
            Case 0
                device.PixelShader = shad1
            Case 1
                device.PixelShader = shad2
            Case 2
                device.PixelShader = shad3
            Case 3
                device.PixelShader = shad4
            Case 4
                device.PixelShader = shad5
        End Select

        Dim i As Integer
        For i = 0 To ogg.numX
            device.SetTexture(0, ogg.tex(i))
            device.SetTexture(1, t)
            ogg.mesh.DrawSubset(i)
        Next

        text1.DrawText(Nothing, "Change pixel shader with Up and Down Arrow keys", 0, 0, Color.White)

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
            
            'set a diffuse material for object
            Dim materialDesc As Material
            materialDesc.Diffuse = Color.White
            device.Material = materialDesc
        End If
    End Sub

    Private Sub Form1_KeyDown(ByVal sender As Object, ByVal e As System.Windows.Forms.KeyEventArgs) Handles MyBase.KeyDown
        'change shader
        Select Case e.KeyCode
            Case Keys.Up
                n += 1
                If n > 4 Then n = 0
            Case Keys.Down
                n -= 1
                If n < 0 Then n = 4
        End Select
    End Sub

End Class
