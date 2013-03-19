using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Text;

namespace AlgoQuest.Configuration.Import
{
    public class DataTypeMappingElement : ConfigurationElement 
    {
        [ConfigurationProperty("baseType", DefaultValue = "", IsKey = true, IsRequired = true)]
        public string BaseType
        {
            get
            {
                return ((string)(base["baseType"]));
            }
            set
            {
                base["baseType"] = value;
            }
        }

        [ConfigurationProperty("defaultType", DefaultValue = "", IsKey = true, IsRequired = true)]
        public string DefaultType
        {
            get
            {
                return ((string)(base["defaultType"]));
            }
            set
            {
                base["defaultType"] = value;
            }
        }

        [ConfigurationProperty("finalType", DefaultValue = "", IsKey = false, IsRequired = true)]
        public string FinalType
        {
            get
            {
                return ((string)(base["finalType"]));
            }
            set
            {
                base["finalType"] = value;
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
