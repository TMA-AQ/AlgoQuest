using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Configuration;

namespace AlgoQuest.Configuration.Core
{
    public class OdbcConnectionSection : ConfigurationSection
    {
        [ConfigurationProperty("Connections")]
        public OdbcConnectionCollection MapItems
        {
            get { return ((OdbcConnectionCollection)(base["Connections"])); }
        }
    }
}
