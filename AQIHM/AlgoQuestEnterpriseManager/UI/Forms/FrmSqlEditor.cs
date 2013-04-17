using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using gudusoft.gsqlparser;
using gudusoft.gsqlparser.Units;
using AlgoQuest.Core.DatabaseManagement;
using AlgoQuest.Core.Compute;
using AlgoQuest.Configuration.Core;
using System.Diagnostics;
using System.Threading;
using System.IO;
using System.Configuration;

namespace AlgoQuest.UI.Forms
{
    public partial class FrmSqlEditor : Form
    {
        public class ConnectAttr
        {
            public enum type
            {
                LOCAL,
                REMOTE,
                ODBC
            }
            public type _type;
            public string _server;
            public UInt16 _port;
            public string _database;
            public string _odbc_conn_str;
        }

        private ConnectAttr _attr;
        private List<RequestData> _requestList;
        private string _pathFile;
        private string _multiThreadResult = string.Empty;
        private Request _requests;

        public FrmSqlEditor(ConnectAttr attr)
        {
            InitializeComponent();
            _attr = attr;
            this.Text = String.Format("Analyseur de requêtes - {0}", attr._database);
            if (_attr._type == FrmSqlEditor.ConnectAttr.type.LOCAL)
            {
                populateRequestList();
                scSql.Panel2.ClientSizeChanged += new EventHandler(Panel2_ClientSizeChanged);
            }
        }

        public FrmSqlEditor(string SelectedBase, string scriptFilePath)
        {
            InitializeComponent();
            _attr._database = SelectedBase;
            this.Text = String.Format("Analyseur de requêtes - {0}", _attr._database);
            populateRequestList();
            try
            {
                using (StreamReader sr = new StreamReader(scriptFilePath))
                {
                    rtbEditor.Text = sr.ReadToEnd();
                    analyseRequest();
                }
            }
            catch
            {
                MessageBox.Show("Le fichier sélectionné n'a pas pu être chargé.", "Erreur", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            
        }

        private void populateRequestList()
        {
            tlcRequest.Items.Clear();
            AppSettingsReader _appReader = new AppSettingsReader();
            String scriptPath = _appReader.GetValue("ScriptPath", typeof(System.String)).ToString();
            _requests = new Request(scriptPath, _attr._database);
            _requestList = _requests.GetRequest();
            foreach (RequestData rd in _requestList)
                tlcRequest.Items.Add(rd);
        }

        private Color setWordColor(string word)
        {
            Color color = Color.Empty;
            if (string.Compare(word,"toto",true)==0)
                color = Color.Red;
            return color;
        }

        private void analyseRequest()
        {
            Cursor.Current = Cursors.WaitCursor;

            //for more format options, please check document

            lzbasetype.gFmtOpt.Select_Columnlist_Style = TAlignStyle.asStacked;
            lzbasetype.gFmtOpt.Select_Columnlist_Comma = TLinefeedsCommaOption.lfAfterComma;
            lzbasetype.gFmtOpt.SelectItemInNewLine = false;
            lzbasetype.gFmtOpt.AlignAliasInSelectList = true;
            lzbasetype.gFmtOpt.TreatDistinctAsVirtualColumn = false;

            //setup more format options ...

            lzbasetype.gFmtOpt.linenumber_enabled = false;

            lzbasetype.gFmtOpt.HighlightingFontname = "Courier New";
            lzbasetype.gFmtOpt.HighlightingFontsize = 10;

            //for other elements you want to customize, please check document
            lzbasetype.gFmtOpt.HighlightingElements[(int)TLzHighlightingElement.sfkIdentifer].SetForegroundInRGB("#008000");
            lzbasetype.gFmtOpt.HighlightingElements[(int)TLzHighlightingElement.sfkIdentifer].StyleBold = true;
            lzbasetype.gFmtOpt.HighlightingElements[(int)TLzHighlightingElement.sfkIdentifer].StyleItalic = false;
            lzbasetype.gFmtOpt.HighlightingElements[(int)TLzHighlightingElement.sfkIdentifer].StyleStrikeout = false;
            lzbasetype.gFmtOpt.HighlightingElements[(int)TLzHighlightingElement.sfkIdentifer].StyleUnderline = false;
            TGSqlParser parser = new TGSqlParser(TDbVendor.DbVOracle);
            parser.SqlText.Text = rtbEditor.Text;
            int i = parser.PrettyPrint();
            if (i == 0)
            {
                rtbEditor.Rtf = parser.ToRTF(TOutputFmt.ofrtf);
            }
            Cursor.Current = Cursors.Default;
        }

        private void tsbAnalyse_Click(object sender, EventArgs e)
        {
            analyseRequest();
        }

        private void tsbSave_Click(object sender, EventArgs e)
        {
            RequestData rd = new RequestData();
            if (tlcRequest.SelectedItem != null)
                rd = (RequestData)tlcRequest.SelectedItem;

            rd.Content = rtbEditor.Text;

            FrmSaveRequest frmSaveRequest = new FrmSaveRequest(rd, _requestList);
            DialogResult dr = frmSaveRequest.ShowDialog(this);
            if (dr == System.Windows.Forms.DialogResult.OK)
            {
                _requests.SaveRequest(rd);
                populateRequestList();
            }
        }

        private void tlcRequest_SelectedIndexChanged(object sender, EventArgs e)
        {
            rtbEditor.Text = ((RequestData)tlcRequest.SelectedItem).Content;
            tsbAnalyse_Click(sender, e);
            scSql.SplitterDistance = rtbEditor.Lines.Length * 20;
            tsbOpen.Enabled = false;
        }

        private void hideResult()
        {
            ssResult.Visible = false;
            tsslDuration.Visible = false;
            tsslNumber.Visible = false;
            tsbOpen.Enabled = false;
            dgResult.DataSource = null;
        }

        private int threadNumber(string request)
        {
            int threadNumber = 0;
            string[] tb = request.Split(new char[]{';'},StringSplitOptions.RemoveEmptyEntries);
            if (tb.Length > 1)
            {
                string command = tb[tb.Length - 1].Trim();
                if(command.Length>0)
                    if (command.Substring(0, 1) == "x")
                    {
                        string strNumber = command.Substring(1);
                        bool blNumber = int.TryParse(strNumber, out threadNumber);
                        if (!blNumber)
                            threadNumber = 0;
                    }
            }
            return threadNumber;
        }

        private void tsbExecute_Click(object sender, EventArgs e)
        {
            analyseRequest();
            hideResult();
            Stopwatch watch = new Stopwatch();

            AppSettingsReader _appReader = new AppSettingsReader();
            String dbPath = _appReader.GetValue("DataBasePath", typeof(System.String)).ToString();
            String cfgPath = _appReader.GetValue("ConfigPath", typeof(System.String)).ToString();
            Int32 nbRecordsToPrint = (Int32)_appReader.GetValue("NbRecordsToPrint", typeof(int));
            try
            {
                string request = rtbEditor.Text;
                int numThread = threadNumber(request);
                request = request.Substring(0, request.LastIndexOf(";")+1);
                ParameterizedThreadStart pThreadStart = new ParameterizedThreadStart(executeRequest);
                if (numThread > 0)
                {
                    this.Cursor = Cursors.WaitCursor;
                    _multiThreadResult = string.Empty;
                    Thread[] arrThread = new Thread[numThread];
                    for (int i = 0; i < arrThread.Length; i++)
                    {
                        pThreadStart = new ParameterizedThreadStart(executeRequestSingle);
                        arrThread[i] = new Thread(pThreadStart);
                        arrThread[i].Name = i.ToString();
                        arrThread[i].Start(request);
                    }
                    for (int i = 0; i < arrThread.Length; i++)
                    {
                        arrThread[i].Join();
                    }
                    MessageBox.Show(_multiThreadResult, String.Format("Résultats de l'exécution des {0} threads", arrThread.Length));
                    this.Cursor = Cursors.Default;
                }
                else
                {
                    Thread t1 = new Thread(pThreadStart);
                    t1.Start(request);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("L'exécution de la requête a échoué. \n" + ex.Message, "Erreur", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
            }
        }

        private void executeRequest(object state)
        {
            try
            {
                this.Invoke(new Action(() => { this.Cursor = Cursors.WaitCursor; }));
                toolStrip1.Invoke(new Action(() => { toolStrip1.Enabled = false; }));
                AppSettingsReader _appReader = new AppSettingsReader();
                String cfgPath = _appReader.GetValue("ConfigPath", typeof(System.String)).ToString();
                Int32 nbRecordsToPrint = (Int32)_appReader.GetValue("NbRecordsToPrint", typeof(int));
                string request = (string)state;
                Stopwatch watch = new Stopwatch();
                watch.Start();
                ISelectRequest sr;

                if ((_attr._odbc_conn_str != null) && (_attr._odbc_conn_str != ""))
                {
                    sr = new OdbcRequest(_attr._odbc_conn_str);
                }
                else if ((_attr._server != "") && (_attr._port != 0))
                {
                    sr = new SelectRequestRemote(_attr._server, _attr._port, _attr._database, nbRecordsToPrint);
                }
                else
                {
                    sr = new SelectRequest(cfgPath, _attr._database, nbRecordsToPrint);
                }

                System.Data.DataTable dt = sr.Execute(request);
                watch.Stop();
                ssResult.Invoke(new Action(() => { ssResult.Visible = true; }));

                ssResult.Invoke(new Action(() => { tsslDuration.Visible = true; }));
                ssResult.Invoke(new Action(() => { tsslNumber.Visible = true; }));

                scSql.Invoke(new Action(() => { scSql.Panel2Collapsed = false; }));
                scSql.Invoke(new Action(() => { scSql.SplitterDistance = rtbEditor.Lines.Length * 20; }));
                ssResult.Invoke(new Action(() =>
                {
                    tsslDuration.Text = String.Format("Durée d'exécution : {0}:{1}:{2}"
                        , watch.Elapsed.Hours.ToString().PadLeft(2, '0')
                        , watch.Elapsed.Minutes.ToString().PadLeft(2, '0')
                        , watch.Elapsed.Seconds.ToString().PadLeft(2, '0')
                        );
                }));
                ssResult.Invoke(new Action(() => { tsslNumber.Text = String.Format("Nombre d'enregistrements : {0})", dt.Rows.Count.ToString()); }));
                dgResult.Invoke(new Action(() => { dgResult.DataSource = dt; }));
                _pathFile = sr.PathFile;
                if (!string.IsNullOrEmpty(_pathFile))
                    toolStrip1.Invoke(new Action(() => { tsbOpen.Enabled = true; }));
            }
            catch (Exception ex)
            {
                this.Invoke(new Action(() =>
                { MessageBox.Show("L'exécution de la requête a échoué. \n" + ex.Message, "Erreur", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1); }
                ));
            }
            toolStrip1.Invoke(new Action(() => { toolStrip1.Enabled = true; }));
            this.Invoke(new Action(() => { this.Cursor = Cursors.Default; }));
        }

        private void executeRequestSingle(object state)
        {
            try
            {
                AppSettingsReader _appReader = new AppSettingsReader();
                String dbPath = _appReader.GetValue("DataBasePath", typeof(System.String)).ToString();
                Int32 nbRecordsToPrint = (Int32)_appReader.GetValue("NbRecordsToPrint", typeof(int));
                SelectRequest sr = new SelectRequest(dbPath, _attr._database, nbRecordsToPrint);
                string request = (string)state;
                Stopwatch watch = new Stopwatch();
                watch.Start();
                sr.ExecuteTest(request);
                watch.Stop();
                _multiThreadResult += String.Format("\nDurée d'exécution pour le thread {0} : {1}:{2}:{3}:{4}ms"
                        , Thread.CurrentThread.Name
                        , watch.Elapsed.Hours.ToString().PadLeft(2, '0')
                        , watch.Elapsed.Minutes.ToString().PadLeft(2, '0')
                        , watch.Elapsed.Seconds.ToString().PadLeft(2, '0')
                        , watch.Elapsed.Milliseconds.ToString().PadLeft(3, '0')
                        );
            }
            finally{}
        }

        private void tsbOpen_Click(object sender, EventArgs e)
        {
            try
            {
                Process.Start(_pathFile);
            }
            catch (Exception ex)
            {
                MessageBox.Show("L'ouverture du fichier de résultat a échoué : \n" + ex.Message, "Erreur", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        void Panel2_ClientSizeChanged(object sender, EventArgs e)
        {
            dgResult.MaximumSize = new Size(scSql.Panel2.ClientSize.Width, scSql.Panel2.ClientSize.Height - ssResult.Height);
        }

    }
}
