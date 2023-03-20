using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

using GMap.NET.MapProviders;
using GMap.NET;
using GMap.NET.WindowsForms;
using GMap.NET.WindowsForms.Markers;

using OpenTK;
using OpenTK.Graphics.OpenGL;
using System.Data.Entity.Core.Objects;
using System.IO;
  
using StbImageSharp;
/*
SERIAL'A BASILAN      =====>  "packageNum,explode1,explode2,pressure,X_Jiro,Y_Jiro,Z_Jiro,X_Ivme,Y_Ivme ,Z_Ivme,GPSe ,GPSb"
BYTE BOYUTU (39Byte)  =====>     1BYTE   ,  1BYTE ,  1BYTE , 4BYTE  ,4BYTE ,4BYTE ,4BYTE ,4BYTE , 4BYTE ,4BYTE ,4BYTE,4BYTE  
*/

namespace TOBBETUROCKETRY
{
    enum PackageElements
    {
        packageNum,
        explode1,
        explode2,
        pressure,
        X_jiro,
        Y_jiro,
        Z_jiro,
        X_ivme,
        Y_ivme,
        Z_ivme,
        GPSe,
        GPSb
    }
    public partial class TOBBETUROCKETRY : Form
    {
        static readonly int DATA_COUNT = 12;

        SerialPort serialPort = new SerialPort();

        public string filenameRoket = "RoketValues.csv";
        public string filenameFaydaliYuk = "FaydaliYukValues.csv";
        //https://www.google.com/maps?q=39.9272,32.8644
        private string recivedData = "0,0,0,0,0,0,0,0,0,0,39.9103241,32.8529681";
        private string[] values = new string[DATA_COUNT];



        public TOBBETUROCKETRY()
        {
            InitializeComponent();
            FetchAvailablePorts();
            values = recivedData.Split(',');
            btnBaglantiyiBitir.Enabled = false;
        }
        void FetchAvailablePorts()
        {
            numericUpDownBaudRate.Value = 9600;
            comboBoxComPort.Items.Clear();
            string[] ports = SerialPort.GetPortNames();
            comboBoxComPort.Items.AddRange(ports);
            if (comboBoxComPort.Items.Count > 0) comboBoxComPort.SelectedIndex = comboBoxComPort.Items.Count - 1;
            else comboBoxComPort.Text = "Takılı Cihaz Yok";
            lblDurum.ForeColor = Color.Red;
            lblDurum.Text = "Bağlantı Yok!";
        }

        #region BAĞLANTI BAŞLATMA / BİTİRME
        private void btnBaglan_Click(object sender, EventArgs e)
        {

            serialPort = new SerialPort();
            if (comboBoxComPort.SelectedItem != null)
            {
                serialPort.PortName = comboBoxComPort.SelectedItem.ToString();
                btnBaglan.Enabled = false;
                btnBaglantiyiBitir.Enabled = true;
            }
            else
            {
                FetchAvailablePorts();
                MessageBox.Show("COM PORT'u Yeniden Seçiniz ");
                btnBaglan.Enabled = true;
                btnBaglantiyiBitir.Enabled = false;
            }
            serialPort.BaudRate = (int)numericUpDownBaudRate.Value;
            serialPort.Parity = Parity.None;
            serialPort.DataBits = 8;
            serialPort.StopBits = StopBits.One;
            serialPort.ReadBufferSize = 200000000;
            serialPort.DataReceived += serialPort_DataReceived;

            try
            {
                serialPort.Open();
                Thread.Sleep(1000);
                lblDurum.ForeColor = Color.Green;
                lblDurum.Text = "Bağlandı!";
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error occured while opening port");
                FetchAvailablePorts();
                btnBaglan.Enabled = true;
                btnBaglantiyiBitir.Enabled = false;
            }
        }
        private void serialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                if (serialPort.IsOpen)
                {
                    recivedData = serialPort.ReadLine();
                    this.Invoke(new Action(ProcessingData));
                    this.Invoke(new Action(ValueUpdate));
                }
            }
            catch (Exception) { }

        }
        private void ProcessingData()
        {
            try { values = recivedData.Split(','); }
            catch (Exception) { for (int i = 0; i < DATA_COUNT; i++) values[i] = "0"; }
        }
        private void ValueUpdate()
        {
            //burası her data geldiğinde çalışır
            Thread.Sleep(100); //will sleep for 100ms
            //TODO: burda degerleri guncelle 
            textBoxPaketNum_AnaBil.Text = GetPinValue(PackageElements.packageNum);
            textBoxBasinc_AnaBil.Text = GetPinValue(PackageElements.pressure);
            textBoxGPSEnlem_AnaBil.Text = GetPinValue(PackageElements.GPSe);
            textBoxGPSBoylam_AnaBil.Text = GetPinValue(PackageElements.GPSb);
            textBoxJiroX_AnaBil.Text = GetPinValue(PackageElements.X_jiro);
            textBoxJiroY_AnaBil.Text = GetPinValue(PackageElements.Y_jiro);
            textBoxJiroZ_AnaBil.Text = GetPinValue(PackageElements.Z_jiro);
            textBoxIvmeX_AnaBil.Text = GetPinValue(PackageElements.X_ivme);
            textBoxIvmeY_AnaBil.Text = GetPinValue(PackageElements.Y_ivme);
            textBoxIvmeZ_AnaBil.Text = GetPinValue(PackageElements.Z_ivme);

            x_angle_3d_model = GetPinValueFloat(PackageElements.X_jiro);
            y_angle_3d_model = GetPinValueFloat(PackageElements.Y_jiro) + 90;
            z_angle_3d_model = GetPinValueFloat(PackageElements.Z_jiro);

            try
            {
                glControl1.Invalidate();
                RefreshMapToNewGPS();
            }
            catch (Exception ex) { MessageBox.Show(ex.Message.ToString(), "Error"); }

        }
        private void btnBaglantiyiBitir_Click(object sender, EventArgs e)
        {
            if (serialPort.IsOpen)
            {
                serialPort.Close();
                btnBaglan.Enabled = true;
                btnBaglantiyiBitir.Enabled = false;
                lblDurum.ForeColor = Color.Red;
                lblDurum.Text = "Bağlantı Yok!";
            }
            else
            {
                btnBaglan.Enabled = true;
                btnBaglantiyiBitir.Enabled = false;
                lblDurum.Text = "Cihaz Bağlantısı Koptu!";
                lblDurum.ForeColor = Color.Red;
            }
        }
        #endregion


        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            try
            {
                if (serialPort.IsOpen)
                {
                    MessageBox.Show("COM PORT bağlantısını kapatmadınız! Otomatik sonlandırılıyor...");
                    serialPort.Close();
                }
            }
            catch (Exception ex) { MessageBox.Show("" + ex); }
        }
        private void Form1_Load(object sender, EventArgs e)
        {
            gMapAnaBilgisayar.DragButton = MouseButtons.Left;
            gMapAnaBilgisayar.MouseWheelZoomEnabled = true;
            gMapAnaBilgisayar.MapProvider = GMapProviders.GoogleMap;
            gMapAnaBilgisayar.Position = new PointLatLng(39.9272, 32.8644);
            gMapAnaBilgisayar.MinZoom = 1;
            gMapAnaBilgisayar.MaxZoom = 100;
            gMapAnaBilgisayar.Zoom = 16;

            gMapGorevYuku.DragButton = MouseButtons.Left;
            gMapGorevYuku.MouseWheelZoomEnabled = true;
            gMapGorevYuku.MapProvider = GMapProviders.GoogleMap;
            gMapGorevYuku.Position = new PointLatLng(39.9272, 32.8644);
            gMapGorevYuku.MinZoom = 1;
            gMapGorevYuku.MaxZoom = 100;
            gMapGorevYuku.Zoom = 16;

        }
        private void RefreshMapToNewGPS()
        {
            gMapAnaBilgisayar.Position = new PointLatLng(float.Parse(GetPinValue(PackageElements.GPSe).Replace(".", ",")), float.Parse(GetPinValue(PackageElements.GPSb).Replace(".", ",")));
            gMapAnaBilgisayar.MinZoom = 1;
            gMapAnaBilgisayar.MaxZoom = 100;
            gMapAnaBilgisayar.Zoom = 16;
            gMapGorevYuku.Position = new PointLatLng(GetPinValueFloat(PackageElements.GPSe), GetPinValueFloat(PackageElements.GPSb));
            gMapGorevYuku.MinZoom = 1;
            gMapGorevYuku.MaxZoom = 100;
            gMapGorevYuku.Zoom = 16;
        }
        private void btnRefresh_Click(object sender, EventArgs e)
        {
            FetchAvailablePorts();
            x_angle_3d_model += .5f;
            y_angle_3d_model += .5f;
            z_angle_3d_model += .5f;
            glControl1.Invalidate();
        }
        private string GetPinValue(PackageElements pe)
        {
            return values[(int)pe];
        }
        private float GetPinValueFloat(PackageElements pe)
        {
            return float.Parse(GetPinValue(pe).IndexOf(".") >= 0 ? GetPinValue(pe).Replace(".", ",") : GetPinValue(pe));
        }

        #region 3D NESNE OPENGL

        List<Vector3> vertices = new List<Vector3>();
        List<int> triangles = new List<int>();
        List<Vector3> normals = new List<Vector3>();
        List<Vector2> texCoords = new List<Vector2>();
        int VertexBufferObject, VertexArrayObject, ShaderObject, TextureObject;
        float x_angle_3d_model =0, y_angle_3d_model = 0, z_angle_3d_model = 0;

        private void glControl1_AutoSizeChanged(object sender, EventArgs e)
        {
            GL.Clear(ClearBufferMask.ColorBufferBit);
            GL.Clear(ClearBufferMask.DepthBufferBit);

            Matrix4 perspective = Matrix4.CreatePerspectiveFieldOfView(1.04f, 4 / 3, 1, 10000);
            Matrix4 lookat = Matrix4.LookAt(25, 0, 0, 0, 0, 0, 0, 1, 0);
            GL.MatrixMode(MatrixMode.Projection);
            GL.LoadIdentity();
            GL.LoadMatrix(ref perspective);
            GL.MatrixMode(MatrixMode.Modelview);
            GL.LoadIdentity();
            GL.LoadMatrix(ref lookat);
            GL.Viewport(0, 0, glControl1.Width, glControl1.Height);
        }

        private void glControl1_Paint(object sender, PaintEventArgs e)
        {
           
           GL.Clear(ClearBufferMask.ColorBufferBit);
           GL.Clear(ClearBufferMask.DepthBufferBit);

            GL.Begin(BeginMode.Lines);
                GL.Color3(Color.Blue);
                GL.Vertex3(-20.0, 0.0, 0.0);
                GL.Vertex3( 20.0, 0.0, 0.0);


                GL.Color3(Color.Green);
                GL.Vertex3(0.0,  20.0, 0.0);
                GL.Vertex3(0.0, -20.0, 0.0);

                GL.Color3(Color.Red);
                GL.Vertex3(0.0, 0.0,  20.0);
                GL.Vertex3(0.0, 0.0, -20.0);
            GL.End();

            GL.BindBuffer(BufferTarget.ArrayBuffer, VertexArrayObject);
            GL.BindVertexArray(VertexArrayObject);
            GL.UseProgram(ShaderObject);
            

            int modelAddress = GL.GetUniformLocation(ShaderObject, "model");
            int viewAddress = GL.GetUniformLocation(ShaderObject, "view");
            int projectionAddress = GL.GetUniformLocation(ShaderObject, "projection");

            Matrix4 rotX, rotY, rotZ;
            rotX = Matrix4.CreateRotationX(x_angle_3d_model);
            rotY = Matrix4.CreateRotationY(y_angle_3d_model);
            rotZ = Matrix4.CreateRotationZ(z_angle_3d_model);

            Matrix4 model = Matrix4.Identity * rotX * rotY * rotZ;

            Matrix4 view = Matrix4.LookAt(0, 0, 8, 0, 0, 0, 0, 1, 0);
            Matrix4 perspective = Matrix4.CreatePerspectiveFieldOfView(1.04f, 4 / 3, 1, 10000);

            GL.UniformMatrix4(modelAddress, true,ref model);
            GL.UniformMatrix4(viewAddress, true, ref view);
            GL.UniformMatrix4(projectionAddress, true, ref perspective);

            GL.ActiveTexture(TextureUnit.Texture0);
            GL.BindTexture(TextureTarget.Texture2D, TextureObject);

            int texLocation = GL.GetUniformLocation(TextureObject, "texture_diffuse1");

            GL.Uniform1(texLocation, 0);

            GL.DrawArrays(BeginMode.Triangles,0, triangles.Count / 3);

            //GraphicsContext.CurrentContext.VSync = true;

            glControl1.SwapBuffers();

        }
        private void glControl1_Load(object sender, EventArgs e)
        {
            GL.ClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            GL.Enable(EnableCap.DepthTest);//sonradan yazdık

            ModelLoadToBuffer("Patriot___MIM-104"); 
        }


        private void ModelLoadToBuffer(string filePath)
        {

            ObjImporter.LoadModel(filePath+".obj", out vertices, out triangles, out normals, out texCoords);
            System.Console.WriteLine("vertices : " + vertices.Count);
            System.Console.WriteLine("triangles : " + triangles.Count);
            System.Console.WriteLine("normals : " + normals.Count);
            System.Console.WriteLine("texCoords : " + texCoords.Count);


            List<float> vertexBuffer = new List<float>();

            for (int i = 0; i < triangles.Count; i += 3)
            {
                vertexBuffer.Add(vertices[triangles[i]].X);
                vertexBuffer.Add(vertices[triangles[i]].Y);
                vertexBuffer.Add(vertices[triangles[i]].Z);

                vertexBuffer.Add(texCoords[triangles[i + 1]].X);
                vertexBuffer.Add(texCoords[triangles[i + 1]].Y);

                vertexBuffer.Add(normals[triangles[i + 2]].X);
                vertexBuffer.Add(normals[triangles[i + 2]].Y);
                vertexBuffer.Add(normals[triangles[i + 2]].Z);
            }

            VertexBufferObject = GL.GenBuffer();
            GL.BindBuffer(BufferTarget.ArrayBuffer, VertexBufferObject);
            GL.BufferData(BufferTarget.ArrayBuffer, sizeof(float) * vertexBuffer.Count, vertexBuffer.ToArray(), BufferUsageHint.StaticDraw);

            VertexArrayObject = GL.GenVertexArray();
            GL.BindVertexArray(VertexArrayObject);

            GL.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, 8 * sizeof(float), 0);
            GL.VertexAttribPointer(1, 2, VertexAttribPointerType.Float, false, 8 * sizeof(float), 3 * sizeof(float));
            GL.VertexAttribPointer(2, 3, VertexAttribPointerType.Float, false, 8 * sizeof(float), 5 * sizeof(float));

            GL.EnableVertexAttribArray(0);
            GL.EnableVertexAttribArray(1);
            GL.EnableVertexAttribArray(2);

            string vertexPath = "model_vert.shader";
            string fragmentPath = "model_frag.shader";

            string VertexShaderSource = File.ReadAllText(vertexPath);
            string FragmentShaderSource = File.ReadAllText(fragmentPath);

            int VertexShader = GL.CreateShader(ShaderType.VertexShader);
            GL.ShaderSource(VertexShader, VertexShaderSource);

            int FragmentShader = GL.CreateShader(ShaderType.FragmentShader);
            GL.ShaderSource(FragmentShader, FragmentShaderSource);

            GL.CompileShader(VertexShader);

            GL.GetShader(VertexShader, ShaderParameter.CompileStatus, out int success);
            if (success == 0)
            {
                string infoLog = GL.GetShaderInfoLog(VertexShader);
                Console.WriteLine(infoLog);
            }

            GL.CompileShader(FragmentShader);

            GL.GetShader(FragmentShader, ShaderParameter.CompileStatus, out success);
            if (success == 0)
            {
                string infoLog = GL.GetShaderInfoLog(FragmentShader);
                Console.WriteLine(infoLog);
            }

            ShaderObject = GL.CreateProgram();

            GL.AttachShader(ShaderObject, VertexShader);
            GL.AttachShader(ShaderObject, FragmentShader);

            GL.LinkProgram(ShaderObject);

            GL.GetProgram(ShaderObject, GetProgramParameterName.LinkStatus, out success);
            if (success == 0)
            {
                string infoLog = GL.GetProgramInfoLog(ShaderObject);
                Console.WriteLine(infoLog);
            }
            TextureObject = GL.GenTexture();
            GL.BindTexture(TextureTarget.Texture2D, TextureObject);

            GL.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapS, (int)TextureWrapMode.Repeat);
            GL.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapT, (int)TextureWrapMode.Repeat);
            GL.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMinFilter, (int)TextureMinFilter.Linear);
            GL.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMagFilter, (int)TextureMagFilter.Linear);

            StbImage.stbi_set_flip_vertically_on_load(1);

            // Load the image.
            ImageResult image = ImageResult.FromStream(File.OpenRead(filePath+ "_diffuse.png"), ColorComponents.RedGreenBlueAlpha);
            GL.TexImage2D(TextureTarget.Texture2D, 0, PixelInternalFormat.Rgba, image.Width, image.Height, 0, PixelFormat.Rgba, PixelType.UnsignedByte, image.Data);

        }
        #endregion
    }
}
