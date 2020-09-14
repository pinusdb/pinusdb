%include {
#include "pdb.h"
#include "expr/pdb_db_int.h"
#include "expr/parse.h"
#include "expr/expr_value.h"
#include "expr/target_list.h"
#include "expr/record_list.h"
#include "expr/sql_parser.h"
#include "expr/column_item.h"
#include "expr/group_opt.h"
#include "expr/orderby_opt.h"
#include "expr/limit_opt.h"

void pdbSetError(SQLParser* pParse, const char* pErrMsg);
void pdbSelect(SQLParser* pParse, TargetList* pTagList, Token* pSrcTab, 
  ExprValue* pWhere, GroupOpt* pGroup, OrderByOpt* pOrderBy, LimitOpt* pLimit);
//void pdbSelect2(SQLParser* pParse, TargetList* pTagList1, Token* pSrcTab, ExprValue* pWhere1, 
//  GroupOpt* pGroup, OrderByOpt* pOrderBy, LimitOpt* pLimit, TargetList* pTagList2, ExprValue* pWhere2);
void pdbCreateTable(SQLParser* pParse, Token* pTabName, ColumnList* pColList);
void pdbAlterTable(SQLParser* pParse, Token* pTabName, ColumnList* pColList);

void pdbDelete(SQLParser* pParse, Token* pTabName, ExprValue* pWhere);
void pdbAttachTable(SQLParser* pParse, Token* pTabName);
void pdbDetachTable(SQLParser* pParse, Token* pTabName);
void pdbDropTable(SQLParser* pParse, Token* pTabToken);

void pdbAttachFile(SQLParser* pParse, Token* pTabName, Token* pDate, Token* pType);
void pdbDetachFile(SQLParser* pParse, Token* pTabName, Token* pDate);
void pdbDropFile(SQLParser* pParse, Token* pTabName, Token* pDate);

void pdbAddUser(SQLParser* pParse, Token* pNameToken, Token* pPwdToken);
void pdbChangePwd(SQLParser* pParse, Token* pNameToken, Token* pPwdToken);
void pdbChangeRole(SQLParser* pParse, Token* pNameToken, Token* pRoleToken);
void pdbDropUser(SQLParser* pParse, Token* pNameToken);

void pdbInsert(SQLParser* pParse, Token* pTabName, TargetList* pColList, RecordList* pRecList);
}

%token_prefix  TK_
%token_type    {Token}
%default_type  {Token}
%extra_argument {SQLParser *pParse}
%syntax_error {
  const char* errmsg = TOKEN.str_;
  pdbSetError(pParse, errmsg);
}
%name pdbParse

%nonassoc  ILLEGAL SPACE COMMENT FUNCTION INSERT INTO VALUES DELETE TOP NOTIN TINYINT SMALLINT INT FLOAT ISNULL ISNOTNULL TIMEVAL.

//////////////////// The Add User ///////////////////////////////////

cmd ::= ADD USER username(N) IDENTIFIED BY STRING(P) SEMI. {
  pdbAddUser(pParse, &N, &P);
}

cmd ::= DROP USER username(U) SEMI. {
  pdbDropUser(pParse, &U);
}

//set password for sa=password('test')
cmd ::= SET PASSWORD FOR username(U) EQ PASSWORD LP STRING(X) RP SEMI. {
  pdbChangePwd(pParse, &U, &X); 
}

cmd ::= SET ROLE FOR username(U) EQ ID(R) SEMI. {
  pdbChangeRole(pParse, &U, &R);
}

//%type username { Token }
username(U) ::= STRING(N).            { U = N; }
username(U) ::= ID(N).                { U = N; }

//////////////////// The DROP TABLE /////////////////////////////////
cmd ::= DROP TABLE ID(X) SEMI. {
  pdbDropTable(pParse, &X);
}

cmd ::= ATTACH TABLE ID(X) SEMI. {
  pdbAttachTable(pParse, &X);
}

cmd ::= DETACH TABLE ID(X) SEMI. {
  pdbDetachTable(pParse, &X);
}

/////////////////// DataPart ////////////////////////////////////////

cmd ::= ATTACH DATAFILE STRING(D) COMMA STRING(T) FROM ID(X) SEMI. {
  pdbAttachFile(pParse, &X, &D, &T);
} 

cmd ::= DETACH DATAFILE STRING(D) FROM ID(T) SEMI. {
  pdbDetachFile(pParse, &T, &D);
}

cmd ::= DROP DATAFILE STRING(D) FROM ID(T) SEMI. {
  pdbDropFile(pParse, &T, &D);
}


/////////////////// The CREATE Command //////////////////////////////

cmd ::= CREATE TABLE ID(X) LP cre_columnlist(Y) RP SEMI. {
  pdbCreateTable(pParse, &X, Y);
}

cmd ::= ALTER TABLE ID(X) LP cre_columnlist(Y) RP SEMI. {
  pdbAlterTable(pParse, &X, Y);
}

%type cre_columnlist             { ColumnList* }
%destructor cre_columnlist       { ColumnList::FreeColumnList($$); }
%type cre_column                 { ColumnItem* }
%destructor cre_column           { ColumnItem::FreeColumnItem($$); }

cre_columnlist(A) ::= cre_columnlist(X) COMMA cre_column(Y).  {
  A = ColumnList::AppendColumnItem(X, Y);
}
cre_columnlist(A) ::= cre_column(X). {
  A = ColumnList::AppendColumnItem(nullptr, X);
}

cre_column(A) ::= ID(X) BOOL_TYPE.      { A = ColumnItem::MakeColumnItem(&X, PDB_FIELD_TYPE::TYPE_BOOL); }
cre_column(A) ::= ID(X) TINYINT_TYPE.   { A = ColumnItem::MakeColumnItem(&X, PDB_FIELD_TYPE::TYPE_INT8); }
cre_column(A) ::= ID(X) SMALLINT_TYPE.  { A = ColumnItem::MakeColumnItem(&X, PDB_FIELD_TYPE::TYPE_INT16); }
cre_column(A) ::= ID(X) INT_TYPE.       { A = ColumnItem::MakeColumnItem(&X, PDB_FIELD_TYPE::TYPE_INT32); }
cre_column(A) ::= ID(X) BIGINT_TYPE.    { A = ColumnItem::MakeColumnItem(&X, PDB_FIELD_TYPE::TYPE_INT64); }
cre_column(A) ::= ID(X) FLOAT_TYPE.     { A = ColumnItem::MakeColumnItem(&X, PDB_FIELD_TYPE::TYPE_FLOAT); }
cre_column(A) ::= ID(X) DOUBLE_TYPE.    { A = ColumnItem::MakeColumnItem(&X, PDB_FIELD_TYPE::TYPE_DOUBLE); }
cre_column(A) ::= ID(X) STRING_TYPE.    { A = ColumnItem::MakeColumnItem(&X, PDB_FIELD_TYPE::TYPE_STRING); }
cre_column(A) ::= ID(X) BLOB_TYPE.      { A = ColumnItem::MakeColumnItem(&X, PDB_FIELD_TYPE::TYPE_BLOB); }
cre_column(A) ::= ID(X) DATETIME_TYPE.  { A = ColumnItem::MakeColumnItem(&X, PDB_FIELD_TYPE::TYPE_DATETIME); }
cre_column(A) ::= ID(X) REAL2_TYPE.     { A = ColumnItem::MakeColumnItem(&X, PDB_FIELD_TYPE::TYPE_REAL2); }
cre_column(A) ::= ID(X) REAL3_TYPE.     { A = ColumnItem::MakeColumnItem(&X, PDB_FIELD_TYPE::TYPE_REAL3); }
cre_column(A) ::= ID(X) REAL4_TYPE.     { A = ColumnItem::MakeColumnItem(&X, PDB_FIELD_TYPE::TYPE_REAL4); }
cre_column(A) ::= ID(X) REAL6_TYPE.     { A = ColumnItem::MakeColumnItem(&X, PDB_FIELD_TYPE::TYPE_REAL6); }


/////////////////// Delete //////////////////////////////////////////

cmd ::= DELETE FROM ID(X) where_opt(Y) SEMI.
{
  pdbDelete(pParse, &X, Y);
}

/////////////////// Insert //////////////////////////////////////////
cmd ::= INSERT INTO ID(X) LP target_list(T) RP VALUES record_list(L) SEMI.
{
  pdbInsert(pParse, &X, T, L);
}

/////////////////// The SELECT //////////////////////////////////////
cmd ::= SELECT target_list(T) FROM ID(X) where_opt(W) groupby_opt(G) orderby_opt(O) limit_opt(L) SEMI.
{
  pdbSelect(pParse, T, &X, W, G, O, L);
}

//cmd ::= SELECT target_list(A) FROM LP SELECT target_list(T) FROM ID(X) where_opt(W) groupby_opt(G) orderby_opt(O) limit_opt(L) RP where_opt(B) SEMI.
//{
//  pdbSelect2(pParse, T, &X, W, G, O, L, A, B);
//}

cmd ::= SELECT target_list(T) SEMI.
{
  pdbSelect(pParse, T, nullptr, nullptr, nullptr, nullptr, nullptr);
}

%type where_opt       { ExprValue* }
%destructor where_opt { ExprValue::FreeExprValue($$); }

where_opt(A) ::= .                  { A = nullptr; }
where_opt(A) ::= WHERE expr_val(X). { A = X; }

%type groupby_opt                 { GroupOpt* }
%destructor groupby_opt           { delete ($$); }

groupby_opt(A) ::= .                           { A = nullptr; }
groupby_opt(A) ::= GROUP BY ID(X).             { A = new GroupOpt(&X); }
groupby_opt(A) ::= GROUP BY ID(X) expr_val(T). { A = new GroupOpt(&X, T); }

%type orderby_opt                 { OrderByOpt* }
%destructor orderby_opt           { delete ($$); }

orderby_opt(A) ::= . { A = nullptr; }
orderby_opt(A) ::= ORDER BY ID(X).      { A = new OrderByOpt(&X, true); }
orderby_opt(A) ::= ORDER BY ID(X) ASC.  { A = new OrderByOpt(&X, true); }
orderby_opt(A) ::= ORDER BY ID(X) DESC. { A = new OrderByOpt(&X, false); }

%type limit_opt                   { LimitOpt* }
%destructor limit_opt             { delete ($$); }

limit_opt(A) ::= . { A = nullptr; }
limit_opt(A) ::= LIMIT INTEGER(Y).                   { A = new LimitOpt(&Y); }
limit_opt(A) ::= LIMIT INTEGER(X) COMMA INTEGER(Y).  { A = new LimitOpt(&X, &Y); }

%left AND.
%left EQ NE.
%left GT GE LT LE.

%type record_list        { RecordList* }
%destructor record_list  { RecordList::FreeRecordList($$); }

record_list(A) ::= LP expr_val_list(R) RP. 
{ A = RecordList::AppendRecordList(nullptr, R); }
record_list(A) ::= record_list(L) COMMA LP expr_val_list(R) RP. 
{ A = RecordList::AppendRecordList(L, R); }

%type expr_val  { ExprValue* }
%destructor expr_val { ExprValue::FreeExprValue($$); }

expr_val(A) ::= LP expr_val(X) RP.             { A = X; }
expr_val(A) ::= STAR.                          { A = ExprValue::MakeStarValue(); }
expr_val(A) ::= ID(X).                         { A = ExprValue::MakeID(&X); }
expr_val(A) ::= TRUE.                          { A = ExprValue::MakeBoolValue(true); }
expr_val(A) ::= FALSE.                         { A = ExprValue::MakeBoolValue(false); }
expr_val(A) ::= INTEGER(X).                    { A = ExprValue::MakeIntValue(false, &X); }
expr_val(A) ::= PLUS INTEGER(X).               { A = ExprValue::MakeIntValue(false, &X); }
expr_val(A) ::= MINUS INTEGER(X).              { A = ExprValue::MakeIntValue(true, &X); }
expr_val(A) ::= DOUBLE(X).                     { A = ExprValue::MakeDoubleValue(false, &X); }
expr_val(A) ::= PLUS DOUBLE(X).                { A = ExprValue::MakeDoubleValue(false, &X); }
expr_val(A) ::= MINUS DOUBLE(X).               { A = ExprValue::MakeDoubleValue(true, &X); }
expr_val(A) ::= STRING(X).                     { A = ExprValue::MakeStringValue(&X); }
expr_val(A) ::= BLOB(X).                       { A = ExprValue::MakeBlobValue(&X); }
expr_val(A) ::= INTEGER(X) ID(U).              { A = ExprValue::MakeTimeValue(false, &X, &U); }
expr_val(A) ::= PLUS INTEGER(X) ID(U).         { A = ExprValue::MakeTimeValue(false, &X, &U); }
expr_val(A) ::= MINUS INTEGER(X) ID(U).        { A = ExprValue::MakeTimeValue(true, &X, &U); }

expr_val(A) ::= ID(N) LP expr_val_list(L) RP.  { A = ExprValue::MakeFunction(&N, L); }
expr_val(A) ::= ADD(N) LP expr_val_list(L) RP. { A = ExprValue::MakeFunction(&N, L); }

expr_val(A) ::= expr_val(X) LT expr_val(Y).           { A = ExprValue::MakeCompare(TK_LT, X, Y); }
expr_val(A) ::= expr_val(X) LE expr_val(Y).           { A = ExprValue::MakeCompare(TK_LE, X, Y); }
expr_val(A) ::= expr_val(X) GT expr_val(Y).           { A = ExprValue::MakeCompare(TK_GT, X, Y); }
expr_val(A) ::= expr_val(X) GE expr_val(Y).           { A = ExprValue::MakeCompare(TK_GE, X, Y); }
expr_val(A) ::= expr_val(X) EQ expr_val(Y).           { A = ExprValue::MakeCompare(TK_EQ, X, Y); }
expr_val(A) ::= expr_val(X) NE expr_val(Y).           { A = ExprValue::MakeCompare(TK_NE, X, Y); }
expr_val(A) ::= expr_val(X) AND expr_val(Y).          { A = ExprValue::MakeCompare(TK_AND, X, Y); }
expr_val(A) ::= ID(X) LIKE STRING(Y).                 { A = ExprValue::MakeLike(&X, &Y); }
expr_val(A) ::= ID(X) IS NOT NULL.                    { A = ExprValue::MakeIsNotNull(&X); }
expr_val(A) ::= ID(X) IS NULL.                        { A = ExprValue::MakeIsNull(&X); }
expr_val(A) ::= STRING(X) IS NOT NULL.                { A = ExprValue::MakeIsNotNull(&X); }
expr_val(A) ::= STRING(X) IS NULL.                    { A = ExprValue::MakeIsNull(&X); }
expr_val(A) ::= ID(X) IN LP expr_val_list(L) RP.      { A = ExprValue::MakeIn(&X, L); }
expr_val(A) ::= ID(X) NOT IN LP expr_val_list(L) RP.  { A = ExprValue::MakeNotIn(&X, L); }

%type expr_val_list        { ExprValueList* }
%destructor expr_val_list  { ExprValueList::FreeExprValueList($$); }

expr_val_list(A) ::= .                                    { A = nullptr; }
expr_val_list(A) ::= expr_val(V).                         { A = ExprValueList::AppendExprValue(nullptr, V); }
expr_val_list(A) ::= expr_val_list(L) COMMA expr_val(V).  { A = ExprValueList::AppendExprValue(L, V); }

%type target_list                { TargetList* }
%destructor target_list          { TargetList::FreeTargetList($$); }

target_list(A) ::= expr_val(V).                                { A = TargetList::AppendExprValue(nullptr, V, nullptr); }
target_list(A) ::= expr_val(V) AS ID(X).                       { A = TargetList::AppendExprValue(nullptr, V, &X); }
target_list(A) ::= target_list(L) COMMA expr_val(V).           { A = TargetList::AppendExprValue(L, V, nullptr); }
target_list(A) ::= target_list(L) COMMA expr_val(V) AS ID(X).  { A = TargetList::AppendExprValue(L, V, &X); }