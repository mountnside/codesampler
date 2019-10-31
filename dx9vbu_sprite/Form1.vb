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
        Me.Text = "Sprite rendering"

    End Sub

#End Region


    Dim tex1 As Texture 'texture used for sprite
    Dim tex2 As Texture 'texture used for sprite
    

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

        
        'create 2 texture for sprite rendering
        tex1 = createTexture(Application.StartupPath & "\texture0.bmp")
        'set purple as transparent color
        tex2 = createTexture(Application.StartupPath & "\mouse.bmp", Color.FromArgb(255, 255, 0, 255).ToArgb) 
              

    End Sub



    Private Sub Form1_Closed(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Closed
        sprite.Dispose()
        device.Dispose()
        'end application
        End
    End Sub

    Private Sub Form1_Paint(ByVal sender As Object, ByVal e As System.Windows.Forms.PaintEventArgs) Handles MyBase.Paint

        device.Clear(ClearFlags.Target Or ClearFlags.ZBuffer, Color.Blue, 1, 0) 'clear the screen
        device.BeginScene() 'begin 3D scene


        'for setting sprite and text
        Dim r As Rectangle
        Dim angle As Single = Environment.TickCount / 1000.0F

        'begin sprite rendering
        sprite.Begin(SpriteFlags.AlphaBlend)

        'define zone of the texture that we want to display
        r = New Rectangle(0, 0, 128, 128)
        'draw a sprite setting transformation matrix
        sprite.Transform = Matrix.Transformation2D(New Vector2(0, 0), 0, New Vector2(1, 1), New Vector2(64, 64), angle, New Vector2(260, 100))
        sprite.Draw(tex1, r, New Vector3(0, 0, 0), New Vector3(0, 0, 0), Color.White)        'colore semitrasparente

        'setting texture zone for animated sprite
        Select Case Int((Environment.TickCount / 200) Mod 4)
            Case 0
                r = New Rectangle(0, 0, 64, 64)
            Case 1
                r = New Rectangle(67, 0, 64, 64)
            Case 2
                r = New Rectangle(135, 0, 64, 64)
            Case 3
                r = New Rectangle(67, 0, 64, 64)
        End Select

        'draw 7 animated sprite
        Dim i As Integer
        For i = 0 To 6
            sprite.Transform = Matrix.Transformation2D(New Vector2(1, 1), 0, New Vector2(1, 1), New Vector2(0, 0), 0, New Vector2(50 + i * 80, 300))
            sprite.Draw(tex2, r, New Vector3(0, 0, 0), New Vector3(0, 0, 0), Color.White)
        Next
        'end sprite rendering
        sprite.End()


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

        End If
    End Sub

End Class
