package cn.pinusdb.jdbc;

enum SqlPartType {
	TK_INSERT(1),
	TK_INTO(2),
	TK_ID(3),
	TK_LP(4),
	TK_RP(5),
	TK_COMMA(6),
	TK_VALUES(7),
	TK_PARAM(8);
	
	private int tkType_;
	private SqlPartType(int tkType) {
		this.tkType_ = tkType;
	}
	
	public int getTypeVal() {
		return tkType_;
	}
}
