using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PDB.DotNetSDK
{
  internal class ProtoHeader
  {
    public static int kHeadLen { get { return 64; } }
    public static uint kProtoVersion { get { return 0x00010000; } }
    
    public static uint MethodCmdLoginReq { get { return 0x010001; } }
    public static uint MethodCmdLoginRep { get { return 0x010002; } }
    public static uint MethodCmdQueryReq { get { return 0x010003; } }
    public static uint MethodCmdQueryRep { get { return 0x010004; } }
    public static uint MethodCmdInsertReq { get { return 0x010005; } }
    public static uint MethodCmdInsertRep { get { return 0x010006; } }
    public static uint MethodCmdNonQueryReq { get { return 0x010007; } }
    public static uint MethodCmdNonQueryRep { get { return 0x010008; } }
    public static uint MethodCmdInsertTableReq { get { return 0x010009; } }
    public static uint MethodCmdInsertTableRep { get { return 0x01000A; } }


    private const int kHeadVerOffset = 0;
    private const int kHeadMethodOffset = 4;
    private const int kHeadBodyLenOffset = 8;
    private const int kHeadRetValOffset = 12;
    private const int kHeadFieldCntOffset = 16;
    private const int kHeadRecordCntOffset = 20;
    private const int kHeadErrPosOffset = 24;
    private const int kHeadBodyCrcOffset = 56;
    private const int kHeadHeadCrcOffset = 60;

    public ProtoHeader(byte[] buffer)
    {
      if (buffer == null)
      {
        headProto = new byte[kHeadLen];
      }
      else
      {
        if (buffer.Count() < kHeadLen)
          throw new PDBException(PDBErrorCode.PdbE_PACKET_ERROR, "报文错误：报文头长度不合法");
        headProto = buffer;
      }
    }
    
    public UInt32 GetVersion()
    {
      return IntTool.GetUint32(headProto, kHeadVerOffset);
    }

    public UInt32 GetMethod()
    {
      return IntTool.GetUint32(headProto, kHeadMethodOffset);
    }

    public UInt32 GetBodyLen()
    {
      return IntTool.GetUint32(headProto, kHeadBodyLenOffset);
    }

    public UInt32 GetReturnVal()
    {
      return IntTool.GetUint32(headProto, kHeadRetValOffset);
    }

    public UInt32 GetFieldCnt()
    {
      return IntTool.GetUint32(headProto, kHeadFieldCntOffset);
    }

    public UInt32 GetRecordCnt()
    {
      return IntTool.GetUint32(headProto, kHeadRecordCntOffset);
    }

    public UInt32 GetErrPos()
    {
      return IntTool.GetUint32(headProto, kHeadErrPosOffset);
    }

    public UInt32 GetBodyCrc()
    {
      return IntTool.GetUint32(headProto, kHeadBodyCrcOffset);
    }

    public UInt32 GetHeadCrc()
    {
      return IntTool.GetUint32(headProto, kHeadHeadCrcOffset);
    }
    
    public void SetVersion(UInt32 version)
    {
      IntTool.PutUint32(headProto, kHeadVerOffset, version);
    }

    public void SetMethod(UInt32 methodId)
    {
      IntTool.PutUint32(headProto, kHeadMethodOffset, methodId);
    }

    public void SetBodyLen(UInt32 bodyLen)
    {
      IntTool.PutUint32(headProto, kHeadBodyLenOffset, bodyLen);
    }

    public void SetReturnVal(UInt32 retVal)
    {
      IntTool.PutUint32(headProto, kHeadRetValOffset, retVal);
    }

    public void SetFieldCnt(UInt32 fieldCnt)
    {
      IntTool.PutUint32(headProto, kHeadFieldCntOffset, fieldCnt);
    }

    public void SetRecordCnt(UInt32 recordCnt)
    {
      IntTool.PutUint32(headProto, kHeadRecordCntOffset, recordCnt);
    }

    public void SetErrPos(UInt32 errPos)
    {
      IntTool.PutUint32(headProto, kHeadErrPosOffset, errPos);
    }

    public void SetBodyCrc(UInt32 bodyCrc)
    {
      IntTool.PutUint32(headProto, kHeadBodyCrcOffset, bodyCrc);
    }

    public void SetHeadCrc()
    {
      IntTool.PutUint32(headProto, kHeadHeadCrcOffset, CRC.Crc32(headProto, 0, 60));
    }
    
    private byte[] headProto { get; set; }
    
  }
}
