package cn.pinusdb.jdbc;

class SqlPart {
	private SqlPartType partType_;
	private String partStr_;
	
	SqlPart(String partStr, SqlPartType partType) {
		partStr_ = partStr;
		partType_ = partType;
	}
	
	public SqlPartType getPartType() {
		return partType_;
	}
	
	public String getPartStr() {
		return partStr_;
	}
}
