%include {
#include "pdb.h"
#include "expr/pdb_db_int.h"
#include "expr/parse.h"
#include "expr/expr_item.h"
#include "expr/record_list.h"
#include "expr/sql_parser.h"
#include "expr/column_item.h"
#include "expr/group_opt.h"
#include "expr/orderby_opt.h"
#include "expr/limit_opt.h"

void pdbSetError(SQLParser* pParse, const char* pErrMsg);
void pdbSelect(SQLParser* pParse, ExprList* pTagList, Token* pSrcTab, ExprItem* pWhere, GroupOpt* pGroup, OrderByOpt* pOrderBy, LimitOpt* pLimit);
void pdbCreateTable(SQLParser* pParse, Token* pTabName, ColumnList* pColList);
void pdbAlterTable(SQLParser* pParse, Token* pTabName, ColumnList* pColList);

void pdbDelete(SQLParser* pParse, Token* pTabName, ExprItem* pWhere);
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

void pdbInsert(SQLParser* pParse, Token* pTabName, ExprList* pColList, RecordList* pRecList);
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
cre_column(A) ::= ID(X) BIGINT_TYPE.    { A = ColumnItem::MakeColumnItem(&X, PDB_FIELD_TYPE::TYPE_INT64); }
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

cmd ::= SELECT target_list(T) SEMI.
{
  pdbSelect(pParse, T, nullptr, nullptr, nullptr, nullptr, nullptr);
}

%type target_list               { ExprList* }
%destructor target_list         { ExprList::FreeExprList($$); }
%type target_item               { ExprItem* }
%destructor target_item         { ExprItem::FreeExprItem($$); }

target_item(A) ::= STAR(S).         { A = ExprItem::MakeValue(TK_STAR, &S); }
target_item(A) ::= ID(N).           { A = ExprItem::MakeValue(TK_ID, &N); }
target_item(A) ::= ID(N) AS ID(X).  { A = ExprItem::MakeValue(TK_ID, &N, &X); }
target_item(A) ::= ID(N) LP arg_list(L) RP(R).       { A = ExprItem::MakeFunction(TK_FUNCTION, &N, L, &R); }
target_item(A) ::= ID(N) LP arg_list(L) RP AS ID(X). { A = ExprItem::MakeFunction(TK_FUNCTION, &N, L, &X); }
target_item(A) ::= ID(N) LP RP(R).       { A = ExprItem::MakeFunction(TK_FUNCTION, &N, nullptr, &R); }
target_item(A) ::= ID(N) LP RP AS ID(X). { A = ExprItem::MakeFunction(TK_FUNCTION, &N, nullptr, &X); }

target_list(A) ::= target_item(X). {
  A = ExprList::AppendExprItem(nullptr, X);
}
target_list(A) ::= target_list(P) COMMA target_item(X). {
  A = ExprList::AppendExprItem(P, X);
}

%type arg_item                  { ExprItem* }
%destructor arg_item            { ExprItem::FreeExprItem($$); }
%type arg_list                  { ExprList* }
%destructor arg_list            { ExprList::FreeExprList($$); }

arg_item(A) ::= ID(X).          { A = ExprItem::MakeValue(TK_ID, &X); }
arg_item(A) ::= STAR(X).        { A = ExprItem::MakeValue(TK_STAR, &X); }
arg_item(A) ::= timeval(T).     { A = T; }
arg_item(A) ::= STRING(X).      { A = ExprItem::MakeValue(TK_STRING, &X); }

arg_list(A) ::= arg_item(I).      
{ A = ExprList::AppendExprItem(nullptr, I); }
arg_list(A) ::= arg_list(P) COMMA arg_item(I).
{ A = ExprList::AppendExprItem(P, I); }

%type where_opt                  { ExprItem* }
%destructor where_opt            { ExprItem::FreeExprItem($$); }

where_opt(A) ::= .                      { A = nullptr; }
where_opt(A) ::= WHERE condi_expr(X).   { A = X; }

%type groupby_opt                 { GroupOpt* }
%destructor groupby_opt           { delete ($$); }

groupby_opt(A) ::= . { A = nullptr; }
groupby_opt(A) ::= GROUP BY ID(X).   { A = new GroupOpt(&X); }
groupby_opt(A) ::= GROUP BY ID(X) timeval(T). { A = new GroupOpt(&X, T); }

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

/////////////////////////////////// Condition Expression ////////////

%type condi_expr { ExprItem* }
%destructor condi_expr { ExprItem::FreeExprItem($$); }

condi_expr(A) ::= ID(X) LT userval(Y).       { A = ExprItem::MakeCondition(TK_LT, &X, Y); }
condi_expr(A) ::= ID(X) LE userval(Y).       { A = ExprItem::MakeCondition(TK_LE, &X, Y); }
condi_expr(A) ::= ID(X) GT userval(Y).       { A = ExprItem::MakeCondition(TK_GT, &X, Y); }
condi_expr(A) ::= ID(X) GE userval(Y).       { A = ExprItem::MakeCondition(TK_GE, &X, Y); }
condi_expr(A) ::= ID(X) EQ userval(Y).       { A = ExprItem::MakeCondition(TK_EQ, &X, Y); }
condi_expr(A) ::= ID(X) NE userval(Y).       { A = ExprItem::MakeCondition(TK_NE, &X, Y); }
condi_expr(A) ::= ID(X) LIKE userval(Y).     { A = ExprItem::MakeCondition(TK_LIKE, &X, Y); }
condi_expr(A) ::= ID(X) IS NOT NULL.         { A = ExprItem::MakeCondition(TK_ISNOTNULL, &X, nullptr); }
condi_expr(A) ::= ID(X) IS NULL.             { A = ExprItem::MakeCondition(TK_ISNULL, &X, nullptr); }
condi_expr(A) ::= userval(X) EQ userval(Y).  { A = ExprItem::MakeCondition(TK_EQ, X, Y); }
condi_expr(A) ::= userval(X) NE userval(Y).  { A = ExprItem::MakeCondition(TK_NE, X, Y); }
condi_expr(A) ::= ID(X) IN LP userval_list(L) RP.       { A = ExprItem::MakeFuncCondition(TK_IN, &X, L); }
condi_expr(A) ::= ID(X) NOT IN LP userval_list(L) RP.   { A = ExprItem::MakeFuncCondition(TK_NOTIN, &X, L); }


condi_expr(A) ::= condi_expr(X) AND condi_expr(Y).  { A = ExprItem::MakeCondition(TK_AND, X, Y); }

%type userval { ExprItem* }
%destructor userval { ExprItem::FreeExprItem($$); }

userval(A) ::= TRUE(X).                        { A = ExprItem::MakeValue(TK_TRUE, &X); }
userval(A) ::= FALSE(X).                       { A = ExprItem::MakeValue(TK_FALSE, &X); }
userval(A) ::= INTEGER(X).                     { A = ExprItem::MakeValue(TK_INTEGER, &X); }
userval(A) ::= DOUBLE(X).                      { A = ExprItem::MakeValue(TK_DOUBLE, &X); }
userval(A) ::= PLUS INTEGER(X).                { A = ExprItem::MakeValue(TK_INTEGER, &X); }
userval(A) ::= PLUS DOUBLE(X).                 { A = ExprItem::MakeValue(TK_DOUBLE, &X); }
userval(A) ::= MINUS INTEGER(X). [UINTEGER]    { A = ExprItem::MakeValue(TK_UINTEGER, &X); }
userval(A) ::= MINUS DOUBLE(X).   [UDOUBLE]    { A = ExprItem::MakeValue(TK_UDOUBLE, &X); }
userval(A) ::= STRING(X).                      { A = ExprItem::MakeValue(TK_STRING, &X); }
userval(A) ::= BLOB(X).                        { A = ExprItem::MakeValue(TK_BLOB, &X); }
userval(A) ::= ID(N) LP arg_list(L) RP.        { A = ExprItem::MakeFunction(TK_FUNCTION, &N, L, nullptr); } 
userval(A) ::= ID(N) LP RP.                    { A = ExprItem::MakeFunction(TK_FUNCTION, &N, nullptr, nullptr); } 

%type timeval       { ExprItem* }
%destructor timeval { ExprItem::FreeExprItem($$);}

timeval(A) ::= INTEGER(X) ID(U).        { A = ExprItem::MakeTimeVal(true, &X, &U); }
timeval(A) ::= PLUS INTEGER(X) ID(U).   { A = ExprItem::MakeTimeVal(true, &X, &U); }
timeval(A) ::= MINUS INTEGER(X) ID(U).  { A = ExprItem::MakeTimeVal(false, &X, &U); }

%type userval_list       { ExprList* }
%destructor userval_list { ExprList::FreeExprList($$); }

userval_list(A) ::= userval(V).                        
{ A = ExprList::AppendExprItem(nullptr, V); }
userval_list(A) ::= userval_list(L) COMMA userval(V).  
{ A = ExprList::AppendExprItem(L, V); }

%type record_list        { RecordList* }
%destructor record_list  { RecordList::FreeRecordList($$); }

record_list(A) ::= LP userval_list(U) RP.
{ A = RecordList::AppendRecordList(nullptr, U); }
record_list(A) ::= record_list(L) COMMA LP userval_list(U) RP.
{ A = RecordList::AppendRecordList(L, U); }
