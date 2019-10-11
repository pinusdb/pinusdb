using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PDB.DotNetSDK
{
  public enum PDBType
  {
    /// <summary>
    /// 空类型
    /// </summary>
    Null = 0,

    /// <summary>
    /// Bool类型
    /// </summary>
    Bool = 1,

    /// <summary>
    /// 8字节，整数
    /// </summary>
    BigInt = 2,

    /// <summary>
    /// 8字节，时间戳，精确到毫秒
    /// </summary>
    DateTime = 3,
    
    /// <summary>
    /// 8字节，双精度浮点数
    /// </summary>
    Double = 4,

    /// <summary>
    /// 字符串
    /// </summary>
    String = 5,

    /// <summary>
    /// 二进制数据
    /// </summary>
    Blob = 6,
    
  }
}
