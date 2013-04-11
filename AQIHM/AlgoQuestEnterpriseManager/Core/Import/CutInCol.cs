using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using AlgoQuest.Core.DatabaseManagement;
using AlgoQuest.Configuration.Core;
using System.IO;

namespace AlgoQuest.Core.Import
{
    public class CutInCol
    {
        private string _cutInColPath = string.Empty;
        private string _iniPropertiesPath = string.Empty;


        public CutInCol(DataBase db)
        {
            //récupération du chemin vers l'exécutable
            IniProperties ip = new IniProperties(db.IniPropertiesPath);
            _cutInColPath = ip.Keys["aq-tools"].ToString();
            _iniPropertiesPath = db.IniPropertiesPath;
        }

        public void Process(int numTable)
        {
            ProcessStartInfo pInfo = new ProcessStartInfo(_cutInColPath, "--aq-ini=\"" + _iniPropertiesPath + "\" --load-db --load-table=" + numTable);
            pInfo.RedirectStandardOutput = true;
            pInfo.UseShellExecute = false;
            pInfo.CreateNoWindow = true;
            Process p = System.Diagnostics.Process.Start(pInfo);
            p.WaitForExit();
        }

        public void Process(int numTable, int numColumn)
        {
            ProcessStartInfo pInfo = new ProcessStartInfo(_cutInColPath
                , "\"" + _iniPropertiesPath + "\" " + numTable.ToString() + " " + numColumn.ToString());
            pInfo.RedirectStandardOutput = true;
            pInfo.UseShellExecute = false;
            pInfo.CreateNoWindow = true;
            Process p = System.Diagnostics.Process.Start(pInfo);
            p.WaitForExit();
        }
    }
}
