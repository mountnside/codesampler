'=======================
' ROBYDX demo for CodeSampler.com website
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
        Me.Text = "Texture"

    End Sub

#End Region

    Dim vertices(5) As CustomVertex.PositionNormalTextured 'untransformed
    Dim angle As Single 'rotation angle
    Dim texture As Texture

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
        'create transformed primitive
        vertices(0).X = -5
        vertices(0).Y = -5
        vertices(0).Tu = 1
        vertices(0).Tv = 0

        vertices(1).X = 5
        vertices(1).Y = 5
        vertices(1).Tu = 0
        vertices(1).Tv = 1

        vertices(2).X = 5
        vertices(2).Y = -5
        vertices(2).Tu = 0
        vertices(2).Tv = 0

        vertices(3).X = -5
        vertices(3).Y = -5
        vertices(3).Tu = 1
        vertices(3).Tv = 0

        vertices(4).X = -5
        vertices(4).Y = 5
        vertices(4).Tu = 1
        vertices(4).Tv = 1

        vertices(5).X = 5
        vertices(5).Y = 5
        vertices(5).Tu = 0
        vertices(5).Tv = 1

        texture = createTexture(Application.StartupPath + "\texture0.bmp", Color.White.ToArgb)  'nella posizione dell'eseguibile


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
        tempMatrix = Matrix.Multiply(Matrix.RotationZ(-angle), Matrix.Translation(-8, -4, 0))
        device.Transform.World = tempMatrix
        'set texture
        device.SetTexture(0, texture)
        device.VertexFormat = CustomVertex.PositionNormalTextured.Format
        'draw primitives
        device.DrawUserPrimitives(PrimitiveType.TriangleList, 2, vertices)


        'draw primitives without texture
        tempMatrix = Matrix.Multiply(Matrix.RotationZ(-angle), Matrix.Translation(8, 4, 0))
        device.Transform.World = tempMatrix
        'remove texture
        device.SetTexture(0, Nothing)
        device.VertexFormat = CustomVertex.PositionNormalTextured.Format
        'draw primitives
        device.DrawUserPrimitives(PrimitiveType.TriangleList, 2, vertices)

       

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
