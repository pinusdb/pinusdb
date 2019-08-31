package cn.pinusdb.jdbc;

import java.sql.SQLException;

class ProtoHeader {
	
	public static int getHeadLen() {
		return 64;
	}
	
	public static int getProtoVersion() {
		return 0x00010000;
	}
	
	public static int getMethodCmdLoginReq() {
		return 0x010001;
	}
	
	public static int getMethodCmdLoginRep() {
		return 0x010002;
	}
	
	public static int getMethodCmdQueryReq() {
		return 0x010003;
	}
	
	public static int getMethodCmdQueryRep() {
		return 0x010004;
	}
	
	public static int getMethodCmdInsertReq() {
		return 0x010005;
	}
	
	public static int getMethodCmdInsertRep() {
		return 0x010006;
	}
	
	public static int getMethodCmdNonQueryReq() {
		return 0x010007;
	}
	
	public static int getMethodCmdNonQueryRep() {
		return 0x010008;
	}
	
	public static int getMethodCmdInsertTabReq() {
		return 0x010009;
	}
	
	public static int getMethodCmdInsertTabRep() {
		return 0x01000A;
	}
	
	private final int kHeadVerOffset = 0;
	private final int kHeadMethodOffset = 4;
	private final int kHeadBodyLenOffset = 8;
	private final int kHeadRetValOffset = 12;
	private final int kHeadFeidlCntOffset = 16;
	private final int kHeadRecordCntOffset = 20;
	private final int kHeadErrPosOffset = 24;
	private final int kHeadBodyCrcOffset = 56;
	private final int kHeadHeadCrcOffset = 60;
	
	private byte[] headProto_;
	
	public ProtoHeader(byte[] buffer) throws SQLException {
		if (buffer == null) {
			this.headProto_ = new byte[getHeadLen()];
		}
		else {
			if (buffer.length < getHeadLen())
				throw new SQLException("报文错误，报文长度不合法", "58005", PDBErrCode.PdbE_PACKET_ERROR);
			
			this.headProto_ = buffer;
		}
	}
	
	public int getVersion() {
		return IntTool.getInt32(headProto_, kHeadVerOffset);
	}
	
	public int getMethod() {
		return IntTool.getInt32(headProto_, kHeadMethodOffset);
	}
	
	public int getBodyLen() {
		return IntTool.getInt32(headProto_, kHeadBodyLenOffset);
	}
	
	public int getReturnVal() {
		return IntTool.getInt32(headProto_, kHeadRetValOffset);
	}
	
	public int getFieldCnt() {
		return IntTool.getInt32(headProto_, kHeadFeidlCntOffset);
	}
	
	public int getRecordCnt() {
		return IntTool.getInt32(headProto_, kHeadRecordCntOffset);
	}
	
	public int getErrPos() {
		return IntTool.getInt32(headProto_, kHeadErrPosOffset);
	}
	
	public int getBodyCrc() {
		return IntTool.getInt32(headProto_, kHeadBodyCrcOffset);
	}
	
	public int getHeadCrc() {
		return IntTool.getInt32(headProto_, kHeadHeadCrcOffset);
	}
	
	public void setVersion(int version) {
		IntTool.setInt32(headProto_, kHeadVerOffset, version);
	}
	
	public void setMethod(int methodId) {
		IntTool.setInt32(headProto_, kHeadMethodOffset, methodId);
	}
	
	public void setBodyLen(int bodyLen) {
		IntTool.setInt32(headProto_, kHeadBodyLenOffset, bodyLen);
	}
	
	public void setReturnVal(int retVal) {
		IntTool.setInt32(headProto_, kHeadRetValOffset, retVal);
	}
	
	public void setFieldCnt(int fieldCnt) {
		IntTool.setInt32(headProto_, kHeadFeidlCntOffset, fieldCnt);
	}
	
	public void setRecordCnt(int recCnt) {
		IntTool.setInt32(headProto_, kHeadRecordCntOffset, recCnt);
	}
	
	public void setErrPos(int errPos) {
		IntTool.setInt32(headProto_, kHeadErrPosOffset, errPos);
	}
	
	public void setBodyCrc(int bodyCrc) {
		IntTool.setInt32(headProto_, kHeadBodyCrcOffset, bodyCrc);
	}
	
	public void updateHeadCrc() {
		IntTool.setInt32(headProto_, kHeadHeadCrcOffset, CRC.crc32(headProto_, 0, kHeadHeadCrcOffset));
	}
}
