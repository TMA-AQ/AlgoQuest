using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Configuration.Install;
using System.Linq;
using System.IO;
using System.Collections.Specialized;
using System.Diagnostics;
using System.Configuration;


namespace AlgoQuestEnterpriseManagerCustomActionsSetup
{
    [RunInstaller(true)]
    public partial class IniPropertiesGenerate : System.Configuration.Install.Installer
    {
        public IniPropertiesGenerate()
        {
            InitializeComponent();
        }

        public override void Install(IDictionary stateSaver)
        {
            base.Install(stateSaver);
            writeIniParameters();
            runSentinel();
        }

        public override void Commit(IDictionary savedState)
        {
            base.Commit(savedState);
        }

        private void writeIniParameters()
        {
            StreamReader sr = new StreamReader(Path.Combine(Context.Parameters["TARGETDIR"],"Bin", "ini.properties"));
            string parameters = sr.ReadToEnd();
            sr.Close();
            sr.Dispose();
            AppSettingsReader _appReader = new AppSettingsReader();
            String dbPath = _appReader.GetValue("DataBasePath", typeof(System.String)).ToString();
            string resultparameters = parameters.Replace("[PATH]", dbPath);
            resultparameters = resultparameters.Replace("\\", "\\\\");
            resultparameters = resultparameters.Replace("\\\\\\\\", "\\\\");
            resultparameters = resultparameters.Replace("\\\\\\", "\\\\");
            StreamWriter sw = new StreamWriter(Path.Combine(Context.Parameters["TARGETDIR"], "DB", "ini.properties"), false);
            sw.Write(resultparameters);
            sw.Flush();
            sw.Close();
            sw.Dispose();
        }

        private void runSentinel()
        {
            ProcessStartInfo pInfo = new ProcessStartInfo();
            pInfo.CreateNoWindow = true;
            pInfo.UseShellExecute = true;
            pInfo.FileName = Path.Combine(Context.Parameters["TARGETDIR"], "Bin", "SentinelInstall.bat");
            Process p = System.Diagnostics.Process.Start(pInfo);
            p.WaitForExit();
        }
    }
}
