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


#line 40 "parse.c"
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
#define YYNOCODE 103
#define YYACTIONTYPE unsigned short int
#define pdbParseTOKENTYPE Token
typedef union {
  pdbParseTOKENTYPE yy0;
  OrderByOpt* yy9;
  ExprItem* yy49;
  Token yy66;
  GroupOpt* yy99;
  LimitOpt* yy101;
  ColumnItem* yy116;
  ExprList* yy130;
  ColumnList* yy197;
  int yy205;
} YYMINORTYPE;
#define YYSTACKDEPTH 100
#define pdbParseARG_SDECL SQLParser *pParse;
#define pdbParseARG_PDECL ,SQLParser *pParse
#define pdbParseARG_FETCH SQLParser *pParse = yypParser->pParse
#define pdbParseARG_STORE yypParser->pParse = pParse
#define YYNSTATE 196
#define YYNRULE 77
#define YYERRORSYMBOL 87
#define YYERRSYMDT yy205
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
 /*    10 */    86,  154,  102,  114,  115,  130,  112,  151,  127,  152,
 /*    20 */    18,  165,  248,    1,  123,  125,  195,  103,  104,   10,
 /*    30 */    22,  153,  158,  163,  171,  176,  181,  186,  191,   38,
 /*    40 */    49,  168,  104,    8,   58,   79,  194,   84,   19,  118,
 /*    50 */   119,   11,  239,    9,   65,   99,  129,  116,  108,  110,
 /*    60 */    94,  106,  120,  122,  104,   14,  137,  249,   17,   96,
 /*    70 */   100,   83,   78,  144,  145,   63,   50,   62,   78,   53,
 /*    80 */   193,  139,   65,   96,   97,   98,  101,   23,   39,  105,
 /*    90 */    90,   42,   32,  192,   20,   21,  190,  207,   97,   98,
 /*   100 */   101,  156,  202,  105,  189,   96,   24,  188,   25,  187,
 /*   110 */    26,  185,   27,   28,   29,  184,   31,  198,   30,  183,
 /*   120 */    97,   98,  101,  182,  180,  105,   33,  179,  178,   16,
 /*   130 */    35,   37,   34,   36,  175,  199,  177,   41,  203,  174,
 /*   140 */    43,  172,  173,   40,   45,  170,  169,  167,  274,  166,
 /*   150 */    44,   48,  205,   46,  164,   47,  162,   52,   15,   51,
 /*   160 */   161,  160,  204,  157,  159,   54,  155,   57,   56,    2,
 /*   170 */    55,  206,  150,  147,  197,   59,   61,   60,  148,  142,
 /*   180 */   149,   67,   64,  143,  250,  208,   66,  140,  223,    3,
 /*   190 */   136,    4,  128,    5,  196,    6,  138,  209,  135,    7,
 /*   200 */   222,  141,  181,   80,  181,   13,  181,  181,   85,   82,
 /*   210 */   146,   81,  181,   93,  134,   90,   89,  181,  181,  181,
 /*   220 */   181,  181,  181,  181,  181,  127,  181,  181,  181,  181,
 /*   230 */   181,  133,  181,  181,  124,  181,  181,  181,  181,  181,
 /*   240 */   181,  181,  181,  132,  181,  131,  181,  181,  181,  181,
 /*   250 */   181,  181,   92,  181,  181,  181,  181,  181,  181,  181,
 /*   260 */   121,  117,  113,  181,  111,  181,  109,  107,  181,   12,
 /*   270 */   181,  181,  181,   95,  181,  126,  181,  181,   87,  181,
 /*   280 */    88,  181,  181,   91,
};
static YYCODETYPE yy_lookahead[] = {
 /*     0 */    44,   45,   46,   47,   48,   49,   50,   51,   52,   53,
 /*    10 */     8,   35,   66,   10,   11,   94,   31,   40,   41,   98,
 /*    20 */    25,   35,   26,   21,   79,   80,   99,   81,   25,   27,
 /*    30 */    28,   55,   56,   57,   58,   59,   60,   61,   62,   37,
 /*    40 */    38,   55,   25,   25,   42,   43,   33,   33,   41,   10,
 /*    50 */    11,   22,   26,   35,   40,   66,   54,   72,   73,   74,
 /*    60 */    75,   76,   77,   78,   25,   36,   70,   26,   39,   66,
 /*    70 */    81,   90,   91,   68,   69,   33,   36,   90,   91,   39,
 /*    80 */    35,   40,   40,   66,   81,   82,   83,   29,   36,   86,
 /*    90 */    64,   39,   34,   32,   35,   26,   99,    0,   81,   82,
 /*   100 */    83,   63,    0,   86,   33,   66,   30,   35,   89,   32,
 /*   110 */    31,   99,   29,   32,   25,   33,   26,    0,   33,   35,
 /*   120 */    81,   82,   83,   32,   99,   86,   30,   33,   35,   26,
 /*   130 */    31,   26,   89,   35,   99,    0,   32,   26,    0,   33,
 /*   140 */    25,   32,   35,   35,   25,   99,   33,   99,   88,   33,
 /*   150 */    40,   26,    0,   41,   32,   35,   99,   26,   35,   35,
 /*   160 */    33,   35,    0,   35,   32,   25,   99,   26,   35,   22,
 /*   170 */    41,    0,   35,   24,    0,   36,   32,   35,   35,   24,
 /*   180 */    66,   35,   26,   35,   26,    0,   91,   66,    0,   89,
 /*   190 */    26,   23,   35,   24,    0,   25,   66,    0,   97,   26,
 /*   200 */     0,   67,  102,   36,  102,   26,  102,  102,   26,   32,
 /*   210 */    65,   35,  102,   35,   96,   64,   26,  102,  102,  102,
 /*   220 */   102,  102,  102,  102,  102,   41,  102,  102,  102,  102,
 /*   230 */   102,   95,  102,  102,   80,  102,  102,  102,  102,  102,
 /*   240 */   102,  102,  102,   93,  102,   92,  102,  102,  102,  102,
 /*   250 */   102,  102,   71,  102,  102,  102,  102,  102,  102,  102,
 /*   260 */   101,  101,  101,  102,  101,  102,  101,  101,  102,   89,
 /*   270 */   102,  102,  102,  101,  102,  100,  102,  102,   92,  102,
 /*   280 */    93,  102,  102,  100,
};
#define YY_SHIFT_USE_DFLT (-56)
static short yy_shift_ofst[] = {
 /*     0 */     2,  147,   18,  168,  169,  170,  173,  194,  -56,  -56,
 /*    10 */    29,   18,  179,  174,  123,  103,  102,   -5,    7,   59,
 /*    20 */    69,   97,   58,   76,   18,   79,   83,   81,   89,   85,
 /*    30 */    90,  117,   96,   18,   99,   98,  105,  135,   52,  108,
 /*    40 */   111,  138,  115,  110,  119,  112,  120,  125,  152,   40,
 /*    50 */   124,  131,  162,  140,  129,  133,  141,  171,  139,  142,
 /*    60 */   144,  146,   42,  156,  185,  146,  -56,  -44,  -56,  -56,
 /*    70 */   -56,  -56,  -56,  -56,  -56,  -56,  -56,  -56,  -56,  167,
 /*    80 */   176,  177,  146,   14,  182,  197,  184,   26,  190,  200,
 /*    90 */   178,  181,  178,  -15,   17,  -56,  -56,  -56,  -11,  -56,
 /*   100 */   -56,  -54,  -56,  -56,  -56,  -56,   17,  -56,   17,  -56,
 /*   110 */    17,  -56,    3,  -56,  -56,  -56,   39,  -56,  -56,  -56,
 /*   120 */    17,  -56,  -55,  154,  -56,  -56,  -56,  157,  -56,  -56,
 /*   130 */   -23,  151,  145,  134,   -4,  164,  188,  130,   41,  121,
 /*   140 */   158,  155,  148,    5,  -56,  -56,  149,  143,  114,  137,
 /*   150 */   -56,  -56,  -24,  -56,   38,  -56,  128,  -56,  132,  126,
 /*   160 */   127,   38,  -56,  122,  -14,  116,   38,  -56,  113,   38,
 /*   170 */   -56,  109,  107,  106,   38,  -56,  104,   93,   94,   38,
 /*   180 */   -56,   91,   84,   82,   38,  -56,   77,   72,   71,   38,
 /*   190 */   -56,   61,   45,   13,   38,  -56,
};
#define YY_REDUCE_USE_DFLT (-80)
static short yy_reduce_ofst[] = {
 /*     0 */    60,  -80,  100,  -80,  -80,  -80,  -80,  -80,  -80,  -80,
 /*    10 */   -80,  180,  -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,
 /*    20 */   -80,  -80,  -80,  -80,   19,  -80,  -80,  -80,  -80,  -80,
 /*    30 */   -80,  -80,  -80,   43,  -80,  -80,  -80,  -80,  -80,  -80,
 /*    40 */   -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,
 /*    50 */   -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,
 /*    60 */   -80,  -13,  -80,  -80,  -80,   95,  -80,  -80,  -80,  -80,
 /*    70 */   -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,
 /*    80 */   -80,  -80,  -19,  -80,  -80,  -80,  186,  187,  -80,  -80,
 /*    90 */   183,  -80,  175,  -80,  172,  -80,  -80,  -80,  -80,  -80,
 /*   100 */   -80,  -80,  -80,  -80,  -80,  -80,  166,  -80,  165,  -80,
 /*   110 */   163,  -80,  161,  -80,  -80,  -80,  160,  -80,  -80,  -80,
 /*   120 */   159,  -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,  -79,
 /*   130 */   153,  150,  136,  118,  101,  -80,  -80,  -80,  -80,  -80,
 /*   140 */   -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,
 /*   150 */   -80,  -80,  -80,  -80,   67,  -80,  -80,  -80,  -80,  -80,
 /*   160 */   -80,   57,  -80,  -80,  -80,  -80,   48,  -80,  -80,   46,
 /*   170 */   -80,  -80,  -80,  -80,   35,  -80,  -80,  -80,  -80,   25,
 /*   180 */   -80,  -80,  -80,  -80,   12,  -80,  -80,  -80,  -80,   -3,
 /*   190 */   -80,  -80,  -80,  -80,  -73,  -80,
};
static YYACTIONTYPE yy_default[] = {
 /*     0 */   273,  273,  273,  273,  273,  273,  273,  273,  200,  201,
 /*    10 */   273,  273,  273,  273,  273,  273,  273,  273,  273,  273,
 /*    20 */   273,  273,  273,  273,  273,  273,  273,  273,  273,  273,
 /*    30 */   273,  273,  273,  273,  273,  273,  273,  273,  273,  273,
 /*    40 */   273,  273,  273,  273,  273,  273,  273,  273,  273,  273,
 /*    50 */   273,  273,  273,  273,  273,  273,  273,  273,  273,  273,
 /*    60 */   273,  273,  273,  273,  273,  273,  210,  273,  212,  213,
 /*    70 */   214,  215,  216,  217,  218,  219,  220,  221,  211,  273,
 /*    80 */   273,  273,  273,  273,  273,  273,  273,  273,  273,  273,
 /*    90 */   273,  240,  273,  273,  273,  251,  265,  266,  273,  267,
 /*   100 */   268,  273,  269,  270,  271,  272,  273,  252,  273,  253,
 /*   110 */   273,  254,  273,  255,  258,  259,  273,  256,  260,  261,
 /*   120 */   273,  257,  273,  273,  262,  263,  264,  273,  238,  225,
 /*   130 */   273,  239,  241,  244,  273,  273,  273,  273,  273,  273,
 /*   140 */   273,  273,  273,  245,  246,  247,  273,  273,  242,  273,
 /*   150 */   243,  224,  273,  226,  236,  227,  273,  237,  273,  273,
 /*   160 */   273,  236,  228,  273,  273,  273,  236,  229,  273,  236,
 /*   170 */   230,  273,  273,  273,  236,  231,  273,  273,  273,  236,
 /*   180 */   232,  273,  273,  273,  236,  233,  273,  273,  273,  236,
 /*   190 */   234,  273,  273,  273,  236,  235,
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
  "DELETE",        "TOP",           "TRUE",          "FALSE",       
  "IN",            "TINYINT",       "SMALLINT",      "INT",         
  "ISTRUE",        "ISFALSE",       "FLOAT",         "ISNULL",      
  "ISNOTNULL",     "ADD",           "USER",          "IDENTIFIED",  
  "BY",            "STRING",        "SEMI",          "DROP",        
  "SET",           "PASSWORD",      "FOR",           "EQ",          
  "LP",            "RP",            "ROLE",          "ID",          
  "TABLE",         "ATTACH",        "DETACH",        "DATAFILE",    
  "COMMA",         "FROM",          "CREATE",        "ALTER",       
  "BOOL_TYPE",     "BIGINT_TYPE",   "DOUBLE_TYPE",   "STRING_TYPE", 
  "BLOB_TYPE",     "DATETIME_TYPE",  "REAL2_TYPE",    "REAL3_TYPE",  
  "REAL4_TYPE",    "REAL6_TYPE",    "SELECT",        "STAR",        
  "AVG_FUNC",      "COUNT_FUNC",    "LAST_FUNC",     "MAX_FUNC",    
  "MIN_FUNC",      "SUM_FUNC",      "FIRST_FUNC",    "AS",          
  "WHERE",         "GROUP",         "INTEGER",       "ORDER",       
  "ASC",           "DESC",          "LIMIT",         "AND",         
  "NE",            "GT",            "GE",            "LT",          
  "LE",            "LIKE",          "IS",            "NOT",         
  "NULL",          "DOUBLE",        "PLUS",          "MINUS",       
  "UINTEGER",      "UDOUBLE",       "BLOB",          "error",       
  "cmd",           "username",      "cre_columnlist",  "cre_column",  
  "from",          "where_opt",     "selcollist",    "groupby_opt", 
  "orderby_opt",   "limit_opt",     "sclp",          "as",          
  "condi_expr",    "userval",     
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
 /*  26 */ "cmd ::= DELETE from where_opt SEMI",
 /*  27 */ "cmd ::= SELECT selcollist from where_opt groupby_opt orderby_opt limit_opt SEMI",
 /*  28 */ "sclp ::= selcollist COMMA",
 /*  29 */ "sclp ::=",
 /*  30 */ "selcollist ::= sclp STAR",
 /*  31 */ "selcollist ::= sclp ID as",
 /*  32 */ "selcollist ::= sclp AVG_FUNC LP ID RP as",
 /*  33 */ "selcollist ::= sclp COUNT_FUNC LP ID RP as",
 /*  34 */ "selcollist ::= sclp COUNT_FUNC LP STAR RP as",
 /*  35 */ "selcollist ::= sclp LAST_FUNC LP ID RP as",
 /*  36 */ "selcollist ::= sclp MAX_FUNC LP ID RP as",
 /*  37 */ "selcollist ::= sclp MIN_FUNC LP ID RP as",
 /*  38 */ "selcollist ::= sclp SUM_FUNC LP ID RP as",
 /*  39 */ "selcollist ::= sclp FIRST_FUNC LP ID RP as",
 /*  40 */ "as ::=",
 /*  41 */ "as ::= AS ID",
 /*  42 */ "from ::= FROM ID",
 /*  43 */ "where_opt ::=",
 /*  44 */ "where_opt ::= WHERE condi_expr",
 /*  45 */ "groupby_opt ::=",
 /*  46 */ "groupby_opt ::= GROUP BY ID",
 /*  47 */ "groupby_opt ::= GROUP BY ID INTEGER ID",
 /*  48 */ "orderby_opt ::=",
 /*  49 */ "orderby_opt ::= ORDER BY ID",
 /*  50 */ "orderby_opt ::= ORDER BY ID ASC",
 /*  51 */ "orderby_opt ::= ORDER BY ID DESC",
 /*  52 */ "limit_opt ::=",
 /*  53 */ "limit_opt ::= LIMIT INTEGER",
 /*  54 */ "limit_opt ::= LIMIT INTEGER COMMA INTEGER",
 /*  55 */ "condi_expr ::= ID LT userval",
 /*  56 */ "condi_expr ::= ID LE userval",
 /*  57 */ "condi_expr ::= ID GT userval",
 /*  58 */ "condi_expr ::= ID GE userval",
 /*  59 */ "condi_expr ::= ID EQ userval",
 /*  60 */ "condi_expr ::= ID NE userval",
 /*  61 */ "condi_expr ::= ID LIKE userval",
 /*  62 */ "condi_expr ::= ID EQ TRUE",
 /*  63 */ "condi_expr ::= ID EQ FALSE",
 /*  64 */ "condi_expr ::= ID NE TRUE",
 /*  65 */ "condi_expr ::= ID NE FALSE",
 /*  66 */ "condi_expr ::= ID IS NOT NULL",
 /*  67 */ "condi_expr ::= ID IS NULL",
 /*  68 */ "condi_expr ::= condi_expr AND condi_expr",
 /*  69 */ "userval ::= INTEGER",
 /*  70 */ "userval ::= DOUBLE",
 /*  71 */ "userval ::= PLUS INTEGER",
 /*  72 */ "userval ::= PLUS DOUBLE",
 /*  73 */ "userval ::= MINUS INTEGER",
 /*  74 */ "userval ::= MINUS DOUBLE",
 /*  75 */ "userval ::= STRING",
 /*  76 */ "userval ::= BLOB",
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
    case 90:
#line 107 "parse.y"
{ ColumnList::FreeColumnList((yypminor->yy197)); }
#line 549 "parse.c"
      break;
    case 91:
#line 109 "parse.y"
{ ColumnItem::FreeColumnItem((yypminor->yy116)); }
#line 554 "parse.c"
      break;
    case 93:
#line 207 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy49)); }
#line 559 "parse.c"
      break;
    case 94:
#line 144 "parse.y"
{ ExprList::FreeExprList((yypminor->yy130)); }
#line 564 "parse.c"
      break;
    case 95:
#line 213 "parse.y"
{ delete ((yypminor->yy99)); }
#line 569 "parse.c"
      break;
    case 96:
#line 220 "parse.y"
{ delete ((yypminor->yy9)); }
#line 574 "parse.c"
      break;
    case 97:
#line 228 "parse.y"
{ delete ((yypminor->yy101)); }
#line 579 "parse.c"
      break;
    case 98:
#line 146 "parse.y"
{ ExprList::FreeExprList((yypminor->yy130)); }
#line 584 "parse.c"
      break;
    case 100:
#line 241 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy49)); }
#line 589 "parse.c"
      break;
    case 101:
#line 260 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy49)); }
#line 594 "parse.c"
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
  { 88, 7 },
  { 88, 4 },
  { 88, 10 },
  { 88, 7 },
  { 89, 1 },
  { 89, 1 },
  { 88, 4 },
  { 88, 4 },
  { 88, 4 },
  { 88, 8 },
  { 88, 6 },
  { 88, 6 },
  { 88, 7 },
  { 88, 7 },
  { 90, 3 },
  { 90, 1 },
  { 91, 2 },
  { 91, 2 },
  { 91, 2 },
  { 91, 2 },
  { 91, 2 },
  { 91, 2 },
  { 91, 2 },
  { 91, 2 },
  { 91, 2 },
  { 91, 2 },
  { 88, 4 },
  { 88, 8 },
  { 98, 2 },
  { 98, 0 },
  { 94, 2 },
  { 94, 3 },
  { 94, 6 },
  { 94, 6 },
  { 94, 6 },
  { 94, 6 },
  { 94, 6 },
  { 94, 6 },
  { 94, 6 },
  { 94, 6 },
  { 99, 0 },
  { 99, 2 },
  { 92, 2 },
  { 93, 0 },
  { 93, 2 },
  { 95, 0 },
  { 95, 3 },
  { 95, 5 },
  { 96, 0 },
  { 96, 3 },
  { 96, 4 },
  { 96, 4 },
  { 97, 0 },
  { 97, 2 },
  { 97, 4 },
  { 100, 3 },
  { 100, 3 },
  { 100, 3 },
  { 100, 3 },
  { 100, 3 },
  { 100, 3 },
  { 100, 3 },
  { 100, 3 },
  { 100, 3 },
  { 100, 3 },
  { 100, 3 },
  { 100, 4 },
  { 100, 3 },
  { 100, 3 },
  { 101, 1 },
  { 101, 1 },
  { 101, 2 },
  { 101, 2 },
  { 101, 2 },
  { 101, 2 },
  { 101, 1 },
  { 101, 1 },
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
#line 47 "parse.y"
{
  pdbAddUser(pParse, &yymsp[-4].minor.yy66, &yymsp[-1].minor.yy0);
}
#line 888 "parse.c"
        /* No destructor defined for ADD */
        /* No destructor defined for USER */
        /* No destructor defined for IDENTIFIED */
        /* No destructor defined for BY */
        /* No destructor defined for SEMI */
        break;
      case 1:
#line 51 "parse.y"
{
  pdbDropUser(pParse, &yymsp[-1].minor.yy66);
}
#line 900 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for USER */
        /* No destructor defined for SEMI */
        break;
      case 2:
#line 56 "parse.y"
{
  pdbChangePwd(pParse, &yymsp[-6].minor.yy66, &yymsp[-2].minor.yy0); 
}
#line 910 "parse.c"
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
#line 60 "parse.y"
{
  pdbChangeRole(pParse, &yymsp[-3].minor.yy66, &yymsp[-1].minor.yy0);
}
#line 925 "parse.c"
        /* No destructor defined for SET */
        /* No destructor defined for ROLE */
        /* No destructor defined for FOR */
        /* No destructor defined for EQ */
        /* No destructor defined for SEMI */
        break;
      case 4:
#line 65 "parse.y"
{ yygotominor.yy66 = yymsp[0].minor.yy0; }
#line 935 "parse.c"
        break;
      case 5:
#line 66 "parse.y"
{ yygotominor.yy66 = yymsp[0].minor.yy0; }
#line 940 "parse.c"
        break;
      case 6:
#line 69 "parse.y"
{
  pdbDropTable(pParse, &yymsp[-1].minor.yy0);
}
#line 947 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 7:
#line 73 "parse.y"
{
  pdbAttachTable(pParse, &yymsp[-1].minor.yy0);
}
#line 957 "parse.c"
        /* No destructor defined for ATTACH */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 8:
#line 77 "parse.y"
{
  pdbDetachTable(pParse, &yymsp[-1].minor.yy0);
}
#line 967 "parse.c"
        /* No destructor defined for DETACH */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 9:
#line 83 "parse.y"
{
  pdbAttachFile(pParse, &yymsp[-1].minor.yy0, &yymsp[-5].minor.yy0, &yymsp[-3].minor.yy0);
}
#line 977 "parse.c"
        /* No destructor defined for ATTACH */
        /* No destructor defined for DATAFILE */
        /* No destructor defined for COMMA */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 10:
#line 87 "parse.y"
{
  pdbDetachFile(pParse, &yymsp[-1].minor.yy0, &yymsp[-3].minor.yy0);
}
#line 989 "parse.c"
        /* No destructor defined for DETACH */
        /* No destructor defined for DATAFILE */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 11:
#line 91 "parse.y"
{
  pdbDropFile(pParse, &yymsp[-1].minor.yy0, &yymsp[-3].minor.yy0);
}
#line 1000 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for DATAFILE */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 12:
#line 98 "parse.y"
{
  pdbCreateTable(pParse, &yymsp[-4].minor.yy0, yymsp[-2].minor.yy197);
}
#line 1011 "parse.c"
        /* No destructor defined for CREATE */
        /* No destructor defined for TABLE */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for SEMI */
        break;
      case 13:
#line 102 "parse.y"
{
  pdbAlterTable(pParse, &yymsp[-4].minor.yy0, yymsp[-2].minor.yy197);
}
#line 1023 "parse.c"
        /* No destructor defined for ALTER */
        /* No destructor defined for TABLE */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for SEMI */
        break;
      case 14:
#line 111 "parse.y"
{
  yygotominor.yy197 = ColumnList::AppendColumnItem(yymsp[-2].minor.yy197, yymsp[0].minor.yy116);
}
#line 1035 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 15:
#line 114 "parse.y"
{
  yygotominor.yy197 = ColumnList::AppendColumnItem(nullptr, yymsp[0].minor.yy116);
}
#line 1043 "parse.c"
        break;
      case 16:
#line 118 "parse.y"
{ yygotominor.yy116 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_BOOL); }
#line 1048 "parse.c"
        /* No destructor defined for BOOL_TYPE */
        break;
      case 17:
#line 119 "parse.y"
{ yygotominor.yy116 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_INT64); }
#line 1054 "parse.c"
        /* No destructor defined for BIGINT_TYPE */
        break;
      case 18:
#line 120 "parse.y"
{ yygotominor.yy116 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_DOUBLE); }
#line 1060 "parse.c"
        /* No destructor defined for DOUBLE_TYPE */
        break;
      case 19:
#line 121 "parse.y"
{ yygotominor.yy116 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_STRING); }
#line 1066 "parse.c"
        /* No destructor defined for STRING_TYPE */
        break;
      case 20:
#line 122 "parse.y"
{ yygotominor.yy116 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_BLOB); }
#line 1072 "parse.c"
        /* No destructor defined for BLOB_TYPE */
        break;
      case 21:
#line 123 "parse.y"
{ yygotominor.yy116 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_DATETIME); }
#line 1078 "parse.c"
        /* No destructor defined for DATETIME_TYPE */
        break;
      case 22:
#line 124 "parse.y"
{ yygotominor.yy116 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL2); }
#line 1084 "parse.c"
        /* No destructor defined for REAL2_TYPE */
        break;
      case 23:
#line 125 "parse.y"
{ yygotominor.yy116 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL3); }
#line 1090 "parse.c"
        /* No destructor defined for REAL3_TYPE */
        break;
      case 24:
#line 126 "parse.y"
{ yygotominor.yy116 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL4); }
#line 1096 "parse.c"
        /* No destructor defined for REAL4_TYPE */
        break;
      case 25:
#line 127 "parse.y"
{ yygotominor.yy116 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL6); }
#line 1102 "parse.c"
        /* No destructor defined for REAL6_TYPE */
        break;
      case 26:
#line 133 "parse.y"
{
  pdbDelete(pParse, &yymsp[-2].minor.yy66, yymsp[-1].minor.yy49);
}
#line 1110 "parse.c"
        /* No destructor defined for DELETE */
        /* No destructor defined for SEMI */
        break;
      case 27:
#line 139 "parse.y"
{
  pdbSelect(pParse, yymsp[-6].minor.yy130, &yymsp[-5].minor.yy66, yymsp[-4].minor.yy49, yymsp[-3].minor.yy99, yymsp[-2].minor.yy9, yymsp[-1].minor.yy101);
}
#line 1119 "parse.c"
        /* No destructor defined for SELECT */
        /* No destructor defined for SEMI */
        break;
      case 28:
#line 148 "parse.y"
{ yygotominor.yy130 = yymsp[-1].minor.yy130; }
#line 1126 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 29:
#line 149 "parse.y"
{ yygotominor.yy130 = nullptr; }
#line 1132 "parse.c"
        break;
      case 30:
#line 150 "parse.y"
{
  yygotominor.yy130 = ExprList::AppendExprItem(yymsp[-1].minor.yy130, ExprItem::MakeExpr(TK_STAR, nullptr, nullptr, &yymsp[0].minor.yy0));
}
#line 1139 "parse.c"
        break;
      case 31:
#line 153 "parse.y"
{
  ExprItem* pFieldItem = ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-1].minor.yy0);
  pFieldItem->SetAliasName(&yymsp[0].minor.yy66);
  yygotominor.yy130 = ExprList::AppendExprItem(yymsp[-2].minor.yy130, pFieldItem);
}
#line 1148 "parse.c"
        break;
      case 32:
#line 159 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_AVG_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy66);
  yygotominor.yy130 = ExprList::AppendExprItem(yymsp[-5].minor.yy130, pFunc);
}
#line 1156 "parse.c"
        /* No destructor defined for AVG_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 33:
#line 164 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_COUNT_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy66);
  yygotominor.yy130 = ExprList::AppendExprItem(yymsp[-5].minor.yy130, pFunc);
}
#line 1167 "parse.c"
        /* No destructor defined for COUNT_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 34:
#line 169 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_COUNT_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy66);
  yygotominor.yy130 = ExprList::AppendExprItem(yymsp[-5].minor.yy130, pFunc);
}
#line 1178 "parse.c"
        /* No destructor defined for COUNT_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 35:
#line 174 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_LAST_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy66);
  yygotominor.yy130 = ExprList::AppendExprItem(yymsp[-5].minor.yy130, pFunc);
}
#line 1189 "parse.c"
        /* No destructor defined for LAST_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 36:
#line 179 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_MAX_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy66);
  yygotominor.yy130 = ExprList::AppendExprItem(yymsp[-5].minor.yy130, pFunc);
}
#line 1200 "parse.c"
        /* No destructor defined for MAX_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 37:
#line 184 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_MIN_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy66);
  yygotominor.yy130 = ExprList::AppendExprItem(yymsp[-5].minor.yy130, pFunc);
}
#line 1211 "parse.c"
        /* No destructor defined for MIN_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 38:
#line 189 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_SUM_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy66);
  yygotominor.yy130 = ExprList::AppendExprItem(yymsp[-5].minor.yy130, pFunc);
}
#line 1222 "parse.c"
        /* No destructor defined for SUM_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 39:
#line 194 "parse.y"
{
  ExprItem* pFunc = ExprItem::MakeFunction(TK_FIRST_FUNC, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy66);
  yygotominor.yy130 = ExprList::AppendExprItem(yymsp[-5].minor.yy130, pFunc);
}
#line 1233 "parse.c"
        /* No destructor defined for FIRST_FUNC */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 40:
#line 200 "parse.y"
{ yygotominor.yy66.str_ = nullptr; yygotominor.yy66.len_ = 0; }
#line 1241 "parse.c"
        break;
      case 41:
#line 201 "parse.y"
{ yygotominor.yy66 = yymsp[0].minor.yy0; }
#line 1246 "parse.c"
        /* No destructor defined for AS */
        break;
      case 42:
#line 204 "parse.y"
{yygotominor.yy66 = yymsp[0].minor.yy0;}
#line 1252 "parse.c"
        /* No destructor defined for FROM */
        break;
      case 43:
#line 209 "parse.y"
{ yygotominor.yy49 = nullptr; }
#line 1258 "parse.c"
        break;
      case 44:
#line 210 "parse.y"
{ yygotominor.yy49 = yymsp[0].minor.yy49; }
#line 1263 "parse.c"
        /* No destructor defined for WHERE */
        break;
      case 45:
#line 215 "parse.y"
{ yygotominor.yy99 = nullptr; }
#line 1269 "parse.c"
        break;
      case 46:
#line 216 "parse.y"
{ yygotominor.yy99 = new GroupOpt(&yymsp[0].minor.yy0); }
#line 1274 "parse.c"
        /* No destructor defined for GROUP */
        /* No destructor defined for BY */
        break;
      case 47:
#line 217 "parse.y"
{ yygotominor.yy99 = new GroupOpt(&yymsp[-2].minor.yy0, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1281 "parse.c"
        /* No destructor defined for GROUP */
        /* No destructor defined for BY */
        break;
      case 48:
#line 222 "parse.y"
{ yygotominor.yy9 = nullptr; }
#line 1288 "parse.c"
        break;
      case 49:
#line 223 "parse.y"
{ yygotominor.yy9 = new OrderByOpt(&yymsp[0].minor.yy0, true); }
#line 1293 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        break;
      case 50:
#line 224 "parse.y"
{ yygotominor.yy9 = new OrderByOpt(&yymsp[-1].minor.yy0, true); }
#line 1300 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        /* No destructor defined for ASC */
        break;
      case 51:
#line 225 "parse.y"
{ yygotominor.yy9 = new OrderByOpt(&yymsp[-1].minor.yy0, false); }
#line 1308 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        /* No destructor defined for DESC */
        break;
      case 52:
#line 230 "parse.y"
{ yygotominor.yy101 = nullptr; }
#line 1316 "parse.c"
        break;
      case 53:
#line 231 "parse.y"
{ yygotominor.yy101 = new LimitOpt(&yymsp[0].minor.yy0); }
#line 1321 "parse.c"
        /* No destructor defined for LIMIT */
        break;
      case 54:
#line 232 "parse.y"
{ yygotominor.yy101 = new LimitOpt(&yymsp[-2].minor.yy0, &yymsp[0].minor.yy0); }
#line 1327 "parse.c"
        /* No destructor defined for LIMIT */
        /* No destructor defined for COMMA */
        break;
      case 55:
#line 243 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_LT, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), yymsp[0].minor.yy49, nullptr); }
#line 1334 "parse.c"
        /* No destructor defined for LT */
        break;
      case 56:
#line 244 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_LE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), yymsp[0].minor.yy49, nullptr); }
#line 1340 "parse.c"
        /* No destructor defined for LE */
        break;
      case 57:
#line 245 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_GT, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), yymsp[0].minor.yy49, nullptr); }
#line 1346 "parse.c"
        /* No destructor defined for GT */
        break;
      case 58:
#line 246 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_GE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), yymsp[0].minor.yy49, nullptr); }
#line 1352 "parse.c"
        /* No destructor defined for GE */
        break;
      case 59:
#line 247 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_EQ, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), yymsp[0].minor.yy49, nullptr); }
#line 1358 "parse.c"
        /* No destructor defined for EQ */
        break;
      case 60:
#line 248 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_NE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), yymsp[0].minor.yy49, nullptr); }
#line 1364 "parse.c"
        /* No destructor defined for NE */
        break;
      case 61:
#line 249 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_LIKE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), yymsp[0].minor.yy49, nullptr); }
#line 1370 "parse.c"
        /* No destructor defined for LIKE */
        break;
      case 62:
#line 250 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_ISTRUE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), nullptr, nullptr); }
#line 1376 "parse.c"
        /* No destructor defined for EQ */
        /* No destructor defined for TRUE */
        break;
      case 63:
#line 251 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_ISFALSE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), nullptr, nullptr); }
#line 1383 "parse.c"
        /* No destructor defined for EQ */
        /* No destructor defined for FALSE */
        break;
      case 64:
#line 252 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_ISFALSE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), nullptr, nullptr); }
#line 1390 "parse.c"
        /* No destructor defined for NE */
        /* No destructor defined for TRUE */
        break;
      case 65:
#line 253 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_ISTRUE, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), nullptr, nullptr); }
#line 1397 "parse.c"
        /* No destructor defined for NE */
        /* No destructor defined for FALSE */
        break;
      case 66:
#line 254 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_ISNOTNULL, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-3].minor.yy0), nullptr, nullptr); }
#line 1404 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NOT */
        /* No destructor defined for NULL */
        break;
      case 67:
#line 255 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_ISNULL, ExprItem::MakeExpr(TK_ID, nullptr, nullptr, &yymsp[-2].minor.yy0), nullptr, nullptr); }
#line 1412 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NULL */
        break;
      case 68:
#line 257 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_AND, yymsp[-2].minor.yy49, yymsp[0].minor.yy49, nullptr); }
#line 1419 "parse.c"
        /* No destructor defined for AND */
        break;
      case 69:
#line 262 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_INTEGER, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1425 "parse.c"
        break;
      case 70:
#line 263 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_DOUBLE, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1430 "parse.c"
        break;
      case 71:
#line 264 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_INTEGER, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1435 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 72:
#line 265 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_DOUBLE, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1441 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 73:
#line 266 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_UINTEGER, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1447 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 74:
#line 267 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_UDOUBLE, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1453 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 75:
#line 268 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_STRING, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1459 "parse.c"
        break;
      case 76:
#line 269 "parse.y"
{ yygotominor.yy49 = ExprItem::MakeExpr(TK_BLOB, nullptr, nullptr, &yymsp[0].minor.yy0); }
#line 1464 "parse.c"
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
#line 37 "parse.y"

  const char* errmsg = TOKEN.str_;
  pdbSetError(pParse, errmsg);

#line 1511 "parse.c"
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
