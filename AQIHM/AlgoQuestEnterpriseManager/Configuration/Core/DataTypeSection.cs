using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Text;

namespace AlgoQuest.Configuration.Core
{
    public class DataTypeSection : ConfigurationSection
    {
        [ConfigurationProperty("DataTypes")]
        public DataTypeCollection MapItems
        {
            get { return ((DataTypeCollection)(base["DataTypes"])); }
        }

    }
}
