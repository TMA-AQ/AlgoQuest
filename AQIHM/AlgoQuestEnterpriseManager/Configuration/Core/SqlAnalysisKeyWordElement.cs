using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Configuration;

namespace AlgoQuest.Configuration.Import
{
    public class SqlAnalysisKeyWordElement : ConfigurationElement
    {
        [ConfigurationProperty("keyword", DefaultValue = "", IsKey = true, IsRequired = true)]
        public string Keyword
        {
            get
            {
                return ((string)(base["keyword"]));
            }
            set
            {
                base["keyword"] = value;
            }
        }

        [ConfigurationProperty("color", DefaultValue = "", IsKey = true, IsRequired = true)]
        public string Color
        {
            get
            {
                return ((string)(base["color"]));
            }
            set
            {
                base["color"] = value;
            }
        }

        [ConfigurationProperty("font", DefaultValue = "", IsKey = true, IsRequired = true)]
        public string Font
        {
            get
            {
                return ((string)(base["font"]));
            }
            set
            {
                base["font"] = value;
            }
        }
    }
}
