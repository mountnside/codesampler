'=======================
' ROBYDX demo for CodeSampler.com website
' www.robydx.135.it
' 2005 Italy
'=======================

Imports Microsoft.DirectX
Imports Microsoft.DirectX.Direct3D


Public Class Form1
    Inherits System.Windows.Forms.Form
    'color for the background

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
        Me.Text = "Lighting"

    End Sub

#End Region


    Dim angle As Single 'rotation angle
    Dim model As oggX
    Dim lightMode As Integer

    Private Sub Form1_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load

        MsgBox("Use key 1,2 and 3 to change lights model")
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
        'create a model
        model = createMesh(Application.StartupPath & "\cube.x", True, True, Application.StartupPath)

        'setting a more distante camera
        device.Transform.View = Matrix.LookAtLH(New Vector3(0, 50, -100), New Vector3(0, 0, 0), New Vector3(0, 1, 0))

        'enable light 1
        device.RenderState.Lighting = True
        device.Lights(0).Enabled = True

        Dim e As New KeyEventArgs(Keys.D1)
        Me.Form1_KeyDown(Nothing, e)
    End Sub



    Private Sub Form1_Closed(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Closed
        'end application
        End
    End Sub

    Private Sub Form1_Paint(ByVal sender As Object, ByVal e As System.Windows.Forms.PaintEventArgs) Handles MyBase.Paint

        'transformation matrix
        Dim tempMatrix As Matrix
        'syncronize rotation angle with timer
        angle = Environment.TickCount / 1000.0F

        device.Clear(ClearFlags.Target Or ClearFlags.ZBuffer, Color.Blue, 1, 0) 'clear the screen
        device.BeginScene() 'begin 3D scene

        'set transformation matrix
        tempMatrix = Matrix.RotationY(angle)

        device.Transform.World = tempMatrix

        'loop all mesh subset
        Dim i As Integer
        For i = 0 To model.numX
            'set material
            device.Material = model.mat(i)
            'set texture
            device.SetTexture(0, model.tex(i))
            'draw mesh
            model.mesh.DrawSubset(i)
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
            'setting a more distante camera
            device.Transform.View = Matrix.LookAtLH(New Vector3(0, 50, -100), New Vector3(0, 0, 0), New Vector3(0, 1, 0))

            'enable light 1
            device.RenderState.Lighting = True
            device.Lights(0).Enabled = True

            If lightMode = 0 Then
                Me.Form1_KeyDown(Nothing, New KeyEventArgs(Keys.D1))
            End If

            If lightMode = 1 Then
                Me.Form1_KeyDown(Nothing, New KeyEventArgs(Keys.D2))
            End If

            If lightMode = 2 Then
                Me.Form1_KeyDown(Nothing, New KeyEventArgs(Keys.D3))
            End If
        End If
    End Sub

    'change lights setting
    Private Sub Form1_KeyDown(ByVal sender As Object, ByVal e As System.Windows.Forms.KeyEventArgs) Handles MyBase.KeyDown
        Select Case e.KeyCode
            Case Keys.D1
                'directional
                device.Lights(0).Type = LightType.Directional
                device.Lights(0).Diffuse = Color.Green
                device.Lights(0).Direction = New Vector3(0, 0, 1)
                device.Lights(0).Update()
                lightMode = 0
                Me.Text = "Directional Light"
            Case Keys.D2
                'point
                device.Lights(0).Type = LightType.Point
                device.Lights(0).Diffuse = Color.Red
                device.Lights(0).Direction = New Vector3(1, 1, 1)
                device.Lights(0).Position = New Vector3(0, 0, 0)
                device.Lights(0).Range = 1000
                device.Lights(0).Attenuation1 = 0.01
                device.Lights(0).Update()
                lightMode = 1
                Me.Text = "Point Light"
            Case Keys.D3
                'spot
                device.Lights(0).Type = LightType.Spot
                device.Lights(0).Diffuse = Color.Yellow
                device.Lights(0).Direction = New Vector3(0, 0, 1)
                device.Lights(0).Position = New Vector3(0, 0, 0)
                device.Lights(0).Falloff = 5
                device.Lights(0).Range = 1000
                device.Lights(0).InnerConeAngle = 45 * GradToRad
                device.Lights(0).OuterConeAngle = 90 * GradToRad
                device.Lights(0).Attenuation1 = 0.01
                device.Lights(0).Update()
                lightMode = 2
                Me.Text = "Spot Light"
        End Select
    End Sub
End Class
