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

#line 46 "parse.c"
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
#define YYNOCODE 92
#define YYACTIONTYPE unsigned short int
#define pdbParseTOKENTYPE Token
typedef union {
  pdbParseTOKENTYPE yy0;
  ExprValue* yy21;
  GroupOpt* yy25;
  ColumnList* yy27;
  ColumnItem* yy40;
  LimitOpt* yy45;
  Token yy98;
  ExprValueList* yy123;
  TargetList* yy126;
  OrderByOpt* yy139;
  RecordList* yy142;
  int yy183;
} YYMINORTYPE;
#define YYSTACKDEPTH 100
#define pdbParseARG_SDECL SQLParser *pParse;
#define pdbParseARG_PDECL ,SQLParser *pParse
#define pdbParseARG_FETCH SQLParser *pParse = yypParser->pParse
#define pdbParseARG_STORE yypParser->pParse = pParse
#define YYNSTATE 202
#define YYNRULE 83
#define YYERRORSYMBOL 78
#define YYERRSYMDT yy183
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
 /*     0 */   123,  132,   36,   96,  114,  128,  130,   93,   97,  132,
 /*    10 */   232,   94,   11,  286,   99,   68,   69,   70,   71,   72,
 /*    20 */    73,   74,   75,   76,   77,  100,   14,  132,   37,   17,
 /*    30 */   200,  168,  136,  134,  128,  130,   93,   97,  122,   91,
 /*    40 */   103,  134,  128,  130,   93,   97,  132,   98,  101,  102,
 /*    50 */   105,  109,  113,  119,  156,  127,  170,   86,  136,  134,
 /*    60 */   128,  130,   93,   97,  127,  167,  199,    1,  139,  141,
 /*    70 */   149,  201,  145,   10,   22,  132,  177,  136,  134,  128,
 /*    80 */   130,   93,   97,   38,   49,  172,  181,  164,   58,   79,
 /*    90 */   171,  127,  152,  127,  147,  174,  126,  122,  127,  125,
 /*   100 */   179,  165,  241,  122,  127,  120,  136,  134,  128,  130,
 /*   110 */    93,   97,   23,  121,  180,  142,  144,   32,    8,  176,
 /*   120 */   122,  110,  153,  106,  116,  118,  242,  161,    9,  122,
 /*   130 */   195,  196,  148,  112,  172,  108,  188,  160,   84,  122,
 /*   140 */   190,   63,  176,   83,   78,   65,  205,    2,   65,   62,
 /*   150 */    78,   50,   39,  198,   53,   42,   40,  194,   41,  193,
 /*   160 */   209,    3,  243,   43,  191,   45,   44,  189,   46,  230,
 /*   170 */   187,   48,   47,  186,  211,    4,  185,   35,  210,   52,
 /*   180 */   192,  184,   51,   54,  197,   91,   56,  183,   57,   55,
 /*   190 */   182,    5,  212,    6,  178,   34,   59,   61,   60,  175,
 /*   200 */     7,  173,  202,   64,   67,  166,  214,  229,  163,   66,
 /*   210 */   169,   33,  162,   13,   12,  159,  157,  158,  203,   15,
 /*   220 */   151,  150,   16,  146,  208,  140,  137,  143,   20,   81,
 /*   230 */    18,  135,   21,   19,   80,  213,  204,   85,  231,  215,
 /*   240 */    82,  124,  133,   24,   90,  228,  131,   88,  129,  111,
 /*   250 */   117,   87,  107,  138,   26,  115,   89,   31,   27,   25,
 /*   260 */   104,   30,   28,   29,  176,  176,  176,  176,  176,  176,
 /*   270 */   176,  176,   92,  176,  176,  176,  176,  155,   95,  176,
 /*   280 */   176,  176,  154,
};
static YYCODETYPE yy_lookahead[] = {
 /*     0 */    18,   28,   32,   30,   22,   61,   62,   63,   64,   28,
 /*    10 */    23,   29,   19,   79,   32,   41,   42,   43,   44,   45,
 /*    20 */    46,   47,   48,   49,   50,   29,   33,   28,   23,   36,
 /*    30 */    89,   30,   59,   60,   61,   62,   63,   64,   37,   52,
 /*    40 */    58,   60,   61,   62,   63,   64,   28,   65,   66,   67,
 /*    50 */    68,   69,   70,   71,    5,   89,   90,    8,   59,   60,
 /*    60 */    61,   62,   63,   64,   89,   90,   32,   18,   72,   73,
 /*    70 */    74,   23,   76,   24,   25,   28,   77,   59,   60,   61,
 /*    80 */    62,   63,   64,   34,   35,   37,   38,   23,   39,   40,
 /*    90 */    30,   89,   90,   89,   90,   77,   30,   37,   89,   90,
 /*   100 */    51,   37,   23,   37,   89,   90,   59,   60,   61,   62,
 /*   110 */    63,   64,   26,   30,   84,   74,   75,   31,   22,   89,
 /*   120 */    37,   58,   30,   58,   74,   75,   23,   30,   32,   37,
 /*   130 */    55,   56,   30,   70,   37,   70,   57,   84,   30,   37,
 /*   140 */    37,   30,   89,   81,   82,   37,    0,   19,   37,   81,
 /*   150 */    82,   33,   33,   21,   36,   36,   32,   32,   23,   21,
 /*   160 */     0,   80,   23,   22,   58,   22,   37,   58,   38,    0,
 /*   170 */    23,   23,   32,   88,    0,   20,   87,   28,    0,   23,
 /*   180 */    54,   86,   32,   22,   53,   52,   32,   83,   23,   38,
 /*   190 */    32,   21,    0,   22,   32,   80,   33,   29,   32,   32,
 /*   200 */    23,   89,    0,   23,   32,   29,    0,    0,   85,   82,
 /*   210 */    29,   27,    7,   23,   80,   29,    6,   32,    0,   32,
 /*   220 */    29,   76,   23,   29,    0,   22,   89,   75,   32,   32,
 /*   230 */    22,   89,   23,   38,   33,    0,    0,   23,    0,    0,
 /*   240 */    29,   29,   89,   27,   23,    0,   89,   32,   89,   32,
 /*   250 */    75,   38,   32,   89,   28,   73,   83,   23,   26,   80,
 /*   260 */    32,   30,   29,   22,   91,   91,   91,   91,   91,   91,
 /*   270 */    91,   91,   89,   91,   91,   91,   91,   89,   89,   91,
 /*   280 */    91,   91,   89,
};
#define YY_SHIFT_USE_DFLT (-57)
static short yy_shift_ofst[] = {
 /*     0 */    49,  128,   96,  155,  170,  171,  177,  202,  -57,  -57,
 /*    10 */    -7,   96,  190,  218,  187,  199,  224,  208,  195,  196,
 /*    20 */   209,  235,   86,  216,   96,  226,  232,  233,  241,  231,
 /*    30 */   234,  236,  184,   96,  149,  -30,    5,  146,  119,  124,
 /*    40 */   135,  160,  141,  129,  143,  130,  140,  148,  174,  118,
 /*    50 */   150,  156,  178,  161,  151,  154,  165,  192,  163,  166,
 /*    60 */   168,  172,  111,  180,  206,  172,  -57,  -26,  -57,  -57,
 /*    70 */   -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  -57,  201,
 /*    80 */   197,  211,  172,  108,  214,  239,  213,  215,  -13,  221,
 /*    90 */   245,  -18,   47,  -18,  -18,  -27,  -57,  -18,  -57,   -4,
 /*   100 */   -18,  -57,  -57,  228,  -57,   65,  220,  -57,  -57,   63,
 /*   110 */   217,  -57,  -57,  -57,  182,   50,  175,  -57,  -57,  -57,
 /*   120 */    83,  -57,  -18,  212,  -18,   66,  -57,   47,  -18,  -57,
 /*   130 */   -18,  -57,  -18,  -56,  -18,  -56,  -18,  -19,   47,  203,
 /*   140 */   -57,   41,  152,  -57,  -57,  194,  -18,  102,  -57,  145,
 /*   150 */   191,  -18,   92,  -57,  -57,  -57,  210,  185,  186,  -18,
 /*   160 */    97,  205,  181,   64,  207,  176,  -18,    1,  -57,  -18,
 /*   170 */    60,  -57,  -18,   18,  167,  -57,   -1,  162,  -57,  -18,
 /*   180 */    48,  158,  133,  131,  126,   79,  147,  169,  109,  103,
 /*   190 */   106,  139,  138,  125,   75,  -57,  -57,  132,   34,  -18,
 /*   200 */    47,  238,
};
#define YY_REDUCE_USE_DFLT (-67)
static short yy_reduce_ofst[] = {
 /*     0 */   -66,  -67,   81,  -67,  -67,  -67,  -67,  -67,  -67,  -67,
 /*    10 */   -67,  134,  -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,
 /*    20 */   -67,  -67,  -67,  -67,  179,  -67,  -67,  -67,  -67,  -67,
 /*    30 */   -67,  -67,  -67,  115,  -67,  -67,  -67,  -67,  -67,  -67,
 /*    40 */   -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,
 /*    50 */   -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,
 /*    60 */   -67,   68,  -67,  -67,  -67,  127,  -67,  -67,  -67,  -67,
 /*    70 */   -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,
 /*    80 */   -67,  -67,   62,  -67,  -67,  -67,  -67,  -67,  173,  -67,
 /*    90 */   -67,  183,  -67,  188,  189,  -67,  -67,  193,  -67,  -67,
 /*   100 */    15,  -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,
 /*   110 */   -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,
 /*   120 */   -67,  -67,  164,  -67,    9,  -67,  -67,  -67,  159,  -67,
 /*   130 */   157,  -67,  153,  -67,  142,  -67,  137,  -67,  -67,  -67,
 /*   140 */   -67,  -67,  -67,  -67,  -67,  -67,    4,  -67,  -67,  -67,
 /*   150 */   -67,    2,  -67,  -67,  -67,  -67,  -67,  -67,  -67,   53,
 /*   160 */   -67,  -67,  123,  -67,  -67,  -67,  -25,  -67,  -67,  -34,
 /*   170 */   -67,  -67,  112,  -67,  -67,  -67,  -67,  -67,  -67,   30,
 /*   180 */   -67,  -67,  104,   95,   89,   85,  -67,  -67,  -67,  -67,
 /*   190 */   -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,  -67,  -59,
 /*   200 */   -67,  -67,
};
static YYACTIONTYPE yy_default[] = {
 /*     0 */   285,  285,  285,  285,  285,  285,  285,  285,  206,  207,
 /*    10 */   285,  285,  285,  285,  285,  285,  285,  285,  285,  285,
 /*    20 */   285,  285,  285,  285,  285,  285,  285,  285,  285,  285,
 /*    30 */   285,  285,  285,  285,  285,  285,  285,  285,  285,  285,
 /*    40 */   285,  285,  285,  285,  285,  285,  285,  285,  285,  285,
 /*    50 */   285,  285,  285,  285,  285,  285,  285,  285,  285,  285,
 /*    60 */   285,  285,  285,  285,  285,  285,  216,  285,  218,  219,
 /*    70 */   220,  221,  222,  223,  224,  225,  226,  227,  217,  285,
 /*    80 */   285,  285,  285,  285,  285,  285,  285,  285,  285,  285,
 /*    90 */   285,  285,  233,  285,  285,  285,  246,  285,  247,  248,
 /*   100 */   278,  249,  250,  251,  259,  285,  252,  260,  255,  285,
 /*   110 */   253,  261,  256,  254,  257,  285,  285,  274,  275,  258,
 /*   120 */   285,  262,  285,  285,  278,  285,  263,  279,  285,  266,
 /*   130 */   285,  267,  285,  268,  285,  269,  285,  270,  280,  285,
 /*   140 */   271,  285,  285,  272,  273,  285,  278,  285,  276,  285,
 /*   150 */   285,  278,  285,  277,  265,  264,  285,  285,  285,  285,
 /*   160 */   285,  285,  285,  285,  285,  285,  278,  285,  245,  278,
 /*   170 */   285,  244,  285,  283,  285,  284,  281,  285,  282,  285,
 /*   180 */   285,  285,  232,  234,  237,  285,  285,  285,  285,  285,
 /*   190 */   285,  285,  285,  285,  238,  239,  240,  285,  285,  235,
 /*   200 */   236,  285,
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
  "DELETE",        "TOP",           "NOTIN",         "TINYINT",     
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
  "WHERE",         "GROUP",         "ORDER",         "ASC",         
  "DESC",          "LIMIT",         "INTEGER",       "AND",         
  "NE",            "GT",            "GE",            "LT",          
  "LE",            "STAR",          "TRUE",          "FALSE",       
  "PLUS",          "MINUS",         "DOUBLE",        "BLOB",        
  "LIKE",          "IS",            "NOT",           "NULL",        
  "IN",            "AS",            "error",         "cmd",         
  "username",      "cre_columnlist",  "cre_column",    "where_opt",   
  "target_list",   "record_list",   "groupby_opt",   "orderby_opt", 
  "limit_opt",     "expr_val",      "expr_val_list",
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
 /*  29 */ "cmd ::= SELECT target_list SEMI",
 /*  30 */ "where_opt ::=",
 /*  31 */ "where_opt ::= WHERE expr_val",
 /*  32 */ "groupby_opt ::=",
 /*  33 */ "groupby_opt ::= GROUP BY ID",
 /*  34 */ "groupby_opt ::= GROUP BY ID expr_val",
 /*  35 */ "orderby_opt ::=",
 /*  36 */ "orderby_opt ::= ORDER BY ID",
 /*  37 */ "orderby_opt ::= ORDER BY ID ASC",
 /*  38 */ "orderby_opt ::= ORDER BY ID DESC",
 /*  39 */ "limit_opt ::=",
 /*  40 */ "limit_opt ::= LIMIT INTEGER",
 /*  41 */ "limit_opt ::= LIMIT INTEGER COMMA INTEGER",
 /*  42 */ "record_list ::= LP expr_val_list RP",
 /*  43 */ "record_list ::= record_list COMMA LP expr_val_list RP",
 /*  44 */ "expr_val ::= LP expr_val RP",
 /*  45 */ "expr_val ::= STAR",
 /*  46 */ "expr_val ::= ID",
 /*  47 */ "expr_val ::= TRUE",
 /*  48 */ "expr_val ::= FALSE",
 /*  49 */ "expr_val ::= INTEGER",
 /*  50 */ "expr_val ::= PLUS INTEGER",
 /*  51 */ "expr_val ::= MINUS INTEGER",
 /*  52 */ "expr_val ::= DOUBLE",
 /*  53 */ "expr_val ::= PLUS DOUBLE",
 /*  54 */ "expr_val ::= MINUS DOUBLE",
 /*  55 */ "expr_val ::= STRING",
 /*  56 */ "expr_val ::= BLOB",
 /*  57 */ "expr_val ::= INTEGER ID",
 /*  58 */ "expr_val ::= PLUS INTEGER ID",
 /*  59 */ "expr_val ::= MINUS INTEGER ID",
 /*  60 */ "expr_val ::= ID LP expr_val_list RP",
 /*  61 */ "expr_val ::= ADD LP expr_val_list RP",
 /*  62 */ "expr_val ::= expr_val LT expr_val",
 /*  63 */ "expr_val ::= expr_val LE expr_val",
 /*  64 */ "expr_val ::= expr_val GT expr_val",
 /*  65 */ "expr_val ::= expr_val GE expr_val",
 /*  66 */ "expr_val ::= expr_val EQ expr_val",
 /*  67 */ "expr_val ::= expr_val NE expr_val",
 /*  68 */ "expr_val ::= expr_val AND expr_val",
 /*  69 */ "expr_val ::= ID LIKE STRING",
 /*  70 */ "expr_val ::= ID IS NOT NULL",
 /*  71 */ "expr_val ::= ID IS NULL",
 /*  72 */ "expr_val ::= STRING IS NOT NULL",
 /*  73 */ "expr_val ::= STRING IS NULL",
 /*  74 */ "expr_val ::= ID IN LP expr_val_list RP",
 /*  75 */ "expr_val ::= ID NOT IN LP expr_val_list RP",
 /*  76 */ "expr_val_list ::=",
 /*  77 */ "expr_val_list ::= expr_val",
 /*  78 */ "expr_val_list ::= expr_val_list COMMA expr_val",
 /*  79 */ "target_list ::= expr_val",
 /*  80 */ "target_list ::= expr_val AS ID",
 /*  81 */ "target_list ::= target_list COMMA expr_val",
 /*  82 */ "target_list ::= target_list COMMA expr_val AS ID",
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
    case 81:
#line 113 "parse.y"
{ ColumnList::FreeColumnList((yypminor->yy27)); }
#line 563 "parse.c"
      break;
    case 82:
#line 115 "parse.y"
{ ColumnItem::FreeColumnItem((yypminor->yy40)); }
#line 568 "parse.c"
      break;
    case 83:
#line 166 "parse.y"
{ ExprValue::FreeExprValue((yypminor->yy21)); }
#line 573 "parse.c"
      break;
    case 84:
#line 251 "parse.y"
{ TargetList::FreeTargetList((yypminor->yy126)); }
#line 578 "parse.c"
      break;
    case 85:
#line 198 "parse.y"
{ RecordList::FreeRecordList((yypminor->yy142)); }
#line 583 "parse.c"
      break;
    case 86:
#line 172 "parse.y"
{ delete ((yypminor->yy25)); }
#line 588 "parse.c"
      break;
    case 87:
#line 179 "parse.y"
{ delete ((yypminor->yy139)); }
#line 593 "parse.c"
      break;
    case 88:
#line 187 "parse.y"
{ delete ((yypminor->yy45)); }
#line 598 "parse.c"
      break;
    case 89:
#line 206 "parse.y"
{ ExprValue::FreeExprValue((yypminor->yy21)); }
#line 603 "parse.c"
      break;
    case 90:
#line 244 "parse.y"
{ ExprValueList::FreeExprValueList((yypminor->yy123)); }
#line 608 "parse.c"
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
  { 79, 7 },
  { 79, 4 },
  { 79, 10 },
  { 79, 7 },
  { 80, 1 },
  { 80, 1 },
  { 79, 4 },
  { 79, 4 },
  { 79, 4 },
  { 79, 8 },
  { 79, 6 },
  { 79, 6 },
  { 79, 7 },
  { 79, 7 },
  { 81, 3 },
  { 81, 1 },
  { 82, 2 },
  { 82, 2 },
  { 82, 2 },
  { 82, 2 },
  { 82, 2 },
  { 82, 2 },
  { 82, 2 },
  { 82, 2 },
  { 82, 2 },
  { 82, 2 },
  { 79, 5 },
  { 79, 9 },
  { 79, 9 },
  { 79, 3 },
  { 83, 0 },
  { 83, 2 },
  { 86, 0 },
  { 86, 3 },
  { 86, 4 },
  { 87, 0 },
  { 87, 3 },
  { 87, 4 },
  { 87, 4 },
  { 88, 0 },
  { 88, 2 },
  { 88, 4 },
  { 85, 3 },
  { 85, 5 },
  { 89, 3 },
  { 89, 1 },
  { 89, 1 },
  { 89, 1 },
  { 89, 1 },
  { 89, 1 },
  { 89, 2 },
  { 89, 2 },
  { 89, 1 },
  { 89, 2 },
  { 89, 2 },
  { 89, 1 },
  { 89, 1 },
  { 89, 2 },
  { 89, 3 },
  { 89, 3 },
  { 89, 4 },
  { 89, 4 },
  { 89, 3 },
  { 89, 3 },
  { 89, 3 },
  { 89, 3 },
  { 89, 3 },
  { 89, 3 },
  { 89, 3 },
  { 89, 3 },
  { 89, 4 },
  { 89, 3 },
  { 89, 4 },
  { 89, 3 },
  { 89, 5 },
  { 89, 6 },
  { 90, 0 },
  { 90, 1 },
  { 90, 3 },
  { 84, 1 },
  { 84, 3 },
  { 84, 3 },
  { 84, 5 },
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
#line 53 "parse.y"
{
  pdbAddUser(pParse, &yymsp[-4].minor.yy98, &yymsp[-1].minor.yy0);
}
#line 908 "parse.c"
        /* No destructor defined for ADD */
        /* No destructor defined for USER */
        /* No destructor defined for IDENTIFIED */
        /* No destructor defined for BY */
        /* No destructor defined for SEMI */
        break;
      case 1:
#line 57 "parse.y"
{
  pdbDropUser(pParse, &yymsp[-1].minor.yy98);
}
#line 920 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for USER */
        /* No destructor defined for SEMI */
        break;
      case 2:
#line 62 "parse.y"
{
  pdbChangePwd(pParse, &yymsp[-6].minor.yy98, &yymsp[-2].minor.yy0); 
}
#line 930 "parse.c"
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
#line 66 "parse.y"
{
  pdbChangeRole(pParse, &yymsp[-3].minor.yy98, &yymsp[-1].minor.yy0);
}
#line 945 "parse.c"
        /* No destructor defined for SET */
        /* No destructor defined for ROLE */
        /* No destructor defined for FOR */
        /* No destructor defined for EQ */
        /* No destructor defined for SEMI */
        break;
      case 4:
#line 71 "parse.y"
{ yygotominor.yy98 = yymsp[0].minor.yy0; }
#line 955 "parse.c"
        break;
      case 5:
#line 72 "parse.y"
{ yygotominor.yy98 = yymsp[0].minor.yy0; }
#line 960 "parse.c"
        break;
      case 6:
#line 75 "parse.y"
{
  pdbDropTable(pParse, &yymsp[-1].minor.yy0);
}
#line 967 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 7:
#line 79 "parse.y"
{
  pdbAttachTable(pParse, &yymsp[-1].minor.yy0);
}
#line 977 "parse.c"
        /* No destructor defined for ATTACH */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 8:
#line 83 "parse.y"
{
  pdbDetachTable(pParse, &yymsp[-1].minor.yy0);
}
#line 987 "parse.c"
        /* No destructor defined for DETACH */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 9:
#line 89 "parse.y"
{
  pdbAttachFile(pParse, &yymsp[-1].minor.yy0, &yymsp[-5].minor.yy0, &yymsp[-3].minor.yy0);
}
#line 997 "parse.c"
        /* No destructor defined for ATTACH */
        /* No destructor defined for DATAFILE */
        /* No destructor defined for COMMA */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 10:
#line 93 "parse.y"
{
  pdbDetachFile(pParse, &yymsp[-1].minor.yy0, &yymsp[-3].minor.yy0);
}
#line 1009 "parse.c"
        /* No destructor defined for DETACH */
        /* No destructor defined for DATAFILE */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 11:
#line 97 "parse.y"
{
  pdbDropFile(pParse, &yymsp[-1].minor.yy0, &yymsp[-3].minor.yy0);
}
#line 1020 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for DATAFILE */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 12:
#line 104 "parse.y"
{
  pdbCreateTable(pParse, &yymsp[-4].minor.yy0, yymsp[-2].minor.yy27);
}
#line 1031 "parse.c"
        /* No destructor defined for CREATE */
        /* No destructor defined for TABLE */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for SEMI */
        break;
      case 13:
#line 108 "parse.y"
{
  pdbAlterTable(pParse, &yymsp[-4].minor.yy0, yymsp[-2].minor.yy27);
}
#line 1043 "parse.c"
        /* No destructor defined for ALTER */
        /* No destructor defined for TABLE */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for SEMI */
        break;
      case 14:
#line 117 "parse.y"
{
  yygotominor.yy27 = ColumnList::AppendColumnItem(yymsp[-2].minor.yy27, yymsp[0].minor.yy40);
}
#line 1055 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 15:
#line 120 "parse.y"
{
  yygotominor.yy27 = ColumnList::AppendColumnItem(nullptr, yymsp[0].minor.yy40);
}
#line 1063 "parse.c"
        break;
      case 16:
#line 124 "parse.y"
{ yygotominor.yy40 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_BOOL); }
#line 1068 "parse.c"
        /* No destructor defined for BOOL_TYPE */
        break;
      case 17:
#line 125 "parse.y"
{ yygotominor.yy40 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_INT64); }
#line 1074 "parse.c"
        /* No destructor defined for BIGINT_TYPE */
        break;
      case 18:
#line 126 "parse.y"
{ yygotominor.yy40 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_DOUBLE); }
#line 1080 "parse.c"
        /* No destructor defined for DOUBLE_TYPE */
        break;
      case 19:
#line 127 "parse.y"
{ yygotominor.yy40 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_STRING); }
#line 1086 "parse.c"
        /* No destructor defined for STRING_TYPE */
        break;
      case 20:
#line 128 "parse.y"
{ yygotominor.yy40 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_BLOB); }
#line 1092 "parse.c"
        /* No destructor defined for BLOB_TYPE */
        break;
      case 21:
#line 129 "parse.y"
{ yygotominor.yy40 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_DATETIME); }
#line 1098 "parse.c"
        /* No destructor defined for DATETIME_TYPE */
        break;
      case 22:
#line 130 "parse.y"
{ yygotominor.yy40 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL2); }
#line 1104 "parse.c"
        /* No destructor defined for REAL2_TYPE */
        break;
      case 23:
#line 131 "parse.y"
{ yygotominor.yy40 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL3); }
#line 1110 "parse.c"
        /* No destructor defined for REAL3_TYPE */
        break;
      case 24:
#line 132 "parse.y"
{ yygotominor.yy40 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL4); }
#line 1116 "parse.c"
        /* No destructor defined for REAL4_TYPE */
        break;
      case 25:
#line 133 "parse.y"
{ yygotominor.yy40 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL6); }
#line 1122 "parse.c"
        /* No destructor defined for REAL6_TYPE */
        break;
      case 26:
#line 139 "parse.y"
{
  pdbDelete(pParse, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy21);
}
#line 1130 "parse.c"
        /* No destructor defined for DELETE */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 27:
#line 145 "parse.y"
{
  pdbInsert(pParse, &yymsp[-6].minor.yy0, yymsp[-4].minor.yy126, yymsp[-1].minor.yy142);
}
#line 1140 "parse.c"
        /* No destructor defined for INSERT */
        /* No destructor defined for INTO */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for VALUES */
        /* No destructor defined for SEMI */
        break;
      case 28:
#line 151 "parse.y"
{
  pdbSelect(pParse, yymsp[-7].minor.yy126, &yymsp[-5].minor.yy0, yymsp[-4].minor.yy21, yymsp[-3].minor.yy25, yymsp[-2].minor.yy139, yymsp[-1].minor.yy45);
}
#line 1153 "parse.c"
        /* No destructor defined for SELECT */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 29:
#line 161 "parse.y"
{
  pdbSelect(pParse, yymsp[-1].minor.yy126, nullptr, nullptr, nullptr, nullptr, nullptr);
}
#line 1163 "parse.c"
        /* No destructor defined for SELECT */
        /* No destructor defined for SEMI */
        break;
      case 30:
#line 168 "parse.y"
{ yygotominor.yy21 = nullptr; }
#line 1170 "parse.c"
        break;
      case 31:
#line 169 "parse.y"
{ yygotominor.yy21 = yymsp[0].minor.yy21; }
#line 1175 "parse.c"
        /* No destructor defined for WHERE */
        break;
      case 32:
#line 174 "parse.y"
{ yygotominor.yy25 = nullptr; }
#line 1181 "parse.c"
        break;
      case 33:
#line 175 "parse.y"
{ yygotominor.yy25 = new GroupOpt(&yymsp[0].minor.yy0); }
#line 1186 "parse.c"
        /* No destructor defined for GROUP */
        /* No destructor defined for BY */
        break;
      case 34:
#line 176 "parse.y"
{ yygotominor.yy25 = new GroupOpt(&yymsp[-1].minor.yy0, yymsp[0].minor.yy21); }
#line 1193 "parse.c"
        /* No destructor defined for GROUP */
        /* No destructor defined for BY */
        break;
      case 35:
#line 181 "parse.y"
{ yygotominor.yy139 = nullptr; }
#line 1200 "parse.c"
        break;
      case 36:
#line 182 "parse.y"
{ yygotominor.yy139 = new OrderByOpt(&yymsp[0].minor.yy0, true); }
#line 1205 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        break;
      case 37:
#line 183 "parse.y"
{ yygotominor.yy139 = new OrderByOpt(&yymsp[-1].minor.yy0, true); }
#line 1212 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        /* No destructor defined for ASC */
        break;
      case 38:
#line 184 "parse.y"
{ yygotominor.yy139 = new OrderByOpt(&yymsp[-1].minor.yy0, false); }
#line 1220 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        /* No destructor defined for DESC */
        break;
      case 39:
#line 189 "parse.y"
{ yygotominor.yy45 = nullptr; }
#line 1228 "parse.c"
        break;
      case 40:
#line 190 "parse.y"
{ yygotominor.yy45 = new LimitOpt(&yymsp[0].minor.yy0); }
#line 1233 "parse.c"
        /* No destructor defined for LIMIT */
        break;
      case 41:
#line 191 "parse.y"
{ yygotominor.yy45 = new LimitOpt(&yymsp[-2].minor.yy0, &yymsp[0].minor.yy0); }
#line 1239 "parse.c"
        /* No destructor defined for LIMIT */
        /* No destructor defined for COMMA */
        break;
      case 42:
#line 201 "parse.y"
{ yygotominor.yy142 = RecordList::AppendRecordList(nullptr, yymsp[-1].minor.yy123); }
#line 1246 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 43:
#line 203 "parse.y"
{ yygotominor.yy142 = RecordList::AppendRecordList(yymsp[-4].minor.yy142, yymsp[-1].minor.yy123); }
#line 1253 "parse.c"
        /* No destructor defined for COMMA */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 44:
#line 208 "parse.y"
{ yygotominor.yy21 = yymsp[-1].minor.yy21; }
#line 1261 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 45:
#line 209 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeStarValue(); }
#line 1268 "parse.c"
        /* No destructor defined for STAR */
        break;
      case 46:
#line 210 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeID(&yymsp[0].minor.yy0); }
#line 1274 "parse.c"
        break;
      case 47:
#line 211 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeBoolValue(true); }
#line 1279 "parse.c"
        /* No destructor defined for TRUE */
        break;
      case 48:
#line 212 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeBoolValue(false); }
#line 1285 "parse.c"
        /* No destructor defined for FALSE */
        break;
      case 49:
#line 213 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeIntValue(false, &yymsp[0].minor.yy0); }
#line 1291 "parse.c"
        break;
      case 50:
#line 214 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeIntValue(false, &yymsp[0].minor.yy0); }
#line 1296 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 51:
#line 215 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeIntValue(true, &yymsp[0].minor.yy0); }
#line 1302 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 52:
#line 216 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeDoubleValue(false, &yymsp[0].minor.yy0); }
#line 1308 "parse.c"
        break;
      case 53:
#line 217 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeDoubleValue(false, &yymsp[0].minor.yy0); }
#line 1313 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 54:
#line 218 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeDoubleValue(true, &yymsp[0].minor.yy0); }
#line 1319 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 55:
#line 219 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeStringValue(&yymsp[0].minor.yy0); }
#line 1325 "parse.c"
        break;
      case 56:
#line 220 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeBlobValue(&yymsp[0].minor.yy0); }
#line 1330 "parse.c"
        break;
      case 57:
#line 221 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeTimeValue(false, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1335 "parse.c"
        break;
      case 58:
#line 222 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeTimeValue(false, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1340 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 59:
#line 223 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeTimeValue(true, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1346 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 60:
#line 225 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeFunction(&yymsp[-3].minor.yy0, yymsp[-1].minor.yy123); }
#line 1352 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 61:
#line 226 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeFunction(&yymsp[-3].minor.yy0, yymsp[-1].minor.yy123); }
#line 1359 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 62:
#line 228 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeCompare(TK_LT, yymsp[-2].minor.yy21, yymsp[0].minor.yy21); }
#line 1366 "parse.c"
        /* No destructor defined for LT */
        break;
      case 63:
#line 229 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeCompare(TK_LE, yymsp[-2].minor.yy21, yymsp[0].minor.yy21); }
#line 1372 "parse.c"
        /* No destructor defined for LE */
        break;
      case 64:
#line 230 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeCompare(TK_GT, yymsp[-2].minor.yy21, yymsp[0].minor.yy21); }
#line 1378 "parse.c"
        /* No destructor defined for GT */
        break;
      case 65:
#line 231 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeCompare(TK_GE, yymsp[-2].minor.yy21, yymsp[0].minor.yy21); }
#line 1384 "parse.c"
        /* No destructor defined for GE */
        break;
      case 66:
#line 232 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeCompare(TK_EQ, yymsp[-2].minor.yy21, yymsp[0].minor.yy21); }
#line 1390 "parse.c"
        /* No destructor defined for EQ */
        break;
      case 67:
#line 233 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeCompare(TK_NE, yymsp[-2].minor.yy21, yymsp[0].minor.yy21); }
#line 1396 "parse.c"
        /* No destructor defined for NE */
        break;
      case 68:
#line 234 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeCompare(TK_AND, yymsp[-2].minor.yy21, yymsp[0].minor.yy21); }
#line 1402 "parse.c"
        /* No destructor defined for AND */
        break;
      case 69:
#line 235 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeLike(&yymsp[-2].minor.yy0, &yymsp[0].minor.yy0); }
#line 1408 "parse.c"
        /* No destructor defined for LIKE */
        break;
      case 70:
#line 236 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeIsNotNull(&yymsp[-3].minor.yy0); }
#line 1414 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NOT */
        /* No destructor defined for NULL */
        break;
      case 71:
#line 237 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeIsNull(&yymsp[-2].minor.yy0); }
#line 1422 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NULL */
        break;
      case 72:
#line 238 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeIsNotNull(&yymsp[-3].minor.yy0); }
#line 1429 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NOT */
        /* No destructor defined for NULL */
        break;
      case 73:
#line 239 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeIsNull(&yymsp[-2].minor.yy0); }
#line 1437 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NULL */
        break;
      case 74:
#line 240 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeIn(&yymsp[-4].minor.yy0, yymsp[-1].minor.yy123); }
#line 1444 "parse.c"
        /* No destructor defined for IN */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 75:
#line 241 "parse.y"
{ yygotominor.yy21 = ExprValue::MakeNotIn(&yymsp[-5].minor.yy0, yymsp[-1].minor.yy123); }
#line 1452 "parse.c"
        /* No destructor defined for NOT */
        /* No destructor defined for IN */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 76:
#line 246 "parse.y"
{ yygotominor.yy123 = nullptr; }
#line 1461 "parse.c"
        break;
      case 77:
#line 247 "parse.y"
{ yygotominor.yy123 = ExprValueList::AppendExprValue(nullptr, yymsp[0].minor.yy21); }
#line 1466 "parse.c"
        break;
      case 78:
#line 248 "parse.y"
{ yygotominor.yy123 = ExprValueList::AppendExprValue(yymsp[-2].minor.yy123, yymsp[0].minor.yy21); }
#line 1471 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 79:
#line 253 "parse.y"
{ yygotominor.yy126 = TargetList::AppendExprValue(nullptr, yymsp[0].minor.yy21, nullptr); }
#line 1477 "parse.c"
        break;
      case 80:
#line 254 "parse.y"
{ yygotominor.yy126 = TargetList::AppendExprValue(nullptr, yymsp[-2].minor.yy21, &yymsp[0].minor.yy0); }
#line 1482 "parse.c"
        /* No destructor defined for AS */
        break;
      case 81:
#line 255 "parse.y"
{ yygotominor.yy126 = TargetList::AppendExprValue(yymsp[-2].minor.yy126, yymsp[0].minor.yy21, nullptr); }
#line 1488 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 82:
#line 256 "parse.y"
{ yygotominor.yy126 = TargetList::AppendExprValue(yymsp[-4].minor.yy126, yymsp[-2].minor.yy21, &yymsp[0].minor.yy0); }
#line 1494 "parse.c"
        /* No destructor defined for COMMA */
        /* No destructor defined for AS */
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
#line 43 "parse.y"

  const char* errmsg = TOKEN.str_;
  pdbSetError(pParse, errmsg);

#line 1543 "parse.c"
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
