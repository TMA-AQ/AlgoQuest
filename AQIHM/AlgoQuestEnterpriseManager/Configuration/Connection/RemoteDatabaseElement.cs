using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Configuration;

namespace AlgoQuest.Configuration.Connection
{
    public class RemoteDatabaseElement : ConfigurationElement
    {
        [ConfigurationProperty("key", DefaultValue = "", IsKey = true, IsRequired = true)]
        public string key
        {
            get
            {
                return ((string)(base["key"]));
            }
            set
            {
                base["key"] = value;
            }
        }

        [ConfigurationProperty("host", DefaultValue = "", IsKey = false, IsRequired = true)]
        public string host
        {
            get
            {
                return ((string)(base["host"]));
            }
            set
            {
                base["host"] = value;
            }
        }

        [ConfigurationProperty("port", DefaultValue = "", IsKey = false, IsRequired = true)]
        public string port
        {
            get
            {
                return ((string)(base["port"]));
            }
            set
            {
                base["port"] = value;
            }
        }

        [ConfigurationProperty("dbname", DefaultValue = "", IsKey = false, IsRequired = false)]
        public string dbname
        {
            get
            {
                return ((string)(base["dbname"]));
            }
            set
            {
                base["dbname"] = value;
            }
        }
    }
}
