using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO.Ports;
using System.Threading;
using System.Windows.Forms;

using GMap.NET.MapProviders;
using GMap.NET;

using OpenTK;
using OpenTK.Graphics.OpenGL;
using System.IO;

using StbImageSharp;
/*
SERIAL'A BASILAN      =====>  "packageNum_Ana,patlama1,patlama2,basinc_Ana,X_jiro,Y_jiro,Z_jiro,X_ivme,Y_ivme,Z_ivme,GPSe_Ana,GPSb_Ana,packageNum_Gorev,sicaklik_Gorev,nem_Gorev,basinc_Gorev,GPSe_Gorev,GPSb_Gorev"
BYTE BOYUTU (60Byte)  =====>     1BYTE       ,  1BYTE ,  1BYTE , 4BYTE    ,4BYTE ,4BYTE ,4BYTE ,4BYTE , 4BYTE ,4BYTE ,4BYTE  ,4BYTE   ,  1BYTE         ,  4BYTE       ,  4BYTE  , 4BYTE      ,4BYTE     ,  4BYTE ,
*/

namespace TOBBETUROCKETRY
{
    //https://www.google.com/maps?q=39.9272,32.8644
    public partial class TOBBETUROCKETRY : Form
    {
        static readonly int DATA_COUNT = 18;
        private readonly string nameOfModel = "rocket"; // without ".obj"


        SerialPort serialPort = new SerialPort();

        public string filenameRoket = "RoketValues.csv";
        public string filenameFaydaliYuk = "FaydaliYukValues.csv";
        private string recivedData = "0,0,0,0,0,0,0,0,0,0,39.9103241,32.8529681,0,0,0,0,39.9103241,32.8529681";
        private string[] values = new string[DATA_COUNT];

        public TOBBETUROCKETRY()
        {
            InitializeComponent();
            FetchAvailablePorts();
            values = recivedData.Split(',');
            btnBaglantiyiBitir.Enabled = false;
        }

        #region BAĞLANTI BAŞLATMA / BİTİRME
        private void FetchAvailablePorts()
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
            textBoxPaketNum_AnaBil.Text = GetPinValue(PackageElements.packageNum_Ana);
            textBoxBasinc_AnaBil.Text = GetPinValue(PackageElements.basinc_Ana);
            textBoxGPSEnlem_AnaBil.Text = GetPinValue(PackageElements.GPSe_Ana);
            textBoxGPSBoylam_AnaBil.Text = GetPinValue(PackageElements.GPSb_Ana);
            textBoxJiroX_AnaBil.Text = GetPinValue(PackageElements.X_jiro);
            textBoxJiroY_AnaBil.Text = GetPinValue(PackageElements.Y_jiro);
            textBoxJiroZ_AnaBil.Text = GetPinValue(PackageElements.Z_jiro);
            textBoxIvmeX_AnaBil.Text = GetPinValue(PackageElements.X_ivme);
            textBoxIvmeY_AnaBil.Text = GetPinValue(PackageElements.Y_ivme);
            textBoxIvmeZ_AnaBil.Text = GetPinValue(PackageElements.Z_ivme);

            x_angle_3d_model = GetPinValueFloat(PackageElements.X_jiro);
            y_angle_3d_model = GetPinValueFloat(PackageElements.Y_jiro) + 90;
            z_angle_3d_model = GetPinValueFloat(PackageElements.Z_jiro);

            textBoxPaketNum_GorevYuku.Text = GetPinValue(PackageElements.packageNum_Gorev);
            textBoxSicaklik_GorevYuku.Text = GetPinValue(PackageElements.packageNum_Gorev);
            textBoxNem_GorevYuku.Text = GetPinValue(PackageElements.packageNum_Gorev);
            textBoxBasinc_GorevYuku.Text = GetPinValue(PackageElements.basinc_Gorev);
            textBoxGPSEnlem_GorevYuku.Text = GetPinValue(PackageElements.GPSe_Gorev);
            textBoxGPSBoylam_GorevYuku.Text = GetPinValue(PackageElements.GPSb_Gorev);

            try
            {
                s.Invalidate();
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


        #region EVENTS
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
            gMapAnaBilgisayar.Position = new PointLatLng(GetPinValueFloat(PackageElements.GPSe_Ana), GetPinValueFloat(PackageElements.GPSb_Ana));
            gMapAnaBilgisayar.MinZoom = 1;
            gMapAnaBilgisayar.MaxZoom = 100;
            gMapAnaBilgisayar.Zoom = 16;

            gMapGorevYuku.DragButton = MouseButtons.Left;
            gMapGorevYuku.MouseWheelZoomEnabled = true;
            gMapGorevYuku.MapProvider = GMapProviders.GoogleMap;
            gMapGorevYuku.Position = new PointLatLng(GetPinValueFloat(PackageElements.GPSe_Gorev), GetPinValueFloat(PackageElements.GPSb_Gorev));
            gMapGorevYuku.MinZoom = 1;
            gMapGorevYuku.MaxZoom = 100;
            gMapGorevYuku.Zoom = 16;

        }
        private void btnRefresh_Click(object sender, EventArgs e)
        {
            FetchAvailablePorts();
            s.Invalidate();
        }
        #endregion


        #region EK FONKSİYONLAR

        private string GetPinValue(PackageElements pe)
        {
            return values[(int)pe];
        }
        private float GetPinValueFloat(PackageElements pe)
        {
            return float.Parse(GetPinValue(pe).IndexOf(".") >= 0 ? GetPinValue(pe).Replace(".", ",") : GetPinValue(pe));
        }
        private void RefreshMapToNewGPS()
        {
            gMapAnaBilgisayar.Position = new PointLatLng(GetPinValueFloat(PackageElements.GPSe_Ana), GetPinValueFloat(PackageElements.GPSb_Ana));
            gMapAnaBilgisayar.MinZoom = 1;
            gMapAnaBilgisayar.MaxZoom = 100;
            gMapAnaBilgisayar.Zoom = 16;
            gMapGorevYuku.Position = new PointLatLng(GetPinValueFloat(PackageElements.GPSe_Gorev), GetPinValueFloat(PackageElements.GPSb_Gorev));
            gMapGorevYuku.MinZoom = 1;
            gMapGorevYuku.MaxZoom = 100;
            gMapGorevYuku.Zoom = 16;
        }

        #endregion


        #region 3D NESNE OPENGL

        struct Material
        {
            public Vector3 ambient;
            public Vector3 diffuse;
            public Vector3 specular;
            public float shininess;
        };
        struct DirectionalLight
        {
            public Vector3 direction;
            public Vector3 ambient;
            public Vector3 diffuse;
            public Vector3 specular;
            public float intensity;
        };

        List<Vector3> vertices = new List<Vector3>();
        List<int> triangles = new List<int>();
        List<Vector3> normals = new List<Vector3>();
        List<Vector2> texCoords = new List<Vector2>();
        private List<int[]> faces = new List<int[]>();
        int vertexCount;
        int VertexBufferObject, VertexArrayObject, ShaderObject, TextureObject;
        float x_angle_3d_model = 0, y_angle_3d_model = 0, z_angle_3d_model = 0;
        Material modelMat;
        DirectionalLight light;
        Vector3 cameraPos = new Vector3(6, 0, 0);
        Vector3 cameraDir = new Vector3(0, 0, 0);

        private void glControl1_AutoSizeChanged(object sender, EventArgs e)
        {
            GL.Clear(ClearBufferMask.ColorBufferBit);
            GL.Clear(ClearBufferMask.DepthBufferBit);

            Matrix4 perspective = Matrix4.CreatePerspectiveFieldOfView(1.04f, s.AspectRatio, 0.1f, 1000);
            Matrix4 lookat = Matrix4.LookAt(cameraPos.X, cameraPos.Y, cameraPos.Z, cameraDir.X, cameraDir.Y, cameraDir.Z, 0, 1, 0);
            GL.MatrixMode(MatrixMode.Projection);
            GL.LoadIdentity();
            GL.LoadMatrix(ref perspective);
            GL.MatrixMode(MatrixMode.Modelview);
            GL.LoadIdentity();
            GL.LoadMatrix(ref lookat);
            GL.Viewport(0, 0, s.Width, s.Height);
        }
        private void glControl1_Paint(object sender, PaintEventArgs e)
        {

            GL.Clear(ClearBufferMask.ColorBufferBit);
            GL.Clear(ClearBufferMask.DepthBufferBit);

            Matrix4 perspective = Matrix4.CreatePerspectiveFieldOfView(1.04f, s.AspectRatio, 0.1f, 1000);
            Matrix4 lookat = Matrix4.LookAt(cameraPos.X, cameraPos.Y, cameraPos.Z, cameraDir.X, cameraDir.Y, cameraDir.Z, 0, 1, 0);
            GL.MatrixMode(MatrixMode.Projection);
            GL.LoadIdentity();
            GL.LoadMatrix(ref perspective);
            GL.MatrixMode(MatrixMode.Modelview);
            GL.LoadIdentity();
            GL.LoadMatrix(ref lookat);

            GL.Begin(BeginMode.Lines);
            GL.Color3(Color.Blue);
            GL.Vertex3(0, 0.0, -10.0);
            GL.Vertex3(0.0, 0.0, 10.0);


            GL.Color3(Color.Green);
            GL.Vertex3(0.0, -10, 0.0);
            GL.Vertex3(0.0, 10, 0.0);

            GL.Color3(Color.Red);
            GL.Vertex3(-10.0, 0.0, 0);
            GL.Vertex3(10.0, 0.0, 0);
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

            Matrix4 model = Matrix4.Identity * rotZ * rotY * rotX;

            GL.UniformMatrix4(modelAddress, false, ref model);
            GL.UniformMatrix4(viewAddress, false, ref lookat);
            GL.UniformMatrix4(projectionAddress, false, ref perspective);

            GL.ActiveTexture(TextureUnit.Texture0);
            GL.BindTexture(TextureTarget.Texture2D, TextureObject);

            int texLocation = GL.GetUniformLocation(TextureObject, "texture_diffuse1");

            GL.Uniform1(texLocation, 0);

            int location = GL.GetUniformLocation(ShaderObject, "material.ambient");
            GL.Uniform3(location, modelMat.ambient);
            location = GL.GetUniformLocation(ShaderObject, "material.diffuse");
            GL.Uniform3(location, modelMat.diffuse);
            location = GL.GetUniformLocation(ShaderObject, "material.specular");
            GL.Uniform3(location, modelMat.specular);
            location = GL.GetUniformLocation(ShaderObject, "material.shininess");
            GL.Uniform1(location, modelMat.shininess);

            location = GL.GetUniformLocation(ShaderObject, "light.ambient");
            GL.Uniform3(location, light.ambient);
            location = GL.GetUniformLocation(ShaderObject, "light.diffuse");
            GL.Uniform3(location, light.diffuse);
            location = GL.GetUniformLocation(ShaderObject, "light.specular");
            GL.Uniform3(location, light.specular);
            location = GL.GetUniformLocation(ShaderObject, "light.direction");
            GL.Uniform3(location, light.direction);
            location = GL.GetUniformLocation(ShaderObject, "light.intensity");
            GL.Uniform1(location, light.intensity);

            location = GL.GetUniformLocation(ShaderObject, "camPos");
            GL.Uniform3(location, new Vector3(0, 0, 8));

            GL.DrawArrays(BeginMode.Triangles, 0, vertexCount);

            GL.BindBuffer(BufferTarget.ArrayBuffer, 0);
            GL.BindVertexArray(0);
            GL.UseProgram(0);

            s.SwapBuffers();

        }
        private void glControl1_Load(object sender, EventArgs e)
        {
            GL.ClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            GL.Enable(EnableCap.DepthTest);//sonradan yazdık 
            ModelLoadToBuffer(nameOfModel);
        }
        private void ModelLoadToBuffer(string filePath)
        {
            //"model.obj" file and "model_diffuse.png" file must be stated in same directory

            ObjParser.ParseFile(filePath + ".obj");

            vertices = ObjParser.Vertices;
            normals = ObjParser.Normals;
            texCoords = ObjParser.TexCoords;
            faces = ObjParser.Faces;

            List<float> vertexBuffer = new List<float>();

            foreach (int[] face in faces)
            {
                for (int i = 0; i < 3; i++)
                {
                    Vector3 vertex = vertices[face[i * 3]];
                    Vector2 texCoord = texCoords[face[i * 3 + 1]];
                    Vector3 normal = normals[face[i * 3 + 2]];

                    vertexBuffer.Add(vertex.X);
                    vertexBuffer.Add(vertex.Y);
                    vertexBuffer.Add(vertex.Z);
                    vertexBuffer.Add(texCoord.X);
                    vertexBuffer.Add(texCoord.Y);
                    vertexBuffer.Add(normal.X);
                    vertexBuffer.Add(normal.Y);
                    vertexBuffer.Add(normal.Z);
                }
            }

            vertexCount = vertexBuffer.Count / 8;

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
            ImageResult image = ImageResult.FromStream(File.OpenRead(filePath + "_diffuse.png"), ColorComponents.RedGreenBlueAlpha);
            GL.TexImage2D(TextureTarget.Texture2D, 0, PixelInternalFormat.Rgba, image.Width, image.Height, 0, PixelFormat.Rgba, PixelType.UnsignedByte, image.Data);

            modelMat.ambient = new Vector3(0.5f, 0.5f, 0.5f);
            modelMat.diffuse = new Vector3(0.7f, 0.7f, 0.7f);
            modelMat.specular = new Vector3(0.9f, 0.9f, 0.9f);
            modelMat.shininess = 10;

            light.ambient = new Vector3(0.5f, 0.5f, 0.5f);
            light.diffuse = new Vector3(0.7f, 0.7f, 0.7f);
            light.specular = new Vector3(0.9f, 0.9f, 0.9f);
            light.direction = new Vector3(0, -1, 0);
            light.intensity = 4;
        }
        #endregion
    }
}
