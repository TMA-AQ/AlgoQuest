using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Collections;
using System.IO;
using AlgoQuest.Configuration.Import;
using System.Configuration;

namespace AlgoQuest.Core.Import
{
    public class BaseStructGenerator
    {
        private Connector.SgbdType _sgbdType;

        public BaseStructGenerator(Connector.SgbdType sgbdType)
        { _sgbdType = sgbdType; }

      
        public string GetBaseStruct(string baseName, List<DataImportTable> tableList)
        {
            StringBuilder sb = new StringBuilder();
            sb.AppendLine(baseName);
            sb.AppendLine(tableList.Count.ToString());
            DataTypeMapper dtm = new DataTypeMapper(_sgbdType);

            foreach (DataImportTable table in tableList)
            {
                sb.AppendLine();
                sb.AppendLine(String.Format("\"{0}\" {1} {2} {3}", table.Name, table.Order.ToString(), table.Count.ToString(), table.Columns.Count.ToString()));
                foreach (DataImportColumn column in table.Columns)
                {
                    string finalType = dtm.Map(column.DataType);
                    string length = dtm.MapLength(column.DataType, column.Size);
                    if (finalType == "VARCHAR" || finalType == "VARCHAR2")
                        if (length != string.Empty)
                            length = ((Int64.Parse(length) + 1)).ToString();
                    sb.AppendLine(String.Format("\"{0}\" {1} {2} {3}", column.Name, column.Order.ToString(), length, finalType));
                }
            }
            
            return sb.ToString();
        }

    }
}
