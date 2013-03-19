using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Text;
using AlgoQuest.Configuration.Core;

namespace AlgoQuest.Core.DatabaseManagement
{
    public class DataType
    {
        public string Name { get; set; }
        public int DefaultSize { get; set; }

        public static List<DataType> GetDataTypeList()
        {
            List<DataType> lstType = new List<DataType>();
            DataTypeSection section = (DataTypeSection)ConfigurationManager.GetSection("DataTypeSection");
            List<DataTypeElement> _listElement = section.MapItems.ToList();
            foreach (DataTypeElement element in _listElement)
            {
                DataType dt = new DataType();
                dt.Name = element.DataType;
                if (element.DefaultLength != "")
                    dt.DefaultSize = int.Parse(element.DefaultLength);
                lstType.Add(dt);
            }

            return lstType;
        }
    }
}
