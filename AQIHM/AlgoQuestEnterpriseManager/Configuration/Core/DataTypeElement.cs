using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Text;

namespace AlgoQuest.Configuration.Core
{
    public class DataTypeElement : ConfigurationElement 
    {
        [ConfigurationProperty("dataType", DefaultValue = "", IsKey = true, IsRequired = true)]
        public string DataType
        {
            get
            {
                return ((string)(base["dataType"]));
            }
            set
            {
                base["dataType"] = value;
            }
        }

        
        [ConfigurationProperty("defaultLength", DefaultValue = "", IsKey = false, IsRequired = false)]
        public string DefaultLength
        {
            get
            {
                return ((string)(base["defaultLength"]));
            }
            set
            {
                base["defaultLength"] = value;
            }
        }

    }
}
