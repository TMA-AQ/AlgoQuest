using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;

namespace AlgoQuest.Core.Compute
{
    class AnswerParser
    {

        private String _data;
        private DataTable _result;
        private DataRow _currentRow;
        private int _currentIndex;
        private int _beg;
        private int _end;
        private bool _startDelimiter;
        private bool _endDelimiter;
        private bool _headersParsed;

        public AnswerParser()
        {
            _data = String.Empty;
            _result = new DataTable();
            _currentRow = _result.NewRow();
            _currentIndex = 0;
            _beg = _end = 0;
            _startDelimiter = false;
            _endDelimiter = false;
            _headersParsed = false;
        }

        public void push(String responseData)
        {
            _data += responseData;

            _end = 0;
            while (_end < _data.Length)
            {
                if (!_startDelimiter)
                {
                    if (_data[_end] == ';')
                    {
                        _beg = _end + 1;
                        _startDelimiter = true;
                    }
                }
                else if (!_headersParsed)
                {
                    if ((_data[_end] == ';') || (_data[_end] == '\n'))
                    {
                        if (_beg < _end)
                        {
                            String columnName = _data.Substring(_beg, _end - _beg);
                            columnName = columnName.Trim();
                            _result.Columns.Add(columnName);
                        }
                        if (_data[_end] == '\n')
                        {
                            _headersParsed = true;
                        }
                        _beg = _end + 1;
                    }
                }
                else if ((_data[_end] == ';') || (_data[_end] == '\n'))
                {
                    if (_beg < _end)
                    {
                        // store value
                        String value = _data.Substring(_beg, _end - _beg);
                        value = value.Trim();
                        _beg = _end + 1;

                        // check eos
                        if (value == "EOS")
                        {
                            _endDelimiter = true;
                            break;
                        }

                        _currentRow[_currentIndex++] = value;

                        if (_data[_end] == '\n')
                        {
                            _currentIndex = 0;
                            _result.Rows.Add(_currentRow);
                            _currentRow = _result.NewRow();
                        }
                    }
                    _beg = _end + 1;
                }
                _end++;
            }

            if (!_startDelimiter)
            {
                _data = String.Empty;
            }
            else if (_beg > 0)
            {
                _data = _data.Substring(_beg);
                _beg = _end = 0;
            }
        }

        public DataTable getResult()
        {
            return _result;
        }

        public bool eos()
        {
            return _endDelimiter;
        }
    }
}
