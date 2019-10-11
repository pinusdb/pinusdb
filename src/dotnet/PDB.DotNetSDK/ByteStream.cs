using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PDB.DotNetSDK
{
  internal class ByteStream
  {
    public ByteStream()
    {
      curBuf_ = null;
      bufList_ = new List<ByteBuffer>();
    }

    public void Write(byte[] val, int offset, int dataLen)
    {
      const int bufLen = 128;
      if(curBuf_ == null)
      {
        curBuf_ = new ByteBuffer((dataLen > bufLen ? dataLen : bufLen));
        bufList_.Add(curBuf_);
      }

      if (!curBuf_.WriteBytes(val, offset, dataLen))
      {
        curBuf_ = new ByteBuffer((dataLen > bufLen ? dataLen : bufLen));
        bufList_.Add(curBuf_);
        curBuf_.WriteBytes(val, offset, dataLen);
      }
    }

    public int GetTotalLen()
    {
      int totalLen = 0;
      foreach(ByteBuffer buf in bufList_)
      {
        totalLen += buf.GetDataLength();
      }

      return totalLen;
    }

    public void Read(byte[] disBuf, int offset)
    {
      foreach(ByteBuffer buf in bufList_)
      {
        offset += buf.ReadBytes(disBuf, offset);
      }
    }

    private ByteBuffer curBuf_;
    private List<ByteBuffer> bufList_; 
  }
}
