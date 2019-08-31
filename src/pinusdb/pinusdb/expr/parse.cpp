/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is include which follows the "include" declaration
** in the input file. */
#include <stdio.h>
#line 1 "parse.y"

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


#line 39 "parse.c"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    pdbParseTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is pdbParseTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.
**    pdbParseARG_SDECL     A static variable declaration for the %extra_argument
**    pdbParseARG_PDECL     A parameter declaration for the %extra_argument
**    pdbParseARG_STORE     Code to store %extra_argument into yypParser
**    pdbParseARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
/*  */
#define YYCODETYPE unsigned char
#define YYNOCODE 100
#define YYACTIONTYPE unsigned short int
#define pdbParseTOKENTYPE Token
typedef union {
  pdbParseTOKENTYPE yy0;
  ColumnItem* yy2;
  ColumnList* yy59;
  ExprItem* yy67;
  ExprList* yy124;
  LimitOpt* yy125;
  GroupOpt* yy135;
  Token yy144;
  OrderByOpt* yy189;
  int yy199;
} YYMINORTYPE;
#define YYSTACKDEPTH 100
#define pdbParseARG_SDECL SQLParser *pParse;
#define pdbParseARG_PDECL ,SQLParser *pParse
#define pdbParseARG_FETCH SQLParser *pParse = yypParser->pParse
#define pdbParseARG_STORE yypParser->pParse = pParse
#define YYNSTATE 185
#define YYNRULE 74
#define YYERRORSYMBOL 84
#define YYERRSYMDT yy199
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* Next are that tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
static YYACTIONTYPE yy_action[] = {
 /*     0 */    68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*    10 */   143,  111,  112,   97,   63,    8,   92,   62,   78,  236,
 /*    20 */    16,   65,  169,   11,   97,    9,  107,  108,   93,  142,
 /*    30 */   147,  152,  160,  165,  170,  175,  180,   14,   95,   97,
 /*    40 */    17,   79,  237,   50,  140,  116,   53,  133,  134,  227,
 /*    50 */    96,  119,    1,   89,  191,  141,  128,  105,   10,   22,
 /*    60 */   145,   18,  126,  154,   89,   90,   91,   94,   38,   49,
 /*    70 */    98,   39,  168,   58,   42,   23,   90,   91,   94,   89,
 /*    80 */    32,   98,  157,   19,  118,  167,   83,   20,  166,   21,
 /*    90 */   164,   90,   91,   94,  196,  163,   98,  109,  101,  103,
 /*   100 */    87,   99,  113,  171,   24,  162,   25,   15,   26,   27,
 /*   110 */   161,   28,  159,   29,  158,   30,   31,  156,  187,  155,
 /*   120 */   172,   33,  186,   34,   35,  151,  153,   36,   37,  188,
 /*   130 */   150,  173,   13,  148,  149,   40,  192,   43,  146,  144,
 /*   140 */    45,  138,   12,  139,   44,   47,   46,   48,  137,  136,
 /*   150 */   194,  132,   51,  174,   52,  131,  193,  211,  238,   54,
 /*   160 */   195,  129,   56,   55,   57,  176,  125,  197,  124,  178,
 /*   170 */    64,   59,  127,  121,  177,  123,   60,   61,   67,  185,
 /*   180 */   130,    7,  135,   83,  179,  117,  181,    6,  182,    5,
 /*   190 */     4,    2,  183,  210,  175,  175,  175,   41,   66,   82,
 /*   200 */   122,   86,  175,  175,  116,  175,  175,  175,  175,  175,
 /*   210 */   175,  175,  120,  175,  175,  175,  175,  175,  175,  175,
 /*   220 */   175,  175,  175,  114,  175,  175,  110,  175,  175,  175,
 /*   230 */   175,  175,  175,  175,    3,  175,  175,  175,  175,  106,
 /*   240 */   175,  175,  175,  104,  102,  100,   85,  184,  260,  175,
 /*   250 */   175,  175,  175,   88,  175,  175,  115,  175,   80,  175,
 /*   260 */   175,   81,  175,  175,  175,  175,  175,  175,  175,  175,
 /*   270 */    84,
};
static YYCODETYPE yy_lookahead[] = {
 /*     0 */    44,   45,   46,   47,   48,   49,   50,   51,   52,   53,
 /*    10 */    36,   13,   14,   26,   34,   26,   66,   87,   88,   27,
 /*    20 */    27,   41,   96,   23,   26,   36,   13,   14,   78,   55,
 /*    30 */    56,   57,   58,   59,   60,   61,   62,   37,   66,   26,
 /*    40 */    40,   11,   27,   37,   41,   42,   40,   68,   69,   27,
 /*    50 */    78,   91,   22,   66,    0,   95,   41,   32,   28,   29,
 /*    60 */    63,   26,   70,   36,   66,   78,   79,   80,   38,   39,
 /*    70 */    83,   37,   34,   43,   40,   30,   78,   79,   80,   66,
 /*    80 */    35,   83,   55,   42,   54,   36,   64,   36,   33,   27,
 /*    90 */    96,   78,   79,   80,    0,   34,   83,   72,   73,   74,
 /*   100 */    75,   76,   77,   33,   31,   36,   86,   36,   32,   30,
 /*   110 */    33,   33,   96,   26,   34,   34,   27,   96,    0,   34,
 /*   120 */    36,   31,    0,   86,   32,   96,   33,   36,   27,    0,
 /*   130 */    34,   34,   27,   33,   36,   36,    0,   26,   36,   96,
 /*   140 */    26,   66,   86,   36,   41,   36,   42,   27,   36,   25,
 /*   150 */     0,   36,   36,   96,   27,   25,    0,    0,   27,   26,
 /*   160 */     0,   66,   36,   42,   27,   33,   27,    0,   94,   34,
 /*   170 */    27,   37,   66,   90,   36,   93,   36,   33,   36,    0,
 /*   180 */    67,   27,   65,   64,   96,   36,   33,   26,   36,   25,
 /*   190 */    24,   23,   34,    0,   99,   99,   99,   27,   88,   27,
 /*   200 */    92,   36,   99,   99,   42,   99,   99,   99,   99,   99,
 /*   210 */    99,   99,   89,   99,   99,   99,   99,   99,   99,   99,
 /*   220 */    99,   99,   99,   98,   99,   99,   98,   99,   99,   99,
 /*   230 */    99,   99,   99,   99,   86,   99,   99,   99,   99,   98,
 /*   240 */    99,   99,   99,   98,   98,   98,   71,   96,   85,   99,
 /*   250 */    99,   99,   99,   98,   99,   99,   97,   99,   89,   99,
 /*   260 */    99,   90,   99,   99,   99,   99,   99,   99,   99,   99,
 /*   270 */    97,
};
#define YY_SHIFT_USE_DFLT (-51)
static short yy_shift_ofst[] = {
 /*     0 */    30,  168,  -11,  166,  164,  161,  154,  179,  -51,  -51,
 /*    10 */     0,  -11,  105,  122,   71,   -7,   54,   35,   41,   51,
 /*    20 */    62,   94,   45,   73,  -11,   76,   79,   78,   87,   81,
 /*    30 */    89,  118,   90,  -11,   92,   91,  101,  129,   34,   99,
 /*    40 */   170,  136,  111,  103,  114,  104,  109,  120,  150,    6,
 /*    50 */   116,  127,  156,  133,  121,  126,  137,  160,  134,  140,
 /*    60 */   144,  142,  -20,  143,  167,  142,  -51,  -44,  -51,  -51,
 /*    70 */   -51,  -51,  -51,  -51,  -51,  -51,  -51,  -51,  -51,  162,
 /*    80 */    22,  172,  193,  165,  175,  165,   25,  -13,  -51,  -51,
 /*    90 */   -51,  -50,  -51,  -51,  -28,  -51,  -51,  -51,  -51,  -13,
 /*   100 */   -51,  -13,  -51,  -13,  -51,   13,  -51,  -51,  -51,   -2,
 /*   110 */   -51,  -51,  -51,  -13,  -51,  -51,  149,  -51,  -51,    3,
 /*   120 */   119,  117,  113,   -8,  139,  157,  106,   15,   95,  131,
 /*   130 */   130,  115,  -21,  -51,  -51,  124,  112,   75,  107,  -51,
 /*   140 */   -51,  -26,  -51,   -3,  -51,  102,  -51,  100,   98,   96,
 /*   150 */    -3,  -51,   93,   27,   85,   -3,  -51,   80,   -3,  -51,
 /*   160 */    77,   69,   61,   -3,  -51,   55,   49,   38,   -3,  -51,
 /*   170 */    70,   84,   97,   -3,  -51,  132,  138,  135,   -3,  -51,
 /*   180 */   153,  152,  158,   -3,  -51,
};
#define YY_REDUCE_USE_DFLT (-75)
static short yy_reduce_ofst[] = {
 /*     0 */   163,  -75,  148,  -75,  -75,  -75,  -75,  -75,  -75,  -75,
 /*    10 */   -75,   56,  -75,  -75,  -75,  -75,  -75,  -75,  -75,  -75,
 /*    20 */   -75,  -75,  -75,  -75,   20,  -75,  -75,  -75,  -75,  -75,
 /*    30 */   -75,  -75,  -75,   37,  -75,  -75,  -75,  -75,  -75,  -75,
 /*    40 */   -75,  -75,  -75,  -75,  -75,  -75,  -75,  -75,  -75,  -75,
 /*    50 */   -75,  -75,  -75,  -75,  -75,  -75,  -75,  -75,  -75,  -75,
 /*    60 */   -75,  -70,  -75,  -75,  -75,  110,  -75,  -75,  -75,  -75,
 /*    70 */   -75,  -75,  -75,  -75,  -75,  -75,  -75,  -75,  -75,  169,
 /*    80 */   171,  -75,  -75,  173,  -75,  159,  -75,  155,  -75,  -75,
 /*    90 */   -75,  -75,  -75,  -75,  -75,  -75,  -75,  -75,  -75,  147,
 /*   100 */   -75,  146,  -75,  145,  -75,  141,  -75,  -75,  -75,  128,
 /*   110 */   -75,  -75,  -75,  125,  -75,  -75,  -75,  -75,  -40,  123,
 /*   120 */    83,  108,   82,   74,  -75,  -75,  -75,  -75,  -75,  -75,
 /*   130 */   -75,  -75,  -75,  -75,  -75,  -75,  -75,  -75,  -75,  -75,
 /*   140 */   -75,  -75,  -75,   43,  -75,  -75,  -75,  -75,  -75,  -75,
 /*   150 */    29,  -75,  -75,  -75,  -75,   21,  -75,  -75,   16,  -75,
 /*   160 */   -75,  -75,  -75,   -6,  -75,  -75,  -75,  -75,  -74,  -75,
 /*   170 */   -75,  -75,  -75,   57,  -75,  -75,  -75,  -75,   88,  -75,
 /*   180 */   -75,  -75,  -75,  151,  -75,
};
static YYACTIONTYPE yy_default[] = {
 /*     0 */   259,  259,  259,  259,  259,  259,  259,  259,  189,  190,
 /*    10 */   259,  259,  259,  259,  259,  259,  259,  259,  259,  259,
 /*    20 */   259,  259,  259,  259,  259,  259,  259,  259,  259,  259,
 /*    30 */   259,  259,  259,  259,  259,  259,  259,  259,  259,  259,
 /*    40 */   259,  259,  259,  259,  259,  259,  259,  259,  259,  259,
 /*    50 */   259,  259,  259,  259,  259,  259,  259,  259,  259,  259,
 /*    60 */   259,  259,  259,  259,  259,  259,  198,  259,  200,  201,
 /*    70 */   202,  203,  204,  205,  206,  207,  208,  209,  199,  259,
 /*    80 */   259,  259,  259,  259,  228,  259,  259,  259,  239,  251,
 /*    90 */   252,  259,  253,  254,  259,  255,  256,  257,  258,  259,
 /*   100 */   240,  259,  241,  259,  242,  259,  243,  246,  247,  259,
 /*   110 */   244,  248,  249,  259,  245,  250,  259,  226,  213,  259,
 /*   120 */   227,  229,  232,  259,  259,  259,  259,  259,  259,  259,
 /*   130 */   259,  259,  233,  234,  235,  259,  259,  230,  259,  231,
 /*   140 */   212,  259,  214,  224,  215,  259,  225,  259,  259,  259,
 /*   150 */   224,  216,  259,  259,  259,  224,  217,  259,  224,  218,
 /*   160 */   259,  259,  259,  224,  219,  259,  259,  259,  224,  220,
 /*   170 */   259,  259,  259,  224,  221,  259,  259,  259,  224,  222,
 /*   180 */   259,  259,  259,  224,  223,
};
#define YY_SZ_ACTTAB (sizeof(yy_action)/sizeof(yy_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammer, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  int stateno;       /* The state-number */
  int major;         /* The major token value.  This is the code
                     ** number for the token at this stack level */
  YYMINORTYPE minor; /* The user-supplied minor token value.  This
                     ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
  int yyerrcnt;                 /* Shifts left before out of the error */
  pdbParseARG_SDECL                /* A place to hold %extra_argument */
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void pdbParseTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *yyTokenName[] = { 
  "$",             "ILLEGAL",       "SPACE",         "COMMENT",     
  "FUNCTION",      "INSERT",        "INTO",          "VALUES",      
  "IS",            "NOT",           "NULL",          "DELETE",      
  "TOP",           "TRUE",          "FALSE",         "IN",          
  "TINYINT",       "SMALLINT",      "INT",           "ISTRUE",      
  "ISFALSE",       "FLOAT",         "ADD",           "USER",        
  "IDENTIFIED",    "BY",            "STRING",        "SEMI",        
  "DROP",          "SET",           "PASSWORD",      "FOR",         
  "EQ",            "LP",            "RP",            "ROLE",        
  "ID",            "TABLE",         "ATTACH",        "DETACH",      
  "DATAFILE",      "COMMA",         "FROM",          "CREATE",      
  "BOOL_TYPE",     "BIGINT_TYPE",   "DOUBLE_TYPE",   "STRING_TYPE", 
  "BLOB_TYPE",     "DATETIME_TYPE",  "REAL2_TYPE",    "REAL3_TYPE",  
  "REAL4_TYPE",    "REAL6_TYPE",    "SELECT",        "STAR",        
  "AVG_FUNC",      "COUNT_FUNC",    "LAST_FUNC",     "MAX_FUNC",    
  "MIN_FUNC",      "SUM_FUNC",      "FIRST_FUNC",    "AS",          
  "WHERE",         "GROUP",         "INTEGER",       "ORDER",       
  "ASC",           "DESC",          "LIMIT",         "AND",         
  "NE",            "GT",            "GE",            "LT",          
  "LE",            "LIKE",          "DOUBLE",        "PLUS",        
  "MINUS",         "UINTEGER",      "UDOUBLE",       "BLOB",        
  "error",         "cmd",           "username",      "cre_columnlist",
  "cre_column",    "from",          "where_opt",     "selcollist",  
  "groupby_opt",   "orderby_opt",   "limit_opt",     "sclp",        
  "as",            "condi_expr",    "userval",     
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *yyRuleName[] = {
 /*   0 */ "cmd ::= ADD USER username IDENTIFIED BY STRING SEMI",
 /*   1 */ "cmd ::= DROP USER username SEMI",
 /*   2 */ "cmd ::= SET PASSWORD FOR username EQ PASSWORD LP STRING RP SEMI",
 /*   3 */ "cmd ::= SET ROLE FOR username EQ ID SEMI",
 /*   4 */ "username ::= STRING",
 /*   5 */ "username ::= ID",
 /*   6 */ "cmd ::= DROP TABLE ID SEMI",
 /*   7 */ "cmd ::= ATTACH TABLE ID SEMI",
 /*   8 */ "cmd ::= DETACH TABLE ID SEMI",
 /*   9 */ "cmd ::= ATTACH DATAFILE STRING COMMA STRING FROM ID SEMI",
 /*  10 */ "cmd ::= DETACH DATAFILE STRING FROM ID SEMI",
 /*  11 */ "cmd ::= DROP DATAFILE STRING FROM ID SEMI",
 /*  12 */ "cmd ::= CREATE TABLE ID LP cre_columnlist RP SEMI",
 /*  13 */ "cre_columnlist ::= cre_columnlist COMMA cre_column",
 /*  14 */ "cre_columnlist ::= cre_column",
 /*  15 */ "cre_column ::= ID BOOL_TYPE",
 /*  16 */ "cre_column ::= ID BIGINT_TYPE",
 /*  17 */ "cre_column ::= ID DOUBLE_TYPE",
 /*  18 */ "cre_column ::= ID STRING_TYPE",
 /*  19 */ "cre_column ::= ID BLOB_TYPE",
 /*  20 */ "cre_column ::= ID DATETIME_TYPE",
 /*  21 */ "cre_column ::= ID REAL2_TYPE",
 /*  22 */ "cre_column ::= ID REAL3_TYPE",
 /*  23 */ "cre_column ::= ID REAL4_TYPE",
 /*  24 */ "cre_column ::= ID REAL6_TYPE",
 /*  25 */ "cmd ::= DELETE from where_opt SEMI",
 /*  26 */ "cmd ::= SELECT selcollist from where_opt groupby_opt orderby_opt limit_opt SEMI",
 /*  27 */ "sclp ::= selcollist COMMA",
 /*  28 */ "sclp ::=",
 /*  29 */ "selcollist ::= sclp STAR",
 /*  30 */ "selcollist ::= sclp ID as",
 /*  31 */ "selcollist ::= sclp AVG_FUNC LP ID RP as",
 /*  32 */ "selcollist ::= sclp COUNT_FUNC LP ID RP as",
 /*  33 */ "selcollist ::= sclp COUNT_FUNC LP STAR RP as",
 /*  34 */ "selcollist ::= sclp LAST_FUNC LP ID RP as",
 /*  35 */ "selcollist ::= sclp MAX_FUNC LP ID RP as",
 /*  36 */ "selcollist ::= sclp MIN_FUNC LP ID RP as",
 /*  37 */ "selcollist ::= sclp SUM_FUNC LP ID RP as",
 /*  38 */ "selcollist ::= sclp FIRST_FUNC LP ID RP as",
 /*  39 */ "as ::=",
 /*  40 */ "as ::= AS ID",
 /*  41 */ "from ::= FROM ID",
 /*  42 */ "where_opt ::=",
 /*  43 */ "where_opt ::= WHERE condi_expr",
 /*  44 */ "groupby_opt ::=",
 /*  45 */ "groupby_opt ::= GROUP BY ID",
 /*  46 */ "groupby_opt ::= GROUP BY ID INTEGER ID",
 /*  47 */ "orderby_opt ::=",
 /*  48 */ "orderby_opt ::= ORDER BY ID",
 /*  49 */ "orderby_opt ::= ORDER BY ID ASC",
 /*  50 */ "orderby_opt ::= ORDER BY ID DESC",
 /*  51 */ "limit_opt ::=",
 /*  52 */ "limit_opt ::= LIMIT INTEGER",
 /*  53 */ "limit_opt ::= LIMIT INTEGER COMMA INTEGER",
 /*  54 */ "condi_expr ::= ID LT userval",
 /*  55 */ "condi_expr ::= ID LE userval",
 /*  56 */ "condi_expr ::= ID GT userval",
 /*  57 */ "condi_expr ::= ID GE userval",
 /*  58 */ "condi_expr ::= ID EQ userval",
 /*  59 */ "condi_expr ::= ID NE userval",
 /*  60 */ "condi_expr ::= ID LIKE userval",
 /*  61 */ "condi_expr ::= ID EQ TRUE",
 /*  62 */ "condi_expr ::= ID EQ FALSE",
 /*  63 */ "condi_expr ::= ID NE TRUE",
 /*  64 */ "condi_expr ::= ID NE FALSE",
 /*  65 */ "condi_expr ::= condi_expr AND condi_expr",
 /*  66 */ "userval ::= INTEGER",
 /*  67 */ "userval ::= DOUBLE",
 /*  68 */ "userval ::= PLUS INTEGER",
 /*  69 */ "userval ::= PLUS DOUBLE",
 /*  70 */ "userval ::= MINUS INTEGER",
 /*  71 */ "userval ::= MINUS DOUBLE",
 /*  72 */ "userval ::= STRING",
 /*  73 */ "userval ::= BLOB",
};
#endif /* NDEBUG */

/*
** This function returns the symbolic name associated with a token
** value.
*/
const char *pdbParseTokenName(int tokenType){
#ifndef NDEBUG
  if( tokenType>0 && tokenType<(sizeof(yyTokenName)/sizeof(yyTokenName[0])) ){
    return yyTokenName[tokenType];
  }else{
    return "Unknown";
  }
#else
  return "";
#endif
}

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to pdbParse and pdbParseFree.
*/
void *pdbParseAlloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(YYCODETYPE yymajor, YYMINORTYPE *yypminor){
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
    case 87:
#line 102 "parse.y"
{ ColumnList::FreeColumnList((yypminor->yy59)); }
#line 539 "parse.c"
      break;
    case 88:
#line 104 "parse.y"
{ ColumnItem::FreeColumnItem((yypminor->yy2)); }
#line 544 "parse.c"
      break;
    case 90:
#line 202 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy67)); }
#line 549 "parse.c"
      break;
    case 91:
#line 139 "parse.y"
{ ExprList::FreeExprList((yypminor->yy124)); }
#line 554 "parse.c"
      break;
    case 92:
#line 208 "parse.y"
{ delete ((yypminor->yy135)); }
#line 559 "parse.c"
      break;
    case 93:
#line 215 "parse.y"
{ delete ((yypminor->yy189)); }
#line 564 "parse.c"
      break;
    case 94:
#line 223 "parse.y"
{ delete ((yypminor->yy125)); }
#line 569 "parse.c"
      break;
    case 95:
#line 141 "parse.y"
{ ExprList::FreeExprList((yypminor->yy124)); }
#line 574 "parse.c"
      break;
    case 97:
#line 236 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy67)); }
#line 579 "parse.c"
      break;
    case 98:
#line 254 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy67)); }
#line 584 "parse.c"
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor( yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from pdbParseAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void pdbParseFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
  (*freeProc)((void*)pParser);
}

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  int iLookAhead            /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  /* if( pParser->yyidx<0 ) return YY_NO_ACTION;  */
  i = yy_shift_ofst[stateno];
  if( i==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  if( iLookAhead==YYNOCODE ){
    return YY_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
#ifdef YYFALLBACK
    int iFallback;            /* Fallback token */
    if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
           && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
           yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
      }
#endif
      return yy_find_shift_action(pParser, iFallback);
    }
#endif
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  yyParser *pParser,        /* The parser */
  int iLookAhead            /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  i = yy_reduce_ofst[stateno];
  if( i==YY_REDUCE_USE_DFLT ){
    return yy_default[stateno];
  }
  if( iLookAhead==YYNOCODE ){
    return YY_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer ot the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
  if( yypParser->yyidx>=YYSTACKDEPTH ){
     pdbParseARG_FETCH;
     yypParser->yyidx--;
#ifndef NDEBUG
     if( yyTraceFILE ){
       fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
     }
#endif
     while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
     /* Here code is inserted which will execute if the parser
     ** stack every overflows */
     pdbParseARG_STORE; /* Suppress warning about unused %extra_argument var */
     return;
  }
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = yyNewState;
  yytos->major = yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 85, 7 },
  { 85, 4 },
  { 85, 10 },
  { 85, 7 },
  { 86, 1 },
  { 86, 1 },
  { 85, 4 },
  { 85, 4 },
  { 85, 4 },
  { 85, 8 },
  { 85, 6 },
  { 85, 6 },
  { 85, 7 },
  { 87, 3 },
  { 87, 1 },
  { 88, 2 },
  { 88, 2 },
  { 88, 2 },
  { 88, 2 },
  { 88, 2 },
  { 88, 2 },
  { 88, 2 },
  { 88, 2 },
  { 88, 2 },
  { 88, 2 },
  { 85, 4 },
  { 85, 8 },
  { 95, 2 },
  { 95, 0 },
  { 91, 2 },
  { 91, 3 },
  { 91, 6 },
  { 91, 6 },
  { 91, 6 },
  { 91, 6 },
  { 91, 6 },
  { 91, 6 },
  { 91, 6 },
  { 91, 6 },
  { 96, 0 },
  { 96, 2 },
  { 89, 2 },
  { 90, 0 },
  { 90, 2 },
  { 92, 0 },
  { 92, 3 },
  { 92, 5 },
  { 93, 0 },
  { 93, 3 },
  { 93, 4 },
  { 93, 4 },
  { 94, 0 },
  { 94, 2 },
  { 94, 4 },
  { 97, 3 },
  { 97, 3 },
  { 97, 3 },
  { 97, 3 },
  { 97, 3 },
  { 97, 3 },
  { 97, 3 },
  { 97, 3 },
  { 97, 3 },
  { 97, 3 },
  { 97, 3 },
  { 97, 3 },
  { 98, 1 },
  { 98, 1 },
  { 98, 2 },
  { 98, 2 },
  { 98, 2 },
  { 98, 2 },
  { 98, 1 },
  { 98, 1 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  pdbParseARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<sizeof(yyRuleName)/sizeof(yyRuleName[0]) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0:
#line 46 "parse.y"
{
  pdbAddUser(pParse, &yymsp[-4].minor.yy144, &yymsp[-1].minor.yy0);
}
#line 875 "parse.c"
        /* No destructor defined for ADD */
        /* No destructor defined for USER */
        /* No destructor defined for IDENTIFIED */
        /* No destructor defined for BY */
        /* No destructor defined for SEMI */
        break;
      case 1:
#line 50 "parse.y"
{
  pdbDropUser(pParse, &yymsp[-1].minor.yy144);
}
#line 887 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for USER */
        /* No destructor defined for SEMI */
        break;
      case 2:
#line 55 "parse.y"
{
  pdbChangePwd(pParse, &yymsp[-6].minor.yy144, &yymsp[-2].minor.yy0); 
}
#line 897 "parse.c"
        /* No destructor defined for SET */
        /* No destructor defined for PASSWORD */
        /* No destructor defined for FOR */
        /* No destructor defined for EQ */
        /* No destructor defined for PASSWORD */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for SEMI */
        break;
      case 3:
#line 59 "parse.y"
{
  pdbChangeRole(pParse, &yymsp[-3].minor.yy144, &yymsp[-1].minor.yy0);
}
#line 912 "parse.c"
        /* No destructor defined for SET */
        /* No destructor defined for ROLE */
        /* No destructor defined for FOR */
        /* No destructor defined for EQ */
        /* No destructor defined for SEMI */
        break;
      case 4:
#line 64 "parse.y"
{ yygotominor.yy144 = yymsp[0].minor.yy0; }
#line 922 "parse.c"
        break;
      case 5:
#line 65 "parse.y"
{ yygotominor.yy144 = yymsp[0].minor.yy0; }
#line 927 "parse.c"
        break;
      case 6:
#line 68 "parse.y"
{
  pdbDropTable(pParse, &yymsp[-1].minor.yy0);
}
#line 934 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 7:
#line 72 "parse.y"
{
  pdbAttachTable(pParse, &yymsp[-1].minor.yy0);
}
#line 944 "parse.c"
        /* No destructor defined for ATTACH */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 8:
#line 76 "parse.y"
{
  pdbDetachTable(pParse, &yymsp[-1].minor.yy0);
}
#line 954 "parse.c"
        /* No destructor defined for DETACH */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 9:
#line 82 "parse.y"
{
  pdbAttachFile(pParse, &yymsp[-1].minor.yy0, &yymsp[-5].minor.yy0, &yymsp[-3].minor.yy0);
}
#line 964 "parse.c"
        /* No destructor defined for ATTACH */
        /* No destructor defined for DATAFILE */
        /* No destructor defined for COMMA */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 10:
#line 86 "parse.y"
{
  pdbDetachFile(pParse, &yymsp[-1].minor.yy0, &yymsp[-3].minor.yy0);
}
#line 976 "parse.c"
        /* No destructor defined for DETACH */
        /* No destructor defined for DATAFILE */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 11:
#line 90 "parse.y"
{
  pdbDropFile(pParse, &yymsp[-1].minor.yy0, &yymsp[-3].minor.yy0);
}
#line 987 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for DATAFILE */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 12:
#line 97 "parse.y"
{
  pdbCreateTable(pParse, &yymsp[-4].minor.yy0, yymsp[-2].minor.yy59);
}
#line 998 "parse.c"
        /* No destructor defined for CREATE */
        /* No destructor defined for TABLE */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for SEMI */
        break;
      case 13:
#line 106 "parse.y"
{
  yygotominor.yy59 = ColumnList::AppendColumnItem(yymsp[-2].minor.yy59, yymsp[0].minor.yy2);
}
#line 1010 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 14:
#line 109 "parse.y"
{
  yygotominor.yy59 = ColumnList::AppendColumnItem(nullptr, yymsp[0].minor.yy2);
}
#line 1018 "parse.c"
        break;
      case 15:
#line 113 "parse.y"
{ yygotominor.yy2 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_BOOL); }
#line 1023 "parse.c"
        /* No destructor defined for BOOL_TYPE */
        break;
      case 16:
#line 114 "parse.y"
{ yygotominor.yy2 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_INT64); }
#line 1029 "parse.c"
        /* No destructor defined for BIGINT_TYPE */
        break;
      case 17:
#line 115 "parse.y"
{ yygotominor.yy2 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_DOUBLE); }
#line 1035 "parse.c"
        /* No destructor defined for DOUBLE_TYPE */
        break;
      case 18:
#line 116 "parse.y"
{ yygotominor.yy2 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_STRING); }
#line 1041 "parse.c"
        /* No destructor defined for STRING_TYPE */
        break;
      case 19:
#line 117 "parse.y"
{ yygotominor.yy2 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_BLOB); }
#line 1047 "parse.c"
        /* No destructor defined for BLOB_TYPE */
        break;
      case 20:
#line 118 "parse.y"
{ yygotominor.yy2 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_DATETIME); }
#line 1053 "parse.c"
        /* No destructor defined for DATETIME_TYPE */
        break;
      case 21:
#line 119 "parse.y"
{ yygotominor.yy2 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL2); }
#line 1059 "parse.c"
        /* No destructor defined for REAL2_TYPE */
        break;
      case 22:
#line 120 "parse.y"
{ yygotominor.yy2 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL3); }
#line 1065 "parse.c"
        /* No destructor defined for REAL3_TYPE */
        break;
      case 23:
#line 121 "parse.y"
{ yygotominor.yy2 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL4); }
#line 1071 "parse.c"
        /* No destructor defined for REAL4_TYPE */
        break;
      case 24:
#line 122 "parse.y"
{ yygotominor.yy2 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL6); }
#line 1077 "parse.c"
        /* No destructor defined for REAL6_TYPE */
        break;
      case 25:
#line 128 "parse.y"
{
  pdbDelete(pParse, &yymsp[-2].minor.yy144, yymsp[-1].minor.yy67);
}
#line 1085 "parse.c"
        /* No destructor defined for DELETE */
        /* No destructor defined for SEMI */
        break;
      case 26:
#line 134 "parse.y"
{
  pdbSelect(pParse, yymsp[-6].minor.yy124, &yymsp[-5].minor.yy144, yymsp[-4].minor.yy67, yymsp[-3].minor.yy135, yymsp[-2].minor.yy189, yymsp[-1].minor.yy125);
}
#line 1094 "parse.c"
        /* No destructor defined for SELECT */
        /* No destructor defined for SEMI */
        break;
      case 27:
#line 143 "parse.y"
{ yygotominor.yy124 = yymsp[-1].minor.yy124; }
#line 1101 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 28:
#line 144 "parse.y"
{ yygotominor.yy124 = nullptr; }
#line 1107 "parse.c"
        break;
      case 29:
#line 145 "parse.y"
{
  yygotominor.yy124 = ExprList::AppendExprItem(yymsp[-1].minor.yy124, ExprItem::MakeExpr(TK_STAR, nullptr, nullptr, &yymsp[0].minor.yy0));
}
#line 1114 "parse.c"
        break;
      case 30:
#line 148 "parse.y"
{
  ExprItem* pFieldItem = ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-1].minor.yy0);
  pFieldItem->SetAliasName(&yymsp[0].minor.yy144);
  yygotominor.yy124 = ExprList::AppendExprItem(yymsp[-2].minor.yy124, pFieldItem);
}
#line 1123 "parse.c"
        break;
      case 31:
#line 154 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_AVG_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy144);
  yygotominor.yy124 = ExprList::AppendExprItem(yymsp[-5].minor.yy124, pFunc);
}
#line 1131 "parse.c"
        /* No destructor defined for AVG_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 32:
#line 159 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_COUNT_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy144);
  yygotominor.yy124 = ExprList::AppendExprItem(yymsp[-5].minor.yy124, pFunc);
}
#line 1142 "parse.c"
        /* No destructor defined for COUNT_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 33:
#line 164 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_COUNT_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy144);
  yygotominor.yy124 = ExprList::AppendExprItem(yymsp[-5].minor.yy124, pFunc);
}
#line 1153 "parse.c"
        /* No destructor defined for COUNT_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 34:
#line 169 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_LAST_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy144);
  yygotominor.yy124 = ExprList::AppendExprItem(yymsp[-5].minor.yy124, pFunc);
}
#line 1164 "parse.c"
        /* No destructor defined for LAST_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 35:
#line 174 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_MAX_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy144);
  yygotominor.yy124 = ExprList::AppendExprItem(yymsp[-5].minor.yy124, pFunc);
}
#line 1175 "parse.c"
        /* No destructor defined for MAX_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 36:
#line 179 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_MIN_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy144);
  yygotominor.yy124 = ExprList::AppendExprItem(yymsp[-5].minor.yy124, pFunc);
}
#line 1186 "parse.c"
        /* No destructor defined for MIN_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 37:
#line 184 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_SUM_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy144);
  yygotominor.yy124 = ExprList::AppendExprItem(yymsp[-5].minor.yy124, pFunc);
}
#line 1197 "parse.c"
        /* No destructor defined for SUM_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 38:
#line 189 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_FIRST_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy144);
  yygotominor.yy124 = ExprList::AppendExprItem(yymsp[-5].minor.yy124, pFunc);
}
#line 1208 "parse.c"
        /* No destructor defined for FIRST_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 39:
#line 195 "parse.y"
{ yygotominor.yy144.str_ = nullptr; yygotominor.yy144.len_ = 0; }
#line 1216 "parse.c"
        break;
      case 40:
#line 196 "parse.y"
{ yygotominor.yy144 = yymsp[0].minor.yy0; }
#line 1221 "parse.c"
        /* No destructor defined for AS */
        break;
      case 41:
#line 199 "parse.y"
{yygotominor.yy144 = yymsp[0].minor.yy0;}
#line 1227 "parse.c"
        /* No destructor defined for FROM */
        break;
      case 42:
#line 204 "parse.y"
{ yygotominor.yy67 = nullptr; }
#line 1233 "parse.c"
        break;
      case 43:
#line 205 "parse.y"
{ yygotominor.yy67 = yymsp[0].minor.yy67; }
#line 1238 "parse.c"
        /* No destructor defined for WHERE */
        break;
      case 44:
#line 210 "parse.y"
{ yygotominor.yy135 = nullptr; }
#line 1244 "parse.c"
        break;
      case 45:
#line 211 "parse.y"
{ yygotominor.yy135 = new GroupOpt(&yymsp[0].minor.yy0); }
#line 1249 "parse.c"
        /* No destructor defined for GROUP */
        /* No destructor defined for BY */
        break;
      case 46:
#line 212 "parse.y"
{ yygotominor.yy135 = new GroupOpt(&yymsp[-2].minor.yy0, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1256 "parse.c"
        /* No destructor defined for GROUP */
        /* No destructor defined for BY */
        break;
      case 47:
#line 217 "parse.y"
{ yygotominor.yy189 = nullptr; }
#line 1263 "parse.c"
        break;
      case 48:
#line 218 "parse.y"
{ yygotominor.yy189 = new OrderByOpt(&yymsp[0].minor.yy0, true); }
#line 1268 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        break;
      case 49:
#line 219 "parse.y"
{ yygotominor.yy189 = new OrderByOpt(&yymsp[-1].minor.yy0, true); }
#line 1275 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        /* No destructor defined for ASC */
        break;
      case 50:
#line 220 "parse.y"
{ yygotominor.yy189 = new OrderByOpt(&yymsp[-1].minor.yy0, false); }
#line 1283 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        /* No destructor defined for DESC */
        break;
      case 51:
#line 225 "parse.y"
{ yygotominor.yy125 = nullptr; }
#line 1291 "parse.c"
        break;
      case 52:
#line 226 "parse.y"
{ yygotominor.yy125 = new LimitOpt(&yymsp[0].minor.yy0); }
#line 1296 "parse.c"
        /* No destructor defined for LIMIT */
        break;
      case 53:
#line 227 "parse.y"
{ yygotominor.yy125 = new LimitOpt(&yymsp[-2].minor.yy0, &yymsp[0].minor.yy0); }
#line 1302 "parse.c"
        /* No destructor defined for LIMIT */
        /* No destructor defined for COMMA */
        break;
      case 54:
#line 238 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_LT, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), yymsp[0].minor.yy67, nullptr); }
#line 1309 "parse.c"
        /* No destructor defined for LT */
        break;
      case 55:
#line 239 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_LE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), yymsp[0].minor.yy67, nullptr); }
#line 1315 "parse.c"
        /* No destructor defined for LE */
        break;
      case 56:
#line 240 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_GT, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), yymsp[0].minor.yy67, nullptr); }
#line 1321 "parse.c"
        /* No destructor defined for GT */
        break;
      case 57:
#line 241 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_GE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), yymsp[0].minor.yy67, nullptr); }
#line 1327 "parse.c"
        /* No destructor defined for GE */
        break;
      case 58:
#line 242 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_EQ, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), yymsp[0].minor.yy67, nullptr); }
#line 1333 "parse.c"
        /* No destructor defined for EQ */
        break;
      case 59:
#line 243 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_NE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), yymsp[0].minor.yy67, nullptr); }
#line 1339 "parse.c"
        /* No destructor defined for NE */
        break;
      case 60:
#line 244 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_LIKE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), yymsp[0].minor.yy67, nullptr); }
#line 1345 "parse.c"
        /* No destructor defined for LIKE */
        break;
      case 61:
#line 245 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_ISTRUE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), nullptr, nullptr); }
#line 1351 "parse.c"
        /* No destructor defined for EQ */
        /* No destructor defined for TRUE */
        break;
      case 62:
#line 246 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_ISFALSE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), nullptr, nullptr); }
#line 1358 "parse.c"
        /* No destructor defined for EQ */
        /* No destructor defined for FALSE */
        break;
      case 63:
#line 247 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_ISFALSE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), nullptr, nullptr); }
#line 1365 "parse.c"
        /* No destructor defined for NE */
        /* No destructor defined for TRUE */
        break;
      case 64:
#line 248 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_ISTRUE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), nullptr, nullptr); }
#line 1372 "parse.c"
        /* No destructor defined for NE */
        /* No destructor defined for FALSE */
        break;
      case 65:
#line 251 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_AND, yymsp[-2].minor.yy67, yymsp[0].minor.yy67, nullptr); }
#line 1379 "parse.c"
        /* No destructor defined for AND */
        break;
      case 66:
#line 256 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_INTEGER, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1385 "parse.c"
        break;
      case 67:
#line 257 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_DOUBLE, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1390 "parse.c"
        break;
      case 68:
#line 258 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_INTEGER, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1395 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 69:
#line 259 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_DOUBLE, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1401 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 70:
#line 260 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_UINTEGER, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1407 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 71:
#line 261 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_UDOUBLE, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1413 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 72:
#line 262 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_STRING, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1419 "parse.c"
        break;
      case 73:
#line 263 "parse.y"
{ yygotominor.yy67 = ExprItem::MakeExpr(TK_BLOB, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1424 "parse.c"
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yypParser,yygoto);
  if( yyact < YYNSTATE ){
    yy_shift(yypParser,yyact,yygoto,&yygotominor);
  }else if( yyact == YYNSTATE + YYNRULE + 1 ){
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  pdbParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
  pdbParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  pdbParseARG_FETCH;
#define TOKEN (yyminor.yy0)
#line 36 "parse.y"

  const char* errmsg = TOKEN.str_;
  pdbSetError(pParse, errmsg);

#line 1471 "parse.c"
  pdbParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  pdbParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  pdbParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "pdbParseAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void pdbParse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  pdbParseTOKENTYPE yyminor       /* The value for the token */
  pdbParseARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
    if( yymajor==0 ) return;
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  pdbParseARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,yymajor);
    if( yyact<YYNSTATE ){
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      if( yyendofinput && yypParser->yyidx>=0 ){
        yymajor = 0;
      }else{
        yymajor = YYNOCODE;
      }
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else if( yyact == YY_ERROR_ACTION ){
      int yymx;
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_shift_action(yypParser,YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }else{
      yy_accept(yypParser);
      yymajor = YYNOCODE;
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}
