using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Text;

namespace AlgoQuest.Configuration.Import
{
    public class DataTypeMappingSection : ConfigurationSection
    {
        [ConfigurationProperty("Mappings")]
        public DataTypeMappingCollection MapItems
        {
            get { return ((DataTypeMappingCollection)(base["Mappings"])); }
        }

    }
}
