'=======================
' ROBYDX demo for codesampler website
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
        Me.Text = "Predefined Mesh"

    End Sub

#End Region


    Public text1 As Font 'directX text

    Dim meshModel As Mesh 'mesh that 'll be showed
    Dim angle As Single 'rotation angle

    Private Sub Form1_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load


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

        'create 3D font
        meshModel = Mesh.TextFromFont(device, New Drawing.Font("Arial", 12, FontStyle.Bold), "RobyDx", 1, 1)
       

    End Sub



    Private Sub Form1_Closed(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Closed
        text1.Dispose()
        'end application
        End
    End Sub

    Private Sub Form1_Paint(ByVal sender As Object, ByVal e As System.Windows.Forms.PaintEventArgs) Handles MyBase.Paint

        'camera setting
        device.Transform.View = Matrix.LookAtLH(New Vector3(0, 0, -10), New Vector3(0, 0, 0), New Vector3(0, 1, 0))

        'rotation angle for the mesh
        angle = Environment.TickCount / 1000.0F

        device.Clear(ClearFlags.Target Or ClearFlags.ZBuffer, Color.Blue, 1, 0) 'clear the screen
        device.BeginScene() 'begin 3D scene


        'this function create a quick transformation matrix
        moveObj(1, 1, 1, 0, angle, 0, 0, 0, 0)
       
        'draw mesh
        meshModel.DrawSubset(0)

        Dim r As Rectangle
        r = New Rectangle(0, 0, 640, 480)

        'draw text in a rectangle zone
        text1.DrawText(Nothing, "Press from 1 to 6 to change mesh model ", r, DrawTextFormat.Left, Color.White)


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

    Private Sub Form1_KeyDown(ByVal sender As Object, ByVal e As System.Windows.Forms.KeyEventArgs) Handles MyBase.KeyDown
        Select Case e.KeyCode
            Case Keys.Escape
                End
            Case Keys.D1
                'create 3D text
                meshModel = Mesh.TextFromFont(device, New Drawing.Font("Arial", 12, FontStyle.Bold), "RobyDx", 1, 1)
            Case Keys.D2
                'create box
                meshModel = Mesh.Box(device, 2, 2, 2)
            Case Keys.D3
                'create cylinder
                meshModel = Mesh.Cylinder(device, 2, 2, 2, 20, 1)
            Case Keys.D4
                'create sphere
                meshModel = Mesh.Sphere(device, 2, 20, 20)
            Case Keys.D5
                'create teapot
                meshModel = Mesh.Teapot(device)
            Case Keys.D6
                'create torus
                meshModel = Mesh.Torus(device, 0.5, 1.5, 6, 20)
        End Select
    End Sub



End Class
