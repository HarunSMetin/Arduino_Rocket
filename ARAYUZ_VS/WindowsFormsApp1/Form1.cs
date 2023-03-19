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
/*
  SERIAL'A BASILAN      =====>  "packageNum,explode1,explode2,pressure,X_Jiro,Y_Jiro,Z_Jiro,X_Ivme,Y_Ivme ,Z_Ivme,GPSe ,GPSb"
  BYTE BOYUTU (39Byte)  =====>     1BYTE   ,  1BYTE ,  1BYTE , 4BYTE  ,4BYTE ,4BYTE ,4BYTE ,4BYTE , 4BYTE ,4BYTE ,4BYTE,4BYTE  
*/

namespace TOBBETUROCKETRY
{
    enum PackageElements
    {
        packageNum  ,
        explode1    ,
        explode2    ,
        pressure    ,
        X_jiro      ,
        Y_jiro      ,
        Z_jiro      ,
        X_ivme      ,
        Y_ivme      ,
        Z_ivme      ,
        GPSe        ,
        GPSb        
    }
    public partial class TOBBETUROCKETRY : Form
    { 
        static int DATA_COUNT = 12;

        SerialPort serialPort = new SerialPort();

        public string filenameRoket = "RoketValues.csv"; 
        public string filenameFaydaliYuk = "FaydaliYukValues.csv";
        //https://www.google.com/maps?q=39.9272,32.8644
        private string recivedData = "0,0,0,0,0,0,0,0,0,0,39.9103241,32.8529681";
        private string[] values = new string[DATA_COUNT]; 

        public TOBBETUROCKETRY()
        {
            InitializeComponent();
            fetchAvailablePorts();  
            values = recivedData.Split(','); 
            btnBaglantiyiBitir.Enabled = false;
        }
        void fetchAvailablePorts()
        {
            numericUpDownBaudRate.Value = 9600;
            comboBoxComPort.Items.Clear();
            string[] ports = SerialPort.GetPortNames();
            comboBoxComPort.Items.AddRange(ports);
            if (comboBoxComPort.Items.Count > 0) comboBoxComPort.SelectedIndex = comboBoxComPort.Items.Count-1;
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
                fetchAvailablePorts();
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
                fetchAvailablePorts();
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
                    this.Invoke(new Action(valueUpdate));
                }
            }
            catch (Exception) { }

        }
        private void ProcessingData()
        {
            try { values = recivedData.Split(','); }
            catch (Exception) { for (int i = 0; i < DATA_COUNT; i++) values[i] = "0"; }
        }
        private void valueUpdate()
        {
            //burası her data geldiğinde çalışır
            Thread.Sleep(100); //will sleep for 100ms
            //TODO: burda degerleri guncelle 
            textBoxPaketNum_AnaBil.Text =  GetPinValue(PackageElements.packageNum);
            textBoxBasinc_AnaBil.Text =    GetPinValue(PackageElements.pressure);
            textBoxGPSEnlem_AnaBil.Text =  GetPinValue(PackageElements.GPSe);
            textBoxGPSBoylam_AnaBil.Text = GetPinValue(PackageElements.GPSb);
            textBoxJiroX_AnaBil.Text =     GetPinValue(PackageElements.X_jiro);
            textBoxJiroY_AnaBil.Text =     GetPinValue(PackageElements.Y_jiro);
            textBoxJiroZ_AnaBil.Text =     GetPinValue(PackageElements.Z_jiro); 
            textBoxIvmeX_AnaBil.Text =     GetPinValue(PackageElements.X_ivme);
            textBoxIvmeY_AnaBil.Text =     GetPinValue(PackageElements.Y_ivme);
            textBoxIvmeZ_AnaBil.Text =     GetPinValue(PackageElements.Z_ivme);

            x_angle_3d_model =  GetPinValueFloat(PackageElements.X_jiro);
            y_angle_3d_model =  GetPinValueFloat(PackageElements.Y_jiro) + 90;
            z_angle_3d_model =  GetPinValueFloat(PackageElements.Z_jiro);
             
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
            try{
                if (serialPort.IsOpen) 
                {
                    MessageBox.Show("COM PORT bağlantısını kapatmadınız! Otomatik sonlandırılıyor...");
                    serialPort.Close();
                }
            }
            catch(Exception ex) { MessageBox.Show(""+ex); }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            gMapAnaBilgisayar.DragButton = MouseButtons.Left;
            gMapAnaBilgisayar.MouseWheelZoomEnabled= true;
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
            fetchAvailablePorts();
        }
        private string GetPinValue(PackageElements pe)
        {
            return values[(int)pe];
        }
        private float GetPinValueFloat(PackageElements pe)
        {
            return float.Parse( GetPinValue(pe).IndexOf(".")>=0 ? GetPinValue(pe).Replace(".", ","): GetPinValue(pe));
        }
        #region 3d nesne
        private void silindir(float step, float topla, float radius, float dikey1, float dikey2)
        {
            float eski_step = 0.1f;
            GL.Begin(BeginMode.Quads);//Y_jiro EKSEN CIZIM DAİRENİN
            while (step <= 360)
            {
                if (step < 45)
                    GL.Color3(Color.FromArgb(255, 0, 0));
                else if (step < 90)
                    GL.Color3(Color.FromArgb(255, 255, 255));
                else if (step < 135)
                    GL.Color3(Color.FromArgb(255, 0, 0));
                else if (step < 180)
                    GL.Color3(Color.FromArgb(255, 255, 255));
                else if (step < 225)
                    GL.Color3(Color.FromArgb(255, 0, 0));
                else if (step < 270)
                    GL.Color3(Color.FromArgb(255, 255, 255));
                else if (step < 315)
                    GL.Color3(Color.FromArgb(255, 0, 0));
                else if (step < 360)
                    GL.Color3(Color.FromArgb(255, 255, 255));


                float ciz1_x = (float)(radius * Math.Cos(step * Math.PI / 180F));
                float ciz1_y = (float)(radius * Math.Sin(step * Math.PI / 180F));
                GL.Vertex3(ciz1_x, dikey1, ciz1_y);

                float ciz2_x = (float)(radius * Math.Cos((step + 2) * Math.PI / 180F));
                float ciz2_y = (float)(radius * Math.Sin((step + 2) * Math.PI / 180F));
                GL.Vertex3(ciz2_x, dikey1, ciz2_y);

                GL.Vertex3(ciz1_x, dikey2, ciz1_y);
                GL.Vertex3(ciz2_x, dikey2, ciz2_y);
                step += topla;
            }
            GL.End();
            GL.Begin(BeginMode.Lines);
            step = eski_step;
            topla = step;
            while (step <= 180)// UST KAPAK
            {
                if (step < 45)
                    GL.Color3(Color.FromArgb(255, 1, 1));
                else if (step < 90)
                    GL.Color3(Color.FromArgb(250, 250, 200));
                else if (step < 135)
                    GL.Color3(Color.FromArgb(255, 1, 1));
                else if (step < 180)
                    GL.Color3(Color.FromArgb(250, 250, 200));
                else if (step < 225)
                    GL.Color3(Color.FromArgb(255, 1, 1));
                else if (step < 270)
                    GL.Color3(Color.FromArgb(250, 250, 200));
                else if (step < 315)
                    GL.Color3(Color.FromArgb(255, 1, 1));
                else if (step < 360)
                    GL.Color3(Color.FromArgb(250, 250, 200));


                float ciz1_x = (float)(radius * Math.Cos(step * Math.PI / 180F));
                float ciz1_y = (float)(radius * Math.Sin(step * Math.PI / 180F));
                GL.Vertex3(ciz1_x, dikey1, ciz1_y);

                float ciz2_x = (float)(radius * Math.Cos((step + 180) * Math.PI / 180F));
                float ciz2_y = (float)(radius * Math.Sin((step + 180) * Math.PI / 180F));
                GL.Vertex3(ciz2_x, dikey1, ciz2_y);

                GL.Vertex3(ciz1_x, dikey1, ciz1_y);
                GL.Vertex3(ciz2_x, dikey1, ciz2_y);
                step += topla;
            }
            step = eski_step;
            topla = step;
            while (step <= 180)//ALT KAPAK
            {
                if (step < 45)
                    GL.Color3(Color.FromArgb(255, 1, 1));
                else if (step < 90)
                    GL.Color3(Color.FromArgb(250, 250, 200));
                else if (step < 135)
                    GL.Color3(Color.FromArgb(255, 1, 1));
                else if (step < 180)
                    GL.Color3(Color.FromArgb(250, 250, 200));
                else if (step < 225)
                    GL.Color3(Color.FromArgb(255, 1, 1));
                else if (step < 270)
                    GL.Color3(Color.FromArgb(250, 250, 200));
                else if (step < 315)
                    GL.Color3(Color.FromArgb(255, 1, 1));
                else if (step < 360)
                    GL.Color3(Color.FromArgb(250, 250, 200));

                float ciz1_x = (float)(radius * Math.Cos(step * Math.PI / 180F));
                float ciz1_y = (float)(radius * Math.Sin(step * Math.PI / 180F));
                GL.Vertex3(ciz1_x, dikey2, ciz1_y);

                float ciz2_x = (float)(radius * Math.Cos((step + 180) * Math.PI / 180F));
                float ciz2_y = (float)(radius * Math.Sin((step + 180) * Math.PI / 180F));
                GL.Vertex3(ciz2_x, dikey2, ciz2_y);

                GL.Vertex3(ciz1_x, dikey2, ciz1_y);
                GL.Vertex3(ciz2_x, dikey2, ciz2_y);
                step += topla;
            }
            GL.End();
        }
        private void koni(float step, float topla, float radius1, float radius2, float dikey1, float dikey2)
        {
            float eski_step = 0.1f;
            GL.Begin(BeginMode.Lines);//Y_jiro EKSEN CIZIM DAİRENİN
            while (step <= 360)
            {
                if (step < 45)
                    GL.Color3(1.0, 1.0, 1.0);
                else if (step < 90)
                    GL.Color3(1.0, 0.0, 0.0);
                else if (step < 135)
                    GL.Color3(1.0, 1.0, 1.0);
                else if (step < 180)
                    GL.Color3(1.0, 0.0, 0.0);
                else if (step < 225)
                    GL.Color3(1.0, 1.0, 1.0);
                else if (step < 270)
                    GL.Color3(1.0, 0.0, 0.0);
                else if (step < 315)
                    GL.Color3(1.0, 1.0, 1.0);
                else if (step < 360)
                    GL.Color3(1.0, 0.0, 0.0);


                float ciz1_x = (float)(radius1 * Math.Cos(step * Math.PI / 180F));
                float ciz1_y = (float)(radius1 * Math.Sin(step * Math.PI / 180F));
                GL.Vertex3(ciz1_x, dikey1, ciz1_y);

                float ciz2_x = (float)(radius2 * Math.Cos(step * Math.PI / 180F));
                float ciz2_y = (float)(radius2 * Math.Sin(step * Math.PI / 180F));
                GL.Vertex3(ciz2_x, dikey2, ciz2_y);
                step += topla;
            }
            GL.End();

            GL.Begin(BeginMode.Lines);
            step = eski_step;
            topla = step;
            while (step <= 180)// UST KAPAK
            {
                if (step < 45)
                    GL.Color3(Color.FromArgb(255, 1, 1));
                else if (step < 90)
                    GL.Color3(Color.FromArgb(250, 250, 200));
                else if (step < 135)
                    GL.Color3(Color.FromArgb(255, 1, 1));
                else if (step < 180)
                    GL.Color3(Color.FromArgb(250, 250, 200));
                else if (step < 225)
                    GL.Color3(Color.FromArgb(255, 1, 1));
                else if (step < 270)
                    GL.Color3(Color.FromArgb(250, 250, 200));
                else if (step < 315)
                    GL.Color3(Color.FromArgb(255, 1, 1));
                else if (step < 360)
                    GL.Color3(Color.FromArgb(250, 250, 200));


                float ciz1_x = (float)(radius2 * Math.Cos(step * Math.PI / 180F));
                float ciz1_y = (float)(radius2 * Math.Sin(step * Math.PI / 180F));
                GL.Vertex3(ciz1_x, dikey2, ciz1_y);

                float ciz2_x = (float)(radius2 * Math.Cos((step + 180) * Math.PI / 180F));
                float ciz2_y = (float)(radius2 * Math.Sin((step + 180) * Math.PI / 180F));
                GL.Vertex3(ciz2_x, dikey2, ciz2_y);

                GL.Vertex3(ciz1_x, dikey2, ciz1_y);
                GL.Vertex3(ciz2_x, dikey2, ciz2_y);
                step += topla;
            }
            step = eski_step;
            topla = step;
            GL.End();
        }
        private void Pervane(float yukseklik, float uzunluk, float kalinlik, float egiklik)
        {
            float radius = 10, angle = 45.0f;
            GL.Begin(BeginMode.Quads);

            GL.Color3(Color.Red);
            GL.Vertex3(uzunluk, yukseklik, kalinlik);
            GL.Vertex3(uzunluk, yukseklik + egiklik, -kalinlik);
            GL.Vertex3(0.0, yukseklik + egiklik, -kalinlik);
            GL.Vertex3(0.0, yukseklik, kalinlik);

            GL.Color3(Color.Red);
            GL.Vertex3(-uzunluk, yukseklik + egiklik, kalinlik);
            GL.Vertex3(-uzunluk, yukseklik, -kalinlik);
            GL.Vertex3(0.0, yukseklik, -kalinlik);
            GL.Vertex3(0.0, yukseklik + egiklik, kalinlik);

            GL.Color3(Color.White);
            GL.Vertex3(kalinlik, yukseklik, -uzunluk);
            GL.Vertex3(-kalinlik, yukseklik + egiklik, -uzunluk);
            GL.Vertex3(-kalinlik, yukseklik + egiklik, 0.0);//+
            GL.Vertex3(kalinlik, yukseklik, 0.0);//-

            GL.Color3(Color.White);
            GL.Vertex3(kalinlik, yukseklik + egiklik, +uzunluk);
            GL.Vertex3(-kalinlik, yukseklik, +uzunluk);
            GL.Vertex3(-kalinlik, yukseklik, 0.0);
            GL.Vertex3(kalinlik, yukseklik + egiklik, 0.0);
            GL.End();

        }

        float x_angle_3d_model =180, y_angle_3d_model =0, z_angle_3d_model = -180;
        private void glControl1_Paint(object sender, PaintEventArgs e)
        {

            float step = 1.0f;
            float topla = step;
            float radius = 5.0f;
            float dikey1 = radius, dikey2 = -radius;
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
            GL.Enable(EnableCap.DepthTest);
            GL.DepthFunc(DepthFunction.Less);

            GL.Rotate(x_angle_3d_model, 1.0, 0.0, 0.0);//ÖNEMLİ
            GL.Rotate(z_angle_3d_model, 0.0, 1.0, 0.0);
            GL.Rotate(y_angle_3d_model, 0.0, 0.0, 1.0);

            silindir(step, topla, radius, 3, -10);
           // silindir(0.01f, topla, 0.5f, 9, 9.7f);
            //silindir(0.01f, topla, 0.1f, 5, dikey1 + 5);
            //koni(0.01f, 0.01f, radius, 3.0f, 3, 5);
            koni(0.01f, 0.01f, radius, 0.5f, -10.0f, -15.0f);
            //Pervane(9.0f, 11.0f, 0.2f, 0.5f);

            GL.Begin(BeginMode.Lines);

            GL.Color3(Color.FromArgb(250, 0, 0));
            GL.Vertex3(-30.0, 0.0, 0.0);
            GL.Vertex3(30.0, 0.0, 0.0);


            GL.Color3(Color.FromArgb(0, 0, 0));
            GL.Vertex3(0.0, 30.0, 0.0);
            GL.Vertex3(0.0, -30.0, 0.0);

            GL.Color3(Color.FromArgb(0, 0, 250));
            GL.Vertex3(0.0, 0.0, 30.0);
            GL.Vertex3(0.0, 0.0, -30.0);

            GL.End();
            //GraphicsContext.CurrentContext.VSync = true;
            glControl1.SwapBuffers();
        }
        private void glControl1_Load(object sender, EventArgs e)
        {
            GL.ClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            GL.Enable(EnableCap.DepthTest);//sonradan yazdık
        }
        #endregion
    }
}
