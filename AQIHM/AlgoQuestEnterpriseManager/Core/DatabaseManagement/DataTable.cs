using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AlgoQuest.Core.DatabaseManagement
{
    public class DataTable
    {
        public List<DataColumn> DataColumns { get; set; }

        public string DataTableName { get; set; }

        public int Order { get; set; }

        public Int64 RowCount { get; set; }
    }
}
