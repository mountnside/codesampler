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
        Me.Text = "DirectX Font"

    End Sub

#End Region


    Public text1 As Font 'directX text
    Public text2 As Font 'directX text
    Public text3 As Font 'directX text

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
        text2 = createFont(New Drawing.Font("Times new roman", 24, FontStyle.Italic Or FontStyle.Underline))
        text3 = createFont(New Drawing.Font("Arial", 24, FontStyle.Bold))

    End Sub



    Private Sub Form1_Closed(ByVal sender As Object, ByVal e As System.EventArgs) Handles MyBase.Closed
        text1.Dispose()
        text2.Dispose()
        text3.Dispose()
        'end application
        End
    End Sub

    Private Sub Form1_Paint(ByVal sender As Object, ByVal e As System.Windows.Forms.PaintEventArgs) Handles MyBase.Paint

       

        device.Clear(ClearFlags.Target Or ClearFlags.ZBuffer, Color.Blue, 1, 0) 'clear the screen
        device.BeginScene() 'begin 3D scene


        Dim r As Rectangle
        r = New Rectangle(0, 0, 640, 480)

        'draw text in a rectangle zone
        text1.DrawText(Nothing, "Normal text ", r, DrawTextFormat.Left, Color.White)

        'draw text in a rectangle zone
        r = New Rectangle(0, 100, 640, 480)
        text2.DrawText(Nothing, "Italic text", r, DrawTextFormat.Left, Color.Aqua)

        'draw text in a rectangle zone
        r = New Rectangle(0, 300, 640, 480)
        text3.DrawText(Nothing, "Bold text", r, DrawTextFormat.Left, Color.Red)

      
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
