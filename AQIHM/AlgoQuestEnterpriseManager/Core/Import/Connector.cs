using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AlgoQuest.Core.Import
{
    public class Connector
    {
        public enum SgbdType
        {
            SqlServer,
            Oracle,
            MySql,
            Db2,
            AlgoQuest
        }

        public static IConnector GetConnector(SgbdType type)
        {
            switch (type)
            {
                case SgbdType.SqlServer :
                    return new SqlConnector();
                case SgbdType.MySql :
                    return new MySqlConnector();
                case SgbdType.Oracle :
                    return new OracleConnector();
                case SgbdType.AlgoQuest:
                    return new AQConnector();
                default :
                    return null;
            }
        }

    }
}
