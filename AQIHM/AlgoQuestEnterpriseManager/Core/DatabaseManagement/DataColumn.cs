using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AlgoQuest.Core.DatabaseManagement
{
    public class DataColumn
    {
        public string ColumnName{get; set;}
        public string DataTypeName { get; set; }
        public UInt64 DataTypeLength { get; set; }
        public int Order { get; set; }
    }
}
