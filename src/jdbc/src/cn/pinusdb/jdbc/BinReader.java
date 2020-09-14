package cn.pinusdb.jdbc;

import java.io.UnsupportedEncodingException;
import java.sql.SQLException;
import java.sql.Timestamp;

public class BinReader {

	public BinReader() {
		this.buf_ = null;
		this.offset_ = 0;
	}
	
	public void loadBuffer(byte[] buf) {
		this.buf_ = buf;
		this.offset_ = 0;
	}
	
	public void setOffset(int offset) {
		this.offset_ = offset;
	}
	
	public long readVarint64() {
		int shift = 0;
		long result = 0;
		while (shift < 65) {
			byte b = buf_[offset_++];
			result |= (((long)(b & 0x7F)) << shift);
			if ((b & 0x80) == 0)
				return result;
			
			shift += 7;
		}
		return result;
	}
	
	public long readLongByVarint() {
		long uval = readVarint64();
		return (uval >>> 1) ^ (-(uval & 1));
	}
	
	public byte readInt8() {
		return (byte)buf_[offset_++];
	}
	
	public Timestamp readDateTime() {
		long lval = readLongByVarint();
		long ms = (lval / 1000000) * 1000;
		int nanos = (int)(lval % 1000000) * 1000;
		Timestamp ts = new Timestamp(ms);
		ts.setNanos(nanos);
		return ts;
	}
	
	public boolean readBoolean() {
		return (readInt8() != 0);
	}
	
	public byte readTinyInt() {
		return (byte)readLongByVarint();
	}
	
	public short readSmallInt() {
		return (short)readLongByVarint();
	}
	
	public int readInt() {
		return (int)readLongByVarint();
	}
	
	public float readFloat() {
		int valBits = buf_[offset_ + 3] & 0xFF;
		valBits = (valBits << 8) | (buf_[offset_ + 2] & 0xFF);
		valBits = (valBits << 8) | (buf_[offset_ + 1] & 0xFF);
		valBits = (valBits << 8) | (buf_[offset_] & 0xFF);
		offset_ += 4;
		return Float.intBitsToFloat(valBits);
	}
	
	public double readDouble() {
	    long valBits = buf_[offset_ + 7] & 0xFF;
	    valBits = (valBits << 8) | (buf_[offset_ + 6] & 0xFF);
	    valBits = (valBits << 8) | (buf_[offset_ + 5] & 0xFF);
	    valBits = (valBits << 8) | (buf_[offset_ + 4] & 0xFF);
	    valBits = (valBits << 8) | (buf_[offset_ + 3] & 0xFF);
	    valBits = (valBits << 8) | (buf_[offset_ + 2] & 0xFF);
	    valBits = (valBits << 8) | (buf_[offset_ + 1] & 0xFF);
	    valBits = (valBits << 8) | (buf_[offset_] & 0xFF);
	    offset_ += 8;
		return Double.longBitsToDouble(valBits);
	}
	
	public String readString() throws SQLException {
		int strLen = (int)readVarint64();
		if (offset_ + strLen > buf_.length) {
			throw new SQLException(PDBErrCode.errMsg(PDBErrCode.PdbE_PACKET_ERROR), "58005", PDBErrCode.PdbE_PACKET_ERROR);
		}
		
		String strVal = "";
		if (strLen > 0) {
			try {
				strVal = new String(buf_, offset_, strLen, "UTF-8");
			} catch (UnsupportedEncodingException e) {
				throw new SQLException(PDBErrCode.errMsg(PDBErrCode.PdbE_PACKET_ERROR), "58005", PDBErrCode.PdbE_PACKET_ERROR);
			}
		}

		offset_ += strLen;
		return strVal;
	}
	
	public byte[] readBlob() throws SQLException {
		int valLen = (int)readVarint64();
		if (offset_ + valLen > buf_.length) {
			throw new SQLException(PDBErrCode.errMsg(PDBErrCode.PdbE_PACKET_ERROR), "58005", PDBErrCode.PdbE_PACKET_ERROR);
		}
		
		byte[] blobVal = new byte[valLen];
		System.arraycopy(buf_, offset_, blobVal, 0, valLen);
		offset_ += valLen;
		return blobVal;
	}
	
	private byte[] buf_;
	private int offset_;
}
