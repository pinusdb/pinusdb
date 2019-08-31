package cn.pinusdb.jdbc;

public class ColumnInfo {
	private String name_;
	private PDBDataType type_;
	
	public ColumnInfo(String name, PDBDataType type) {
		this.name_ = name;
		this.type_ = type;
	}
	
	public String getColName() {
		return name_;
	}
	
	public PDBDataType getColType() {
		return type_;
	}
}
