using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Configuration;

namespace AlgoQuest.Configuration.Import
{
    public class SqlAnalysisSection : ConfigurationSection
    {
        [ConfigurationProperty("Keywords")]
        public SqlAnalysisKeyWordCollection MapItems
        {
            get { return ((SqlAnalysisKeyWordCollection)(base["Keywords"])); }
        }
    }
}
