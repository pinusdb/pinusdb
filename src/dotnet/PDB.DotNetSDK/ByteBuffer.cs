using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PDB.DotNetSDK
{
  internal class ByteBuffer
  {
    public ByteBuffer(int bufSize)
    {
      buf_ = new byte[bufSize];
      pos_ = 0;
    }

    public bool WriteBytes(byte[] val, int offset, int dataLen)
    {
      if (pos_ + dataLen > buf_.Length)
        return false;

      Array.Copy(val, offset, buf_, pos_, dataLen);
      pos_ += dataLen;
      return true;
    }

    public int ReadBytes(byte[] distic, int offset)
    {
      if (offset + pos_ > distic.Length)
        throw new OutOfMemoryException();

      Array.Copy(buf_, 0, distic, offset, pos_);
      return pos_;
    }

    public int GetDataLength()
    {
      return pos_;
    }

    private byte[] buf_; //字节缓冲区
    private int pos_;    //已存储的位置
  }
}
