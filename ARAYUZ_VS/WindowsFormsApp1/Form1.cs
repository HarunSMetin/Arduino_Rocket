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

/*
  SERIAL'A BASILAN   =====>  "packageNum,explode1,explode2,pressure,X,Y,Z,GPSe,GPSb
  BYTE BOYUTU    27  =====>     1BYTE   ,  1BYTE ,  1BYTE , 4BYTE  ,4,4,4, 4  ,  4 
*/
namespace TOBBETUROCKETRY
{
    enum PackageElements
    {
        packageNum  ,
        explode1    ,
        explode2    ,
        pressure    ,
        X           ,
        Y           ,
        Z           ,
        GPSe        ,
        GPSb        
    }
    public partial class Form1 : Form
    { 
        static int DATA_COUNT = 9;

        SerialPort serialPort = new SerialPort();

        public string filenameRoket = "RoketValues.csv"; 
        public string filenameFaydaliYuk = "FaydaliYukValues.csv";
        //https://www.google.com/maps?q=39.9272,32.8644
        private string recivedData = "0,0,0,0,0,0,0,39.9272,32.8644";
        private string[] values = new string[DATA_COUNT]; 

        public Form1()
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
            if (comboBoxComPort.Items.Count > 0) comboBoxComPort.SelectedIndex = 0;
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
            catch (Exception) { for (int i = 0; i < values.Length; i++) values[i] = "0"; }
        }
        private void valueUpdate()
        {
            //burası her data geldiğinde çalışır
            Thread.Sleep(100); //will sleep for 100ms
            //TODO: burda degerleri guncelle 
            try
            {
                //TODO: MAPS GÜNCELLEMESİ
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
            // webBrowser1.Navigate("https://www.google.com/maps/search/?api=1&query=" + values[(int)PackageElements.GPSe] + "," + values[(int)PackageElements.GPSb]);
            gMapGorevYuku.DragButton = MouseButtons.Left;
            gMapGorevYuku.MouseWheelZoomEnabled = true;
            gMapGorevYuku.MapProvider = GMapProviders.GoogleMap;
            gMapGorevYuku.Position = new PointLatLng(39.9272, 32.8644);
            gMapGorevYuku.MinZoom = 1;
            gMapGorevYuku.MaxZoom = 100;
            gMapGorevYuku.Zoom = 16;
            
        }

        private void btnRefresh_Click(object sender, EventArgs e)
        {
            fetchAvailablePorts();
        }
    }
}
