using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;

namespace AlgoQuest.Core.Compute
{
    interface ISelectRequest
    {
        string PathFile { get; set; }
        int NbRecords { get; }
        DataTable Execute(string request);
        void ExecuteTest(string request);
    }
}
