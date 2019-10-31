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
        Me.Text = "Point Sprite"

    End Sub

#End Region


    Public text1 As Font 'directX text

    Dim vertexList As New ArrayList   'array list per la gestione di oggetti particella
    Const maxP = 10000 'max particle number
    Dim vertices As CustomVertex.PositionColored() 'colored vertex type for particle

    Dim numIns As Integer = 5 'number of particle that 'll added to particle array every cicle
    Dim tex As Texture 'particle texture

    Dim lastTime As Integer 'to syncronize particle generation to timer


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

        ReDim vertices(0) 'impostare ab inizio la dimensione a zero

        'particle texture
        tex = createTexture(Application.StartupPath & "\particle.bmp", Color.Black.ToArgb)

        'particle system, setting for pointsprite
        device.RenderState.PointSpriteEnable = True
        device.RenderState.PointScaleEnable = True
        device.RenderState.PointSize = 1.0F
        device.RenderState.PointSizeMin = 0.0F
        device.RenderState.PointScaleA = 0.0F
        device.RenderState.PointScaleB = 0.0F
        device.RenderState.PointScaleC = 1.0F



        'alphablending
        device.RenderState.AlphaBlendEnable = True
        device.RenderState.SourceBlend = Blend.SourceAlpha
        device.RenderState.DestinationBlend = Blend.DestinationAlpha

    End Sub



    Private Sub Form1_Closed(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Closed
        text1.Dispose()
        device.Dispose()
        sprite.Dispose()
        'end application
        End
    End Sub

    Private Sub Form1_Paint(ByVal sender As Object, ByVal e As System.Windows.Forms.PaintEventArgs) Handles MyBase.Paint

        If Environment.TickCount - lastTime > 20 Then
            manageParticle()
            lastTime = Environment.TickCount
        End If
        device.Clear(ClearFlags.Target Or ClearFlags.ZBuffer, Color.Blue, 1, 0) 'clear the screen
        device.BeginScene() 'begin 3D scene

        'draw vertex array
        device.SetTexture(0, tex)
        device.VertexFormat = CustomVertex.PositionColored.Format
        moveObj(1, 1, 1, 0, 0, 0, 0, -7, 7)
        device.DrawUserPrimitives(PrimitiveType.PointList, vertices.Length - 1, vertices)



        Dim r As Rectangle
        r = New Rectangle(0, 0, 640, 480)

        'draw text in a rectangle zone
        text1.DrawText(Nothing, "Press up and down to change particle frequency. Particle Number: " & vertices.Length, r, DrawTextFormat.Left, Color.White)


        device.EndScene() 'end 3D scene
        device.Present() 'send graphics to screen
        Me.Invalidate()
    End Sub

    Sub manageParticle()
        'manage particle creation and dynamics
        Dim j As Integer
        For j = 0 To numIns
            'if vertex number is less than maximum number add particle
            If vertexList.Count < maxP Then
                Dim p As New particleSystem(New Vector3(0, -10, 0), Int(Rnd() * 20) + 80, Rnd() + 0.5, 100 * Rnd())
                vertexList.Add(p)
            End If
        Next



        'run particle engine
        Dim partic As particleSystem
        For Each partic In vertexList
            With partic
                .action()
            End With
        Next

        'delete dead particle
        Dim runWhile As Boolean = True
        While runWhile
            runWhile = False
            For Each partic In vertexList
                If partic.life <= 0 Then
                    vertexList.Remove(partic)
                    runWhile = True
                    Exit For
                End If
            Next
        End While



        'create particle array
        ReDim vertices(vertexList.Count)
        Dim i As Integer
        For Each partic In vertexList
            With partic
                vertices(i).Position = New Vector3(.position.X, .position.Y, .position.Z)
                vertices(i).Color = .colorV
            End With
            i += 1
        Next
    End Sub


    Private Sub Form1_KeyDown(ByVal sender As Object, ByVal e As System.Windows.Forms.KeyEventArgs) Handles MyBase.KeyDown
        Select Case e.KeyCode
            Case Keys.Up
                numIns -= 1
                If numIns < 0 Then numIns = 0
            Case Keys.Down
                numIns += 1
            Case Keys.Escape
                End
        End Select
    End Sub



    Private Sub Form1_Resize(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Resize

        If Not (device Is Nothing Or Me.WindowState = FormWindowState.Minimized) Then
            'device must be reset
            resetDevice()
            'all setting must be updated
            defaultSetting()

            'particle system, setting for pointsprite
            device.RenderState.PointSpriteEnable = True
            device.RenderState.PointScaleEnable = True
            device.RenderState.PointSize = 1.0F
            device.RenderState.PointSizeMin = 0.0F
            device.RenderState.PointScaleA = 0.0F
            device.RenderState.PointScaleB = 0.0F
            device.RenderState.PointScaleC = 1.0F



            'alphablending
            device.RenderState.AlphaBlendEnable = True
            device.RenderState.SourceBlend = Blend.SourceAlpha
            device.RenderState.DestinationBlend = Blend.DestinationAlpha


        End If
    End Sub





End Class

'particle object
Public Class particleSystem
    Public position As Vector3 'position of the particle
    Public direction As Integer 'direction
    Public speed As Single 'speed
    Public life As Integer 'when life go to 0 particle'll be removed from list
    Private t As Single 'tick used in particle calculation
    Public colorV As Integer 'color of particle in int value

    Private red As Byte 'color of particle
    Private green As Byte 'color of particle
    Private blue As Byte 'color of particle

    Public Sub New(ByVal pos As Vector3, ByVal dir As Integer, ByVal initialSpeed As Single, ByVal initialLife As Integer)
        'initial value of the particle
        position = pos
        direction = dir
        speed = initialSpeed
        life = initialLife

        red = Rnd() * 255
        green = Rnd() * 255
        blue = Rnd() * 255

    End Sub

    Public Sub action()
        'move particle and reduce particle life
        t += 0.1
        life -= 1
        position = Vector3.Add(position, New Vector3(Math.Cos(direction * (Math.PI / 180)) * speed, Math.Sin(direction * (Math.PI / 180)) * speed - 0.3 * t, 0))
        colorV = Color.FromArgb(red, green, blue, Math.Abs(life * 2 + 50)).ToArgb
    End Sub

End Class