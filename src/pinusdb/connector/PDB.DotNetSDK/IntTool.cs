using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PDB.DotNetSDK
{

  internal class IntTool
  {
    public static long DecodeZigZag64(ulong n)
    {
      return (long)(n >> 1) ^ -(long)(n & 1);
    }
    public static ulong EncodeZigZag64(long n)
    {
      return (ulong)((n << 1) ^ (n >> 63));
    }

    public static sbyte ReadSInt8(byte[] buffer, ref int offset)
    {
      sbyte result = (sbyte)buffer[offset];
      offset++;
      return result;
    }
    
    public static uint ReadRawVarint32(byte[] buffer, ref int offset)
    {
      int tmp = buffer[offset++];
      if (tmp < 128)
      {
        return (uint)tmp;
      }
      int result = tmp & 0x7f;
      if ((tmp = buffer[offset++]) < 128)
      {
        result |= tmp << 7;
      }
      else
      {
        result |= (tmp & 0x7f) << 7;
        if ((tmp = buffer[offset++]) < 128)
        {
          result |= tmp << 14;
        }
        else
        {
          result |= (tmp & 0x7f) << 14;
          if ((tmp = buffer[offset++]) < 128)
          {
            result |= tmp << 21;
          }
          else
          {
            result |= (tmp & 0x7f) << 21;
            result |= (tmp = buffer[offset++]) << 28;
            if (tmp >= 128)
            {
              for (int i = 0; i < 5; i++)
              {
                if (buffer[offset++] < 128)
                {
                  return (uint)result;
                }
              }
            }
          }
        }
      }
      return (uint)result;
    }

    public static ulong ReadRawVarint64(byte[] buffer, ref int offset)
    {
      int shift = 0;
      ulong result = 0;
      while (shift < 64)
      {
        byte b = buffer[offset++];
        result |= (ulong)(b & 0x7F) << shift;
        if ((b & 0x80) == 0)
        {
          return result;
        }
        shift += 7;
      }
      return result;
    }
    static void Copy(byte[] src, int srcOffset, byte[] dst, int dstOffset, int count)
    {
      if (count > 12)
      {
        Buffer.BlockCopy(src, srcOffset, dst, dstOffset, count);
      }
      else
      {
        int stop = srcOffset + count;
        for (int i = srcOffset; i < stop; i++)
        {
          dst[dstOffset++] = src[i];
        }
      }
    }
    public static float ReadFloat(byte[] buffer, ref int offset)
    {
      if (BitConverter.IsLittleEndian)
      {
        float ret = BitConverter.ToSingle(buffer, offset);
        offset += 4;
        return ret;
      }
      else
      {
        int size = 4;
        byte[] bytes = new byte[size];
        Copy(buffer, offset, bytes, 0, size);
        offset += size;
        for (int first = 0, last = bytes.Length - 1; first < last; first++, last--)
        {
          byte temp = bytes[first];
          bytes[first] = bytes[last];
          bytes[last] = temp;
        }
        return BitConverter.ToSingle(bytes, 0);
      }
    }

    public static double ReadDouble(byte[] buffer, ref int offset)
    {
      return BitConverter.Int64BitsToDouble((long)ReadRawLittleEndian64(buffer,ref offset));
    }
    static ulong ReadRawLittleEndian64(byte[] buffer, ref int offset)
    {
      ulong b1 = buffer[offset++];
      ulong b2 = buffer[offset++];
      ulong b3 = buffer[offset++];
      ulong b4 = buffer[offset++];
      ulong b5 = buffer[offset++];
      ulong b6 = buffer[offset++];
      ulong b7 = buffer[offset++];
      ulong b8 = buffer[offset++];
      return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24)
             | (b5 << 32) | (b6 << 40) | (b7 << 48) | (b8 << 56);
    }

    public static void WriteNull(ByteStream byteStream)
    {
      byte[] buf = new byte[1];
      buf[0] = (byte)PDBType.Null;
      byteStream.Write(buf, 0, 1);
    }

    public static void WriteVarint64(ByteStream byteStream, long value)
    {
      const int B = 128;
      int pos = 0;
      ulong uVal = IntTool.EncodeZigZag64(value);
      byte[] buf = new byte[10];
      buf[pos++] = (byte)PDBType.BigInt;
      
      while (uVal >= B)
      {
        buf[pos++] = (byte)((uVal & (B - 1)) | B);
        uVal >>= 7;
      }

      buf[pos++] = (byte)uVal;
      byteStream.Write(buf, 0, pos);
    }
    
    public static void WriteDouble(ByteStream byteStream, double value)
    {
      byte[] tmpBuf = new byte[9];
      byte[] valBuf = BitConverter.GetBytes(value);

      tmpBuf[0] = (byte)PDBType.Double;
      if (!BitConverter.IsLittleEndian)
      {
        tmpBuf[1] = valBuf[7];
        tmpBuf[2] = valBuf[6];
        tmpBuf[3] = valBuf[5];
        tmpBuf[4] = valBuf[4];
        tmpBuf[5] = valBuf[3];
        tmpBuf[6] = valBuf[2];
        tmpBuf[7] = valBuf[1];
        tmpBuf[8] = valBuf[0];
      }
      else
      {
        tmpBuf[1] = valBuf[0];
        tmpBuf[2] = valBuf[1];
        tmpBuf[3] = valBuf[2];
        tmpBuf[4] = valBuf[3];
        tmpBuf[5] = valBuf[4];
        tmpBuf[6] = valBuf[5];
        tmpBuf[7] = valBuf[6];
        tmpBuf[8] = valBuf[7];
      }

      byteStream.Write(tmpBuf, 0, 9);
    }

    public static void WriteDatetime(ByteStream byteStream, DateTime value)
    {
      const int B = 128;
      int pos = 0;
      byte[] buf = new byte[10];
      buf[pos++] = (byte)PDBType.DateTime;

      if (value.Year >= 3000)
        throw new PDBException(PDBErrorCode.PdbE_INVALID_DATETIME_VAL);

      long dtVal = (value.ToUniversalTime().Ticks - 621355968000000000) / 10000;
      if (dtVal < 0)
        throw new PDBException(PDBErrorCode.PdbE_INVALID_DATETIME_VAL);

      ulong uDtVal = (ulong)dtVal;
      
      while(uDtVal >= B)
      {
        buf[pos++] = (byte)((uDtVal & (B - 1)) | B);
        uDtVal >>= 7;
      }

      buf[pos++] = (byte)uDtVal;
      byteStream.Write(buf, 0, pos);
    }

    public static void WriteBool(ByteStream byteStream, bool value)
    {
      byte[] buf = new byte[2];
      buf[0] = (byte)PDBType.Bool;

      buf[1] = (byte)(value ? 1 : 0);
      byteStream.Write(buf, 0, 2);
    }

    public static void WriteString(ByteStream byteStream, string value)
    {
      const int B = 128;
      int pos = 0;
      byte[] typeBuf = new byte[3];
      byte[] valBuf = Encoding.UTF8.GetBytes(value);
      typeBuf[pos++] = (byte)PDBType.String;
      uint uLen = (uint)valBuf.Length;
      if (uLen >= 8192)
        throw new PDBException(PDBErrorCode.PdbE_RECORD_TOO_LONG);

      if (uLen >= B)
      {
        typeBuf[pos++] = (byte)((uLen & (B - 1)) | B);
        uLen >>= 7;
      }
      typeBuf[pos++] = (byte)uLen;
      byteStream.Write(typeBuf, 0, pos);
      byteStream.Write(valBuf, 0, valBuf.Length);
    }

    public static void WriteBlob(ByteStream byteStream, byte[] value)
    {
      const int B = 128;
      int pos = 0;
      byte[] typeBuf = new byte[3];
      typeBuf[pos++] = (byte)PDBType.Blob;
      uint uLen = (uint)value.Length;
      if (uLen >= 8192)
        throw new PDBException(PDBErrorCode.PdbE_RECORD_TOO_LONG);

      if (uLen >= B)
      {
        typeBuf[pos++] = (byte)((uLen & (B - 1)) | B);
        uLen >>= 7;
      }
      typeBuf[pos++] = (byte)uLen;
      byteStream.Write(typeBuf, 0, pos);
      byteStream.Write(value, 0, value.Length);
    }
    

    public static UInt32 GetUint32(byte[] buf, int offset)
    {
      UInt32 val = buf[offset + 3];
      val = (val << 8) | buf[offset + 2];
      val = (val << 8) | buf[offset + 1];
      val = (val << 8) | buf[offset];
      return val;
    }

    public static void PutUint32(byte[] buf, int offset, UInt32 val)
    {
      buf[offset] = (byte)(val & 0xFF);
      buf[offset + 1] = (byte)((val >> 8) & 0xFF);
      buf[offset + 2] = (byte)((val >> 16) & 0xFF);
      buf[offset + 3] = (byte)((val >> 24) & 0xFF);
    }
  }
}
