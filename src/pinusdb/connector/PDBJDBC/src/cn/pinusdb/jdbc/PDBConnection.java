package cn.pinusdb.jdbc;

import java.sql.Array;
import java.sql.Blob;
import java.sql.CallableStatement;
import java.sql.Clob;
import java.sql.DatabaseMetaData;
import java.sql.NClob;
import java.sql.PreparedStatement;
import java.sql.SQLClientInfoException;
import java.sql.SQLException;
import java.sql.SQLWarning;
import java.sql.SQLXML;
import java.sql.Savepoint;
import java.sql.Statement;
import java.sql.Struct;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.Executor;
import java.io.IOException;

import java.net.InetSocketAddress;
import java.net.Socket;

public class PDBConnection implements java.sql.Connection {

	private int serPort_;
	private String serHost_;
	private String userName_;
	private String userPwd_;
	private Socket socket_ = null;
	
	private final int kUserNameLen = 48;
	private final int kPwdLen = 4;
	private final int kDefaultTimeOut = 65000;
	
	PDBConnection(String server, int port, String user, String pwd) 
		throws SQLException
	{
		this.serHost_ = server;
		this.serPort_ = port;
		this.userName_ = user;
		this.userPwd_ = pwd;
		if (serHost_.isEmpty() || userName_.isEmpty() || userPwd_.isEmpty() 
			|| serPort_ <= 0 || serPort_ >= 65536)
		{
			throw new IllegalArgumentException();
		}
		
		open();
	}
	
	public void open() throws SQLException
	{
		socket_ = new Socket();
		try {
			socket_.setReuseAddress(true);
			socket_.setKeepAlive(true);
			socket_.setTcpNoDelay(true);
			socket_.setSoLinger(true, 0);
			socket_.setSoTimeout(kDefaultTimeOut);
			socket_.connect(new InetSocketAddress(serHost_, serPort_));
		} catch (IOException e) {
			throw new SQLException("网络错误", "58005", PDBErrCode.PdbE_NET_ERROR);
		}
		login();
	}
	
    void request(byte[] buf) throws SQLException {
		if (socket_ == null)
			throw new SQLException("网络错误：未连接", "58005", PDBErrCode.PdbE_NET_ERROR);
		
		try {
			socket_.getOutputStream().write(buf, 0, buf.length);
		} catch (IOException e) {
			throw new SQLException(e.getMessage(), "58005", PDBErrCode.PdbE_NET_ERROR);
		}
	}
	
    byte[] recv() throws SQLException {
		if (socket_ == null)
			throw new SQLException("网络错误：未连接", "58005", PDBErrCode.PdbE_NET_ERROR);
		
		int totalLen = 0;
		int headLen = ProtoHeader.getHeadLen();
		byte[] headBuf = new byte[headLen];
		while(totalLen < headLen) {
			int tmpLen = headLen - totalLen;
			int recvLen = 0;
			try {
				recvLen = socket_.getInputStream().read(headBuf, totalLen, tmpLen);
			} catch (IOException e) {
				throw new SQLException(e.getMessage(), "58005", PDBErrCode.PdbE_NET_ERROR);
			}
			if (recvLen < 0) {
				throw new SQLException(PDBErrCode.errMsg(PDBErrCode.PdbE_NET_ERROR), 
					"58005", PDBErrCode.PdbE_NET_ERROR);
			}
			totalLen += recvLen;
		}
		
		ProtoHeader proHdr = new ProtoHeader(headBuf);
		int headCrc = proHdr.getHeadCrc();
		int tmpHeadCrc = CRC.crc32(headBuf, 0, 60);
		if (headCrc != tmpHeadCrc) {
			throw new SQLException("报文错误,回复报文中,报文头校验错误", "58005", PDBErrCode.PdbE_PACKET_ERROR);
		}
		
		int bodyLen = proHdr.getBodyLen();
		if (bodyLen == 0) {
			return headBuf;
		}
		
		byte[] packetBuf = new byte[ProtoHeader.getHeadLen() + bodyLen];
		System.arraycopy(headBuf, 0, packetBuf, 0, headBuf.length);
		totalLen = 0;
		while (totalLen < bodyLen) {
			int tmpLen = bodyLen - totalLen;
			int recvLen = 0;
			try {
				recvLen = socket_.getInputStream().read(packetBuf, 
					(ProtoHeader.getHeadLen() + totalLen), tmpLen);
			} catch (IOException e) {
				throw new SQLException("网络错误,读取报文失败", "58005", PDBErrCode.PdbE_NET_ERROR);
			}
			totalLen += recvLen;
		}
		
		int bodyCrc = proHdr.getBodyCrc();
		int tmpBodyCrc = CRC.crc32(packetBuf, ProtoHeader.getHeadLen(), bodyLen);
		if (tmpBodyCrc != bodyCrc) {
			throw new SQLException("报文错误,回复报文中,报文体校验失败", "58005", PDBErrCode.PdbE_PACKET_ERROR);
		}
		
		return packetBuf;
	}
	
	public void dispose() throws SQLException {
		this.close();
	}
	
	private void login() throws SQLException {
		byte[] loginPacket = new byte[ProtoHeader.getHeadLen() + kUserNameLen + kPwdLen];
		ProtoHeader proHdr = new ProtoHeader(loginPacket);
		
		for (int i = 0; i < loginPacket.length; i++) {
			loginPacket[i] = 0;
		}
		
		byte[] nameArr = this.userName_.getBytes();
		byte[] pwdArr = this.userPwd_.getBytes();
		if (nameArr.length >= kUserNameLen) {
			throw new SQLException("错误的参数: 用户名不合法", "58005", PDBErrCode.PdbE_INVALID_USER_NAME);
		}
		
		System.arraycopy(nameArr, 0, loginPacket, ProtoHeader.getHeadLen(), nameArr.length);
		int pwd32 = CRC.crc32(pwdArr, 0, pwdArr.length);
		IntTool.setInt32(loginPacket, (ProtoHeader.getHeadLen() + kUserNameLen), pwd32);
		int bodyCrc = CRC.crc32(loginPacket, ProtoHeader.getHeadLen(), (kUserNameLen + kPwdLen));
		
		proHdr.setVersion(ProtoHeader.getProtoVersion());
		proHdr.setMethod(ProtoHeader.getMethodCmdLoginReq());
		proHdr.setBodyLen((kUserNameLen + kPwdLen));
		proHdr.setBodyCrc(bodyCrc);
		proHdr.updateHeadCrc();
		
		request(loginPacket);
		
		byte[] recvBuf = recv();
		ProtoHeader recvHdr = new ProtoHeader(recvBuf);
		if (recvHdr.getMethod() != ProtoHeader.getMethodCmdLoginRep()) {
			throw new SQLException("报文错误,回复方法错误", "58005", PDBErrCode.PdbE_PACKET_ERROR);
		}
		
		int loginRet = recvHdr.getReturnVal();
		if (loginRet != PDBErrCode.PdbE_OK) {
			throw new SQLException(PDBErrCode.errMsg(loginRet), "58005", loginRet);
		}
	}
	
	public PDBCommand createCommand() {
		return new PDBCommand(this);
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
	public Statement createStatement() throws SQLException {
		return new PDBStatement(this);
	}

	@Override
	public PreparedStatement prepareStatement(String sql) throws SQLException {
		return new PDBPreStatement(this, sql);
	}

	@Override
	public CallableStatement prepareCall(String sql) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public String nativeSQL(String sql) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void setAutoCommit(boolean autoCommit) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public boolean getAutoCommit() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void commit() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void rollback() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void close() throws SQLException {
		try {
			socket_.close();
		} catch (IOException e) {
			throw new SQLException("网络错误", "58005", PDBErrCode.PdbE_NET_ERROR);
		}
	}

	@Override
	public boolean isClosed() throws SQLException {
		return socket_.isClosed();
	}

	@Override
	public DatabaseMetaData getMetaData() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void setReadOnly(boolean readOnly) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public boolean isReadOnly() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void setCatalog(String catalog) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public String getCatalog() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void setTransactionIsolation(int level) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public int getTransactionIsolation() throws SQLException {
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
	public Statement createStatement(int resultSetType, int resultSetConcurrency) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public PreparedStatement prepareStatement(String sql, int resultSetType, int resultSetConcurrency)
			throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public CallableStatement prepareCall(String sql, int resultSetType, int resultSetConcurrency) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public Map<String, Class<?>> getTypeMap() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void setTypeMap(Map<String, Class<?>> map) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void setHoldability(int holdability) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public int getHoldability() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public Savepoint setSavepoint() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public Savepoint setSavepoint(String name) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void rollback(Savepoint savepoint) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void releaseSavepoint(Savepoint savepoint) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public Statement createStatement(int resultSetType, int resultSetConcurrency, int resultSetHoldability)
			throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public PreparedStatement prepareStatement(String sql, int resultSetType, int resultSetConcurrency,
			int resultSetHoldability) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public CallableStatement prepareCall(String sql, int resultSetType, int resultSetConcurrency,
			int resultSetHoldability) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public PreparedStatement prepareStatement(String sql, int autoGeneratedKeys) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public PreparedStatement prepareStatement(String sql, int[] columnIndexes) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public PreparedStatement prepareStatement(String sql, String[] columnNames) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public Clob createClob() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public Blob createBlob() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public NClob createNClob() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public SQLXML createSQLXML() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public boolean isValid(int timeout) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void setClientInfo(String name, String value) throws SQLClientInfoException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void setClientInfo(Properties properties) throws SQLClientInfoException {
		throw new UnsupportedOperationException("不支持的方法"); 
		
	}

	@Override
	public String getClientInfo(String name) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public Properties getClientInfo() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public Array createArrayOf(String typeName, Object[] elements) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public Struct createStruct(String typeName, Object[] attributes) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void setSchema(String schema) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public String getSchema() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void abort(Executor executor) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public void setNetworkTimeout(Executor executor, int milliseconds) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}

	@Override
	public int getNetworkTimeout() throws SQLException {
		throw new UnsupportedOperationException("不支持的方法"); 
	}
}
