package cn.pinusdb.jdbc;

import java.io.UnsupportedEncodingException;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.List;

class ByteStream {
	private ByteBuffer curBuf_;
	private List<ByteBuffer> bufList_;

	private final byte DType_Bool = (byte)1;
	private final byte DType_TinyInt = (byte)2;
	private final byte DType_SmallInt = (byte)3;
	private final byte DType_Int = (byte)4;
	private final byte DType_Long = (byte)5;
	private final byte DType_DateTime = (byte)6;
	private final byte DType_Float = (byte)7;
	private final byte DType_Double = (byte)8;
	private final byte DType_String = (byte)9;
	private final byte DType_Blob = (byte)10;
	
	ByteStream(){
		curBuf_ = null;
		bufList_ = new ArrayList<ByteBuffer>();
	}
	
	void writeString(String val) throws SQLException {
		int pos = 0;
		byte[] typeBuf = new byte[3];
		byte[] valBuf = null;
		
		try {
			valBuf = val.getBytes("UTF-8");
		} catch (UnsupportedEncodingException e){
			throw new SQLException(PDBErrCode.errMsg(PDBErrCode.PdbE_INVALID_PARAM), "58005",
				PDBErrCode.PdbE_INVALID_PARAM);
		}
		typeBuf[pos++] = DType_String;
		if (val.length() >= 8192) {
			throw new SQLException(PDBErrCode.errMsg(PDBErrCode.PdbE_RECORD_TOO_LONG), 
				"58005", PDBErrCode.PdbE_RECORD_TOO_LONG);
		}
		pos = writeLen(typeBuf, pos, val.length());

		write(typeBuf, 0, pos);
		write(valBuf, 0, valBuf.length);
	}
	
	void writeBlob(byte[] val) throws SQLException {
		int pos = 0;
		byte[] typeBuf = new byte[3];
		
		typeBuf[pos++] = DType_Blob;
		if (val.length >= 8192) {
			throw new SQLException(PDBErrCode.errMsg(PDBErrCode.PdbE_RECORD_TOO_LONG), 
				"58005", PDBErrCode.PdbE_RECORD_TOO_LONG);
		}
		
		pos = writeLen(typeBuf, pos, val.length);
		write(typeBuf, 0, pos);
		write(val, 0, val.length);
	}
	
	void writeBoolean(boolean val) {
		byte[] buf = new byte[2];
		buf[0] = (byte)DType_Bool;
		buf[1] = (byte)(val ? 1 : 0);
		write(buf, 0, 2);
	}
	
	void writeTinyInt(byte val) {
		byte[] buf = new byte[11];
		buf[0] = (byte)DType_TinyInt;
		int offset = writeVarint64(buf, 1, encodeZigZag64(val));
		write(buf, 0, offset);
	}
	
	void writeSmallInt(short val) {
		byte[] buf = new byte[11];
		buf[0] = (byte)DType_SmallInt;
		int offset = writeVarint64(buf, 1, encodeZigZag64(val));
		write(buf, 0, offset);
	}
	
	void writeInt(int val) {
		byte[] buf = new byte[11];
		buf[0] = (byte)DType_Int;
		int offset = writeVarint64(buf, 1, encodeZigZag64(val));
		write(buf, 0, offset);
	}
	
	void writeLong(long val) {
		byte[] buf = new byte[11];
		buf[0] = (byte)DType_Long;
		int offset =writeVarint64(buf, 1, encodeZigZag64(val));
		write(buf, 0, offset);
	}
	
	void writeDateTime(Timestamp ts) {
		byte[] buf = new byte[11];
		buf[0] = (byte)DType_DateTime;
		long tsval = ts.getTime() * 1000 + (ts.getNanos() / 1000) % 1000;
		int offset = writeVarint64(buf, 1, encodeZigZag64(tsval));
		write(buf, 0, offset);
	}
	
	void writeFloat(float val) {
		byte[] buf= new byte[5];
		buf[0] = (byte)DType_Float;
		int tmpVal = Float.floatToRawIntBits(val);
		buf[1] = (byte)(tmpVal & 0xFF);
		buf[2] = (byte)((tmpVal >> 8) & 0xFF);
		buf[3] = (byte)((tmpVal >> 16) & 0xFF);
		buf[4] = (byte)((tmpVal >> 24) & 0xFF);
		write(buf, 0, 5);
	}
	
	void writeDouble(double val) {
		byte[] buf = new byte[9];
		buf[0] = (byte)DType_Double;
		long tmpVal = Double.doubleToRawLongBits(val);
		buf[1] = (byte)(tmpVal & 0xFF);
		buf[2] = (byte)((tmpVal >> 8) & 0xFF);
		buf[3] = (byte)((tmpVal >> 16) & 0xFF);
		buf[4] = (byte)((tmpVal >> 24) & 0xFF);
		buf[5] = (byte)((tmpVal >> 32) & 0xFF);
		buf[6] = (byte)((tmpVal >> 40) & 0xFF);
		buf[7] = (byte)((tmpVal >> 48) & 0xFF);
		buf[8] = (byte)((tmpVal >> 56) & 0xFF);
		write(buf, 0, 9);
	}
	
	long encodeZigZag64(long n) {
		return (n << 1) ^ (n >> 63);
	}
	
	int writeVarint64(byte[] buf, int offset, long val) {
		while (true) {
			if ((val & ~0x7F) == 0) {
				buf[offset++] = (byte)val;
				return offset;
			} else {
				buf[offset++] = (byte)((val & 0x7F) | 0x80);
				val >>>= 7;
			}
		}
	}
	
	int writeLen(byte[] buf, int offset, int val) {
		while (true) {
			if ((val & ~0x7F) == 0) {
				buf[offset++] = (byte)val;
				return offset;
			} else {
				buf[offset++] = (byte)((val & 0x7F) | 0x80);
				val >>>= 7;
			}
		}
	}
	
	void write(byte[] val, int offset, int dataLen) {
		final int bufLen = 128;
		if (curBuf_ == null) {
			curBuf_ = new ByteBuffer((dataLen > bufLen ? dataLen : bufLen));
			bufList_.add(curBuf_);
		}
		
		if (!curBuf_.writeBytes(val, offset, dataLen)) {
			curBuf_ = new ByteBuffer((dataLen > bufLen ? dataLen : bufLen));
			bufList_.add(curBuf_);
			curBuf_.writeBytes(val, offset, dataLen);
		}
	}
	
	int getTotalLen() {
		int totalLen = 0;
		for(ByteBuffer buf : bufList_) {
			totalLen += buf.getDataLength();
		}
		
		return totalLen;
	}
	
	void read(byte[] disBuf, int offset) {
		for(ByteBuffer buf : bufList_) {
			offset += buf.readBytes(disBuf, offset);
		}
	}
	
}
