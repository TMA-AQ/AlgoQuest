using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Configuration;

namespace AlgoQuest.Configuration.Core
{
    public class OdbcConnectionCollection : ConfigurationElementCollection, IEnumerable<OdbcConnectionElement>
    {
        protected override ConfigurationElement CreateNewElement()
        {
            return new OdbcConnectionElement();
        }

        protected override object GetElementKey(ConfigurationElement element)
        {
            return ((OdbcConnectionElement)(element)).key.ToString();
        }

        public OdbcConnectionElement this[int idx]
        {
            get
            {
                return (OdbcConnectionElement)BaseGet(idx);
            }
        }

        public new IEnumerator<OdbcConnectionElement> GetEnumerator()
        {
            int count = base.Count;
            for (int i = 0; i < count; i++)
            {
                yield return base.BaseGet(i) as OdbcConnectionElement;
            }
        }
    }
}
