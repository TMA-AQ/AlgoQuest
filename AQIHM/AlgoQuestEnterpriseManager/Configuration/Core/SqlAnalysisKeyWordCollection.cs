using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Configuration;

namespace AlgoQuest.Configuration.Import
{
    public class SqlAnalysisKeyWordCollection : ConfigurationElementCollection, IEnumerable<SqlAnalysisKeyWordElement>
    {
        protected override ConfigurationElement CreateNewElement()
        {
            return new SqlAnalysisKeyWordElement();
        }

        protected override object GetElementKey(ConfigurationElement element)
        {
            return ((SqlAnalysisKeyWordElement)(element)).Keyword.ToString();
        }

        public SqlAnalysisKeyWordElement this[int idx]
        {
            get
            {
                return (SqlAnalysisKeyWordElement)BaseGet(idx);
            }
        }

        public new IEnumerator<SqlAnalysisKeyWordElement> GetEnumerator()
        {
            int count = base.Count;
            for (int i = 0; i < count; i++)
            {
                yield return base.BaseGet(i) as SqlAnalysisKeyWordElement;
            }
        }
    }
}
