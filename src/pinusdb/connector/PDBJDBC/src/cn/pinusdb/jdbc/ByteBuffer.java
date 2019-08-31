package cn.pinusdb.jdbc;

class ByteBuffer {
	private byte[] buf_;
	private int pos_;
	
	ByteBuffer(int bufSize) {
		buf_ = new byte[bufSize];
		pos_ = 0;
	}
	
	boolean writeBytes(byte[] val, int offset, int dataLen) {
		if (pos_ + dataLen > buf_.length) {
			return false;
		}
		
		System.arraycopy(val, offset, buf_, pos_, dataLen);
		pos_ += dataLen;
		return true;
	}
	
	int readBytes(byte[] distic, int offset) {
		System.arraycopy(buf_, 0, distic, offset, pos_);
		return pos_;
	}
	
	int getDataLength() {
		return pos_;
	}
}
