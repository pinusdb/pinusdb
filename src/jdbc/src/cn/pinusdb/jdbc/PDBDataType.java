package cn.pinusdb.jdbc;

import java.sql.SQLException;

enum PDBDataType {

	PDB_Null(0),
	PDB_Bool(1),
	PDB_TinyInt(2),
	PDB_SmallInt(3),
	PDB_Int(4),
	PDB_BigInt(5),
	PDB_DateTime(6),
	PDB_Float(7),
	PDB_Double(8),
	PDB_String(9),
	PDB_Blob(10);
	
	private int typeVal_;
	
	public static PDBDataType valueOf(int typeVal) throws SQLException{
		switch(typeVal) {
		case 0: return PDB_Null;
		case 1: return PDB_Bool;
		case 2: return PDB_TinyInt;
		case 3: return PDB_SmallInt;
		case 4: return PDB_Int;
		case 5: return PDB_BigInt;
		case 6: return PDB_DateTime;
		case 7: return PDB_Float;
		case 8: return PDB_Double;
		case 9: return PDB_String;
		case 10: return PDB_Blob;
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
