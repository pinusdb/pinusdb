using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;

namespace PDB.DotNetSDK
{
  public class PDBConnection : IDisposable
  {
    private TcpClient sock;

    private const int kUserNameLen = 48;
    private const int kPwdLen = 4;

    /// <summary>
    /// PinDB连接对象---构造函数
    /// </summary>
    /// <param name="connStr">连接字符串，如：server=127.0.0.1;port=4517;username=sa;password=pinusdb</param>
    public PDBConnection(string connStr)
    {
      this.Port = 0;
      string[] strArr = connStr.Split(new char[] { ';' }, StringSplitOptions.RemoveEmptyEntries);

      foreach (string str in strArr)
      {
        string[] itemArr = str.Split(new char[] { '=' }, StringSplitOptions.RemoveEmptyEntries);
        if (itemArr.Count() != 2)
        {
          throw new PDBException(PDBErrorCode.PdbE_INVALID_PARAM);
        }

        if (itemArr[0].Trim().ToLower().Equals("server"))
        {
          HostName = itemArr[1].Trim();
        }
        else if (itemArr[0].Trim().ToLower().Equals("port"))
        {
          int port = 0;
          if (int.TryParse(itemArr[1].Trim(), out port))
          {
            Port = port;
          }
          else
          {
            throw new PDBException(PDBErrorCode.PdbE_INVALID_PARAM);
          }
        }
        else if (itemArr[0].Trim().ToLower().Equals("username"))
        {
          UserName = itemArr[1].Trim();
        }
        else if (itemArr[0].Trim().ToLower().Equals("password"))
        {
          Password = itemArr[1].Trim();
        }
      }

      if (string.IsNullOrEmpty(HostName) || Port <= 0 || Port >= 65536
        || string.IsNullOrEmpty(UserName) || string.IsNullOrEmpty(Password))
      {
        throw new PDBException(PDBErrorCode.PdbE_INVALID_PARAM);
      }

      this.ConnStr = connStr;
      sock = null;
    }
    ~PDBConnection()
    {
      this.Close();
    }

    /// <summary>
    /// 打开PinDB数据库连接
    /// </summary>
    public void Open()
    {
      this.Close();
      sock = new TcpClient(HostName, Port);
      sock.SendBufferSize = (256 * 1024);
      sock.ReceiveBufferSize = (64 * 1024);
      sock.NoDelay = true;
      sock.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive, true);

      try
      {
        Login();
      }
      catch (Exception ex)
      {
        this.Close();
        throw ex;
      }

    }

    /// <summary>
    /// 关闭PinDB数据库连接
    /// </summary>
    public void Close()
    {
      if (sock != null)
      {
        sock.Close();
      }
      sock = null;
    }

    internal void Request(byte[] buf)
    {
      if (sock == null)
        throw new PDBException(PDBErrorCode.PdbE_NET_ERROR, "网络错误：未连接");

      sock.GetStream().Write(buf, 0, buf.Length);
    }

    internal byte[] Recv()
    {
      if (sock == null)
        throw new PDBException(PDBErrorCode.PdbE_NET_ERROR, "网络错误：未连接");

      int totalLen = 0;

      byte[] headBuf = new byte[ProtoHeader.kHeadLen];

      while (totalLen < ProtoHeader.kHeadLen)
      {
        int tmpLen = ProtoHeader.kHeadLen - totalLen;
        int recvLen = sock.GetStream().Read(headBuf, totalLen, tmpLen);
        totalLen += recvLen;
      }

      ProtoHeader proHdr = new ProtoHeader(headBuf);
      UInt32 headCrc = proHdr.GetHeadCrc();
      UInt32 tmpHeadCrc = CRC.Crc32(headBuf, 0, 60);
      if (headCrc != tmpHeadCrc)
      {
        throw new PDBException(PDBErrorCode.PdbE_NET_ERROR, "报文错误：回复报文中，回复头校验错误");
      }

      int bodyLen = (int)proHdr.GetBodyLen();

      if (bodyLen == 0)
      {
        return headBuf;
      }

      byte[] packetBuf = new byte[ProtoHeader.kHeadLen + bodyLen];
      Array.Copy(headBuf, 0, packetBuf, 0, ProtoHeader.kHeadLen);
      totalLen = 0;
      while (totalLen < bodyLen)
      {
        int tmpLen = bodyLen - totalLen;
        int recvLen = sock.GetStream().Read(packetBuf, (ProtoHeader.kHeadLen + totalLen), tmpLen);
        totalLen += recvLen;
      }

      UInt32 bodyCrc = proHdr.GetBodyCrc();

      UInt32 tmpBodyCrc = CRC.Crc32(packetBuf, ProtoHeader.kHeadLen, bodyLen);
      if (tmpBodyCrc != bodyCrc)
      {
        throw new PDBException(PDBErrorCode.PdbE_PACKET_ERROR, "报文错误：回复报文中，回复头校验错误");
      }

      return packetBuf;
    }

    /// <summary>
    /// 创建命令对象
    /// </summary>
    /// <returns></returns>
    public PDBCommand CreateCommand()
    {
      return new PDBCommand(this);
    }

    public void Dispose()
    {
      this.Close();
    }

    private void Login()
    {
      byte[] loginPacket = new byte[ProtoHeader.kHeadLen + kUserNameLen + kPwdLen];
      ProtoHeader proHdr = new ProtoHeader(loginPacket);

      Array.Clear(loginPacket, 0, (loginPacket.Length));
      
      byte[] nameArray = System.Text.Encoding.Default.GetBytes(UserName);
      byte[] pwdArray = System.Text.Encoding.Default.GetBytes(Password);

      if (nameArray.Length >= kUserNameLen)
      {
        throw new PDBException(PDBErrorCode.PdbE_INVALID_USER_NAME, "非法的用户名：用户名不合法");
      }
      for (int i = 0; i < nameArray.Length; i++)
      {
        loginPacket[ProtoHeader.kHeadLen + i] = nameArray[i];
      }
      UInt64 pwd64 = CRC.Crc64(pwdArray);
      UInt32 pwd32 = (UInt32)(pwd64 & 0xFFFFFFFF);
      IntTool.PutUint32(loginPacket, (ProtoHeader.kHeadLen + kUserNameLen), pwd32);

      UInt32 bodyCrc = CRC.Crc32(loginPacket, ProtoHeader.kHeadLen, (kUserNameLen + kPwdLen));

      proHdr.SetVersion(ProtoHeader.kProtoVersion);
      proHdr.SetMethod(ProtoHeader.MethodCmdLoginReq);
      proHdr.SetBodyLen((uint)(kUserNameLen + kPwdLen));
      proHdr.SetReturnVal(0);
      proHdr.SetRecordCnt((uint)0);
      proHdr.SetBodyCrc(bodyCrc);
      proHdr.SetHeadCrc();

      Request(loginPacket);

      byte[] resBuf = Recv();
      ProtoHeader resHdr = new ProtoHeader(resBuf);
      if (resHdr.GetMethod() != ProtoHeader.MethodCmdLoginRep)
      {
        throw new PDBException(PDBErrorCode.PdbE_PACKET_ERROR);
      }
      if (resHdr.GetReturnVal() != 0)
      {
        throw new PDBException(PDBErrorCode.PdbE_NET_ERROR, "网络错误：登录失败," + resHdr.GetReturnVal().ToString());
      }

    }

    private string ConnStr { get; set; }
    private string HostName { get; set; }
    private int Port { get; set; }
    private string UserName { get; set; }
    private string Password { get; set; }


  }
}
