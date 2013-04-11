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
        private string _server;
        private UInt16 _port;
        private string _selectedBase;
        private List<RequestData> _requestList;
        private string _pathFile;
        private string _odbc_conn_str;
        private string _multiThreadResult = string.Empty;

        public FrmSqlEditor(string SelectedBase)
        {
            InitializeComponent();
            _selectedBase = SelectedBase;
            string[] fields = SelectedBase.Split(new string[] { ":" }, StringSplitOptions.RemoveEmptyEntries);
            if (fields.Length > 1)
            {
                _server = fields[0];
                _port = UInt16.Parse(fields[1]);
                _selectedBase = fields[2];
            }
            this.Text = String.Format("Analyseur de requêtes - {0}", SelectedBase);
            populateRequestList();
            scSql.Panel2.ClientSizeChanged += new EventHandler(Panel2_ClientSizeChanged);

            // fill odbc connections
            try
            {
                AppSettingsReader _appReader = new AppSettingsReader();
                OdbcConnectionSection section = (OdbcConnectionSection)ConfigurationManager.GetSection("OdbcConnectionSection");
                if (section != null)
                {
                    List<OdbcConnectionElement> _listMapElement = section.MapItems.AsQueryable().Cast<OdbcConnectionElement>().ToList<OdbcConnectionElement>();
                    foreach (OdbcConnectionElement e in _listMapElement)
                    {
                        this.odbcCombo.Items.Add(e.value);
                    }
                }
            }
            catch (ConfigurationErrorsException)
            {
            }
        }

       

        public FrmSqlEditor(string SelectedBase, string scriptFilePath)
        {
            InitializeComponent();
            _selectedBase = SelectedBase;
            this.Text = String.Format("Analyseur de requêtes - {0}", _selectedBase);
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

        //private void rtbEditor_KeyPressed(object sender, KeyPressEventArgs e)
        //{
        //    int startIndex = 0;
        //    int lastSpacePosition = 0;
        //    lastSpacePosition = rtbEditor.Text.LastIndexOf(" ");
        //    if (lastSpacePosition > 0)
        //        startIndex = lastSpacePosition;

        //    string currentWord = string.Empty;
        //    if (rtbEditor.Text.Length > 0)
        //    {
        //        currentWord = rtbEditor.Text.Substring(startIndex).Trim();
        //        if (currentWord != string.Empty)
        //        {
        //            rtbEditor.Select(startIndex, rtbEditor.TextLength - startIndex);
        //            rtbEditor.SelectionColor = setWordColor(currentWord);
        //            rtbEditor.SelectionLength = 0;
        //            rtbEditor.SelectionStart = rtbEditor.TextLength;
        //        }
        //    }

        //}

        private void populateRequestList()
        {
            tlcRequest.Items.Clear();
            AppSettingsReader _appReader = new AppSettingsReader();
            String dbPath = _appReader.GetValue("DataBasePath", typeof(System.String)).ToString();
            Request rq = new Request(dbPath, _selectedBase);
            _requestList = rq.GetRequest();
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
                AppSettingsReader _appReader = new AppSettingsReader();
                String dbPath = _appReader.GetValue("DataBasePath", typeof(System.String)).ToString();
                Request request = new Request(dbPath, _selectedBase);
                request.SaveRequest(rd);
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
            // SelectRequest sr = new SelectRequest(cfgPath, _selectedBase, nbRecordsToPrint);
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
                if ((_odbc_conn_str == null) || (_odbc_conn_str == ""))
                {
                    sr = new SelectRequest(cfgPath, _selectedBase, nbRecordsToPrint);
                    // sr = new SelectRequestRemote(_server, _port, _selectedBase, nbRecordsToPrint);
                }
                else
                {
                    sr = new OdbcRequest(_odbc_conn_str);
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
                SelectRequest sr = new SelectRequest(dbPath, _selectedBase, nbRecordsToPrint);
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

        private void odbcCombo_SelectedIndexChanged(object sender, EventArgs e)
        {
            _odbc_conn_str = this.odbcCombo.SelectedItem.ToString();
        }
    }
}
