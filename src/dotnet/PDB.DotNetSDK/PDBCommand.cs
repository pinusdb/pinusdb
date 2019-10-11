using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;

namespace PDB.DotNetSDK
{

  public class PDBCommand
  {

    private const int kTableNameLen = 48;
    /// <summary>
    /// 执行插入时，成功插入的条数
    /// </summary>
    public int SuccessCount { get; private set; }
    /// <summary>
    /// 执行插入时，每条数据的执行结果
    /// </summary>
    public List<PDBErrorCode> InsertResult { get; private set; }
    
    /// <summary>
    /// 命令对象
    /// </summary>
    /// <param name="conn">连接对象</param>
    public PDBCommand(PDBConnection conn)
    {
      ConnObj = conn;
      SuccessCount = 0;
      InsertResult = new List<PDBErrorCode>();
    }
    
    /// <summary>
    /// 插入数据
    /// </summary>
    /// <param name="sql">要执行的SQL语句列表</param>
    /// <returns>执行结果</returns>
    public PDBErrorCode ExecuteInsert(string sql)
    {
      PDBErrorCode retVal = PDBErrorCode.PdbE_OK;
      lock (ConnObj)
      {
        byte[] sendPacket = MakeRequestPacket(ProtoHeader.MethodCmdInsertReq, sql);
        ConnObj.Request(sendPacket);

        byte[] recvPacket = ConnObj.Recv();
        DecodeInsertPacket(recvPacket, out retVal);
      }
      
      return retVal ;
    }

    /// <summary>
    /// 插入数据
    /// </summary>
    /// <param name="sql">要执行的SQL语句列表</param>
    /// <param name="parms">参数</param>
    /// <returns>执行结果</returns>
    public PDBErrorCode ExecuteInsert(string sql, params PDBParameter[] parms)
    {
      string newSql = BuildSql(sql, parms);
      return ExecuteInsert(newSql);
    }

    public PDBErrorCode ExecuteInsert(DataTable dataTable)
    {
      return ExecuteInsert(dataTable.TableName, dataTable);
    }

    public PDBErrorCode ExecuteInsert(string tabName, DataTable dataTable)
    {
      PDBErrorCode retVal = PDBErrorCode.PdbE_OK;

      lock(ConnObj)
      {
        byte[] sendPacket = MakeInsertTableRequestPacket(tabName, dataTable);
        ConnObj.Request(sendPacket);

        byte[] recvPacket = ConnObj.Recv();
        DecodeInsertTablePacket(recvPacket, out retVal);
      }

      return retVal;
    }


    private string BuildSql(string sql, params PDBParameter[] parms)
    {
      List<string> sqlPart = SplitSql(sql);

      StringBuilder sqlBuilder = new StringBuilder(8192);
      Dictionary<string, string> paramDic = new Dictionary<string, string>();

      foreach(PDBParameter par in parms)
      {
        paramDic[par.ParameterName] = par.ToString();
      }

      foreach(string part in sqlPart)
      {
        if (part.StartsWith("@"))
          sqlBuilder.Append(paramDic[part]);
        else
          sqlBuilder.Append(part);
      }

      return sqlBuilder.ToString();
    }
    private int getPos(String str)
    {
      int len = str.Length;
      int bgPos = 0;
      bool endFlag = false;
      for (; bgPos < (len - 1) && !endFlag;)
      {
        switch (str[bgPos])
        {
          case ' ':
          case '\t':
          case '\n':
          case '\f':
          case '\r':
            bgPos++;
            break;
          case '-':
            if (str[bgPos + 1] == '-')
            {
              bgPos = str.IndexOf('\n', bgPos);
              if (bgPos == -1)
                return bgPos;
              else
                bgPos += 1;
            }
            else
            {
              endFlag = true;
            }
            break;
          default:
            endFlag = true;
            break;
        }
      }
      return bgPos;
    }
    private List<string> SplitSql(string sqlStr)
    {
      int pos = getPos(sqlStr);
      if(pos<0)
        throw new PDBException(PDBErrorCode.PdbE_SQL_ERROR, "SQL语句错误 Error SQL:" + sqlStr);

      sqlStr = sqlStr.Substring(pos);

      int curIdx = 0;
      int partBg = 0;
      char[] findChars = { '@', '\'', '"' ,'-'};
      List<string> sqlPart = new List<string>();

      while (curIdx < sqlStr.Length)
      {
        partBg = curIdx;
        curIdx = sqlStr.IndexOfAny(findChars, curIdx);
        if (curIdx < 0)
        {
          sqlPart.Add(sqlStr.Substring(partBg));
          break;
        }

        if (curIdx > partBg)
        {
          sqlPart.Add(sqlStr.Substring(partBg, (curIdx - partBg)));
        }

        partBg = curIdx;
        curIdx++;

        switch(sqlStr[partBg])
        {
          case '@':
            while (curIdx < sqlStr.Length && 
              ((sqlStr[curIdx] >= '0' && sqlStr[curIdx] <= '9')
              || (sqlStr[curIdx] >= 'a' && sqlStr[curIdx] <= 'z')
              || (sqlStr[curIdx] >= 'A' && sqlStr[curIdx] <= 'Z')
              || (sqlStr[curIdx] == '_')))
            {
              curIdx++;
            }
            sqlPart.Add(sqlStr.Substring(partBg, (curIdx - partBg)));
            break;
          case '\'':
            curIdx = sqlStr.IndexOf('\'', curIdx);
            if (curIdx < 0)
            {
              throw new PDBException(PDBErrorCode.PdbE_INVALID_PARAM, "SQL语句错误 Error SQL:" + sqlStr);
            }

            curIdx++;
            sqlPart.Add(sqlStr.Substring(partBg, (curIdx - partBg)));
            break;
          case '"':
            curIdx = sqlStr.IndexOf('"', curIdx);
            if (curIdx < 0)
            {
              throw new PDBException(PDBErrorCode.PdbE_INVALID_PARAM, "SQL语句错误 Error SQL:" + sqlStr);
            }

            curIdx++;
            sqlPart.Add(sqlStr.Substring(partBg, (curIdx - partBg)));
            break;
          case '-':
            if (curIdx >= sqlStr.Length)
            {
              throw new PDBException(PDBErrorCode.PdbE_INVALID_PARAM, "SQL语句错误 Error SQL:" + sqlStr);
            }

            if (sqlStr[curIdx] == '-')
            {
              curIdx = sqlStr.IndexOf('\n', curIdx);
              if (curIdx < 0)
              {
                curIdx = sqlStr.Length;
              }
              else
              {
                curIdx++;
              }
            }

            sqlPart.Add(sqlStr.Substring(partBg, (curIdx - partBg)));
            break;
        }
      }
      
      return sqlPart;
    }

    /// <summary>
    /// 查询
    /// </summary>
    /// <param name="sql">执行的SQL</param>
    /// <returns>查询的结果集</returns>
    public DataTable ExecuteQuery(string sql)
    {
      SuccessCount = 0;
      PDBErrorCode retVal = 0;

      DataTable dtResult = new DataTable();

      lock (ConnObj)
      {
        byte[] sendPacket = MakeRequestPacket(ProtoHeader.MethodCmdQueryReq, sql);
        ConnObj.Request(sendPacket);

        byte[] recvPacket = ConnObj.Recv();
        DecodePacket(recvPacket, out retVal, dtResult);
        if (retVal != PDBErrorCode.PdbE_OK)
        {
          throw new PDBException(retVal);
        }
      }   
      return dtResult;
    }

    /// <summary>
    /// 查询
    /// </summary>
    /// <param name="sql">执行的SQL</param>
    /// <param name="parms">参数数组</param>
    /// <returns>查询的结果集</returns>
    public DataTable ExecuteQuery(string sql, params PDBParameter[] parms)
    {
      SuccessCount = 0;
      string newSql = BuildSql(sql, parms);

      return ExecuteQuery(newSql);
    }

    /// <summary>
    /// 其他，例如：添加用户，删除用户，删除数据 等等..
    /// </summary>
    /// <param name="sql">执行的SQL</param>
    public void ExecuteNonQuery(string sql)
    {
      SuccessCount = 0;
      lock (ConnObj)
      {
        byte[] sendPacket = MakeRequestPacket(ProtoHeader.MethodCmdNonQueryReq, sql);
        ConnObj.Request(sendPacket);

        byte[] recvPacket = ConnObj.Recv();
        ProtoHeader proHdr = new ProtoHeader(recvPacket);

        if (proHdr.GetMethod() != ProtoHeader.MethodCmdNonQueryRep)
        {
          throw new PDBException(PDBErrorCode.PdbE_PACKET_ERROR, "报文错误:Error Packet");
        }

        PDBErrorCode errCode = (PDBErrorCode)proHdr.GetReturnVal();
        
        if (errCode != PDBErrorCode.PdbE_OK)
        {
          throw new PDBException(errCode);
        }
      }

    }

    /// <summary>
    /// 其他，例如：添加用户，删除用户，删除数据 等等..
    /// </summary>
    /// <param name="sql">执行的SQL</param>
    /// <param name="parms">参数数组</param>
    public void ExecuteNonQuery(string sql, params PDBParameter[] parms)
    {
      SuccessCount = 0;
      string newSql = BuildSql(sql, parms);

      ExecuteNonQuery(newSql);
    }

    //private byte[] MakeQueryRequestPacket(string sql)
    //{
    //  int bodyLen = 0;
    //  int totalLen = 0;
    //
    //  if (sql.Length == 0)
    //    throw new PDBException(EDBErrorCode.PinE_ERROR_SQL,"SQL语句错误:执行的SQL语句不能为空");
    //
    //  byte[] tmpSql = Encoding.UTF8.GetBytes(sql);
    //
    //  bodyLen = tmpSql.Count();
    //  totalLen = ProtoHeader.kHeadLen + bodyLen;
    //
    //  byte[] sendBuf = new byte[totalLen];
    //  Array.Clear(sendBuf, 0, totalLen);
    //
    //  Array.Copy(tmpSql, 0, sendBuf, ProtoHeader.kHeadLen, tmpSql.Count());
    //
    //  UInt32 bodyCrc = CRC.Crc32(sendBuf, ProtoHeader.kHeadLen, bodyLen);
    //
    //  ProtoHeader proHdr = new ProtoHeader(sendBuf);
    //  proHdr.SetVersion(ProtoHeader.kProtoVersion);
    //  proHdr.SetMethod(ProtoHeader.MethodCmdQueryReq);
    //  proHdr.SetBodyLen((uint)bodyLen);
    //  proHdr.SetReturnVal(0);
    //  proHdr.SetRecordCnt(1);
    //  proHdr.SetBodyCrc(bodyCrc);
    //  proHdr.SetHeadCrc();
    //
    //  return sendBuf;
    //}

    private byte[] MakeRequestPacket(uint reqMethod, string sql)
    {
      int bodyLen = 0;
      int totalLen = 0;

      if (sql.Length == 0)
        throw new PDBException(PDBErrorCode.PdbE_SQL_ERROR, "SQL语句错误:执行的SQL语句不能为空");

      byte[] tmpSql = Encoding.UTF8.GetBytes(sql);

      bodyLen = tmpSql.Count();
      totalLen = ProtoHeader.kHeadLen + bodyLen;

      byte[] packetBuf = new byte[totalLen];
      Array.Clear(packetBuf, 0, totalLen);

      Array.Copy(tmpSql, 0, packetBuf, ProtoHeader.kHeadLen, tmpSql.Count());
      uint bodyCrc = CRC.Crc32(packetBuf, ProtoHeader.kHeadLen, bodyLen);

      ProtoHeader proHdr = new ProtoHeader(packetBuf);
      proHdr.SetVersion(ProtoHeader.kProtoVersion);
      proHdr.SetMethod(reqMethod);
      proHdr.SetBodyLen((uint)bodyLen);
      proHdr.SetReturnVal(0);
      proHdr.SetRecordCnt(1);
      proHdr.SetBodyCrc(bodyCrc);
      proHdr.SetHeadCrc();

      return packetBuf;
    }

    private byte[] MakeInsertTableRequestPacket(string tabName, DataTable dataTable)
    {
      int bodyLen = 0;
      int totalLen = 0;

      int fieldCnt = dataTable.Columns.Count;

      if (string.IsNullOrEmpty(tabName))
        throw new PDBException(PDBErrorCode.PdbE_INVALID_TABLE_NAME, "非法的表名");

      if (Encoding.UTF8.GetByteCount(tabName) >= kTableNameLen)
        throw new PDBException(PDBErrorCode.PdbE_INVALID_TABLE_NAME, "表名过长");

      ByteStream byteStream = new ByteStream();
      IntTool.WriteString(byteStream, tabName);

      foreach(DataColumn colInfo in dataTable.Columns)
      {
        IntTool.WriteString(byteStream, colInfo.ColumnName);
      }

      foreach(DataRow dr in dataTable.Rows)
      {
        for(int fieldIdx = 0; fieldIdx < fieldCnt; fieldIdx++)
        {
          if (dr[fieldIdx] is DBNull)
          {
            IntTool.WriteNull(byteStream);
          }
          else if (dr[fieldIdx] is bool)
          {
            IntTool.WriteBool(byteStream, Convert.ToBoolean(dr[fieldIdx]));
          }
          else if ((dr[fieldIdx] is sbyte) || (dr[fieldIdx] is short)
            || (dr[fieldIdx] is int) || (dr[fieldIdx] is long))
          {
            IntTool.WriteVarint64(byteStream, Convert.ToInt64(dr[fieldIdx]));
          }
          else if (dr[fieldIdx] is double || dr[fieldIdx] is float)
          {
            IntTool.WriteDouble(byteStream, Convert.ToDouble(dr[fieldIdx]));
          }
          else if (dr[fieldIdx] is DateTime)
          {
            IntTool.WriteDatetime(byteStream, Convert.ToDateTime(dr[fieldIdx]));
          }
          else if (dr[fieldIdx] is string)
          {
            IntTool.WriteString(byteStream, Convert.ToString(dr[fieldIdx]));
          }
          else if (dr[fieldIdx] is byte[])
          {
            IntTool.WriteBlob(byteStream, (byte[])dr[fieldIdx]);
          }
          else
            throw new Exception("不支持的值");
        }
      }

      bodyLen = byteStream.GetTotalLen();
      totalLen = ProtoHeader.kHeadLen + bodyLen;

      byte[] packetBuf = new byte[totalLen];
      Array.Clear(packetBuf, 0, totalLen);

      byteStream.Read(packetBuf, ProtoHeader.kHeadLen);
      uint bodyCrc = CRC.Crc32(packetBuf, ProtoHeader.kHeadLen, bodyLen);

      ProtoHeader proHdr = new ProtoHeader(packetBuf);
      proHdr.SetVersion(ProtoHeader.kProtoVersion);
      proHdr.SetMethod(ProtoHeader.MethodCmdInsertTableReq);
      proHdr.SetBodyLen((uint)bodyLen);
      proHdr.SetFieldCnt((uint)fieldCnt);
      proHdr.SetRecordCnt((uint)dataTable.Rows.Count);
      proHdr.SetBodyCrc(bodyCrc);
      proHdr.SetHeadCrc();

      return packetBuf;
    }

    private void DecodePacket(byte[] resPacket, out PDBErrorCode retVal, DataTable resultTab)
    {
      ProtoHeader proHdr = new ProtoHeader(resPacket);

      retVal = (PDBErrorCode)proHdr.GetReturnVal();

      resultTab.Clear();
      resultTab.Columns.Clear();

      int offset = ProtoHeader.kHeadLen;
      uint fieldCnt = proHdr.GetFieldCnt();

      if (retVal != PDBErrorCode.PdbE_OK)
        return;

      List<object> headList = GetRecord(fieldCnt, resPacket, ref offset);
      foreach (object obj in headList)
      {
        if (obj.GetType() == typeof(System.String))
        {
          string str = (string)obj;
          string[] strArr = str.Split(new char[] { ';' }, StringSplitOptions.RemoveEmptyEntries);
          if (strArr.Count() != 2)
          {
            throw new PDBException(PDBErrorCode.PdbE_PACKET_ERROR, "报文错误：非法的表头");
          }

          DataColumn colInfo = null;
          switch (strArr[0])
          {
            case "bool":
              colInfo = new DataColumn(strArr[1], typeof(bool));
              break;
            case "bigint":
              colInfo = new DataColumn(strArr[1], typeof(long));
              break;
            case "datetime":
              colInfo = new DataColumn(strArr[1], typeof(DateTime));
              break;
            case "double":
              colInfo = new DataColumn(strArr[1], typeof(double));
              break;
            case "string":
              colInfo = new DataColumn(strArr[1], typeof(string));
              break;
            case "blob":
              colInfo = new DataColumn(strArr[1], typeof(byte[]));
              break;
            default:
              throw new PDBException(PDBErrorCode.PdbE_PACKET_ERROR, "报文错误：非法的表头");
          }

          resultTab.Columns.Add(colInfo);
        }
        else
        {
          throw new PDBException(PDBErrorCode.PdbE_PACKET_ERROR, "报文错误：非法的表头");
        }
      }

      while (offset < resPacket.Count())
      {
        DataRow dataRow = resultTab.NewRow();
        List<object> dataList = GetRecord(fieldCnt, resPacket, ref offset);

        if (dataList.Count() != headList.Count())
        {
          throw new PDBException(PDBErrorCode.PdbE_PACKET_ERROR);
        }

        for (int i = 0; i < dataList.Count(); i++)
        {
          if (dataList[i] != null)
          {
            dataRow[i] = dataList[i];
          }
          else
          {
            dataRow[i] = DBNull.Value;
          }
        }
        resultTab.Rows.Add(dataRow);

      }
    }

    private void DecodeInsertPacket(byte[] resPacket, out PDBErrorCode retVal)
    {
      ProtoHeader proHdr = new ProtoHeader(resPacket);
      
      if (resPacket.Count() != ProtoHeader.kHeadLen)
      {
        retVal = PDBErrorCode.PdbE_PACKET_ERROR;
        return;
      }

      SuccessCount = (int)proHdr.GetRecordCnt();
      InsertResult.Clear();
      for(int i = 0; i < SuccessCount; i++)
      {
        InsertResult.Add(PDBErrorCode.PdbE_OK);
      }
      
      retVal = (PDBErrorCode)proHdr.GetReturnVal();
      if (retVal != PDBErrorCode.PdbE_OK)
      {
        InsertResult.Add(retVal);
      }
    }

    private void DecodeInsertTablePacket(byte[] resPacket, out PDBErrorCode retVal)
    {
      ProtoHeader proHdr = new ProtoHeader(resPacket);
      
      InsertResult.Clear();
      SuccessCount = 0;
      retVal = (PDBErrorCode)proHdr.GetReturnVal();
      if (retVal == PDBErrorCode.PdbE_OK)
      {
        SuccessCount = (int)proHdr.GetRecordCnt();
        for(int i = 0; i < SuccessCount; i++)
        {
          InsertResult.Add(PDBErrorCode.PdbE_OK);
        }
      }
      else if (retVal == PDBErrorCode.PdbE_INSERT_PART_ERROR)
      {
        int offset = ProtoHeader.kHeadLen;
        int recCnt = (int)proHdr.GetRecordCnt();
        for (int i = 0; i < recCnt; i++)
        {
          long recErrVal = IntTool.DecodeZigZag64(IntTool.ReadRawVarint64(resPacket, ref offset));
          PDBErrorCode recErrCode = (PDBErrorCode)(recErrVal);
          if (recErrCode == PDBErrorCode.PdbE_OK)
            SuccessCount++;

          InsertResult.Add(recErrCode);
        }
      }
    }

    private PDBConnection ConnObj { get; set; }

    private List<object> GetRecord(uint fieldCnt, byte[] buf, ref int offset)
    {    
      int valType = 0;
      List<object> objList = new List<object>();
      
      for (uint i = 0; i < fieldCnt; i++)
      {
        valType = (int)IntTool.ReadRawVarint32(buf, ref offset);
        object obj = GetStoreData(buf, ref offset, valType);
        objList.Add(obj);
      }

      return objList;
    }
    
    private object GetStoreData(byte[] buf, ref int offset, int valType)
    {
      object obj;
      switch ((PDBType)valType)
      {
        case PDBType.Null:
          return DBNull.Value;

        case PDBType.Bool:
          return IntTool.ReadSInt8(buf, ref offset) != 0;

        case PDBType.BigInt:
          return IntTool.DecodeZigZag64(IntTool.ReadRawVarint64(buf, ref offset));

        case PDBType.Double:
          return IntTool.ReadDouble(buf, ref offset);

        case PDBType.DateTime:      
          ulong lval= IntTool.ReadRawVarint64(buf, ref offset);
          return new DateTime(1970, 1, 1).Add(TimeZoneInfo.Local.BaseUtcOffset).AddMilliseconds(lval);

        case PDBType.String:
          int valLen = 0;
          valLen = (int)IntTool.ReadRawVarint32(buf, ref offset);
          obj= (object)Encoding.UTF8.GetString(buf, offset, valLen);
          offset += valLen;
          return obj;

        case PDBType.Blob:
          valLen = (int)IntTool.ReadRawVarint32(buf, ref offset);
          byte[] blobVal = new byte[valLen];
          Array.Copy(buf, offset, blobVal, 0, valLen);
          obj= (object)blobVal;
          offset += valLen;
          return obj;
      }
      return DBNull.Value;
    }
    
  }
}
