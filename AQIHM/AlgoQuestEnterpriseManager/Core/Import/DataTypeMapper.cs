using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Text;
using AlgoQuest.Configuration.Import;

namespace AlgoQuest.Core.Import
{
    public class DataTypeMapper
    {
        private List<DataTypeMappingElement> _listMapElement;
        public DataTypeMapper(Connector.SgbdType sgbdType)
        {
            DataTypeMappingSection section = (DataTypeMappingSection)ConfigurationManager.GetSection("DataTypeMappingSection");
            _listMapElement = section.MapItems.AsQueryable().Cast<DataTypeMappingElement>()
                                        .Where(v => v.BaseType == sgbdType.ToString())
                                        .ToList<DataTypeMappingElement>();
        }


        public string Map(string OriginalDataType)
        {

            if (_listMapElement.Exists(v => String.Compare(v.DefaultType, OriginalDataType, true) == 0))
                return _listMapElement.Single(v => String.Compare(v.DefaultType, OriginalDataType, true) == 0).FinalType;
            else
                return OriginalDataType;
        }

        public string MapLength(string OriginalDataType, string Length)
        {
            if(_listMapElement.Exists(
                            v =>(String.Compare(v.DefaultType, OriginalDataType, true) == 0)
                                    )
                )
            {
                if (
                    (_listMapElement.Single(v => String.Compare(v.DefaultType, OriginalDataType, true) == 0).FinalType == "VARCHAR")
                    ||
                    (_listMapElement.Single(v => String.Compare(v.DefaultType, OriginalDataType, true) == 0).FinalType == "VARCHAR2")
                    )
                    return Length;
                else
                    return _listMapElement.Single(v => String.Compare(v.DefaultType, OriginalDataType, true) == 0).DefaultLength;
            }
            else
                return Length;

            //if (
            //    _listMapElement.Exists(v =>
            //        (
            //        String.Compare(v.DefaultType, OriginalDataType, true) == 0)
            //        &&
            //        v.DefaultLength != string.Empty
            //        )
            //    )
            //    return _listMapElement.Single(v => String.Compare(v.DefaultType, OriginalDataType, true) == 0).DefaultLength;
            //else
            //    return Length;
        }
    }
}
