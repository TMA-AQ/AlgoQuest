using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Configuration;

namespace AlgoQuest.Configuration.Connection
{
    public class RemoteDatabaseSection : ConfigurationSection
    {
        [ConfigurationProperty("Databases")]
        public RemoteDatabaseCollection MapItems
        {
            get { return ((RemoteDatabaseCollection)(base["Databases"])); }
        }
    }
}
