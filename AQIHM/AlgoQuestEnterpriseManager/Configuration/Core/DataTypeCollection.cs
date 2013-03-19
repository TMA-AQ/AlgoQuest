using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Text;

namespace AlgoQuest.Configuration.Core
{
    public class DataTypeCollection : ConfigurationElementCollection, IEnumerable<DataTypeElement>
    {
        protected override ConfigurationElement CreateNewElement()
        {
            return new DataTypeElement();
        }

        protected override object GetElementKey(ConfigurationElement element)
        {
            return ((DataTypeElement)(element)).DataType.ToString();
        }

        public DataTypeElement this[int idx]
        {
            get
            {
                return (DataTypeElement)BaseGet(idx);
            }
        }

        public new IEnumerator<DataTypeElement> GetEnumerator()
        {
            int count = base.Count;
            for (int i = 0; i < count; i++)
            {
                yield return base.BaseGet(i) as DataTypeElement;
            }
        }
    }
}
