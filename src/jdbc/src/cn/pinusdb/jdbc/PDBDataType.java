package cn.pinusdb.jdbc;

import java.sql.SQLException;

enum PDBDataType {

	PDB_Null(0),
	PDB_Bool(1),
	PDB_BigInt(2),
	PDB_DateTime(3),
	PDB_Double(4),
	PDB_String(5),
	PDB_Blob(6);
	
	private int typeVal_;
	
	public static PDBDataType valueOf(int typeVal) throws SQLException{
		switch(typeVal) {
		case 0: return PDB_Null;
		case 1: return PDB_Bool;
		case 2: return PDB_BigInt;
		case 3: return PDB_DateTime;
		case 4: return PDB_Double;
		case 5: return PDB_String;
		case 6: return PDB_Blob;
		}
		
		throw new SQLException("错误的数据类型");
	}
	
	private PDBDataType(int typeVal) {
		typeVal_ = typeVal;
	}
	
	public int getTypeVal() {
		return typeVal_;
	}
	
}
