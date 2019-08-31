%include {
#include "pdb.h"
#include "expr/pdb_db_int.h"
#include "expr/parse.h"
#include "expr/expr_item.h"
#include "expr/sql_parser.h"
#include "expr/column_item.h"
#include "expr/group_opt.h"
#include "expr/orderby_opt.h"
#include "expr/limit_opt.h"

void pdbSetError(SQLParser* pParse, const char* pErrMsg);
void pdbSelect(SQLParser* pParse, ExprList* pSelList, Token* pSrcTab, ExprItem* pWhere, GroupOpt* pGroup, OrderByOpt* pOrderBy, LimitOpt* pLimit);
void pdbCreateTable(SQLParser* pParse, Token* pTabName, ColumnList* pColList);

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

%nonassoc  ILLEGAL SPACE COMMENT FUNCTION INSERT INTO VALUES IS NOT NULL DELETE TOP TRUE FALSE IN TINYINT SMALLINT INT ISTRUE ISFALSE FLOAT.

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

cmd ::= DELETE from(X) where_opt(Y) SEMI.
{
  pdbDelete(pParse, &X, Y);
}

/////////////////// The SELECT //////////////////////////////////////
cmd ::= SELECT selcollist(W) from(X) where_opt(Y) groupby_opt(G) orderby_opt(O) limit_opt(L) SEMI. 
{
  pdbSelect(pParse, W, &X, Y, G, O, L);
}

%type selcollist                { ExprList* }
%destructor selcollist          { ExprList::FreeExprList($$); }
%type sclp                      { ExprList* }
%destructor sclp                { ExprList::FreeExprList($$); }

sclp(A) ::= selcollist(X) COMMA.             { A = X; }
sclp(A) ::= .                                { A = nullptr; }
selcollist(A) ::= sclp(P) STAR(Q).           {
  A = ExprList::AppendExprItem(P, ExprItem::MakeExpr(TK_STAR, nullptr, nullptr, &Q));
}
selcollist(A) ::= sclp(P) ID(X) as(Y).       {
  ExprItem* pFieldItem = ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &X);
  pFieldItem->SetAliasName(&Y);
  A = ExprList::AppendExprItem(P, pFieldItem);
}

selcollist(A) ::= sclp(P) AVG_FUNC LP ID(X) RP as(Z). {
  ExprItem* pFunc = ExprItem::MakeFunction(TK_AVG_FUNC, &X, &Z);
  A = ExprList::AppendExprItem(P, pFunc);
}

selcollist(A) ::= sclp(P) COUNT_FUNC LP ID(X) RP as(Z). {
  ExprItem* pFunc = ExprItem::MakeFunction(TK_COUNT_FUNC, &X, &Z);
  A = ExprList::AppendExprItem(P, pFunc);
}

selcollist(A) ::= sclp(P) COUNT_FUNC LP STAR(S) RP as(Z). {
  ExprItem* pFunc = ExprItem::MakeFunction(TK_COUNT_FUNC, &S, &Z);
  A = ExprList::AppendExprItem(P, pFunc);
}

selcollist(A) ::= sclp(P) LAST_FUNC LP ID(X) RP as(Z). {
  ExprItem* pFunc = ExprItem::MakeFunction(TK_LAST_FUNC, &X, &Z);
  A = ExprList::AppendExprItem(P, pFunc);
}

selcollist(A) ::= sclp(P) MAX_FUNC LP ID(X) RP as(Z). {
  ExprItem* pFunc = ExprItem::MakeFunction(TK_MAX_FUNC, &X, &Z);
  A = ExprList::AppendExprItem(P, pFunc);
}

selcollist(A) ::= sclp(P) MIN_FUNC LP ID(X) RP as(Z). {
  ExprItem* pFunc = ExprItem::MakeFunction(TK_MIN_FUNC, &X, &Z);
  A = ExprList::AppendExprItem(P, pFunc);
}

selcollist(A) ::= sclp(P) SUM_FUNC LP ID(X) RP as(Z). {
  ExprItem* pFunc = ExprItem::MakeFunction(TK_SUM_FUNC, &X, &Z);
  A = ExprList::AppendExprItem(P, pFunc);
}

selcollist(A) ::= sclp(P) FIRST_FUNC LP ID(X) RP as(Z). {
  ExprItem* pFunc = ExprItem::MakeFunction(TK_FIRST_FUNC, &X, &Z);
  A = ExprList::AppendExprItem(P, pFunc);
}

//%type as {Token}
as(X) ::= .               { X.str_ = nullptr; X.len_ = 0; }
as(X) ::= AS ID(Y).       { X = Y; }

%type from                            {Token}
from(A) ::= FROM ID(X).               {A = X;}

%type where_opt                  { ExprItem* }
%destructor where_opt            { ExprItem::FreeExprItem($$); }

where_opt(A) ::= .                      { A = nullptr; }
where_opt(A) ::= WHERE condi_expr(X).   { A = X; }

%type groupby_opt                 { GroupOpt* }
%destructor groupby_opt           { delete ($$); }

groupby_opt(A) ::= . { A = nullptr; }
groupby_opt(A) ::= GROUP BY ID(X).   { A = new GroupOpt(&X); }
groupby_opt(A) ::= GROUP BY ID(X) INTEGER(Y) ID(Z). { A = new GroupOpt(&X, &Y, &Z); }

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

condi_expr(A) ::= ID(X) LT userval(Y).   { A = ExprItem::MakeExpr(TK_LT, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &X), Y, nullptr); }
condi_expr(A) ::= ID(X) LE userval(Y).   { A = ExprItem::MakeExpr(TK_LE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &X), Y, nullptr); }
condi_expr(A) ::= ID(X) GT userval(Y).   { A = ExprItem::MakeExpr(TK_GT, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &X), Y, nullptr); }
condi_expr(A) ::= ID(X) GE userval(Y).   { A = ExprItem::MakeExpr(TK_GE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &X), Y, nullptr); }
condi_expr(A) ::= ID(X) EQ userval(Y).   { A = ExprItem::MakeExpr(TK_EQ, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &X), Y, nullptr); }
condi_expr(A) ::= ID(X) NE userval(Y).   { A = ExprItem::MakeExpr(TK_NE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &X), Y, nullptr); }
condi_expr(A) ::= ID(X) LIKE userval(Y). { A = ExprItem::MakeExpr(TK_LIKE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &X), Y, nullptr); }
condi_expr(A) ::= ID(X) EQ TRUE.         { A = ExprItem::MakeExpr(TK_ISTRUE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &X), nullptr, nullptr); }
condi_expr(A) ::= ID(X) EQ FALSE.        { A = ExprItem::MakeExpr(TK_ISFALSE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &X), nullptr, nullptr); }
condi_expr(A) ::= ID(X) NE TRUE.         { A = ExprItem::MakeExpr(TK_ISFALSE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &X), nullptr, nullptr); }
condi_expr(A) ::= ID(X) NE FALSE.        { A = ExprItem::MakeExpr(TK_ISTRUE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &X), nullptr, nullptr); }


condi_expr(A) ::= condi_expr(X) AND condi_expr(Y).  { A = ExprItem::MakeExpr(TK_AND, X, Y, nullptr); }

%type userval { ExprItem* }
%destructor userval { ExprItem::FreeExprItem($$); }

userval(A) ::= INTEGER(X).                     { A = ExprItem::MakeExpr(TK_INTEGER, nullptr, nullptr, &X); }
userval(A) ::= DOUBLE(X).                      { A = ExprItem::MakeExpr(TK_DOUBLE, nullptr, nullptr, &X); }
userval(A) ::= PLUS INTEGER(X).                { A = ExprItem::MakeExpr(TK_INTEGER, nullptr, nullptr, &X); }
userval(A) ::= PLUS DOUBLE(X).                 { A = ExprItem::MakeExpr(TK_DOUBLE, nullptr, nullptr, &X); }
userval(A) ::= MINUS INTEGER(X). [UINTEGER]    { A = ExprItem::MakeExpr(TK_UINTEGER, nullptr, nullptr, &X); }
userval(A) ::= MINUS DOUBLE(X).   [UDOUBLE]    { A = ExprItem::MakeExpr(TK_UDOUBLE, nullptr, nullptr, &X); }
userval(A) ::= STRING(X).                      { A = ExprItem::MakeExpr(TK_STRING, nullptr, nullptr, &X); }
userval(A) ::= BLOB(X).                        { A = ExprItem::MakeExpr(TK_BLOB, nullptr, nullptr, &X); }


