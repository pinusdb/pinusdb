package cn.pinusdb.jdbc;

import java.sql.Connection;
import java.sql.Driver;
import java.sql.DriverPropertyInfo;
import java.sql.SQLException;
import java.sql.SQLFeatureNotSupportedException;
import java.util.Properties;
import java.util.logging.Logger;

public class PDBDriver implements Driver {

	//Register ourselvers with the DriverManager
	static {
		try {
			java.sql.DriverManager.registerDriver(new PDBDriver());
		} catch (SQLException e) {
			throw new RuntimeException("Can't register PinusDB JDBC Driver!!!");
		}
	}
	
	private static final String urlPrefix = "jdbc:pinusdb://";
	private static final String userPrefix = "user=";
	private static final String pwdPrefix = "password=";
	
	
	@Override
	public boolean acceptsURL(String url) throws SQLException {
		if (url == null) {
			return false;
		}
		
		String trimUrl = url.trim();
		if (trimUrl.length() < urlPrefix.length()) {
			return false;
		}
		
		String urlType = trimUrl.substring(0, urlPrefix.length());
		return urlType.compareToIgnoreCase(urlPrefix) == 0;
	}

	@Override
	public Connection connect(String url, Properties info) throws SQLException {
		int tmpIdx = 0;
		int serPort = 0;
		String serHost = null;
		String username = null;
		String password = null;
		if (url == null) {
			return null;
		}
		
		url = url.trim();
		if (!url.startsWith(urlPrefix)) {
			return null;
		}
		
		url = url.substring(urlPrefix.length());
		tmpIdx = url.indexOf(":");
		if (tmpIdx <= 0) {
			return null;
		}
		
		serHost = url.substring(0, tmpIdx);
		url = url.substring(tmpIdx + 1);
		
		tmpIdx = url.indexOf("?");
		if (tmpIdx > 0) {
			serPort = Integer.parseInt(url.substring(0, tmpIdx));
			url = url.substring(tmpIdx + 1);
			
			while(true) {
				if (url.startsWith(userPrefix)) {
					url = url.substring(userPrefix.length());
					tmpIdx = url.indexOf("&");
					if (tmpIdx < 0) {
						tmpIdx = url.length();
					}
					username = url.substring(0, tmpIdx);
				} else if (url.startsWith(pwdPrefix)) {
					url = url.substring(pwdPrefix.length());
					tmpIdx = url.indexOf("&");
					if (tmpIdx < 0) {
						tmpIdx = url.length();
					}
					password = url.substring(0, tmpIdx);
				} else {
					return null;
				}
				
				if (tmpIdx == url.length())
					break;
				
				url = url.substring(tmpIdx + 1);
			}
			
		} else {
			serPort = Integer.parseInt(url);
			
			username = info.getProperty("user");
			password = info.getProperty("password");
		}
		
		if (serPort <= 0 || serPort >= 65536 || 
			serHost == null || username == null || password == null) {
			return null;
		}
		
		return new PDBConnection(serHost, serPort, username, password);
	}

	@Override
	public int getMajorVersion() {
		return 1;
	}

	@Override
	public int getMinorVersion() {
		return 2;
	}

	@Override
	public Logger getParentLogger() throws SQLFeatureNotSupportedException {
		return null;
	}

	@Override
	public DriverPropertyInfo[] getPropertyInfo(String url, Properties info) throws SQLException {
		throw new UnsupportedOperationException("不支持的方法");
	}

	@Override
	public boolean jdbcCompliant() {
		return false;
	}
	
}
