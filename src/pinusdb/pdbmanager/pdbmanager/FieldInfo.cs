using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace PDBManager
{
  public class FieldInfo
  {
    public string FieldName { get; set; }
    public string FieldType { get; set; }

    public FieldInfo(string fieldName, string fieldType)
    {
      this.FieldName = fieldName;
      this.FieldType = fieldType;
    }

    public bool IsKey()
    {
      return FieldName.ToLower().Equals("objectname")
        || FieldName.ToLower().Equals("timestamp");
    }

  }
}
