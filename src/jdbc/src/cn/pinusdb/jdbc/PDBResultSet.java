package cn.pinusdb.jdbc;


import java.io.InputStream;
import java.io.Reader;
import java.math.BigDecimal;
import java.net.URL;
import java.sql.Array;
import java.sql.Blob;
import java.sql.Clob;
import java.sql.Date;
import java.sql.NClob;
import java.sql.Ref;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.RowId;
import java.sql.SQLException;
import java.sql.SQLWarning;
import java.sql.SQLXML;
import java.sql.Statement;
import java.sql.Time;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;

public class PDBResultSet implements ResultSet{
	private List<ColumnInfo> colInfoList_;
	private Hashtable<String, Integer> colPosMap_;
	private List<Object> resultDataList_;
	private int rowCnt_;
	private int curRowIdx_;
	private boolean lastGetIsNull_;
	
	PDBResultSet()
	{
		colInfoList_ = new ArrayList<ColumnInfo>();
		colPosMap_ = new Hashtable<String, Integer>();
		resultDataList_ = new ArrayList<Object>();
		rowCnt_ = 0;
		curRowIdx_ = 0;
		lastGetIsNull_ = false;
	}
	
	public void addColumnInfo(ColumnInfo colInfo) {
		colInfoList_.add(colInfo);
		if (!colPosMap_.contains(colInfo.getColName())) {
			colPosMap_.put(colInfo.getColName(), colInfoList_.size());
		}
	}
	
	public void addValue(Object val) {
		resultDataList_.add(val);
		rowCnt_ = resultDataList_.size() / colInfoList_.size();
	}
	
	@Override
	public <T> T unwrap(Class<T> iface) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public boolean isWrapperFor(Class<?> iface) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public boolean next() throws SQLException {
		lastGetIsNull_ = false;
		curRowIdx_++;
		return curRowIdx_ >= 1 && curRowIdx_ <= rowCnt_;
	}
	
	@Override
	public void close() throws SQLException {
		lastGetIsNull_ = false;
		curRowIdx_ = 0;
		colInfoList_.clear();
		colPosMap_.clear();
		resultDataList_.clear();
		rowCnt_ = 0;
	}
	@Override
	public boolean wasNull() throws SQLException {
		return lastGetIsNull_;
	}
	
	private Object getValue(int rowIdx, int colIdx) throws SQLException {
		if (rowIdx < 1 || rowIdx > rowCnt_) {
			throw new SQLException("无效的行索引", "58005", PDBErrCode.PdbE_INVALID_PARAM);
		}
		
		if (colIdx < 1 || colIdx > colInfoList_.size()) {
			throw new SQLException("无效的列索引", "58005", PDBErrCode.PdbE_INVALID_PARAM);
		}
		
		int objIdx = (rowIdx - 1) * colInfoList_.size() + colIdx - 1;
		Object valObj = resultDataList_.get(objIdx);
		lastGetIsNull_ = (valObj == null);
		return valObj;
	}
	
	@Override
	public String getString(int columnIndex) throws SQLException {
		Object objVal = getValue(curRowIdx_, columnIndex);
		lastGetIsNull_ = (objVal == null);
		if (lastGetIsNull_) {
			return null;
		} else if (objVal instanceof String) {
			return String.valueOf(objVal);
		}
		
		throw new SQLException("类型不匹配", "58005", PDBErrCode.PdbE_VALUE_MISMATCH);
	}
	
	@Override
	public boolean getBoolean(int columnIndex) throws SQLException {
		Object objVal = getValue(curRowIdx_, columnIndex);
		if (lastGetIsNull_) {
			return false;
		} else if (objVal instanceof Boolean) {
			return (boolean)objVal;
		}

		throw new SQLException("类型不匹配", "58005", PDBErrCode.PdbE_VALUE_MISMATCH);
	}
	
	@Override
	public byte getByte(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public short getShort(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public int getInt(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public long getLong(int columnIndex) throws SQLException {
		Object objVal = getValue(curRowIdx_, columnIndex);
		if (lastGetIsNull_) {
			return 0;
		} else if (objVal instanceof Long) {
			return (long)objVal;
		}

		throw new SQLException("类型不匹配", "58005", PDBErrCode.PdbE_VALUE_MISMATCH);
	}
	
	@Override
	public float getFloat(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法");
	}
	
	@Override
	public double getDouble(int columnIndex) throws SQLException {
		Object objVal = getValue(curRowIdx_, columnIndex);
		lastGetIsNull_ = (objVal == null);
		if (lastGetIsNull_) {
			return 0;
		} else if (objVal instanceof Double) {
			return (double)objVal;
		}

		throw new SQLException("类型不匹配", "58005", PDBErrCode.PdbE_VALUE_MISMATCH);
	}
	@Override
	public BigDecimal getBigDecimal(int columnIndex, int scale) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public byte[] getBytes(int columnIndex) throws SQLException {
		Object objVal = getValue(curRowIdx_, columnIndex);
		lastGetIsNull_ = (objVal == null);
		if (lastGetIsNull_) {
			return null;
		} else if (objVal instanceof byte[]) {
			return (byte[])objVal;
		}

		throw new SQLException("类型不匹配", "58005", PDBErrCode.PdbE_VALUE_MISMATCH);
	}
	@Override
	public Date getDate(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Time getTime(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Timestamp getTimestamp(int columnIndex) throws SQLException {
		Object objVal = getValue(curRowIdx_, columnIndex);
		lastGetIsNull_ = (objVal == null);
		if (lastGetIsNull_) {
			return null;
		} else if (objVal instanceof Timestamp) {
			return (Timestamp)objVal;
		}

		throw new SQLException("类型不匹配", "58005", PDBErrCode.PdbE_VALUE_MISMATCH);
	}
	@Override
	public InputStream getAsciiStream(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public InputStream getUnicodeStream(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public InputStream getBinaryStream(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public String getString(String columnLabel) throws SQLException {
		if(!colPosMap_.containsKey(columnLabel)) {
			throw new SQLException("列名不存在", "58005", PDBErrCode.PdbE_INVALID_PARAM);
		}
		
		return getString(colPosMap_.get(columnLabel));
	}
	@Override
	public boolean getBoolean(String columnLabel) throws SQLException {
		if (!colPosMap_.containsKey(columnLabel)) {
			throw new SQLException("列名不存在", "58005", PDBErrCode.PdbE_INVALID_PARAM);
		}
		
		return getBoolean(colPosMap_.get(columnLabel));
	}
	@Override
	public byte getByte(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public short getShort(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public int getInt(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public long getLong(String columnLabel) throws SQLException {
		if(!colPosMap_.containsKey(columnLabel)) {
			throw new SQLException("列名不存在", "58005", PDBErrCode.PdbE_INVALID_PARAM);
		}
		
		return getLong(colPosMap_.get(columnLabel));
	}
	@Override
	public float getFloat(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法");
	}
	@Override
	public double getDouble(String columnLabel) throws SQLException {
		if(!colPosMap_.containsKey(columnLabel)) {
			throw new SQLException("列名不存在", "58005", PDBErrCode.PdbE_INVALID_PARAM);
		}
		
		return getDouble(colPosMap_.get(columnLabel));
	}
	@Override
	public BigDecimal getBigDecimal(String columnLabel, int scale) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public byte[] getBytes(String columnLabel) throws SQLException {
		if(!colPosMap_.containsKey(columnLabel)) {
			throw new SQLException("列名不存在", "58005", PDBErrCode.PdbE_INVALID_PARAM);
		}
		
		return getBytes(colPosMap_.get(columnLabel));
	}
	@Override
	public Date getDate(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Time getTime(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Timestamp getTimestamp(String columnLabel) throws SQLException {
		if(!colPosMap_.containsKey(columnLabel)) {
			throw new SQLException("列名不存在", "58005", PDBErrCode.PdbE_INVALID_PARAM);
		}
		
		return getTimestamp(colPosMap_.get(columnLabel));
	}
	@Override
	public InputStream getAsciiStream(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public InputStream getUnicodeStream(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public InputStream getBinaryStream(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public SQLWarning getWarnings() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void clearWarnings() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public String getCursorName() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public ResultSetMetaData getMetaData() throws SQLException {
		return new PDBResultSetMetaData(colInfoList_);
	}
	@Override
	public Object getObject(int columnIndex) throws SQLException {
		return getValue(curRowIdx_, columnIndex);
	}
	@Override
	public Object getObject(String columnLabel) throws SQLException {
		if(!colPosMap_.containsKey(columnLabel)) {
			throw new SQLException("列名不存在", "58005", PDBErrCode.PdbE_INVALID_PARAM);
		}
		
		return getObject(colPosMap_.get(columnLabel));
	}
	@Override
	public int findColumn(String columnLabel) throws SQLException {
		if(!colPosMap_.containsKey(columnLabel)) {
			throw new SQLException("列名不存在", "58005", PDBErrCode.PdbE_INVALID_PARAM);
		}
		
		return colPosMap_.get(columnLabel);
	}
	@Override
	public Reader getCharacterStream(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Reader getCharacterStream(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public BigDecimal getBigDecimal(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public BigDecimal getBigDecimal(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public boolean isBeforeFirst() throws SQLException {
		return this.curRowIdx_ == 0;
	}
	@Override
	public boolean isAfterLast() throws SQLException {
		return this.curRowIdx_ > rowCnt_;
	}
	@Override
	public boolean isFirst() throws SQLException {
		return this.curRowIdx_ == 1;
	}
	@Override
	public boolean isLast() throws SQLException {
		return this.curRowIdx_ == rowCnt_;
	}
	@Override
	public void beforeFirst() throws SQLException {
		this.curRowIdx_ = 0;
	}
	@Override
	public void afterLast() throws SQLException {
		this.curRowIdx_ = rowCnt_ + 1;
	}
	@Override
	public boolean first() throws SQLException {
		if (rowCnt_ == 0)
			return false;
		
		this.curRowIdx_ = 0;
		return true;
	}
	@Override
	public boolean last() throws SQLException {
		if (rowCnt_ == 0)
			return false;
		
		this.curRowIdx_ = rowCnt_;
		return true;
	}
	@Override
	public int getRow() throws SQLException {
		if (curRowIdx_ <= 0 || curRowIdx_ > rowCnt_)
			return 0;
		
		return curRowIdx_;
	}
	@Override
	public boolean absolute(int row) throws SQLException {
		if (row >= 0) {
			curRowIdx_ = row;
		}
		else if (row < 0){
			curRowIdx_ = rowCnt_ + 1 + row;
		}
		
		if (curRowIdx_ < 0) {
			curRowIdx_ = 0;
		} else if (curRowIdx_ > rowCnt_) {
			curRowIdx_ = rowCnt_ + 1;
		}
		
		return curRowIdx_ >= 1 && curRowIdx_ <= rowCnt_;
	}
	@Override
	public boolean relative(int rows) throws SQLException {
		curRowIdx_ += rows;
		
		if (curRowIdx_ < 0) {
			curRowIdx_ = 0;
		} else if (curRowIdx_ > rowCnt_) {
			curRowIdx_ = rowCnt_ + 1;
		}
		
		return curRowIdx_ >= 1 && curRowIdx_ <= rowCnt_;
	}
	@Override
	public boolean previous() throws SQLException {
		curRowIdx_--;
		if (curRowIdx_ < 0) {
			curRowIdx_ = 0;
		} else if (curRowIdx_ > rowCnt_) {
			curRowIdx_ = rowCnt_ + 1;
		}
		
		return curRowIdx_ >= 1 && curRowIdx_ <= rowCnt_;
	}
	@Override
	public void setFetchDirection(int direction) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public int getFetchDirection() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void setFetchSize(int rows) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public int getFetchSize() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public int getType() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public int getConcurrency() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public boolean rowUpdated() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public boolean rowInserted() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public boolean rowDeleted() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateNull(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBoolean(int columnIndex, boolean x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateByte(int columnIndex, byte x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateShort(int columnIndex, short x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateInt(int columnIndex, int x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateLong(int columnIndex, long x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateFloat(int columnIndex, float x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateDouble(int columnIndex, double x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBigDecimal(int columnIndex, BigDecimal x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateString(int columnIndex, String x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBytes(int columnIndex, byte[] x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateDate(int columnIndex, Date x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateTime(int columnIndex, Time x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateTimestamp(int columnIndex, Timestamp x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateAsciiStream(int columnIndex, InputStream x, int length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBinaryStream(int columnIndex, InputStream x, int length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateCharacterStream(int columnIndex, Reader x, int length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateObject(int columnIndex, Object x, int scaleOrLength) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateObject(int columnIndex, Object x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateNull(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBoolean(String columnLabel, boolean x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateByte(String columnLabel, byte x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateShort(String columnLabel, short x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateInt(String columnLabel, int x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateLong(String columnLabel, long x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateFloat(String columnLabel, float x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateDouble(String columnLabel, double x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBigDecimal(String columnLabel, BigDecimal x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateString(String columnLabel, String x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBytes(String columnLabel, byte[] x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateDate(String columnLabel, Date x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateTime(String columnLabel, Time x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateTimestamp(String columnLabel, Timestamp x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateAsciiStream(String columnLabel, InputStream x, int length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBinaryStream(String columnLabel, InputStream x, int length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateCharacterStream(String columnLabel, Reader reader, int length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateObject(String columnLabel, Object x, int scaleOrLength) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateObject(String columnLabel, Object x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void insertRow() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateRow() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void deleteRow() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void refreshRow() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void cancelRowUpdates() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 

	}
	@Override
	public void moveToInsertRow() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void moveToCurrentRow() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Statement getStatement() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Object getObject(int columnIndex, Map<String, Class<?>> map) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Ref getRef(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Blob getBlob(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Clob getClob(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Array getArray(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Object getObject(String columnLabel, Map<String, Class<?>> map) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Ref getRef(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Blob getBlob(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Clob getClob(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Array getArray(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Date getDate(int columnIndex, Calendar cal) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Date getDate(String columnLabel, Calendar cal) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Time getTime(int columnIndex, Calendar cal) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Time getTime(String columnLabel, Calendar cal) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Timestamp getTimestamp(int columnIndex, Calendar cal) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Timestamp getTimestamp(String columnLabel, Calendar cal) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public URL getURL(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public URL getURL(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateRef(int columnIndex, Ref x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateRef(String columnLabel, Ref x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBlob(int columnIndex, Blob x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBlob(String columnLabel, Blob x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateClob(int columnIndex, Clob x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateClob(String columnLabel, Clob x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateArray(int columnIndex, Array x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateArray(String columnLabel, Array x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public RowId getRowId(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public RowId getRowId(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateRowId(int columnIndex, RowId x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateRowId(String columnLabel, RowId x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public int getHoldability() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public boolean isClosed() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateNString(int columnIndex, String nString) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateNString(String columnLabel, String nString) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateNClob(int columnIndex, NClob nClob) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateNClob(String columnLabel, NClob nClob) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public NClob getNClob(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public NClob getNClob(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public SQLXML getSQLXML(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public SQLXML getSQLXML(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateSQLXML(int columnIndex, SQLXML xmlObject) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateSQLXML(String columnLabel, SQLXML xmlObject) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public String getNString(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public String getNString(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Reader getNCharacterStream(int columnIndex) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public Reader getNCharacterStream(String columnLabel) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateNCharacterStream(int columnIndex, Reader x, long length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateNCharacterStream(String columnLabel, Reader reader, long length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateAsciiStream(int columnIndex, InputStream x, long length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBinaryStream(int columnIndex, InputStream x, long length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateCharacterStream(int columnIndex, Reader x, long length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateAsciiStream(String columnLabel, InputStream x, long length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBinaryStream(String columnLabel, InputStream x, long length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateCharacterStream(String columnLabel, Reader reader, long length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBlob(int columnIndex, InputStream inputStream, long length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBlob(String columnLabel, InputStream inputStream, long length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateClob(int columnIndex, Reader reader, long length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateClob(String columnLabel, Reader reader, long length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateNClob(int columnIndex, Reader reader, long length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateNClob(String columnLabel, Reader reader, long length) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateNCharacterStream(int columnIndex, Reader x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateNCharacterStream(String columnLabel, Reader reader) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateAsciiStream(int columnIndex, InputStream x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBinaryStream(int columnIndex, InputStream x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateCharacterStream(int columnIndex, Reader x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateAsciiStream(String columnLabel, InputStream x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBinaryStream(String columnLabel, InputStream x) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateCharacterStream(String columnLabel, Reader reader) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBlob(int columnIndex, InputStream inputStream) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateBlob(String columnLabel, InputStream inputStream) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateClob(int columnIndex, Reader reader) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateClob(String columnLabel, Reader reader) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateNClob(int columnIndex, Reader reader) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public void updateNClob(String columnLabel, Reader reader) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public <T> T getObject(int columnIndex, Class<T> type) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	@Override
	public <T> T getObject(String columnLabel, Class<T> type) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
	
}
