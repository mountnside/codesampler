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
        Me.Text = "Primitives                             press from 1 to 6 "

    End Sub

#End Region

    Dim point(11) As CustomVertex.TransformedColored 'transformed primitives
    Dim lineList(7) As CustomVertex.TransformedColored 'transformed primitives
    Dim lineStrip(4) As CustomVertex.TransformedColored 'transformed primitives
    Dim triangleList(5) As CustomVertex.TransformedColored 'transformed primitives
    Dim triangleStrip(3) As CustomVertex.TransformedColored 'transformed primitives
    Dim triangleFan(5) As CustomVertex.TransformedColored 'transformed primitives

    Dim primitiveSelected As Integer = 6

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
        '12 points 
        point(0) = New CustomVertex.TransformedColored(100, 50, 0, 1, Color.White.ToArgb())
        point(1) = New CustomVertex.TransformedColored(200, 50, 0, 1, Color.White.ToArgb())
        point(2) = New CustomVertex.TransformedColored(300, 50, 0, 1, Color.White.ToArgb())
        point(3) = New CustomVertex.TransformedColored(400, 50, 0, 1, Color.White.ToArgb())
        point(4) = New CustomVertex.TransformedColored(400, 150, 0, 1, Color.White.ToArgb())
        point(5) = New CustomVertex.TransformedColored(400, 250, 0, 1, Color.White.ToArgb())
        point(6) = New CustomVertex.TransformedColored(400, 350, 0, 1, Color.White.ToArgb())
        point(7) = New CustomVertex.TransformedColored(300, 350, 0, 1, Color.White.ToArgb())
        point(8) = New CustomVertex.TransformedColored(200, 350, 0, 1, Color.White.ToArgb())
        point(9) = New CustomVertex.TransformedColored(100, 350, 0, 1, Color.White.ToArgb())
        point(10) = New CustomVertex.TransformedColored(100, 250, 0, 1, Color.White.ToArgb())
        point(11) = New CustomVertex.TransformedColored(100, 150, 0, 1, Color.White.ToArgb())

        '4 lines 
        lineList(0) = New CustomVertex.TransformedColored(100, 100, 0, 1, Color.Yellow.ToArgb())
        lineList(1) = New CustomVertex.TransformedColored(400, 400, 0, 1, Color.White.ToArgb())
        lineList(2) = New CustomVertex.TransformedColored(100, 400, 0, 1, Color.Yellow.ToArgb())
        lineList(3) = New CustomVertex.TransformedColored(400, 100, 0, 1, Color.Azure.ToArgb())
        lineList(4) = New CustomVertex.TransformedColored(50, 100, 0, 1, Color.Green.ToArgb())
        lineList(5) = New CustomVertex.TransformedColored(50, 400, 0, 1, Color.White.ToArgb())
        lineList(6) = New CustomVertex.TransformedColored(450, 100, 0, 1, Color.Yellow.ToArgb())
        lineList(7) = New CustomVertex.TransformedColored(450, 400, 0, 1, Color.Green.ToArgb())

        '4 lines
        lineStrip(0) = New CustomVertex.TransformedColored(100, 100, 0, 1, Color.Red.ToArgb())
        lineStrip(1) = New CustomVertex.TransformedColored(500, 100, 0, 1, Color.White.ToArgb())
        lineStrip(2) = New CustomVertex.TransformedColored(500, 400, 0, 1, Color.Azure.ToArgb())
        lineStrip(3) = New CustomVertex.TransformedColored(100, 400, 0, 1, Color.Red.ToArgb())
        lineStrip(4) = New CustomVertex.TransformedColored(100, 100, 0, 1, Color.Green.ToArgb())

        '2 triangles
        triangleList(0) = New CustomVertex.TransformedColored(100, 100, 0, 1, Color.Red.ToArgb())
        triangleList(1) = New CustomVertex.TransformedColored(250, 300, 0, 1, Color.Azure.ToArgb())
        triangleList(2) = New CustomVertex.TransformedColored(100, 300, 0, 1, Color.Green.ToArgb())
        triangleList(3) = New CustomVertex.TransformedColored(400, 300, 0, 1, Color.Red.ToArgb())
        triangleList(4) = New CustomVertex.TransformedColored(200, 100, 0, 1, Color.Yellow.ToArgb())
        triangleList(5) = New CustomVertex.TransformedColored(400, 100, 0, 1, Color.Green.ToArgb())

        '2 triangles
        triangleStrip(0) = New CustomVertex.TransformedColored(100, 100, 0, 1, Color.Red.ToArgb())
        triangleStrip(1) = New CustomVertex.TransformedColored(400, 100, 0, 1, Color.Azure.ToArgb())
        triangleStrip(2) = New CustomVertex.TransformedColored(100, 400, 0, 1, Color.Green.ToArgb())
        triangleStrip(3) = New CustomVertex.TransformedColored(400, 400, 0, 1, Color.Yellow.ToArgb())

        '4 triangle
        triangleFan(0) = New CustomVertex.TransformedColored(300, 400, 0, 1, Color.White.ToArgb())
        triangleFan(1) = New CustomVertex.TransformedColored(100, 100, 0, 1, Color.Red.ToArgb())
        triangleFan(2) = New CustomVertex.TransformedColored(200, 150, 0, 1, Color.Azure.ToArgb())
        triangleFan(3) = New CustomVertex.TransformedColored(300, 100, 0, 1, Color.Green.ToArgb())
        triangleFan(4) = New CustomVertex.TransformedColored(400, 150, 0, 1, Color.Yellow.ToArgb())
        triangleFan(5) = New CustomVertex.TransformedColored(500, 100, 0, 1, Color.Blue.ToArgb())

    End Sub

    Function CreateVertex(ByVal x As Single, ByVal y As Single, ByVal z As Single, ByVal rhw As Single, ByVal colorValue As Integer) As CustomVertex.TransformedColored
        Return New CustomVertex.TransformedColored(x, y, z, rhw, ColorValue)
    End Function

    Private Sub Form1_Closed(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Closed
        'end application
        End
    End Sub

    Private Sub Form1_Paint(ByVal sender As Object, ByVal e As System.Windows.Forms.PaintEventArgs) Handles MyBase.Paint

        'transformation matrix
        Dim tempMatrix As Matrix
        'syncronize rotation angle with timer

        device.Clear(ClearFlags.Target Or ClearFlags.ZBuffer, Color.Black, 1, 0) 'clear the screen
        device.BeginScene() 'begin 3D scene

        device.VertexFormat = CustomVertex.TransformedColored.Format

        Select Case primitiveSelected
            Case 1
                'draw pointlist primitive, 12 points
                device.DrawUserPrimitives(PrimitiveType.PointList, 12, point)
            Case 2
                'draw linelist primitive, 4 line
                device.DrawUserPrimitives(PrimitiveType.LineList, 4, lineList)
            Case 3
                'draw linestrip primitive, 4 line
                device.DrawUserPrimitives(PrimitiveType.LineStrip, 4, lineStrip)
            Case 4
                'draw trianglelist primitive, 2 triangle
                device.DrawUserPrimitives(PrimitiveType.TriangleList, 2, triangleList)
            Case 5
                'draw trianglestrip primitive, 2 triangle
                device.DrawUserPrimitives(PrimitiveType.TriangleStrip, 2, triangleStrip)
            Case 6
                'draw trianglefan primitive, 4 triangle
                device.DrawUserPrimitives(PrimitiveType.TriangleFan, 4, triangleFan)
        End Select


        device.EndScene() 'end 3D scene
        device.Present() 'send graphics to screen
        Me.Invalidate()
    End Sub



    Private Sub Form1_Resize(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Resize

        If Not (device Is Nothing Or Me.WindowState = FormWindowState.Minimized) Then
            'device must be reset
            resetDevice()
            'setting again the device
            defaultSetting()
        End If
    End Sub


    Private Sub Form1_KeyDown(ByVal sender As Object, ByVal e As System.Windows.Forms.KeyEventArgs) Handles MyBase.KeyDown
        Select Case e.KeyCode
            Case Keys.D1
                primitiveSelected = 1
            Case Keys.D2
                primitiveSelected = 2
            Case Keys.D3
                primitiveSelected = 3
            Case Keys.D4
                primitiveSelected = 4
            Case Keys.D5
                primitiveSelected = 5
            Case Keys.D6
                primitiveSelected = 6
        End Select
    End Sub
End Class
