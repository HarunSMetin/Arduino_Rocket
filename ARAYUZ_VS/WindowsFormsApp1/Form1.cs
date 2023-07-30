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
using System.Globalization;
using System.Collections;
using System.Windows.Forms.DataVisualization.Charting;
using static System.Net.Mime.MediaTypeNames;
/*
SERIAL'A BASILAN      =====>  "packageNum_Ana,patlama1,patlama2,Irtifa_basinc_Ana,GPSIrtifa_Ana,basinc_Ana,X_jiro,Y_jiro,Z_jiro,X_ivme,Y_ivme,Z_ivme,GPSe_Ana,GPSb_Ana,packageNum_Gorev,sicaklik_Gorev,nem_Gorev,basinc_Gorev,GPSe_Gorev,GPSb_Gorev"
BYTE BOYUTU (68Byte)  =====>     1BYTE       ,  1BYTE ,  1BYTE ,     4BYTE       ,    4BYTE    , 4BYTE    ,4BYTE ,4BYTE ,4BYTE ,4BYTE , 4BYTE ,4BYTE ,4BYTE  ,4BYTE   ,  1BYTE         ,  4BYTE       ,  4BYTE  , 4BYTE      ,4BYTE     ,  4BYTE ,
*/

namespace TOBBETUROCKETRY
{

    enum PackageElements
    {
        packageNum_Ana,
        statusAna,
        Irtifa_basinc_Ana,
        GPSIrtifa_Ana,
        basinc_Ana,
        X_jiro,
        Y_jiro,
        Z_jiro,
        X_ivme,
        Y_ivme,
        Z_ivme,
        Aci_Ana,
        GPSe_Ana,
        GPSb_Ana,
        GPSSatNum_ana,
        statusGorev,
        packageNum_Gorev,
        GPSIrtifa_Gorev,
        sicaklik_Gorev,
        nem_Gorev,
        basinc_Gorev,
        GPSe_Gorev,
        GPSb_Gorev,
        GPSSatNum_Gorev
    }

    //https://www.google.com/maps?q=39.9272,32.8644
    public partial class TOBBETUROCKETRY : Form
    {
        public byte paketNumarasıHYI = 0;
        ////////////////////////////////////////  
        public readonly int Takim_ID = 99;
        public readonly int mapZoomCount = 20; //10-20
        ////////////////////////////////////////    
        static readonly int DATA_COUNT = 24;
        private readonly string nameOfModel = AppDomain.CurrentDomain.BaseDirectory + "/rocket"; // without ".obj" 
        public readonly string filenameRoket = "RoketValues.csv";

        SerialPort serialPort_YerIst = new SerialPort();
        SerialPort SerialPort_HYI = new SerialPort();

        private readonly string dataTitles = "packageNum_Ana,patlama1,patlama2,Irtifa_basinc_Ana,GPSIrtifa_Ana,basinc_Ana,X_jiro,Y_jiro,Z_jiro,X_ivme,Y_ivme,Z_ivme,Aci_Ana,GPSe_Ana,GPSb_Ana,GPSSatNum_ana,packageNum_Gorev,GPSIrtifa_Gorev,sicaklik_Gorev,nem_Gorev,basinc_Gorev,GPSe_Gorev,GPSb_Gorev,GPSSatNum_Gorev";
        private string recivedData = "0,0,0,0,0,0,0,0,0,0,0,0,0,39.9103241,32.8529681,0,0,0,0,0,0,39.9103241,32.8529681,0";
        private string[] values = new string[DATA_COUNT];

        public static string HYI_Port = "";
        public static int HYI_BaudRate = 19200;
        public static int HYI_DataBits = 8;
        public static int HYI_StopBits = 1;
        public static int HYI_Parity = 0;

        public TOBBETUROCKETRY()
        {
            Thread.CurrentThread.CurrentUICulture = new CultureInfo("en-US");
            Thread.CurrentThread.CurrentCulture = new CultureInfo("en-US");
            this.InitializeComponent();

            FetchAvailablePorts();
            values = recivedData.Split(',');
            btnBaglantiyiBitir.Enabled = false;
        }

        #region BAGLANTI BASLATMA / BITIRME
        private void FetchAvailablePorts()
        {
            numericUpDownBaudRate.Value = 19200;
            comboBoxComPort.Items.Clear();
            string[] ports = SerialPort.GetPortNames();
            comboBoxComPort.Items.AddRange(ports);
            if (ports.Length > 0) comboBoxComPort.SelectedIndex = comboBoxComPort.Items.Count - 1;
            else
            {
                comboBoxComPort.Text = "Takili Cihaz Yok";
                lblDurum.ForeColor = Color.Red;
                lblDurum.Text = "Baglanti Yok!";
            }
        }
        private void btnBaglan_Click(object sender, EventArgs e)
        {
            serialPort_YerIst = new SerialPort();
            if (comboBoxComPort.SelectedItem != null)
            {
                serialPort_YerIst.PortName = comboBoxComPort.SelectedItem.ToString();
                btnBaglan.Enabled = false;
                btnBaglantiyiBitir.Enabled = true;
            }
            else
            {
                FetchAvailablePorts();
                MessageBox.Show("COM PORT'u Yeniden Seciniz ");
                btnBaglan.Enabled = true;
                btnBaglantiyiBitir.Enabled = false;
            }
            serialPort_YerIst.BaudRate = (int)numericUpDownBaudRate.Value;
            serialPort_YerIst.Parity = Parity.None;
            serialPort_YerIst.DataBits = 8;
            serialPort_YerIst.StopBits = StopBits.One;
            serialPort_YerIst.ReadBufferSize = 200000000;
            serialPort_YerIst.DataReceived += serialPort_DataReceived;

            try
            {
                serialPort_YerIst.Open();
                Thread.Sleep(1000);
                lblDurum.ForeColor = Color.Green;
                lblDurum.Text = "Baglandi!";
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
                if (serialPort_YerIst.IsOpen)
                {
                    recivedData = serialPort_YerIst.ReadLine();
                    this.Invoke(new Action(ProcessingData));
                    this.Invoke(new Action(ValueUpdate));
                }
            }
            catch (Exception) { }

        }
        private void ProcessingData()
        {
            try { values = recivedData.Split(','); }
            catch (Exception) { for (int i = 0; i < DATA_COUNT; i++) values[i] = "-1"; }
        }
        private void ValueUpdate()
        {
            //burasi her data geldiginde calisir
            Thread.Sleep(100); //will sleep for 100ms
            //TODO: burda degerleri guncelle 
            textBoxPaketNum_AnaBil.Text = GetPinValue(PackageElements.packageNum_Ana);
            textBoxAci_AnaBil.Text = GetPinValue(PackageElements.Aci_Ana);
            textBoxBasincIrtifa_AnaBil.Text = GetPinValue(PackageElements.Irtifa_basinc_Ana);
            textBoxGPSIrtifa_AnaBil.Text = GetPinValue(PackageElements.GPSIrtifa_Ana);

            textBoxBasinc_AnaBil.Text = GetPinValue(PackageElements.basinc_Ana);
            textBoxGPSEnlem_AnaBil.Text = GetPinValue(PackageElements.GPSe_Ana);
            textBoxGPSBoylam_AnaBil.Text = GetPinValue(PackageElements.GPSb_Ana);
            textBoxJiroX_AnaBil.Text = GetPinValue(PackageElements.X_jiro);
            textBoxJiroY_AnaBil.Text = GetPinValue(PackageElements.Y_jiro);
            textBoxJiroZ_AnaBil.Text = GetPinValue(PackageElements.Z_jiro);
            textBoxIvmeX_AnaBil.Text = GetPinValue(PackageElements.X_ivme);
            textBoxIvmeY_AnaBil.Text = GetPinValue(PackageElements.Y_ivme);
            textBoxIvmeZ_AnaBil.Text = GetPinValue(PackageElements.Z_ivme);
            textBoxGPSSatNum_AnaBil.Text = GetPinValue(PackageElements.GPSSatNum_ana);

            y_angle_3d_model = (float)(GetPinValueFloat(PackageElements.Aci_Ana));

            textBoxPaketNum_GorevYuku.Text = GetPinValue(PackageElements.packageNum_Gorev);
            textBoxSicaklik_GorevYuku.Text = GetPinValue(PackageElements.sicaklik_Gorev);
            textBoxNem_GorevYuku.Text = GetPinValue(PackageElements.nem_Gorev);
            textBoxBasinc_GorevYuku.Text = GetPinValue(PackageElements.basinc_Gorev);
            textBoxGPSEnlem_GorevYuku.Text = GetPinValue(PackageElements.GPSe_Gorev);
            textBoxGPSBoylam_GorevYuku.Text = GetPinValue(PackageElements.GPSb_Gorev);
            textBoxGPSIrtifa_GorevYuku.Text = GetPinValue(PackageElements.GPSIrtifa_Gorev);
            textBoxGPSSatNum_GorevYuku.Text = GetPinValue(PackageElements.GPSSatNum_Gorev);

            UpdateChart(chartIrtifa_Ana, GetPinValueFloat(PackageElements.GPSIrtifa_Ana));
            UpdateChart(chartIrtifa_Gorev, GetPinValueFloat(PackageElements.GPSIrtifa_Gorev));
            UpdateChart(chartBasinc_Ana, GetPinValueFloat(PackageElements.basinc_Ana));
            UpdateChart(chartBasinc_Gorev, GetPinValueFloat(PackageElements.basinc_Gorev));
            SetStatus();
            switch (PatlamaToByte())
            {
                case 0:
                    patlama1_no.Visible = true;
                    patlama1_yes.Visible = false;
                    patlama2_no.Visible = true;
                    patlama2_yes.Visible = false;
                    break; 
                case 1: 
                    patlama1_no.Visible = true;
                    patlama1_yes.Visible = false;
                    patlama2_no.Visible = false;
                    patlama2_yes.Visible = true;
                    break; 
                case 2:
                    patlama1_no.Visible = false;
                    patlama1_yes.Visible = true; 
                    patlama2_no.Visible = true;
                    patlama2_yes.Visible = false;
                    break; 
                case 3:
                    patlama1_no.Visible = false;
                    patlama1_yes.Visible = true;
                    patlama2_no.Visible = false;
                    patlama2_yes.Visible = true;
                    break;
            }
 
            if (SerialPort_HYI.IsOpen)
                SerialPort_HYI_dataSend();
            try
            {
                glControl1.Invalidate();
            }
            catch (Exception ex) { MessageBox.Show(ex.Message.ToString(), "Error While Drawing GLControl"); }

            try
            {
                RefreshMapToNewGPS();
            }
            catch (Exception ex) { MessageBox.Show(ex.Message.ToString(), "Error While Refreshing GPS"); }
        }
        private void btnBaglantiyiBitir_Click(object sender, EventArgs e)
        {
            if (serialPort_YerIst.IsOpen)
            {
                serialPort_YerIst.Close();
                btnBaglan.Enabled = true;
                btnBaglantiyiBitir.Enabled = false;
                lblDurum.ForeColor = Color.Red;
                lblDurum.Text = "Baglanti Yok!";
            }
            else
            {
                btnBaglan.Enabled = true;
                btnBaglantiyiBitir.Enabled = false;
                lblDurum.Text = "Cihaz Baglantisi Koptu!";
                lblDurum.ForeColor = Color.Red;
            }
        }
        //HYI 
        private void btnHakemIletisim_Click(object sender, EventArgs e)
        {
            if (HYIDataSendThread == null)
            {
                using (HYI hyi = new HYI())
                {
                    if (hyi.ShowDialog() == DialogResult.OK)
                    {
                        HYI_Port = hyi.HYI_Port();
                        HYI_BaudRate = hyi.HYI_BaudRate();
                        HYI_DataBits = hyi.HYI_DataBits();
                        HYI_StopBits = hyi.HYI_StopBits();
                        HYI_Parity = hyi.HYI_Parity();
                        HYI_Baglan();
                    }
                }
            }
            else
            {
                if (SerialPort_HYI.IsOpen)
                {
                    SerialPort_HYI.Close();
                    MessageBox.Show("Bağlantı Sonlandırılıyor");
                }
                lblHakemDurum.Text = "Bağlantı Sonlandırıldı!";
                lblHakemDurum.ForeColor = Color.White;
                btnHakemIletisim.Text = "Hakem Yer Istasyonuna Baglan";
                HYIDataSendThread?.Abort();
                HYIDataSendThread = null;
            }
        }
        public void HYI_Baglan()
        {
            SerialPort_HYI = new SerialPort();
            if (HYI_Port != null && HYI_Port != "")
            {
                try
                {
                    SerialPort_HYI.PortName = HYI_Port;
                    SerialPort_HYI.BaudRate = HYI_BaudRate;
                    SerialPort_HYI.Parity = (Parity)HYI_Parity;
                    SerialPort_HYI.DataBits = HYI_DataBits;
                    SerialPort_HYI.StopBits = (StopBits)HYI_StopBits;
                    SerialPort_HYI.ReadBufferSize = 200000000;
                }
                catch { }
            }
            else
            {
                MessageBox.Show("Port Secmediniz");
                lblHakemDurum.ForeColor = Color.Red;
                lblHakemDurum.Text = "Baglanti Kurulamadi";
            }

            try
            {
                SerialPort_HYI.Open();
                btnHakemIletisim.Enabled = false;
                Thread.Sleep(1000);
                if (SerialPort_HYI.IsOpen)
                {
                    lblHakemDurum.ForeColor = Color.Green;
                    lblHakemDurum.Text = "Baglandi!";
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error occured while opening port");
                FetchAvailablePorts();
                lblHakemDurum.ForeColor = Color.Red;
                lblHakemDurum.Text = "Baglanti Kurulamadi";
            }
        }
        private void SerialPort_HYI_dataSend()
        {
            try
            {
                if (HYIDataSendThread == null)
                {
                    HYIDataSendThread = new Thread(() =>
                    {
                        byte[] paketstr = new byte[78];
                        while (SerialPort_HYI.IsOpen)
                        {
                            try
                            {
                                paketstr = PaketOlustur();
                                SerialPort_HYI.Write(paketstr, 0, paketstr.Length);
                                Thread.Sleep(250);

                            }
                            catch (Exception ex)
                            {
                                Console.WriteLine(ex.StackTrace);
                            }
                        }
                    });
                    HYIDataSendThread.Start();
                    btnHakemIletisim.Text = "Hakem Yer Istasyonu Baglantısını Kes";
                    Thread.Sleep(1000);
                    btnHakemIletisim.Enabled = true;
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show("HYI Paket Gonderilemedi. Bağlantı Sonlandırılıyor\n" + ex);
                lblHakemDurum.Text = "Bağlantı Kesildi";
                lblHakemDurum.ForeColor = Color.Red;
                SerialPort_HYI.Close();
                HYIDataSendThread?.Abort();
                HYIDataSendThread = null;
            }

        }

        #endregion


        Thread HYIDataSendThread = null;
        Thread FileSaveThread = null;

        #region EVENTS


        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            try
            {
                if (serialPort_YerIst.IsOpen)
                {
                    MessageBox.Show("Yer Istasyonu COM PORT baglantisini kapatmadiniz! Otomatik sonlandiriliyor...");
                    serialPort_YerIst.Close();
                }
                if (SerialPort_HYI.IsOpen)
                {
                    MessageBox.Show("Hakem Istasyonu COM PORT baglantisini kapatmadiniz! Otomatik sonlandiriliyor...");
                    SerialPort_HYI.Close();
                }
            }
            catch (Exception ex) { MessageBox.Show("" + ex); }
            FileSaveThread?.Abort();
            HYIDataSendThread?.Abort();
        }
        private void Form1_Load(object sender, EventArgs e)
        {
            gMapAnaBilgisayar.DragButton = MouseButtons.Left;
            gMapAnaBilgisayar.MouseWheelZoomEnabled = true;
            gMapAnaBilgisayar.MapProvider = GMapProviders.GoogleMap;
            gMapAnaBilgisayar.Position = new PointLatLng(GetPinValueDouble(PackageElements.GPSe_Ana), GetPinValueDouble(PackageElements.GPSb_Ana));
            gMapAnaBilgisayar.MinZoom = 1;
            gMapAnaBilgisayar.MaxZoom = 100;
            gMapAnaBilgisayar.Zoom = mapZoomCount;

            gMapGorevYuku.DragButton = MouseButtons.Left;
            gMapGorevYuku.MouseWheelZoomEnabled = true;
            gMapGorevYuku.MapProvider = GMapProviders.GoogleMap;
            gMapGorevYuku.Position = new PointLatLng(GetPinValueDouble(PackageElements.GPSe_Gorev), GetPinValueDouble(PackageElements.GPSb_Gorev));
            gMapGorevYuku.MinZoom = 1;
            gMapGorevYuku.MaxZoom = 100;
            gMapGorevYuku.Zoom = mapZoomCount;

            patlama1_no.Visible = true;
            patlama1_yes.Visible = false;
            patlama2_no.Visible = true;
            patlama2_yes.Visible = false;
            SetChartAtBegin(chartBasinc_Ana, "Basınç");
            SetChartAtBegin(chartBasinc_Gorev, "Basınç");
            SetChartAtBegin(chartIrtifa_Ana, "GPS İrtifa");
            SetChartAtBegin(chartIrtifa_Gorev, "GPS İrtifa");
        }
        private void btnRefresh_Click(object sender, EventArgs e)
        {
            FetchAvailablePorts();
            glControl1.Invalidate();
            gMapAnaBilgisayar.Position = new PointLatLng(39.9103241f, 32.8529681f);
            gMapAnaBilgisayar.Zoom = mapZoomCount;
            gMapGorevYuku.Position = new PointLatLng(39.9103241f, 32.8529681f);
            gMapGorevYuku.Zoom = mapZoomCount;

            patlama1_no.Visible = true;
            patlama1_yes.Visible = false;
            patlama2_no.Visible = true;
            patlama2_yes.Visible = false;
        }
        private void btnDosyayaKaydet_Click(object sender, EventArgs e)
        {
            if (FileSaveThread == null)
            {
                lblDosyaKayit.Text = "Kayit Ediliyor!";
                FileSaveThread = new Thread(() =>
                {
                    while (true)
                    {
                        if (!File.Exists(filenameRoket))
                        {
                            File.WriteAllText(filenameRoket, dataTitles + "\n" + recivedData + "\n");
                        }
                        else
                        {
                            File.AppendAllText(filenameRoket, recivedData + "\n");
                        }
                        Thread.Sleep(100);
                    }
                }
                );
                FileSaveThread.Start();
                lblDosyaKayit.Text = "Kayit Ediliyor!";
                btnDosyayaKaydet.Text = "Kaydi Bitir";
            }
            else
            {
                FileSaveThread.Abort();
                FileSaveThread = null;
                lblDosyaKayit.Text = "Kayit Edilmiyor!";
                btnDosyayaKaydet.Text = "Dosyaya Kaydetmeye Basla";
            }
        }

        #endregion


        #region EK FONKSIYONLAR

        private string GetPinValue(PackageElements pe)
        {
            return values[(int)pe];
        }
        private float GetPinValueFloat(PackageElements pe)
        {
            return float.Parse(GetPinValue(pe).IndexOf(".") >= 0 ? GetPinValue(pe).Replace(".", ",") : GetPinValue(pe));
        }
        private Double GetPinValueDouble(PackageElements pe)
        {
            try { return Double.Parse(GetPinValue(pe)); }
            catch { return Double.NaN; }
        }
        private byte PatlamaToByte()
        {
            if (!GetBit(0,(byte)GetPinValueFloat(PackageElements.statusAna))  && !GetBit(1, (byte)GetPinValueFloat(PackageElements.statusAna))) return 0x00;
            else if (!GetBit(0, (byte)GetPinValueFloat(PackageElements.statusAna)) && GetBit(1, (byte)GetPinValueFloat(PackageElements.statusAna))) return 0x01;
            else if (GetBit(0, (byte)GetPinValueFloat(PackageElements.statusAna)) && !GetBit(1, (byte)GetPinValueFloat(PackageElements.statusAna))) return 0x02;
            else if (GetBit(0, (byte)GetPinValueFloat(PackageElements.statusAna)) && GetBit(1, (byte)GetPinValueFloat(PackageElements.statusAna))) return 0x03;
            else return 0x00;
        }
        void SetStatus()
        {
            checkBoxGPS.Checked = GetBit(2, (byte)GetPinValueFloat(PackageElements.statusAna))|| GetBit(6, (byte)GetPinValueFloat(PackageElements.statusAna)) || GetBit(7, (byte)GetPinValueFloat(PackageElements.statusAna));
            checkBoxBME.Checked = GetBit(3, (byte)GetPinValueFloat(PackageElements.statusAna))    ;
            checkBoxbBNO.Checked= GetBit(4, (byte)GetPinValueFloat(PackageElements.statusAna))   ;
            checkBoxSD.Checked =  GetBit (5,   (byte)GetPinValueFloat(PackageElements.statusAna));

            checkBoxGps_gorev.Checked = GetBit(2,  (byte)GetPinValueFloat(PackageElements.statusGorev)) || GetBit(6, (byte)GetPinValueFloat(PackageElements.statusGorev)) || GetBit(7, (byte)GetPinValueFloat(PackageElements.statusGorev));
            checkBoxBME_gorev.Checked = GetBit(3,  (byte)GetPinValueFloat(PackageElements.statusGorev));
            checkBoxSD_gorev.Checked = GetBit(5,   (byte)GetPinValueFloat(PackageElements.statusGorev));
            Console.WriteLine(GetPinValueFloat(PackageElements.statusAna));
            Console.WriteLine(GetBit(0, (byte)GetPinValueFloat(PackageElements.statusAna)) + " " + GetBit(1, (byte)GetPinValueFloat(PackageElements.statusAna)) + " "+GetBit(2, (byte)GetPinValueFloat(PackageElements.statusAna))+" "+ GetBit(3, (byte)GetPinValueFloat(PackageElements.statusAna)) + " " + GetBit(4, (byte)GetPinValueFloat(PackageElements.statusAna)) + " " + GetBit(5, (byte)GetPinValueFloat(PackageElements.statusAna)) + " " + GetBit(6, (byte)GetPinValueFloat(PackageElements.statusAna)) + " " + GetBit(7, (byte)GetPinValueFloat(PackageElements.statusAna)) + " ");
        }
        public bool GetBit( byte bitNumber,byte b ) =>  ( b & (1 << bitNumber)) != 0;
 
        private void RefreshMapToNewGPS()
        {
            UpdateMap(gMapAnaBilgisayar, (double)GetPinValueDouble(PackageElements.GPSe_Ana), (double)GetPinValueDouble(PackageElements.GPSb_Ana));
            gMapAnaBilgisayar.ShowCenter = true;
            UpdateMap(gMapGorevYuku, (double)GetPinValueDouble(PackageElements.GPSe_Gorev), (double)GetPinValueDouble(PackageElements.GPSb_Gorev));
            gMapGorevYuku.ShowCenter = true;
        }

        List<PointLatLng> points_ana = new List<PointLatLng>();
        List<PointLatLng> points_gorev = new List<PointLatLng>();

        PointLatLng previousPoint_ana;
        PointLatLng previousPoint_gorev;

        private void UpdateMap(GMapControl gMapControl, double latitude, double longitude)
        {
            if (latitude != 0 && longitude != 0)
            {
                PointLatLng point = new PointLatLng(latitude, longitude);
                GMapMarker marker = new GMarkerGoogle(point, GMarkerGoogleType.red_dot);
                gMapControl.Overlays.Clear();
                GMapOverlay overlay = new GMapOverlay("markers");
                overlay.Markers.Add(marker);
                gMapControl.Overlays.Add(overlay);
                gMapControl.Position = point;


                if (gMapControl == gMapAnaBilgisayar)
                {
                    if (previousPoint_ana != null)
                    {
                        points_ana.Add(previousPoint_ana);
                        points_ana.Add(point);
                        GMapRoute route = new GMapRoute(points_ana, "route");
                        overlay.Routes.Add(route);
                    }
                    previousPoint_ana = point;
                }
                else if (gMapControl == gMapGorevYuku)
                {
                    if (previousPoint_gorev != null)
                    {
                        points_gorev.Add(previousPoint_gorev);
                        points_gorev.Add(point);
                        GMapRoute route = new GMapRoute(points_gorev, "route");
                        overlay.Routes.Add(route);
                    }
                    previousPoint_gorev = point;
                }
            }
        }
        private void UpdateChart(Chart chart1, double value)
        {
            // Yeni veri noktasını ekle
            chart1.Series[0].Points.AddY(value);

            // Veri noktalarının sayısını sınırla
            if (chart1.Series[0].Points.Count > 100)
            {
                chart1.Series[0].Points.RemoveAt(0);
            }

            // Grafik kontrolünü yeniden çiz
            chart1.Invalidate();
        }

        private void SetChartAtBegin(Chart chart1, String AxisY)
        {

            chart1.Series[0].BorderWidth = 2;

            chart1.Series[0].MarkerStyle = MarkerStyle.Square;
            chart1.Series[0].MarkerColor = Color.Green;

            chart1.ChartAreas[0].AxisX.Title = "Zaman";
            chart1.ChartAreas[0].AxisY.Title = AxisY;
            chart1.Series[0].ChartType = SeriesChartType.Line;
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

        private void glControl1_Paint(object sender, PaintEventArgs e)
        {
            GL.Clear(ClearBufferMask.ColorBufferBit);
            GL.Clear(ClearBufferMask.DepthBufferBit);

            Matrix4 perspective = Matrix4.CreatePerspectiveFieldOfView(1.04f, glControl1.AspectRatio, 0.1f, 100);
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

            GL.BindBuffer(BufferTarget.ArrayBuffer, VertexBufferObject);
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

            location = GL.GetUniformLocation(ShaderObject, "light.direction");
            GL.Uniform3(location, light.direction);
            location = GL.GetUniformLocation(ShaderObject, "light.ambient");
            GL.Uniform3(location, light.ambient);
            location = GL.GetUniformLocation(ShaderObject, "light.diffuse");
            GL.Uniform3(location, light.diffuse);
            location = GL.GetUniformLocation(ShaderObject, "light.specular");
            GL.Uniform3(location, light.specular);
            location = GL.GetUniformLocation(ShaderObject, "light.intensity");
            GL.Uniform1(location, light.intensity);

            location = GL.GetUniformLocation(ShaderObject, "camPos");
            GL.Uniform3(location, new Vector3(0, 0, 8));

            GL.DrawArrays(BeginMode.Triangles, 0, vertexCount);
            GL.BindBuffer(BufferTarget.ArrayBuffer, 0);
            GL.BindVertexArray(0);
            GL.UseProgram(0);

            glControl1.SwapBuffers();
        }


        private void glControl1_Load(object sender, EventArgs e)
        {
            GL.ClearColor(0, 0, 0, 0);
            GL.Enable(EnableCap.DepthTest);
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

            Console.WriteLine("vertices : " + vertices.Count);
            Console.WriteLine("normals  : " + normals.Count);
            Console.WriteLine("texCoords: " + texCoords.Count);
            Console.WriteLine("faces    : " + faces.Count);

        }
        #endregion


        #region 78 BIT DONUSTURUCU

        public byte CheckSumHesapla(byte[] o)
        {
            byte checkSum = 0;
            for (int i = 4; i < 75; i++)
            {
                checkSum += o[i];
            }
            return checkSum;
        }

        byte[] byteArray = new byte[4];
        public byte[] PaketOlustur()
        {
            byte[] olusturalacak_paket = new byte[78];
            olusturalacak_paket[0] = 0xFF; // Sabit
            olusturalacak_paket[1] = 0xFF; // Sabit
            olusturalacak_paket[2] = 0x54; // Sabit
            olusturalacak_paket[3] = 0x52; // Sabit
            olusturalacak_paket[4] = (byte)Takim_ID; // Takim ID =
            olusturalacak_paket[5] = paketNumarasıHYI; // Sayac degeri = 0; 

            byteArray =GetBytesFromFloat(GetPinValueFloat(PackageElements.Irtifa_basinc_Ana));
            olusturalacak_paket[6] = byteArray[0];
            olusturalacak_paket[7] = byteArray[1];
            olusturalacak_paket[8] = byteArray[2];
            olusturalacak_paket[9] = byteArray[3];

            byteArray =GetBytesFromFloat(GetPinValueFloat(PackageElements.GPSIrtifa_Ana));
            olusturalacak_paket[10] = byteArray[0];
            olusturalacak_paket[11] = byteArray[1];
            olusturalacak_paket[12] = byteArray[2];
            olusturalacak_paket[13] = byteArray[3];

            byteArray =GetBytesFromFloat(GetPinValueFloat(PackageElements.GPSe_Ana));
            olusturalacak_paket[14] = byteArray[0];
            olusturalacak_paket[15] = byteArray[1];
            olusturalacak_paket[16] = byteArray[2];
            olusturalacak_paket[17] = byteArray[3];

            byteArray =GetBytesFromFloat(GetPinValueFloat(PackageElements.GPSb_Ana));
            olusturalacak_paket[18] = byteArray[0];
            olusturalacak_paket[19] = byteArray[1];
            olusturalacak_paket[20] = byteArray[2];
            olusturalacak_paket[21] = byteArray[3];

            byteArray =GetBytesFromFloat(GetPinValueFloat(PackageElements.GPSIrtifa_Gorev));
            olusturalacak_paket[22] = byteArray[0];
            olusturalacak_paket[23] = byteArray[1];
            olusturalacak_paket[24] = byteArray[2];
            olusturalacak_paket[25] = byteArray[3];

            byteArray =GetBytesFromFloat(GetPinValueFloat(PackageElements.GPSe_Gorev));
            olusturalacak_paket[26] = byteArray[0];
            olusturalacak_paket[27] = byteArray[1];
            olusturalacak_paket[28] = byteArray[2];
            olusturalacak_paket[29] = byteArray[3];

            byteArray =GetBytesFromFloat(GetPinValueFloat(PackageElements.GPSb_Gorev));
            olusturalacak_paket[30] = byteArray[0];
            olusturalacak_paket[31] = byteArray[1];
            olusturalacak_paket[32] = byteArray[2];
            olusturalacak_paket[33] = byteArray[3];

            olusturalacak_paket[34] = 0x00;
            olusturalacak_paket[35] = 0x00;
            olusturalacak_paket[36] = 0x00;
            olusturalacak_paket[37] = 0x00;
            olusturalacak_paket[38] = 0x00;
            olusturalacak_paket[39] = 0x00;
            olusturalacak_paket[40] = 0x00;
            olusturalacak_paket[41] = 0x00;
            olusturalacak_paket[42] = 0x00;
            olusturalacak_paket[43] = 0x00;
            olusturalacak_paket[44] = 0x00;
            olusturalacak_paket[45] = 0x00;

            byteArray =GetBytesFromFloat(GetPinValueFloat(PackageElements.X_jiro));
            olusturalacak_paket[46] = byteArray[0];
            olusturalacak_paket[47] = byteArray[1];
            olusturalacak_paket[48] = byteArray[2];
            olusturalacak_paket[49] = byteArray[3];

            byteArray =GetBytesFromFloat(GetPinValueFloat(PackageElements.Y_jiro));
            olusturalacak_paket[50] = byteArray[0];
            olusturalacak_paket[51] = byteArray[1];
            olusturalacak_paket[52] = byteArray[2];
            olusturalacak_paket[53] = byteArray[3];

            byteArray =GetBytesFromFloat(GetPinValueFloat(PackageElements.Z_jiro));
            olusturalacak_paket[54] = byteArray[0];
            olusturalacak_paket[55] = byteArray[1];
            olusturalacak_paket[56] = byteArray[2];
            olusturalacak_paket[57] = byteArray[3];

            byteArray =GetBytesFromFloat(GetPinValueFloat(PackageElements.X_ivme));
            olusturalacak_paket[58] = byteArray[0]; 
            olusturalacak_paket[59] = byteArray[1]; 
            olusturalacak_paket[60] = byteArray[2]; 
            olusturalacak_paket[61] = byteArray[3]; 

            byteArray = GetBytesFromFloat(GetPinValueFloat(PackageElements.Y_ivme));
            olusturalacak_paket[62] = byteArray[0];
            olusturalacak_paket[63] = byteArray[1];
            olusturalacak_paket[64] = byteArray[2];
            olusturalacak_paket[65] = byteArray[3];


            byteArray = GetBytesFromFloat(GetPinValueFloat(PackageElements.Z_ivme));
            olusturalacak_paket[66] = byteArray[0];
            olusturalacak_paket[67] = byteArray[1];
            olusturalacak_paket[68] = byteArray[2];
            olusturalacak_paket[69] = byteArray[3];

            byteArray = GetBytesFromFloat(GetPinValueFloat(PackageElements.Aci_Ana));
            olusturalacak_paket[70] = byteArray[0];
            olusturalacak_paket[71] = byteArray[1];
            olusturalacak_paket[72] = byteArray[2];
            olusturalacak_paket[73] = byteArray[3];

            olusturalacak_paket[74] = PatlamaToByte();
            olusturalacak_paket[75] = CheckSumHesapla(olusturalacak_paket);
            olusturalacak_paket[76] = 0x0D; // Sabit
            olusturalacak_paket[77] = 0x0A; // Sabit

            paketNumarasıHYI++;
            for (int i = 0; i < 78; i++)
            {
                olusturalacak_paket[i] = Convert.ToByte(string.Format("0x{0:X2}", olusturalacak_paket[i]), 16);
            }
            return olusturalacak_paket;
        }
        public static byte[] GetBytesFromFloat(float value)
        {
            byte[] bytes = new byte[4];
            int intValue = BitConverter.ToInt32(BitConverter.GetBytes(value), 0);
            bytes[0] = (byte)(intValue & 0xff);
            bytes[1] = (byte)((intValue >> 8) & 0xff);
            bytes[2] = (byte)((intValue >> 16) & 0xff);
            bytes[3] = (byte)((intValue >> 24) & 0xff);
            return bytes;
        }
        #endregion
    }
}
