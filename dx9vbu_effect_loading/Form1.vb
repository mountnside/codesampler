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
        Me.Text = "Effect Loading"

    End Sub

#End Region

    Public text1 As Font 'directX text

    Dim angle As Single 'rotation angle

    Dim m As Mesh
    Dim noise As VolumeTexture

    Dim effectPool As New EffectPool

    Dim f As Effect
    Dim tec1 As EffectHandle

    'some parameters
    Dim pScale As Single = 10
    Dim RingScale As Single = 0.66
    Dim AmpScale As Single = 0.7

    'handle for parameters used in shader
    Dim h_WorldIT As EffectHandle
    Dim h_WorldViewProj As EffectHandle
    Dim h_World As EffectHandle
    Dim h_ViewI As EffectHandle
    Dim h_LightPos As EffectHandle
    Dim h_LightColor As EffectHandle
    Dim h_AmbiColor As EffectHandle
    Dim h_WoodColor1 As EffectHandle
    Dim h_WoodColor2 As EffectHandle
    Dim h_Ks As EffectHandle
    Dim h_SpecExpon As EffectHandle
    Dim h_RingScale As EffectHandle
    Dim h_AmpScale As EffectHandle
    Dim h_PScale As EffectHandle
    Dim h_POffset As EffectHandle
    Dim h_NoiseTex As EffectHandle
   
    Private Sub Form1_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        Dim cap As Caps
        cap = Manager.GetDeviceCaps(0, DeviceType.Hardware)
        If (cap.PixelShaderVersion.Major < 2) Then
            MsgBox("pixel shader version 2.0 not supported. Application will be closed")
            End
        End If

        'this function create directX for fullscreen of windowed mode
        'with true you'll use windowed mode
        createDevice(0, 0, 32, Me.Handle, True)
        defaultSetting()
        loadObject()
        'this instruction avoid artifact
        Me.SetStyle(ControlStyles.AllPaintingInWmPaint Or ControlStyles.Opaque, True)
        Me.Invalidate() 'invalidate rendering area
    End Sub

    Sub loadObject()
        'see createFont function in moduleX
        'this function create directX font from .net font
        text1 = createFont(New Drawing.Font("Arial", 10))

        'set a diffuse material for object
        Dim materialDesc As Material
        materialDesc.Diffuse = Color.White
        device.Material = materialDesc

        'create a mesh
        m = Mesh.Teapot(device)

        noise = TextureLoader.FromVolumeFile(device, "noise.dds")

        'create an effect from file
        f = Effect.FromFile(device, Application.StartupPath & "\woodEffect.fx", Nothing, "", ShaderFlags.None, Nothing)
      
        'create handles for passing value to effect
        h_WorldIT = f.GetParameter(Nothing, "WorldIT")
        h_WorldViewProj = f.GetParameter(Nothing, "WorldViewProj")
        h_World = f.GetParameter(Nothing, "World")
        h_ViewI = f.GetParameter(Nothing, "ViewI")
        h_LightPos = f.GetParameter(Nothing, "LightPos")
        h_LightColor = f.GetParameter(Nothing, "LightColor")
        h_AmbiColor = f.GetParameter(Nothing, "AmbiColor")
        h_WoodColor1 = f.GetParameter(Nothing, "WoodColor1")
        h_WoodColor2 = f.GetParameter(Nothing, "WoodColor2")
        h_Ks = f.GetParameter(Nothing, "Ks")
        h_SpecExpon = f.GetParameter(Nothing, "SpecExpon")
        h_RingScale = f.GetParameter(Nothing, "RingScale")
        h_AmpScale = f.GetParameter(Nothing, "AmpScale")
        h_PScale = f.GetParameter(Nothing, "PScale")
        h_POffset = f.GetParameter(Nothing, "POffset")
        h_NoiseTex = f.GetParameter(Nothing, "NoiseTex")

        'create the technique from effect
        tec1 = f.GetTechnique("wood")

        'in this way the effect 'll use the technique that we have created
        f.Technique = tec1

        'this parameter remain the same for all the application
        f.SetValue(h_NoiseTex, noise)
        f.SetValue(h_LightPos, New Single() {10, 0, -10})
    End Sub

    Private Sub Form1_Closed(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Closed
        text1.Dispose()
        device.Dispose()
        'end application
        End
    End Sub

    Private Sub Form1_Paint(ByVal sender As Object, ByVal e As System.Windows.Forms.PaintEventArgs) Handles MyBase.Paint

        'camera setting
        device.Transform.View = Matrix.LookAtLH(New Vector3(0, 0, -3), New Vector3(0, 0, 0), New Vector3(0, 1, 0))

        'rotation angle for the mesh
        angle = Environment.TickCount / 2000.0F

        device.Clear(ClearFlags.Target Or ClearFlags.ZBuffer, Color.Blue, 1, 0) 'clear the screen
        device.BeginScene() 'begin 3D scene

        'this function create a quick transformation matrix
        moveObj(1, 1, 1, 0, angle, 0, 0, 0, 0)

        Dim tempMatrix As Matrix

        'we start using the effect
        f.Begin(FX.DoNotSaveShaderState)
        f.BeginPass(0)

        'we pass parameters to effect using handle
        tempMatrix = device.Transform.World
        tempMatrix.Invert()
        tempMatrix.Transpose(tempMatrix)
        f.SetValue(h_WorldIT, tempMatrix)

        tempMatrix = Matrix.Multiply(device.Transform.World, device.Transform.View)
        tempMatrix.Multiply(device.Transform.Projection)
        f.SetValue(h_WorldViewProj, tempMatrix)
        f.SetValue(h_World, device.Transform.World)
        tempMatrix = device.Transform.View
        tempMatrix.Invert()
        f.SetValue(h_ViewI, tempMatrix)
        f.CommitChanges()

        'draw the mesh
        m.DrawSubset(0)

        'end the effect
        f.EndPass()
        f.End()

        text1.DrawText(Nothing, "Q: Increase scale", 0, 0, Color.White)
        text1.DrawText(Nothing, "A: Decrease scale", 0, 15, Color.White)
        text1.DrawText(Nothing, "W: Increase amplitude", 0, 30, Color.White)
        text1.DrawText(Nothing, "S: Decrease amplitude", 0, 45, Color.White)
        text1.DrawText(Nothing, "E: Increase ring scale", 0, 60, Color.White)
        text1.DrawText(Nothing, "D: Decrease ring scale", 0, 75, Color.White)

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

    'change some parameters for shader
    Private Sub Form1_KeyDown(ByVal sender As Object, ByVal e As System.Windows.Forms.KeyEventArgs) Handles MyBase.KeyDown
        Select Case e.KeyCode
            Case Keys.Q
                pScale += 1
                f.SetValue(h_PScale, pScale)
            Case Keys.A
                pScale -= 1
                f.SetValue(h_PScale, pScale)
            Case Keys.W
                AmpScale += 0.1
                f.SetValue(h_AmpScale, AmpScale)
            Case Keys.S
                AmpScale -= 0.1
                f.SetValue(h_AmpScale, AmpScale)
            Case Keys.E
                RingScale += 0.01
                f.SetValue(h_RingScale, RingScale)
            Case Keys.D
                RingScale -= 0.01
                f.SetValue(h_RingScale, RingScale)
        End Select
    End Sub

End Class
