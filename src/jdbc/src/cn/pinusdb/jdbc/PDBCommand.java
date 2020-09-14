package cn.pinusdb.jdbc;


import java.io.UnsupportedEncodingException;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.List;

public class PDBCommand {
	private int successCnt = 0;
	public int getSuccessCnt() {
		return successCnt;
	}
	
	private PDBConnection conn_;
	
	public PDBCommand(PDBConnection conn) {
		conn_ = conn;
	}
	
	public int executeInsert(String sql) throws SQLException {
		int retVal = PDBErrCode.PdbE_OK;
		successCnt = 0;
		synchronized (conn_) {
			byte[] sendPacket = makeRequestPacket(ProtoHeader.getMethodCmdInsertReq(), sql);
			conn_.request(sendPacket);
			
			byte[] recvPacket = conn_.recv();
			retVal = decodeInsertPacket(recvPacket);
		}
		
		return retVal;
	}
	
	public int[] executeInsertTable(String tabName, List<String> colList, List<Object> valList) throws SQLException {
		int retVal = PDBErrCode.PdbE_OK;
		int[] errArr = new int[valList.size() / colList.size()];
		successCnt = 0;
		byte[] sendPacket = makeInsertTabRequestPacket(tabName, colList, valList);
		synchronized (conn_) {
			conn_.request(sendPacket);
			
			byte[] recvPacket = conn_.recv();
			retVal = decodeInsertTabPacket(recvPacket, errArr);
		}
		
		if (retVal == PDBErrCode.PdbE_OK
			|| retVal == PDBErrCode.PdbE_INSERT_PART_ERROR) {
			return errArr;
		}
			
		throw new SQLException(PDBErrCode.errMsg(retVal), "58005", retVal);
	}
	
	public ResultSet executeQuery(String sql) throws SQLException {
		successCnt = 0;
		int retVal = PDBErrCode.PdbE_OK;
		PDBResultSet resultSet = new PDBResultSet();
		synchronized (conn_) {
			byte[] sendPacket = makeRequestPacket(ProtoHeader.getMethodCmdQueryReq(), sql);
			conn_.request(sendPacket);
			
			byte[] recvPacket = conn_.recv();
			retVal = decodePacket(recvPacket, resultSet);
			if (retVal != PDBErrCode.PdbE_OK) {
				throw new SQLException(PDBErrCode.errMsg(retVal), "58005", retVal);
			}
		}
		
		return resultSet;
	}
	
	public void executeNonQuery(String sql) throws SQLException {
		successCnt = 0;
		synchronized (conn_) {
			byte[] sendPacket = makeRequestPacket(ProtoHeader.getMethodCmdNonQueryReq(), sql);
			conn_.request(sendPacket);
			
			byte[] recvPacket = conn_.recv();
			ProtoHeader proHdr = new ProtoHeader(recvPacket);
			
			if (proHdr.getMethod() != ProtoHeader.getMethodCmdNonQueryRep()) {
				throw new SQLException("报文错误", "58005", PDBErrCode.PdbE_PACKET_ERROR);
			}
			
			int retVal = proHdr.getReturnVal();
			if (retVal != PDBErrCode.PdbE_OK) {
				throw new SQLException(PDBErrCode.errMsg(retVal), "58005", retVal);
			}
		}
	}
	
	private byte[] makeRequestPacket(int reqMethod, String sql) throws SQLException {
		int headLen = ProtoHeader.getHeadLen();
		int bodyLen = 0;
		int totalLen = 0;
		
		if (sql == null || sql.length() == 0 || sql.length() > (4 * 1024 * 1024)) {
			throw new SQLException("非法的SQL语句", "58005", PDBErrCode.PdbE_SQL_ERROR);
		}
		
		byte[] tmpSql = null;
		try {
			tmpSql = sql.getBytes("UTF-8");
		} catch (UnsupportedEncodingException e) {
			throw new SQLException("非法的SQL语句", "58005", PDBErrCode.PdbE_SQL_ERROR);
		}
		
		bodyLen = tmpSql.length;
		totalLen = headLen + bodyLen;
		
		byte[] sendBuf = new byte[totalLen];
		for(int i = 0; i < sendBuf.length; i++) {
			sendBuf[i] = 0;
		}
			
		System.arraycopy(tmpSql, 0, sendBuf, headLen, tmpSql.length);
		
		int bodyCrc = CRC.crc32(sendBuf, headLen, bodyLen);
		
		ProtoHeader proHdr = new ProtoHeader(sendBuf);
		proHdr.setVersion(ProtoHeader.getProtoVersion());
		proHdr.setMethod(reqMethod);
		proHdr.setRecordCnt(1);
		proHdr.setBodyLen(bodyLen);
		proHdr.setBodyCrc(bodyCrc);
		proHdr.updateHeadCrc();
		
		return sendBuf;
	}

	private byte[] makeInsertTabRequestPacket(String tabName, List<String> colList, List<Object> valList) throws SQLException {
		int headLen = ProtoHeader.getHeadLen();
		int bodyLen = 0;
		int totalLen = 0;
		ByteStream packetStream = new ByteStream();
		packetStream.writeString(tabName);
		
		for(String name : colList) {
			packetStream.writeString(name);
		}
		
		for(Object val : valList) {
			if (val instanceof Boolean) {
				packetStream.writeBoolean((Boolean)val);
			} else if (val instanceof Byte) {
				packetStream.writeTinyInt((Byte)val);
			} else if (val instanceof Short) {
				packetStream.writeSmallInt((Short)val);
			} else if (val instanceof Integer) {
				packetStream.writeInt((Integer)val);
			} else if (val instanceof Long) {
				packetStream.writeLong((Long)val);
			} else if (val instanceof Timestamp) {
				packetStream.writeDateTime((Timestamp)val);
			} else if (val instanceof Float) {
				packetStream.writeFloat((Float)val);
			} else if (val instanceof Double) {
				packetStream.writeDouble((Double)val);
			} else if (val instanceof String) {
				packetStream.writeString((String)val);
			} else if (val instanceof byte[]) {
				packetStream.writeBlob((byte[])val);
			} else {
				throw new SQLException(PDBErrCode.errMsg(PDBErrCode.PdbE_INVALID_PARAM),
					"58005", PDBErrCode.PdbE_INVALID_PARAM);
			}
		}
		
		bodyLen = packetStream.getTotalLen();
		totalLen = headLen + bodyLen;
		
		byte[] packetBuf = new byte[totalLen];
		for(int i = 0; i < headLen; i++) {
			packetBuf[i] = 0;
		}
		packetStream.read(packetBuf, headLen);

		int bodyCrc = CRC.crc32(packetBuf, headLen, bodyLen);
		ProtoHeader proHdr = new ProtoHeader(packetBuf);
		proHdr.setVersion(ProtoHeader.getProtoVersion());
		proHdr.setMethod(ProtoHeader.getMethodCmdInsertTabReq());
		proHdr.setRecordCnt(valList.size() / colList.size());
		proHdr.setFieldCnt(colList.size());
		proHdr.setBodyLen(bodyLen);
		proHdr.setBodyCrc(bodyCrc);
		proHdr.updateHeadCrc();
		
		return packetBuf;
	}
	
	private int decodeInsertPacket(byte[] recvPacket) throws SQLException {
		ProtoHeader proHdr = new ProtoHeader(recvPacket);
		int retErrCode = proHdr.getReturnVal();
		
		if (recvPacket.length != ProtoHeader.getHeadLen()) {
			return PDBErrCode.PdbE_PACKET_ERROR;
		}
		
		successCnt = proHdr.getRecordCnt();
		return retErrCode;
	}

	private int decodeInsertTabPacket(byte[] recvPacket, int[] errArray) throws SQLException {
		int retVal = PDBErrCode.PdbE_OK;
		
		ProtoHeader proHdr = new ProtoHeader(recvPacket);
		retVal = proHdr.getReturnVal();
		int recCnt = proHdr.getRecordCnt();
		if (retVal == PDBErrCode.PdbE_OK) {
			successCnt = recCnt;
			for (int i = 0; i < recCnt; i++) {
				errArray[i] = PDBErrCode.PdbE_OK;
			}
		} else if (retVal == PDBErrCode.PdbE_INSERT_PART_ERROR) {
			BinReader binReader = new BinReader();
			binReader.loadBuffer(recvPacket);
			binReader.setOffset(ProtoHeader.getHeadLen());
			successCnt = 0;
			for (int i = 0; i < recCnt; i++) {
				errArray[i] = (int)binReader.readLongByVarint();
				if (errArray[i] == PDBErrCode.PdbE_OK) {
					successCnt++;
				}
			}
		}
		
		return retVal;
	}
	
	private int decodePacket(byte[] recvPacket, PDBResultSet resultSet) 
		throws SQLException {
		int retVal = PDBErrCode.PdbE_OK;
		
		ProtoHeader proHdr = new ProtoHeader(recvPacket);
		retVal = proHdr.getReturnVal();
		if (retVal != PDBErrCode.PdbE_OK) {
			return retVal;
		}
		
		int colCnt = proHdr.getFieldCnt();
		BinReader binReader = new BinReader();
		binReader.loadBuffer(recvPacket);
		binReader.setOffset(ProtoHeader.getHeadLen());
		
		List<Object> headList = getRecord(binReader, colCnt);
		for (Object obj : headList) {
			if (obj instanceof String) {
				String colStr = (String)obj;
				String[] partArr = colStr.split(";");
				if (partArr.length != 2) {
					throw new SQLException("报文错误,非法的表头", "58005", PDBErrCode.PdbE_PACKET_ERROR);
				}
				
				ColumnInfo colInfo = null;
				if (partArr[0].compareTo("bool") == 0) {
					colInfo = new ColumnInfo(partArr[1], PDBDataType.PDB_Bool);
				} else if (partArr[0].compareTo("tinyint") == 0) {
					colInfo = new ColumnInfo(partArr[1], PDBDataType.PDB_TinyInt);
				} else if (partArr[0].compareTo("smallint") == 0) {
					colInfo = new ColumnInfo(partArr[1], PDBDataType.PDB_SmallInt);
				} else if (partArr[0].compareTo("int") == 0) {
					colInfo = new ColumnInfo(partArr[1], PDBDataType.PDB_Int);
				} else if (partArr[0].compareTo("bigint") == 0) {
					colInfo = new ColumnInfo(partArr[1], PDBDataType.PDB_BigInt);
				} else if (partArr[0].compareTo("datetime") == 0) {
					colInfo = new ColumnInfo(partArr[1], PDBDataType.PDB_DateTime);
				} else if (partArr[0].compareTo("float") == 0) {
					colInfo = new ColumnInfo(partArr[1], PDBDataType.PDB_Float);
				} else if (partArr[0].compareTo("double") == 0) {
					colInfo = new ColumnInfo(partArr[1], PDBDataType.PDB_Double);
				} else if (partArr[0].compareTo("string") == 0) {
					colInfo = new ColumnInfo(partArr[1], PDBDataType.PDB_String);
				} else if (partArr[0].compareTo("blob") == 0) {
					colInfo = new ColumnInfo(partArr[1], PDBDataType.PDB_Blob);
				} else {
					throw new SQLException("报文错误", "58005", PDBErrCode.PdbE_PACKET_ERROR);
				}
					
				resultSet.addColumnInfo(colInfo);
			} else {
				throw new SQLException("报文错误", "58005", PDBErrCode.PdbE_PACKET_ERROR);
			}
		}
		
		int rowCnt = proHdr.getRecordCnt();
		for(int i = 0; i < rowCnt; i++) {
			List<Object> recList = getRecord(binReader, colCnt);
			for (Object obj : recList) {
				resultSet.addValue(obj);
			}
		}
		
		return retVal;
	}
	
	private List<Object> getRecord(BinReader binReader, int colCnt) throws SQLException {
		int valType = 0;
		List<Object> objList = new ArrayList<Object>();
		
		for (int i= 0; i < colCnt; i++) {
			valType = binReader.readInt8();
			Object objVal = null;
			switch(valType) {
			case 0: break;
			case 1: objVal = binReader.readBoolean(); break;
			case 2: objVal = binReader.readTinyInt(); break;
			case 3: objVal = binReader.readSmallInt(); break;
			case 4: objVal = binReader.readInt(); break;
			case 5: objVal = binReader.readLongByVarint(); break;
			case 6: objVal = binReader.readDateTime(); break;
			case 7: objVal = binReader.readFloat(); break;
			case 8: objVal = binReader.readDouble(); break;
			case 9: objVal = binReader.readString(); break;
			case 10: objVal = binReader.readBlob(); break;
			default:
				throw new SQLException("报文错误", "58005", PDBErrCode.PdbE_PACKET_ERROR);
			}
			objList.add(objVal);
		}
		
		return objList;
	}
	
}
