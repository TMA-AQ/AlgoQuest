using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AlgoQuest.Core.Import
{
    public class DataImportTable
    {
        public string Name { get; set; }
        public int Order { get; set; }
        public UInt64 Count { get; set; }
        public List<DataImportColumn> Columns { get; set; }
        public bool Loaded { get; set; }
        public bool Loading { get; set; }
    }
}
