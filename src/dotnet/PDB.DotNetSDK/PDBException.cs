using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace PDB.DotNetSDK
{
  public class PDBException : Exception
  {
    public PDBErrorCode errorCode { get; private set; }
    private string errorInfo;
    public PDBException(PDBErrorCode errorCode)
    {
      this.errorCode = errorCode;
      this.errorInfo = PDBErrorMsg.GetErrorMsg(errorCode);
    }
    public PDBException(PDBErrorCode errorCode,string message):base(message)
    {
      this.errorCode = errorCode;
      this.errorInfo = message;
    }
   
    public override string Message
    {
      get
      {
        return errorCode+":"+errorInfo;
      }
    }
  }
}
