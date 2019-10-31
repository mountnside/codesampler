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
        Me.Text = "Alphablending"

    End Sub

#End Region


    Public text1 As Font 'directX text

    Dim meshModel As oggX 'mesh that 'll be showed
    Dim angle As Single 'rotation angle

    Dim SValue As Integer = 14 'memorize alphablending setting
    Dim DValue As Integer = 8 'memorize alphablending setting
    Dim BOper As Integer 'memorize alphablending setting

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

        'default setting
        device.RenderState.SourceBlend = Blend.SourceColor
        device.RenderState.DestinationBlend = Blend.DestinationColor
        device.RenderState.BlendOperation = BlendOperation.Add
        'create 3D font
        meshModel = createMesh(Application.StartupPath + "\cubo.x", True, True, Application.StartupPath)

    End Sub



    Private Sub Form1_Closed(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Closed
        text1.Dispose()
        'end application
        device.Dispose()
        End
    End Sub

    Private Sub Form1_Paint(ByVal sender As Object, ByVal e As System.Windows.Forms.PaintEventArgs) Handles MyBase.Paint

        'camera setting
        device.Transform.View = Matrix.LookAtLH(New Vector3(0, 10, -30), New Vector3(0, 0, 0), New Vector3(0, 1, 0))

        'rotation angle for the mesh
        angle = Environment.TickCount / 1000.0F

        device.Clear(ClearFlags.Target Or ClearFlags.ZBuffer, Color.Blue, 1, 0) 'clear the screen
        device.BeginScene() 'begin 3D scene


       
       
        'draw mesh
        Dim i As Integer
        device.RenderState.AlphaBlendEnable = False
        'this function create a quick transformation matrix
        moveObj(1, 1, 1, 0, angle, 0, 0, -2, 0)
        For i = 0 To meshModel.numX
            device.SetTexture(0, meshModel.tex(i))
            device.Material = meshModel.mat(i)
            meshModel.mesh.DrawSubset(0)
        Next

        device.RenderState.AlphaBlendEnable = True
        'this function create a quick transformation matrix
        moveObj(3, 3, 3, 0, angle, 0, 0, -6, 0)
        For i = 0 To meshModel.numX
            device.SetTexture(0, meshModel.tex(i))
            device.Material = meshModel.mat(i)
            meshModel.mesh.DrawSubset(0)
        Next
        device.RenderState.AlphaBlendEnable = False

        Dim r As Rectangle
        r = New Rectangle(0, 0, 640, 480)

        'draw text in a rectangle zone
        text1.DrawText(Nothing, "Press numpad to change alphablending parameters. Press from 1 to 6 to set a predefined alphablending setting ", r, DrawTextFormat.Left Or DrawTextFormat.WordBreak, Color.White)

        r = New Rectangle(0, 50, 640, 480)
        'show setting
        With device.RenderState
            text1.DrawText(Nothing, "Source Blending: " & .SourceBlend.ToString & Chr(13) & _
            "  Blend Operation: " & .BlendOperation.ToString & Chr(13) & _
            "  Destination blending: " & .DestinationBlend.ToString _
            , r, DrawTextFormat.Left Or DrawTextFormat.WordBreak, Color.White)
        End With

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

            'change value for source blending
            Select Case SValue
                Case 0
                    device.RenderState.SourceBlend = Blend.Zero
                Case 1
                    device.RenderState.SourceBlend = Blend.One
                Case 2
                    device.RenderState.SourceBlend = Blend.InvBlendFactor
                Case 3
                    device.RenderState.SourceBlend = Blend.BlendFactor
                Case 4
                    device.RenderState.SourceBlend = Blend.BothInvSourceAlpha
                Case 5
                    device.RenderState.SourceBlend = Blend.BothSourceAlpha
                Case 6
                    device.RenderState.SourceBlend = Blend.SourceAlphaSat
                Case 7
                    device.RenderState.SourceBlend = Blend.InvDestinationColor
                Case 8
                    device.RenderState.SourceBlend = Blend.DestinationColor
                Case 9
                    device.RenderState.SourceBlend = Blend.InvDestinationAlpha
                Case 10
                    device.RenderState.SourceBlend = Blend.DestinationAlpha
                Case 11
                    device.RenderState.SourceBlend = Blend.InvSourceAlpha
                Case 12
                    device.RenderState.SourceBlend = Blend.SourceAlpha
                Case 13
                    device.RenderState.SourceBlend = Blend.InvSourceColor
                Case 14
                    device.RenderState.SourceBlend = Blend.SourceColor
            End Select


            'change value for destionation blending
            Select Case DValue
                Case 0
                    device.RenderState.DestinationBlend = Blend.Zero
                Case 1
                    device.RenderState.DestinationBlend = Blend.One
                Case 2
                    device.RenderState.DestinationBlend = Blend.InvBlendFactor
                Case 3
                    device.RenderState.DestinationBlend = Blend.BlendFactor
                Case 4
                    device.RenderState.DestinationBlend = Blend.BothInvSourceAlpha
                Case 5
                    device.RenderState.DestinationBlend = Blend.BothSourceAlpha
                Case 6
                    device.RenderState.DestinationBlend = Blend.SourceAlphaSat
                Case 7
                    device.RenderState.DestinationBlend = Blend.InvDestinationColor
                Case 8
                    device.RenderState.DestinationBlend = Blend.DestinationColor
                Case 9
                    device.RenderState.DestinationBlend = Blend.InvDestinationAlpha
                Case 10
                    device.RenderState.DestinationBlend = Blend.DestinationAlpha
                Case 11
                    device.RenderState.DestinationBlend = Blend.InvSourceAlpha
                Case 12
                    device.RenderState.DestinationBlend = Blend.SourceAlpha
                Case 13
                    device.RenderState.DestinationBlend = Blend.InvSourceColor
                Case 14
                    device.RenderState.DestinationBlend = Blend.SourceColor
            End Select

            'change value for blending operation
            Select Case BOper
                Case 0
                    device.RenderState.BlendOperation = BlendOperation.Add
                Case 1
                    device.RenderState.BlendOperation = BlendOperation.Max
                Case 2
                    device.RenderState.BlendOperation = BlendOperation.Min
                Case 3
                    device.RenderState.BlendOperation = BlendOperation.RevSubtract
                Case 4
                    device.RenderState.BlendOperation = BlendOperation.Subtract
            End Select
        End If
    End Sub

    Private Sub Form1_KeyDown(ByVal sender As Object, ByVal e As System.Windows.Forms.KeyEventArgs) Handles MyBase.KeyDown

        'change value
        Select Case e.KeyCode
            Case Keys.NumPad7
                SValue += 1
            Case Keys.NumPad1
                SValue -= 1
            Case Keys.NumPad9
                DValue += 1
            Case Keys.NumPad3
                DValue -= 1
            Case Keys.NumPad8
                BOper += 1
            Case Keys.NumPad2
                BOper -= 1


        End Select


        'change value for source blending
        If SValue < 0 Then SValue = 0
        If SValue > 14 Then SValue = 14
        Select Case SValue
            Case 0
                device.RenderState.SourceBlend = Blend.Zero
            Case 1
                device.RenderState.SourceBlend = Blend.One
            Case 2
                device.RenderState.SourceBlend = Blend.InvBlendFactor
            Case 3
                device.RenderState.SourceBlend = Blend.BlendFactor
            Case 4
                device.RenderState.SourceBlend = Blend.BothInvSourceAlpha
            Case 5
                device.RenderState.SourceBlend = Blend.BothSourceAlpha
            Case 6
                device.RenderState.SourceBlend = Blend.SourceAlphaSat
            Case 7
                device.RenderState.SourceBlend = Blend.InvDestinationColor
            Case 8
                device.RenderState.SourceBlend = Blend.DestinationColor
            Case 9
                device.RenderState.SourceBlend = Blend.InvDestinationAlpha
            Case 10
                device.RenderState.SourceBlend = Blend.DestinationAlpha
            Case 11
                device.RenderState.SourceBlend = Blend.InvSourceAlpha
            Case 12
                device.RenderState.SourceBlend = Blend.SourceAlpha
            Case 13
                device.RenderState.SourceBlend = Blend.InvSourceColor
            Case 14
                device.RenderState.SourceBlend = Blend.SourceColor
        End Select


        'change value for destionation blending
        If DValue < 0 Then DValue = 0
        If DValue > 14 Then DValue = 14
        Select Case DValue
            Case 0
                device.RenderState.DestinationBlend = Blend.Zero
            Case 1
                device.RenderState.DestinationBlend = Blend.One
            Case 2
                device.RenderState.DestinationBlend = Blend.InvBlendFactor
            Case 3
                device.RenderState.DestinationBlend = Blend.BlendFactor
            Case 4
                device.RenderState.DestinationBlend = Blend.BothInvSourceAlpha
            Case 5
                device.RenderState.DestinationBlend = Blend.BothSourceAlpha
            Case 6
                device.RenderState.DestinationBlend = Blend.SourceAlphaSat
            Case 7
                device.RenderState.DestinationBlend = Blend.InvDestinationColor
            Case 8
                device.RenderState.DestinationBlend = Blend.DestinationColor
            Case 9
                device.RenderState.DestinationBlend = Blend.InvDestinationAlpha
            Case 10
                device.RenderState.DestinationBlend = Blend.DestinationAlpha
            Case 11
                device.RenderState.DestinationBlend = Blend.InvSourceAlpha
            Case 12
                device.RenderState.DestinationBlend = Blend.SourceAlpha
            Case 13
                device.RenderState.DestinationBlend = Blend.InvSourceColor
            Case 14
                device.RenderState.DestinationBlend = Blend.SourceColor
        End Select

        'change value for blending operation
        If BOper < 0 Then BOper = 0
        If BOper > 4 Then BOper = 4
        Select Case BOper
            Case 0
                device.RenderState.BlendOperation = BlendOperation.Add
            Case 1
                device.RenderState.BlendOperation = BlendOperation.Max
            Case 2
                device.RenderState.BlendOperation = BlendOperation.Min
            Case 3
                device.RenderState.BlendOperation = BlendOperation.RevSubtract
            Case 4
                device.RenderState.BlendOperation = BlendOperation.Subtract
        End Select

        'some predefined setting
        Select Case e.KeyCode
            Case Keys.D1
                device.RenderState.SourceBlend = Blend.SourceColor
                device.RenderState.DestinationBlend = Blend.DestinationColor
                device.RenderState.BlendOperation = BlendOperation.Add
            Case Keys.D2
                device.RenderState.SourceBlend = Blend.One
                device.RenderState.DestinationBlend = Blend.One
                device.RenderState.BlendOperation = BlendOperation.Add
            Case Keys.D3
                device.RenderState.SourceBlend = Blend.InvDestinationColor
                device.RenderState.DestinationBlend = Blend.InvSourceColor
                device.RenderState.BlendOperation = BlendOperation.Add
            Case Keys.D4
                device.RenderState.SourceBlend = Blend.InvDestinationColor
                device.RenderState.DestinationBlend = Blend.InvSourceColor
                device.RenderState.BlendOperation = BlendOperation.Subtract
            Case Keys.D5
                device.RenderState.SourceBlend = Blend.DestinationColor
                device.RenderState.DestinationBlend = Blend.SourceAlpha
                device.RenderState.BlendOperation = BlendOperation.Add
            Case Keys.D6
                device.RenderState.SourceBlend = Blend.DestinationColor
                device.RenderState.DestinationBlend = Blend.InvDestinationColor
                device.RenderState.BlendOperation = BlendOperation.Add
        End Select
    End Sub


End Class
