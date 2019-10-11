package cn.pinusdb.jdbc;

public class IntTool {
	
	public static int getInt32(byte[] buf, int offset) {
		int val = buf[offset + 3] & 0xFF;
		val = (val << 8) | (buf[offset + 2] & 0xFF);
		val = (val << 8) | (buf[offset + 1] & 0xFF);
		val = (val << 8) | (buf[offset] & 0xFF);
		
		return val;
	}
	
	public static void setInt32(byte[] buf, int offset, int val) {
		buf[offset++] = (byte)(val & 0xFF);
		buf[offset++] = (byte)((val >> 8) & 0xFF);
		buf[offset++] = (byte)((val >> 16) & 0xFF);
		buf[offset++] = (byte)((val >> 24) & 0xFF);
	}
	
}
