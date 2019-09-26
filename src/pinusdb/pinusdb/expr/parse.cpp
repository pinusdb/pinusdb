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

#line 42 "parse.c"
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
#define YYNOCODE 98
#define YYACTIONTYPE unsigned short int
#define pdbParseTOKENTYPE Token
typedef union {
  pdbParseTOKENTYPE yy0;
  ExprList* yy34;
  OrderByOpt* yy35;
  ExprItem* yy55;
  RecordList* yy76;
  LimitOpt* yy81;
  GroupOpt* yy107;
  Token yy114;
  ColumnList* yy157;
  ColumnItem* yy178;
  int yy195;
} YYMINORTYPE;
#define YYSTACKDEPTH 100
#define pdbParseARG_SDECL SQLParser *pParse;
#define pdbParseARG_PDECL ,SQLParser *pParse
#define pdbParseARG_FETCH SQLParser *pParse = yypParser->pParse
#define pdbParseARG_STORE yypParser->pParse = pParse
#define YYNSTATE 204
#define YYNRULE 84
#define YYERRORSYMBOL 79
#define YYERRSYMDT yy195
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
 /*     0 */   151,   11,  254,   86,  176,  115,  113,  119,  116,  115,
 /*    10 */   113,  255,  206,    1,  105,   14,  107,  127,   17,   10,
 /*    20 */    22,  121,  124,  102,  117,  193,  109,  106,  172,   38,
 /*    30 */    49,  198,  199,   31,   58,   79,  103,   63,  191,  107,
 /*    40 */    83,   78,   62,   78,   65,   84,  182,  146,  171,   94,
 /*    50 */   150,  145,   65,  245,   99,   68,   69,   70,   71,   72,
 /*    60 */    73,   74,   75,   76,   77,   97,   98,  100,  101,  104,
 /*    70 */    50,  156,  108,   53,  155,  135,  110,   99,  170,  181,
 /*    80 */    23,  148,  183,    8,   91,   32,  114,  181,   97,   98,
 /*    90 */   100,  101,  104,    9,  128,  108,  111,  166,  114,  162,
 /*   100 */   163,  174,  170,  184,  164,   92,  145,  175,  111,  137,
 /*   110 */   131,  133,   95,  129,  139,  141,  112,  118,  113,   39,
 /*   120 */   159,   33,   42,  169,  119,  177,  162,  168,  112,   30,
 /*   130 */   164,  173,  117,   29,  160,   35,  119,   34,  121,  124,
 /*   140 */   142,  144,  180,  178,   36,   37,   28,  207,  179,   27,
 /*   150 */   121,  124,   26,   40,  165,   41,   25,  211,   24,   43,
 /*   160 */   185,  215,   44,   91,  161,   46,  231,   21,   47,   48,
 /*   170 */   158,  213,  167,  186,  157,   52,  212,   51,   19,  154,
 /*   180 */    20,  200,   54,  153,  152,   56,   57,   55,  214,  187,
 /*   190 */    60,  149,   59,  147,   18,  195,   67,   61,  210,  143,
 /*   200 */    64,  188,   16,  216,  189,  140,   15,  205,  190,  138,
 /*   210 */    13,  232,  126,   66,  136,  134,  125,  256,  123,   12,
 /*   220 */   196,  120,  122,  204,  110,  192,   80,    7,  132,  130,
 /*   230 */   194,    6,  197,   81,  203,   82,  201,   96,    5,    4,
 /*   240 */   202,   85,  217,   93,    2,   88,   45,  230,   87,   90,
 /*   250 */     3,  183,  183,  289,  183,  183,  183,  183,  183,   89,
};
static YYCODETYPE yy_lookahead[] = {
 /*     0 */     5,   19,   23,    8,   91,   92,   93,   60,   91,   92,
 /*    10 */    93,   23,    0,   18,   60,   33,   22,   30,   36,   24,
 /*    20 */    25,   74,   75,   60,   37,   37,   32,   73,   32,   34,
 /*    30 */    35,   57,   58,   23,   39,   40,   73,   30,   59,   22,
 /*    40 */    82,   83,   82,   83,   37,   30,   51,   28,   52,   32,
 /*    50 */    94,   95,   37,   23,   60,   41,   42,   43,   44,   45,
 /*    60 */    46,   47,   48,   49,   50,   71,   72,   73,   74,   75,
 /*    70 */    33,   30,   78,   36,   85,   28,   29,   60,   37,   90,
 /*    80 */    26,   62,   85,   22,   54,   31,   22,   90,   71,   72,
 /*    90 */    73,   74,   75,   32,   30,   78,   32,   30,   22,   95,
 /*   100 */    96,   32,   37,   38,   37,   94,   95,   29,   32,   62,
 /*   110 */    63,   64,   65,   66,   67,   68,   52,   92,   93,   33,
 /*   120 */    23,   27,   36,   30,   60,   30,   95,   96,   52,   30,
 /*   130 */    37,   53,   37,   22,   37,   28,   60,   81,   74,   75,
 /*   140 */    69,   70,   90,   53,   32,   23,   29,    0,   32,   26,
 /*   150 */    74,   75,   28,   32,   95,   23,   81,    0,   27,   22,
 /*   160 */    32,    0,   37,   54,   29,   38,    0,   23,   32,   23,
 /*   170 */    86,    0,   29,   84,    7,   23,    0,   32,   38,   29,
 /*   180 */    32,   55,   22,   32,    6,   32,   23,   38,    0,   87,
 /*   190 */    32,   95,   33,   95,   22,   56,   32,   29,    0,   70,
 /*   200 */    23,   88,   23,    0,   89,   95,   32,    0,   23,   95,
 /*   210 */    23,    0,   32,   83,   95,   95,   60,   23,   32,   81,
 /*   220 */    21,   32,   60,    0,   29,   60,   33,   23,   95,   95,
 /*   230 */    60,   22,   32,   32,   93,   29,   21,   95,   21,   20,
 /*   240 */    32,   23,    0,   61,   19,   32,   22,    0,   38,   23,
 /*   250 */    81,   97,   97,   80,   97,   97,   97,   97,   97,   84,
};
#define YY_SHIFT_USE_DFLT (-54)
static short yy_shift_ofst[] = {
 /*     0 */    -5,  225,   61,  219,  217,  209,  204,  223,  -54,  -54,
 /*    10 */   -18,   61,  187,  207,  174,  179,  198,  172,  140,  148,
 /*    20 */   144,  161,   54,  131,   61,  124,  123,  117,  111,   99,
 /*    30 */    10,   12,   94,   61,  107,  112,  122,  147,   86,  121,
 /*    40 */   132,  157,  137,  125,  224,  127,  136,  146,  171,   37,
 /*    50 */   145,  152,  176,  160,  149,  153,  163,  188,  159,  158,
 /*    60 */   168,  164,    7,  177,  203,  164,  -54,   14,  -54,  -54,
 /*    70 */   -54,  -54,  -54,  -54,  -54,  -54,  -54,  -54,  -54,  193,
 /*    80 */   201,  206,  164,   15,  218,  242,  210,  213,   30,  226,
 /*    90 */   247,   17,  182,   17,   47,   -6,  -54,  -54,  -54,  -54,
 /*   100 */   -54,  -37,  -54,  -54,  -46,  -54,  -54,  -54,  -54,  195,
 /*   110 */    64,  -54,  -54,  -54,  -54,  -54,  -13,   76,  -54,  189,
 /*   120 */   -54,  162,  186,  -54,  156,  180,  -54,  -54,  -54,   -6,
 /*   130 */   -54,   -6,  -54,   -6,  -54,   -6,  -54,   -6,  -54,   -6,
 /*   140 */   -54,   71,  129,  -54,  -54,   19,   -6,  -54,   -6,  -54,
 /*   150 */   -54,  178,  151,  150,   -4,   41,  167,  143,   97,  166,
 /*   160 */   135,   -6,  -54,   67,   -6,  -54,  -54,   -6,   93,  -54,
 /*   170 */    -4,  -54,   78,   69,  -54,   76,   95,   90,  116,  -54,
 /*   180 */   -54,  -54,   -4,   65,  128,  109,  126,  139,  -21,  185,
 /*   190 */   211,  165,  -12,  170,  194,  199,  200,  -26,  -54,  -54,
 /*   200 */   215,  208,  -53,  -54,
};
#define YY_REDUCE_USE_DFLT (-88)
static short yy_reduce_ofst[] = {
 /*     0 */   173,  -88,  169,  -88,  -88,  -88,  -88,  -88,  -88,  -88,
 /*    10 */   -88,  138,  -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,
 /*    20 */   -88,  -88,  -88,  -88,   75,  -88,  -88,  -88,  -88,  -88,
 /*    30 */   -88,  -88,  -88,   56,  -88,  -88,  -88,  -88,  -88,  -88,
 /*    40 */   -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,
 /*    50 */   -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,
 /*    60 */   -88,  -40,  -88,  -88,  -88,  130,  -88,  -88,  -88,  -88,
 /*    70 */   -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,
 /*    80 */   -88,  -88,  -42,  -88,  -88,  -88,  -88,  -88,  175,  -88,
 /*    90 */   -88,   11,  -88,  -44,  -88,  142,  -88,  -88,  -88,  -88,
 /*   100 */   -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,
 /*   110 */   -83,  -88,  -88,  -88,  -88,  -88,  -88,   25,  -88,  -88,
 /*   120 */   -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,  134,
 /*   130 */   -88,  133,  -88,  120,  -88,  119,  -88,  114,  -88,  110,
 /*   140 */   -88,  -88,  -88,  -88,  -88,  -88,   98,  -88,   96,  -88,
 /*   150 */   -88,  -88,  -88,  -88,  -11,  -88,  -88,   84,  -88,  -88,
 /*   160 */   -88,    4,  -88,  -88,   59,  -88,  -88,   31,  -88,  -88,
 /*   170 */    52,  -88,  -88,  -88,  -88,  -87,  -88,  -88,  -88,  -88,
 /*   180 */   -88,  -88,   -3,  -88,  -88,   89,  102,  113,  115,  -88,
 /*   190 */   -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,  -88,
 /*   200 */   -88,  -88,  141,  -88,
};
static YYACTIONTYPE yy_default[] = {
 /*     0 */   288,  288,  288,  288,  288,  288,  288,  288,  208,  209,
 /*    10 */   288,  288,  288,  288,  288,  288,  288,  288,  288,  288,
 /*    20 */   288,  288,  288,  288,  288,  288,  288,  288,  288,  288,
 /*    30 */   288,  288,  288,  288,  288,  288,  288,  288,  288,  288,
 /*    40 */   288,  288,  288,  288,  288,  288,  288,  288,  288,  288,
 /*    50 */   288,  288,  288,  288,  288,  288,  288,  288,  288,  288,
 /*    60 */   288,  288,  288,  288,  288,  288,  218,  288,  220,  221,
 /*    70 */   222,  223,  224,  225,  226,  227,  228,  229,  219,  288,
 /*    80 */   288,  288,  288,  288,  288,  288,  288,  288,  288,  288,
 /*    90 */   288,  288,  246,  288,  288,  288,  257,  269,  270,  271,
 /*   100 */   272,  288,  273,  274,  288,  275,  276,  277,  278,  288,
 /*   110 */   288,  239,  240,  241,  242,  243,  288,  288,  244,  288,
 /*   120 */   281,  288,  288,  282,  288,  288,  283,  279,  280,  288,
 /*   130 */   258,  288,  259,  288,  260,  288,  261,  288,  262,  288,
 /*   140 */   263,  288,  288,  264,  265,  288,  288,  266,  288,  267,
 /*   150 */   268,  288,  288,  288,  288,  288,  288,  288,  288,  288,
 /*   160 */   288,  288,  284,  288,  288,  285,  287,  288,  288,  286,
 /*   170 */   288,  233,  234,  288,  235,  288,  288,  288,  288,  236,
 /*   180 */   238,  237,  288,  288,  288,  245,  247,  250,  288,  288,
 /*   190 */   288,  288,  288,  288,  288,  288,  288,  251,  252,  253,
 /*   200 */   288,  288,  248,  249,
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
  "DELETE",        "TOP",           "IN",            "TINYINT",     
  "SMALLINT",      "INT",           "FLOAT",         "ISNULL",      
  "ISNOTNULL",     "TIMEVAL",       "ADD",           "USER",        
  "IDENTIFIED",    "BY",            "STRING",        "SEMI",        
  "DROP",          "SET",           "PASSWORD",      "FOR",         
  "EQ",            "LP",            "RP",            "ROLE",        
  "ID",            "TABLE",         "ATTACH",        "DETACH",      
  "DATAFILE",      "COMMA",         "FROM",          "CREATE",      
  "ALTER",         "BOOL_TYPE",     "BIGINT_TYPE",   "DOUBLE_TYPE", 
  "STRING_TYPE",   "BLOB_TYPE",     "DATETIME_TYPE",  "REAL2_TYPE",  
  "REAL3_TYPE",    "REAL4_TYPE",    "REAL6_TYPE",    "SELECT",      
  "STAR",          "AS",            "WHERE",         "GROUP",       
  "ORDER",         "ASC",           "DESC",          "LIMIT",       
  "INTEGER",       "AND",           "NE",            "GT",          
  "GE",            "LT",            "LE",            "LIKE",        
  "IS",            "NOT",           "NULL",          "TRUE",        
  "FALSE",         "DOUBLE",        "PLUS",          "MINUS",       
  "UINTEGER",      "UDOUBLE",       "BLOB",          "error",       
  "cmd",           "username",      "cre_columnlist",  "cre_column",  
  "where_opt",     "target_list",   "record_list",   "groupby_opt", 
  "orderby_opt",   "limit_opt",     "target_item",   "arg_list",    
  "arg_item",      "timeval",       "condi_expr",    "userval",     
  "userval_list",
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
 /*  13 */ "cmd ::= ALTER TABLE ID LP cre_columnlist RP SEMI",
 /*  14 */ "cre_columnlist ::= cre_columnlist COMMA cre_column",
 /*  15 */ "cre_columnlist ::= cre_column",
 /*  16 */ "cre_column ::= ID BOOL_TYPE",
 /*  17 */ "cre_column ::= ID BIGINT_TYPE",
 /*  18 */ "cre_column ::= ID DOUBLE_TYPE",
 /*  19 */ "cre_column ::= ID STRING_TYPE",
 /*  20 */ "cre_column ::= ID BLOB_TYPE",
 /*  21 */ "cre_column ::= ID DATETIME_TYPE",
 /*  22 */ "cre_column ::= ID REAL2_TYPE",
 /*  23 */ "cre_column ::= ID REAL3_TYPE",
 /*  24 */ "cre_column ::= ID REAL4_TYPE",
 /*  25 */ "cre_column ::= ID REAL6_TYPE",
 /*  26 */ "cmd ::= DELETE FROM ID where_opt SEMI",
 /*  27 */ "cmd ::= INSERT INTO ID LP target_list RP VALUES record_list SEMI",
 /*  28 */ "cmd ::= SELECT target_list FROM ID where_opt groupby_opt orderby_opt limit_opt SEMI",
 /*  29 */ "target_item ::= STAR",
 /*  30 */ "target_item ::= ID",
 /*  31 */ "target_item ::= ID AS ID",
 /*  32 */ "target_item ::= ID LP arg_list RP AS ID",
 /*  33 */ "target_list ::= target_item",
 /*  34 */ "target_list ::= target_list COMMA target_item",
 /*  35 */ "arg_item ::= ID",
 /*  36 */ "arg_item ::= STAR",
 /*  37 */ "arg_item ::= timeval",
 /*  38 */ "arg_item ::= STRING",
 /*  39 */ "arg_list ::= arg_item",
 /*  40 */ "arg_list ::= arg_list COMMA arg_item",
 /*  41 */ "where_opt ::=",
 /*  42 */ "where_opt ::= WHERE condi_expr",
 /*  43 */ "groupby_opt ::=",
 /*  44 */ "groupby_opt ::= GROUP BY ID",
 /*  45 */ "groupby_opt ::= GROUP BY ID timeval",
 /*  46 */ "orderby_opt ::=",
 /*  47 */ "orderby_opt ::= ORDER BY ID",
 /*  48 */ "orderby_opt ::= ORDER BY ID ASC",
 /*  49 */ "orderby_opt ::= ORDER BY ID DESC",
 /*  50 */ "limit_opt ::=",
 /*  51 */ "limit_opt ::= LIMIT INTEGER",
 /*  52 */ "limit_opt ::= LIMIT INTEGER COMMA INTEGER",
 /*  53 */ "condi_expr ::= ID LT userval",
 /*  54 */ "condi_expr ::= ID LE userval",
 /*  55 */ "condi_expr ::= ID GT userval",
 /*  56 */ "condi_expr ::= ID GE userval",
 /*  57 */ "condi_expr ::= ID EQ userval",
 /*  58 */ "condi_expr ::= ID NE userval",
 /*  59 */ "condi_expr ::= ID LIKE userval",
 /*  60 */ "condi_expr ::= ID IS NOT NULL",
 /*  61 */ "condi_expr ::= ID IS NULL",
 /*  62 */ "condi_expr ::= userval EQ userval",
 /*  63 */ "condi_expr ::= userval NE userval",
 /*  64 */ "condi_expr ::= condi_expr AND condi_expr",
 /*  65 */ "userval ::= TRUE",
 /*  66 */ "userval ::= FALSE",
 /*  67 */ "userval ::= INTEGER",
 /*  68 */ "userval ::= DOUBLE",
 /*  69 */ "userval ::= PLUS INTEGER",
 /*  70 */ "userval ::= PLUS DOUBLE",
 /*  71 */ "userval ::= MINUS INTEGER",
 /*  72 */ "userval ::= MINUS DOUBLE",
 /*  73 */ "userval ::= STRING",
 /*  74 */ "userval ::= BLOB",
 /*  75 */ "userval ::= ID LP arg_list RP",
 /*  76 */ "userval ::= ID LP RP",
 /*  77 */ "timeval ::= INTEGER ID",
 /*  78 */ "timeval ::= PLUS INTEGER ID",
 /*  79 */ "timeval ::= MINUS INTEGER ID",
 /*  80 */ "userval_list ::= userval",
 /*  81 */ "userval_list ::= userval_list COMMA userval",
 /*  82 */ "record_list ::= LP userval_list RP",
 /*  83 */ "record_list ::= record_list COMMA LP userval_list RP",
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
    case 82:
#line 109 "parse.y"
{ ColumnList::FreeColumnList((yypminor->yy157)); }
#line 555 "parse.c"
      break;
    case 83:
#line 111 "parse.y"
{ ColumnItem::FreeColumnItem((yypminor->yy178)); }
#line 560 "parse.c"
      break;
    case 84:
#line 184 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy55)); }
#line 565 "parse.c"
      break;
    case 85:
#line 152 "parse.y"
{ ExprList::FreeExprList((yypminor->yy34)); }
#line 570 "parse.c"
      break;
    case 86:
#line 267 "parse.y"
{ RecordList::FreeRecordList((yypminor->yy76)); }
#line 575 "parse.c"
      break;
    case 87:
#line 190 "parse.y"
{ delete ((yypminor->yy107)); }
#line 580 "parse.c"
      break;
    case 88:
#line 197 "parse.y"
{ delete ((yypminor->yy35)); }
#line 585 "parse.c"
      break;
    case 89:
#line 205 "parse.y"
{ delete ((yypminor->yy81)); }
#line 590 "parse.c"
      break;
    case 90:
#line 154 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy55)); }
#line 595 "parse.c"
      break;
    case 91:
#line 171 "parse.y"
{ ExprList::FreeExprList((yypminor->yy34)); }
#line 600 "parse.c"
      break;
    case 92:
#line 169 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy55)); }
#line 605 "parse.c"
      break;
    case 93:
#line 252 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy55));}
#line 610 "parse.c"
      break;
    case 94:
#line 218 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy55)); }
#line 615 "parse.c"
      break;
    case 95:
#line 236 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy55)); }
#line 620 "parse.c"
      break;
    case 96:
#line 259 "parse.y"
{ ExprList::FreeExprList((yypminor->yy34)); }
#line 625 "parse.c"
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
  { 80, 7 },
  { 80, 4 },
  { 80, 10 },
  { 80, 7 },
  { 81, 1 },
  { 81, 1 },
  { 80, 4 },
  { 80, 4 },
  { 80, 4 },
  { 80, 8 },
  { 80, 6 },
  { 80, 6 },
  { 80, 7 },
  { 80, 7 },
  { 82, 3 },
  { 82, 1 },
  { 83, 2 },
  { 83, 2 },
  { 83, 2 },
  { 83, 2 },
  { 83, 2 },
  { 83, 2 },
  { 83, 2 },
  { 83, 2 },
  { 83, 2 },
  { 83, 2 },
  { 80, 5 },
  { 80, 9 },
  { 80, 9 },
  { 90, 1 },
  { 90, 1 },
  { 90, 3 },
  { 90, 6 },
  { 85, 1 },
  { 85, 3 },
  { 92, 1 },
  { 92, 1 },
  { 92, 1 },
  { 92, 1 },
  { 91, 1 },
  { 91, 3 },
  { 84, 0 },
  { 84, 2 },
  { 87, 0 },
  { 87, 3 },
  { 87, 4 },
  { 88, 0 },
  { 88, 3 },
  { 88, 4 },
  { 88, 4 },
  { 89, 0 },
  { 89, 2 },
  { 89, 4 },
  { 94, 3 },
  { 94, 3 },
  { 94, 3 },
  { 94, 3 },
  { 94, 3 },
  { 94, 3 },
  { 94, 3 },
  { 94, 4 },
  { 94, 3 },
  { 94, 3 },
  { 94, 3 },
  { 94, 3 },
  { 95, 1 },
  { 95, 1 },
  { 95, 1 },
  { 95, 1 },
  { 95, 2 },
  { 95, 2 },
  { 95, 2 },
  { 95, 2 },
  { 95, 1 },
  { 95, 1 },
  { 95, 4 },
  { 95, 3 },
  { 93, 2 },
  { 93, 3 },
  { 93, 3 },
  { 96, 1 },
  { 96, 3 },
  { 86, 3 },
  { 86, 5 },
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
#line 49 "parse.y"
{
  pdbAddUser(pParse, &yymsp[-4].minor.yy114, &yymsp[-1].minor.yy0);
}
#line 926 "parse.c"
        /* No destructor defined for ADD */
        /* No destructor defined for USER */
        /* No destructor defined for IDENTIFIED */
        /* No destructor defined for BY */
        /* No destructor defined for SEMI */
        break;
      case 1:
#line 53 "parse.y"
{
  pdbDropUser(pParse, &yymsp[-1].minor.yy114);
}
#line 938 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for USER */
        /* No destructor defined for SEMI */
        break;
      case 2:
#line 58 "parse.y"
{
  pdbChangePwd(pParse, &yymsp[-6].minor.yy114, &yymsp[-2].minor.yy0); 
}
#line 948 "parse.c"
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
#line 62 "parse.y"
{
  pdbChangeRole(pParse, &yymsp[-3].minor.yy114, &yymsp[-1].minor.yy0);
}
#line 963 "parse.c"
        /* No destructor defined for SET */
        /* No destructor defined for ROLE */
        /* No destructor defined for FOR */
        /* No destructor defined for EQ */
        /* No destructor defined for SEMI */
        break;
      case 4:
#line 67 "parse.y"
{ yygotominor.yy114 = yymsp[0].minor.yy0; }
#line 973 "parse.c"
        break;
      case 5:
#line 68 "parse.y"
{ yygotominor.yy114 = yymsp[0].minor.yy0; }
#line 978 "parse.c"
        break;
      case 6:
#line 71 "parse.y"
{
  pdbDropTable(pParse, &yymsp[-1].minor.yy0);
}
#line 985 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 7:
#line 75 "parse.y"
{
  pdbAttachTable(pParse, &yymsp[-1].minor.yy0);
}
#line 995 "parse.c"
        /* No destructor defined for ATTACH */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 8:
#line 79 "parse.y"
{
  pdbDetachTable(pParse, &yymsp[-1].minor.yy0);
}
#line 1005 "parse.c"
        /* No destructor defined for DETACH */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 9:
#line 85 "parse.y"
{
  pdbAttachFile(pParse, &yymsp[-1].minor.yy0, &yymsp[-5].minor.yy0, &yymsp[-3].minor.yy0);
}
#line 1015 "parse.c"
        /* No destructor defined for ATTACH */
        /* No destructor defined for DATAFILE */
        /* No destructor defined for COMMA */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 10:
#line 89 "parse.y"
{
  pdbDetachFile(pParse, &yymsp[-1].minor.yy0, &yymsp[-3].minor.yy0);
}
#line 1027 "parse.c"
        /* No destructor defined for DETACH */
        /* No destructor defined for DATAFILE */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 11:
#line 93 "parse.y"
{
  pdbDropFile(pParse, &yymsp[-1].minor.yy0, &yymsp[-3].minor.yy0);
}
#line 1038 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for DATAFILE */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 12:
#line 100 "parse.y"
{
  pdbCreateTable(pParse, &yymsp[-4].minor.yy0, yymsp[-2].minor.yy157);
}
#line 1049 "parse.c"
        /* No destructor defined for CREATE */
        /* No destructor defined for TABLE */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for SEMI */
        break;
      case 13:
#line 104 "parse.y"
{
  pdbAlterTable(pParse, &yymsp[-4].minor.yy0, yymsp[-2].minor.yy157);
}
#line 1061 "parse.c"
        /* No destructor defined for ALTER */
        /* No destructor defined for TABLE */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for SEMI */
        break;
      case 14:
#line 113 "parse.y"
{
  yygotominor.yy157 = ColumnList::AppendColumnItem(yymsp[-2].minor.yy157, yymsp[0].minor.yy178);
}
#line 1073 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 15:
#line 116 "parse.y"
{
  yygotominor.yy157 = ColumnList::AppendColumnItem(nullptr, yymsp[0].minor.yy178);
}
#line 1081 "parse.c"
        break;
      case 16:
#line 120 "parse.y"
{ yygotominor.yy178 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_BOOL); }
#line 1086 "parse.c"
        /* No destructor defined for BOOL_TYPE */
        break;
      case 17:
#line 121 "parse.y"
{ yygotominor.yy178 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_INT64); }
#line 1092 "parse.c"
        /* No destructor defined for BIGINT_TYPE */
        break;
      case 18:
#line 122 "parse.y"
{ yygotominor.yy178 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_DOUBLE); }
#line 1098 "parse.c"
        /* No destructor defined for DOUBLE_TYPE */
        break;
      case 19:
#line 123 "parse.y"
{ yygotominor.yy178 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_STRING); }
#line 1104 "parse.c"
        /* No destructor defined for STRING_TYPE */
        break;
      case 20:
#line 124 "parse.y"
{ yygotominor.yy178 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_BLOB); }
#line 1110 "parse.c"
        /* No destructor defined for BLOB_TYPE */
        break;
      case 21:
#line 125 "parse.y"
{ yygotominor.yy178 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_DATETIME); }
#line 1116 "parse.c"
        /* No destructor defined for DATETIME_TYPE */
        break;
      case 22:
#line 126 "parse.y"
{ yygotominor.yy178 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL2); }
#line 1122 "parse.c"
        /* No destructor defined for REAL2_TYPE */
        break;
      case 23:
#line 127 "parse.y"
{ yygotominor.yy178 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL3); }
#line 1128 "parse.c"
        /* No destructor defined for REAL3_TYPE */
        break;
      case 24:
#line 128 "parse.y"
{ yygotominor.yy178 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL4); }
#line 1134 "parse.c"
        /* No destructor defined for REAL4_TYPE */
        break;
      case 25:
#line 129 "parse.y"
{ yygotominor.yy178 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL6); }
#line 1140 "parse.c"
        /* No destructor defined for REAL6_TYPE */
        break;
      case 26:
#line 135 "parse.y"
{
  pdbDelete(pParse, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy55);
}
#line 1148 "parse.c"
        /* No destructor defined for DELETE */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 27:
#line 141 "parse.y"
{
  pdbInsert(pParse, &yymsp[-6].minor.yy0, yymsp[-4].minor.yy34, yymsp[-1].minor.yy76);
}
#line 1158 "parse.c"
        /* No destructor defined for INSERT */
        /* No destructor defined for INTO */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for VALUES */
        /* No destructor defined for SEMI */
        break;
      case 28:
#line 147 "parse.y"
{
  pdbSelect(pParse, yymsp[-7].minor.yy34, &yymsp[-5].minor.yy0, yymsp[-4].minor.yy55, yymsp[-3].minor.yy107, yymsp[-2].minor.yy35, yymsp[-1].minor.yy81);
}
#line 1171 "parse.c"
        /* No destructor defined for SELECT */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 29:
#line 156 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_STAR, &yymsp[0].minor.yy0); }
#line 1179 "parse.c"
        break;
      case 30:
#line 157 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_ID, &yymsp[0].minor.yy0); }
#line 1184 "parse.c"
        break;
      case 31:
#line 158 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_ID, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy0); }
#line 1189 "parse.c"
        /* No destructor defined for AS */
        break;
      case 32:
#line 159 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeFunction(TK_FUNCTION, &yymsp[-5].minor.yy0, yymsp[-3].minor.yy34, &yymsp[0].minor.yy0); }
#line 1195 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for AS */
        break;
      case 33:
#line 161 "parse.y"
{
  yygotominor.yy34 = ExprList::AppendExprItem(nullptr, yymsp[0].minor.yy55);
}
#line 1205 "parse.c"
        break;
      case 34:
#line 164 "parse.y"
{
  yygotominor.yy34 = ExprList::AppendExprItem(yymsp[-2].minor.yy34, yymsp[0].minor.yy55);
}
#line 1212 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 35:
#line 173 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_ID, &yymsp[0].minor.yy0); }
#line 1218 "parse.c"
        break;
      case 36:
#line 174 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_STAR, &yymsp[0].minor.yy0); }
#line 1223 "parse.c"
        break;
      case 37:
#line 175 "parse.y"
{ yygotominor.yy55 = yymsp[0].minor.yy55; }
#line 1228 "parse.c"
        break;
      case 38:
#line 176 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_STRING, &yymsp[0].minor.yy0); }
#line 1233 "parse.c"
        break;
      case 39:
#line 179 "parse.y"
{ yygotominor.yy34 = ExprList::AppendExprItem(nullptr, yymsp[0].minor.yy55); }
#line 1238 "parse.c"
        break;
      case 40:
#line 181 "parse.y"
{ yygotominor.yy34 = ExprList::AppendExprItem(yymsp[-2].minor.yy34, yymsp[0].minor.yy55); }
#line 1243 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 41:
#line 186 "parse.y"
{ yygotominor.yy55 = nullptr; }
#line 1249 "parse.c"
        break;
      case 42:
#line 187 "parse.y"
{ yygotominor.yy55 = yymsp[0].minor.yy55; }
#line 1254 "parse.c"
        /* No destructor defined for WHERE */
        break;
      case 43:
#line 192 "parse.y"
{ yygotominor.yy107 = nullptr; }
#line 1260 "parse.c"
        break;
      case 44:
#line 193 "parse.y"
{ yygotominor.yy107 = new GroupOpt(&yymsp[0].minor.yy0); }
#line 1265 "parse.c"
        /* No destructor defined for GROUP */
        /* No destructor defined for BY */
        break;
      case 45:
#line 194 "parse.y"
{ yygotominor.yy107 = new GroupOpt(&yymsp[-1].minor.yy0, yymsp[0].minor.yy55); }
#line 1272 "parse.c"
        /* No destructor defined for GROUP */
        /* No destructor defined for BY */
        break;
      case 46:
#line 199 "parse.y"
{ yygotominor.yy35 = nullptr; }
#line 1279 "parse.c"
        break;
      case 47:
#line 200 "parse.y"
{ yygotominor.yy35 = new OrderByOpt(&yymsp[0].minor.yy0, true); }
#line 1284 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        break;
      case 48:
#line 201 "parse.y"
{ yygotominor.yy35 = new OrderByOpt(&yymsp[-1].minor.yy0, true); }
#line 1291 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        /* No destructor defined for ASC */
        break;
      case 49:
#line 202 "parse.y"
{ yygotominor.yy35 = new OrderByOpt(&yymsp[-1].minor.yy0, false); }
#line 1299 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        /* No destructor defined for DESC */
        break;
      case 50:
#line 207 "parse.y"
{ yygotominor.yy81 = nullptr; }
#line 1307 "parse.c"
        break;
      case 51:
#line 208 "parse.y"
{ yygotominor.yy81 = new LimitOpt(&yymsp[0].minor.yy0); }
#line 1312 "parse.c"
        /* No destructor defined for LIMIT */
        break;
      case 52:
#line 209 "parse.y"
{ yygotominor.yy81 = new LimitOpt(&yymsp[-2].minor.yy0, &yymsp[0].minor.yy0); }
#line 1318 "parse.c"
        /* No destructor defined for LIMIT */
        /* No destructor defined for COMMA */
        break;
      case 53:
#line 220 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeCondition(TK_LT, &yymsp[-2].minor.yy0, yymsp[0].minor.yy55); }
#line 1325 "parse.c"
        /* No destructor defined for LT */
        break;
      case 54:
#line 221 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeCondition(TK_LE, &yymsp[-2].minor.yy0, yymsp[0].minor.yy55); }
#line 1331 "parse.c"
        /* No destructor defined for LE */
        break;
      case 55:
#line 222 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeCondition(TK_GT, &yymsp[-2].minor.yy0, yymsp[0].minor.yy55); }
#line 1337 "parse.c"
        /* No destructor defined for GT */
        break;
      case 56:
#line 223 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeCondition(TK_GE, &yymsp[-2].minor.yy0, yymsp[0].minor.yy55); }
#line 1343 "parse.c"
        /* No destructor defined for GE */
        break;
      case 57:
#line 224 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeCondition(TK_EQ, &yymsp[-2].minor.yy0, yymsp[0].minor.yy55); }
#line 1349 "parse.c"
        /* No destructor defined for EQ */
        break;
      case 58:
#line 225 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeCondition(TK_NE, &yymsp[-2].minor.yy0, yymsp[0].minor.yy55); }
#line 1355 "parse.c"
        /* No destructor defined for NE */
        break;
      case 59:
#line 226 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeCondition(TK_LIKE, &yymsp[-2].minor.yy0, yymsp[0].minor.yy55); }
#line 1361 "parse.c"
        /* No destructor defined for LIKE */
        break;
      case 60:
#line 227 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeCondition(TK_ISNOTNULL, &yymsp[-3].minor.yy0, nullptr); }
#line 1367 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NOT */
        /* No destructor defined for NULL */
        break;
      case 61:
#line 228 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeCondition(TK_ISNULL, &yymsp[-2].minor.yy0, nullptr); }
#line 1375 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NULL */
        break;
      case 62:
#line 229 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeCondition(TK_EQ, yymsp[-2].minor.yy55, yymsp[0].minor.yy55); }
#line 1382 "parse.c"
        /* No destructor defined for EQ */
        break;
      case 63:
#line 230 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeCondition(TK_NE, yymsp[-2].minor.yy55, yymsp[0].minor.yy55); }
#line 1388 "parse.c"
        /* No destructor defined for NE */
        break;
      case 64:
#line 233 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeCondition(TK_AND, yymsp[-2].minor.yy55, yymsp[0].minor.yy55); }
#line 1394 "parse.c"
        /* No destructor defined for AND */
        break;
      case 65:
#line 238 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_TRUE, &yymsp[0].minor.yy0); }
#line 1400 "parse.c"
        break;
      case 66:
#line 239 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_FALSE, &yymsp[0].minor.yy0); }
#line 1405 "parse.c"
        break;
      case 67:
#line 240 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_INTEGER, &yymsp[0].minor.yy0); }
#line 1410 "parse.c"
        break;
      case 68:
#line 241 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_DOUBLE, &yymsp[0].minor.yy0); }
#line 1415 "parse.c"
        break;
      case 69:
#line 242 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_INTEGER, &yymsp[0].minor.yy0); }
#line 1420 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 70:
#line 243 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_DOUBLE, &yymsp[0].minor.yy0); }
#line 1426 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 71:
#line 244 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_UINTEGER, &yymsp[0].minor.yy0); }
#line 1432 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 72:
#line 245 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_UDOUBLE, &yymsp[0].minor.yy0); }
#line 1438 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 73:
#line 246 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_STRING, &yymsp[0].minor.yy0); }
#line 1444 "parse.c"
        break;
      case 74:
#line 247 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeValue(TK_BLOB, &yymsp[0].minor.yy0); }
#line 1449 "parse.c"
        break;
      case 75:
#line 248 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeFunction(TK_FUNCTION, &yymsp[-3].minor.yy0, yymsp[-1].minor.yy34, nullptr); }
#line 1454 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 76:
#line 249 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeFunction(TK_FUNCTION, &yymsp[-2].minor.yy0, nullptr, nullptr); }
#line 1461 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 77:
#line 254 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeTimeVal(true, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1468 "parse.c"
        break;
      case 78:
#line 255 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeTimeVal(true, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1473 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 79:
#line 256 "parse.y"
{ yygotominor.yy55 = ExprItem::MakeTimeVal(false, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1479 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 80:
#line 262 "parse.y"
{ yygotominor.yy34 = ExprList::AppendExprItem(nullptr, yymsp[0].minor.yy55); }
#line 1485 "parse.c"
        break;
      case 81:
#line 264 "parse.y"
{ yygotominor.yy34 = ExprList::AppendExprItem(yymsp[-2].minor.yy34, yymsp[0].minor.yy55); }
#line 1490 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 82:
#line 270 "parse.y"
{ yygotominor.yy76 = RecordList::AppendRecordList(nullptr, yymsp[-1].minor.yy34); }
#line 1496 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 83:
#line 272 "parse.y"
{ yygotominor.yy76 = RecordList::AppendRecordList(yymsp[-4].minor.yy76, yymsp[-1].minor.yy34); }
#line 1503 "parse.c"
        /* No destructor defined for COMMA */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
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
#line 39 "parse.y"

  const char* errmsg = TOKEN.str_;
  pdbSetError(pParse, errmsg);

#line 1553 "parse.c"
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
