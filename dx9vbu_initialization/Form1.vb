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
    Public bkColor As Color = Color.Blue
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
        Me.Text = "Initialization"

    End Sub

#End Region

    Private Sub Form1_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load

        MsgBox("DirectX initialization sample - use the number keys (0-9) to change the clear color.", MsgBoxStyle.Information, "RobyDx")

        'this function create DirectX for full-screen of windowed mode
        'with true you'll use windowed mode
        createDevice(0, 0, 32, Me.Handle, True)
        
        'this instruction avoid artifact
        Me.SetStyle(ControlStyles.AllPaintingInWmPaint Or ControlStyles.Opaque, True)
        Me.Invalidate() 'invalidate rendering area
    End Sub



    Private Sub Form1_KeyDown(ByVal sender As Object, ByVal e As System.Windows.Forms.KeyEventArgs) Handles MyBase.KeyDown
        Select Case e.KeyCode
            Case Keys.D0
                bkColor = Color.Black
            Case Keys.D1
                bkColor = Color.Aqua
            Case Keys.D2
                bkColor = Color.Red
            Case Keys.D3
                bkColor = Color.Green
            Case Keys.D4
                bkColor = Color.DarkOrange
            Case Keys.D5
                bkColor = Color.MistyRose
            Case Keys.D6
                bkColor = Color.Purple
            Case Keys.D7
                bkColor = Color.Blue
            Case Keys.D8
                bkColor = Color.White
            Case Keys.D9
                bkColor = Color.Yellow
        End Select
    End Sub

    Private Sub Form1_Closed(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Closed
        'end application
        End
    End Sub

    Private Sub Form1_Paint(ByVal sender As Object, ByVal e As System.Windows.Forms.PaintEventArgs) Handles MyBase.Paint

        device.Clear(ClearFlags.Target, bkColor, 1, 0) 'clear the screen
        device.BeginScene() 'begin 3D scene

        'all graphics rendering goes here

        device.EndScene() 'end 3D scene
        device.Present() 'send graphics to screen
        Me.Invalidate()
    End Sub

  

    Private Sub Form1_Resize(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Resize

        If Not (device Is Nothing Or Me.WindowState = FormWindowState.Minimized) Then
            'device must be reset
            resetDevice()
        End If
    End Sub


End Class
