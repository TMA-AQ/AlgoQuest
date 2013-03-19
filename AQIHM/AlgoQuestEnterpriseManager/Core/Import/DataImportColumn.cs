using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AlgoQuest.Core.Import
{
    public class DataImportColumn
    {
        public string Name { get; set; }
        public string DataType { get; set; }
        public string Size { get; set; }
        public int Order { get; set; }
        public bool Loaded { get; set; }
    }
}
