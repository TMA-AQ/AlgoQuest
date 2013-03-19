using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Text;

namespace AlgoQuest.Configuration.Import
{
    public class DataTypeMappingCollection : ConfigurationElementCollection, IEnumerable<DataTypeMappingElement>
    {
        protected override ConfigurationElement CreateNewElement()
        {
            return new DataTypeMappingElement();
        }

        protected override object GetElementKey(ConfigurationElement element)
        {
            return ((DataTypeMappingElement)(element)).BaseType.ToString() + ((DataTypeMappingElement)(element)).DefaultType.ToString();
        }

        public DataTypeMappingElement this[int idx]
        {
            get
            {
                return (DataTypeMappingElement)BaseGet(idx);
            }
        }

        public new IEnumerator<DataTypeMappingElement> GetEnumerator()
        {
            int count = base.Count;
            for (int i = 0; i < count; i++)
            {
                yield return base.BaseGet(i) as DataTypeMappingElement;
            }
        }
    }
}
