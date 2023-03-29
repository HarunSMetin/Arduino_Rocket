using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TOBBETUROCKETRY
{
    public partial class HYI : Form
    {
        string[] ports = null; 
        public HYI()
        {
            InitializeComponent();
            btnOK.DialogResult = DialogResult.OK;
            btnCancel.DialogResult = DialogResult.Cancel;
        }

        private void HYI_Load(object sender, EventArgs e)
        {
            Refresh_Ports();
            comboBoxBaudRate    .SelectedIndex = 8;
            comboBoxDataBits    .SelectedIndex = 3;
            comboBoxStopBits    .SelectedIndex = 1;
            comboBoxParity      .SelectedIndex = 0;
            comboBoxFlowControl .SelectedIndex = 0;
        }
        public string HYI_Port()
        {
            if (ports == null | ports.Length == 0)
            {
                return "";
            }
            return comboBoxPort.Text;
        }
        public int HYI_BaudRate()
        {
            try
            {
                return Int32.Parse(comboBoxBaudRate.Text);
            }
            catch
            {
                return 19200;
            }
        }
        public int HYI_DataBits()
        {
            try
            {
                return Int32.Parse(comboBoxDataBits.Text);
            }
            catch
            {
                return 8;
            }
        }
        public int HYI_StopBits()
        {
            int selected = comboBoxStopBits.SelectedIndex;
            return (selected <= -1 ) ? 1 : selected;
        }
        public int HYI_Parity()
        {
            int selected = comboBoxParity.SelectedIndex;
            return selected < 0 ? 0 : selected; 
        }
        private void btnRefresh_Click(object sender, EventArgs e)
        {
            Refresh_Ports();
        } 
        
        public void Refresh_Ports()
        { 
            comboBoxPort.Items.Clear(); 
            ports = SerialPort.GetPortNames();
            comboBoxPort.Items.AddRange(ports);
            if (ports.Length > 0) comboBoxPort.SelectedIndex = comboBoxPort.Items.Count - 1;
            else comboBoxPort.Text = "Hakem Bagl. Yok!";
        }
 
    }
}
