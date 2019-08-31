using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace PDBManager
{
  public class NameTool
  {
    public static bool ValidFieldName(string fieldName)
    {
      if (fieldName.Length <= 0 || fieldName.Length >= 48)
        return false;

      return ValidName(fieldName);
    }

    public static bool ValidTableName(string tabName)
    {
      if (tabName.Length <= 0 || tabName.Length >= 84)
        return false;

      return ValidName(tabName);
    }


    private static bool ValidName(string name)
    {
      string newName = name.ToLower();

      if ((newName[0] >= 'a' && newName[0] <= 'z'))
      {
        for (int i = 0; i < newName.Length; i++)
        {
          if (!((newName[i] >= 'a' && newName[i] <= 'z')
            || (newName[i] >= '0' && newName[i] <= '9')))
            return false;
        }

        return true;
      }

      return false;
    }
  }
}
