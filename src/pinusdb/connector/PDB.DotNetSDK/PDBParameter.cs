using System;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PDB.DotNetSDK
{
  /// <summary>
  /// PinDB 参数类型
  /// </summary>
  public class PDBParameter : ICloneable
  {
    /// <summary>
    /// The column name
    /// </summary>
    private string parameterName_;

    private PDBType parameterType_;

    /// <summary>
    /// The value of the data in the parameter
    /// </summary>
    private object objValue_;

    /// <summary>
    /// 构造函数
    /// </summary>
    /// <param name="parameterName">参数名，以@开头</param>
    /// <param name="parameterType">参数类型</param>
    public PDBParameter(string parameterName, PDBType parameterType)
    {
      this.parameterName_ = parameterName;
      this.parameterType_ = parameterType;
      this.objValue_ = null;
    }

    /// <summary>
    /// 参数名, 以@开头，如: @user
    /// </summary>
    public string ParameterName
    {
      get { return this.parameterName_; }
    }

    /// <summary>
    /// 参数值,
    /// </summary>
    public object Value
    {
      get
      {
        return objValue_;
      }
      set
      {
        if (value == null || value == DBNull.Value)
        {
          this.objValue_ = value;
        }
        else
        {
          if (value is DateTime)
          {
            if (this.parameterType_ == PDBType.BigInt)
            {
              objValue_ = ((Convert.ToDateTime(value).ToUniversalTime().Ticks - 621355968000000000) / 10000);
            }
            else if(this.parameterType_ == PDBType.DateTime)
            {
              objValue_ = Convert.ToDateTime(value);
            }
            else
            {
              throw new PDBException(PDBErrorCode.PdbE_INVALID_PARAM,"错误的参数：Value Error: DateTime值只能赋值给BigInt类型或DateTime类型");
            }
          }
          else
          {
            switch (this.parameterType_)
            {
              case PDBType.Bool:
                objValue_ = Convert.ToBoolean(value);
                break;
              case PDBType.BigInt:
                objValue_ = Convert.ToInt64(value);
                break;
              case PDBType.Double:
                objValue_ = Convert.ToDouble(value);
                break;
              case PDBType.String:
                if (value is string)
                  objValue_ = value;
                else
                  throw new PDBException(PDBErrorCode.PdbE_INVALID_PARAM, "错误的参数：Value Error: 不支持非string值赋值给String类型");
                break;
              case PDBType.Blob:
                if (value is byte[])
                  objValue_ = value;
                else
                  throw new PDBException(PDBErrorCode.PdbE_INVALID_PARAM, "错误的参数：Value Error: 不支持非byte数组值赋值给Blob类型");
                break;
              case PDBType.DateTime:
                if (value is DateTime)
                  objValue_ = Convert.ToDateTime(value);
                else
                  throw new PDBException(PDBErrorCode.PdbE_INVALID_PARAM, "错误的参数：Value Error: 不支持非DateTime值赋值给DateTime类型");
                break;
            }
          }

        }
      }
    }

    private static char[] blobCharCode =
    {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    private string GetBlobStr()
    {
      if (this.parameterType_ == PDBType.Blob && this.objValue_ != null && this.objValue_ != DBNull.Value)
      {
        StringBuilder strBuilder = new StringBuilder();
        strBuilder.Append("x'");

        byte[] blobVal = this.objValue_ as byte[];
        foreach (byte b in blobVal)
        {
          strBuilder.Append(blobCharCode[b >> 4]);
          strBuilder.Append(blobCharCode[b & 0xF]);
        }

        strBuilder.Append("'");
        return strBuilder.ToString();
      }

      return "x''";
    }

    public override string ToString()
    {
      if (objValue_ == null || objValue_ == DBNull.Value)
      {
        switch (this.parameterType_)
        {
          case PDBType.Bool:
            return "false";
          case PDBType.BigInt:
            return "0";
          case PDBType.Double:
            return "0";
          case PDBType.String:
            return "''";
          case PDBType.Blob:
            return "x''";
          case PDBType.DateTime:
            return "";
        }
      }
      else
      {
        switch (this.parameterType_)
        {
          case PDBType.Bool:
            return ((bool)objValue_) ? "true" : "false";
          case PDBType.BigInt:
            return ((Int64)objValue_).ToString();
          case PDBType.Double:
            return ((double)objValue_).ToString();
          case PDBType.String:
            return "'" + (objValue_ as string).Replace("'", "''") + "'";
          case PDBType.Blob:
            return GetBlobStr();
          case PDBType.DateTime:
            return "'" + ((DateTime)objValue_).ToString("yyyy-MM-dd HH:mm:ss.fff") + "'";
        }
      }

      return "";
    }

    private PDBParameter(PDBParameter source)
    {
      this.parameterName_ = source.parameterName_;
      this.parameterType_ = source.parameterType_;
      this.objValue_ = source.objValue_;
    }

    public object Clone()
    {
      return new PDBParameter(this);
    }
  }
}
