using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Microsoft.Win32;
using System.Diagnostics;

namespace ConfigTool
{
    public partial class Form1 : Form
    {
        RegistryKey configKey = Registry.LocalMachine.CreateSubKey(@"SOFTWARE\XInput.Emu");
        ProcessStartInfo service_command = new ProcessStartInfo("sc.exe");
        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            DialogResult msg = MessageBox.Show("To apply Changes, it's necesary to restart the service, Do you want to apply the changes?", "XInput.Emu Configuration Tool", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
            if (configKey != null && msg == DialogResult.Yes)
            {
                button1.Enabled = false;
                configKey.SetValue("DevHiding", checkBox1.Checked.ToString());
                configKey.SetValue("Sixaxis", checkBox2.Checked.ToString());
                configKey.SetValue("DualShock4", checkBox3.Checked.ToString());
                configKey.SetValue("DualSense", checkBox4.Checked.ToString());
                configKey.SetValue("JoyCons", checkBox5.Checked.ToString());
                configKey.SetValue("ProControllers", checkBox6.Checked.ToString());
                service_command.Arguments = "stop XInput.Emu";
                Process.Start(service_command).WaitForExit();
                service_command.Arguments = "start XInput.Emu";
                Process.Start(service_command).WaitForExit();
                button1.Enabled = true;
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            service_command.Verb = "runas";
            service_command.CreateNoWindow = true;
            service_command.UseShellExecute = false;
            if (configKey != null)
            {
                if (configKey.GetValue("DevHiding") != null) checkBox1.Checked = configKey.GetValue("DevHiding").ToString() == "True";
                else configKey.SetValue("DevHiding", checkBox1.Checked.ToString());
                if (configKey.GetValue("Sixaxis") != null) checkBox2.Checked = configKey.GetValue("Sixaxis").ToString() == "True";
                else configKey.SetValue("Sixaxis", checkBox2.Checked.ToString());
                if (configKey.GetValue("DualShock4") != null) checkBox3.Checked = configKey.GetValue("DualShock4").ToString() == "True";
                else configKey.SetValue("DualShock4", checkBox3.Checked.ToString());
                if (configKey.GetValue("DualSense") != null) checkBox4.Checked = configKey.GetValue("DualSense").ToString() == "True";
                else configKey.SetValue("DualSense", checkBox4.Checked.ToString());
                if (configKey.GetValue("JoyCons") != null) checkBox5.Checked = configKey.GetValue("JoyCons").ToString() == "True";
                else configKey.SetValue("JoyCons", checkBox5.Checked.ToString());
                if (configKey.GetValue("ProControllers") != null) checkBox6.Checked = configKey.GetValue("ProControllers").ToString() == "True";
                else configKey.SetValue("ProControllers", checkBox6.Checked.ToString());
            }
        }
    }
}
