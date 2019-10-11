using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PDBManager
{
  class GlobalVar
  {
    /// <summary>
    /// 是否已经连接
    /// </summary>
    public static bool IsConnected = false;

    /// <summary>
    /// 服务器地址
    /// </summary>
    public static string Server = "";

    /// <summary>
    /// 服务器端口
    /// </summary>
    public static int Port = 0;

    /// <summary>
    /// 用户名
    /// </summary>
    public static string User = "";

    /// <summary>
    /// 密码
    /// </summary>
    public static string Pwd = "";

    /// <summary>
    /// 连接字符串
    /// </summary>
    public static string ConnStr = "";
  }
}
