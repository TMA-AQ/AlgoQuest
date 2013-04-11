using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Configuration;

namespace AlgoQuest.Configuration.Connection
{
    public class RemoteDatabaseCollection : ConfigurationElementCollection, IEnumerable<RemoteDatabaseElement>
    {
        protected override ConfigurationElement CreateNewElement()
        {
            return new RemoteDatabaseElement();
        }

        protected override object GetElementKey(ConfigurationElement element)
        {
            return ((RemoteDatabaseElement)(element)).key.ToString();
        }

        public RemoteDatabaseElement this[int idx]
        {
            get
            {
                return (RemoteDatabaseElement)BaseGet(idx);
            }
        }

        public new IEnumerator<RemoteDatabaseElement> GetEnumerator()
        {
            int count = base.Count;
            for (int i = 0; i < count; i++)
            {
                yield return base.BaseGet(i) as RemoteDatabaseElement;
            }
        }
    }
}
