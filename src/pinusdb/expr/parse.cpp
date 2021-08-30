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
#define YYNOCODE 96
#define YYACTIONTYPE unsigned short int
#define pdbParseTOKENTYPE Token
typedef union {
  pdbParseTOKENTYPE yy0;
  Token yy68;
  GroupOpt* yy73;
  ColumnItem* yy114;
  TargetList* yy120;
  LimitOpt* yy133;
  ExprValue* yy137;
  ExprValueList* yy161;
  ColumnList* yy167;
  OrderByOpt* yy177;
  RecordList* yy180;
  int yy191;
} YYMINORTYPE;
#define YYSTACKDEPTH 100
#define pdbParseARG_SDECL SQLParser *pParse;
#define pdbParseARG_PDECL ,SQLParser *pParse
#define pdbParseARG_FETCH SQLParser *pParse = yypParser->pParse
#define pdbParseARG_STORE yypParser->pParse = pParse
#define YYNSTATE 210
#define YYNRULE 88
#define YYERRORSYMBOL 82
#define YYERRSYMDT yy191
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
 /*    10 */    78,   79,   80,   81,  131,  140,  209,  100,  118,  136,
 /*    20 */   138,   97,  101,  140,  172,   98,   11,  176,  103,  179,
 /*    30 */   180,  189,  135,  178,  130,  104,  130,   23,  173,  299,
 /*    40 */    14,  140,   32,   17,  135,  175,  124,   36,  135,  160,
 /*    50 */   144,  142,  136,  138,   97,  101,  135,  155,  107,  142,
 /*    60 */   136,  138,   97,  101,  208,  102,  105,  106,  109,  113,
 /*    70 */   117,  123,  164,  135,  133,   90,  144,  142,  136,  138,
 /*    80 */    97,  101,  147,  149,  157,    1,  153,  253,  134,   37,
 /*    90 */   129,   10,   22,  188,  185,  130,  140,  130,  184,  150,
 /*   100 */   152,   38,   49,    8,  140,  114,   58,   83,  120,  122,
 /*   110 */   254,  110,  161,    9,  169,  135,  128,  116,  244,  130,
 /*   120 */   213,  180,  187,  112,  198,  196,   87,   82,  203,  204,
 /*   130 */   156,  144,  142,  136,  138,   97,  101,  130,    2,  144,
 /*   140 */   142,  136,  138,   97,  101,  168,   88,   63,  217,  182,
 /*   150 */   184,   95,  206,   65,   65,   62,   82,   50,   39,  207,
 /*   160 */    53,   42,   40,   41,  202,  201,  255,   43,  199,    3,
 /*   170 */    44,   45,  197,   46,  242,  195,   48,  219,   47,    4,
 /*   180 */   194,  193,   35,   51,  200,   52,  192,  218,   54,  191,
 /*   190 */    56,  205,   95,   55,   57,  190,    5,  220,   59,  186,
 /*   200 */     6,   60,  183,    7,  181,   61,   34,  210,   64,  222,
 /*   210 */   174,   67,  241,  177,   66,   33,   13,  170,  171,  167,
 /*   220 */   165,   12,  211,  166,  159,   16,   15,  158,  154,  151,
 /*   230 */   148,  216,  145,   18,  143,  141,   19,   20,   21,  221,
 /*   240 */   126,  243,  132,   24,  212,   25,   89,  127,  125,  139,
 /*   250 */   137,  121,   85,  146,   84,   86,  115,   26,  223,  119,
 /*   260 */   111,   27,  108,   91,   92,   28,   94,  240,   29,  185,
 /*   270 */    93,   31,   30,  185,  185,  185,  185,  185,  185,  162,
 /*   280 */   185,  185,   96,  185,  185,   99,  185,  185,  185,  163,
};
static YYCODETYPE yy_lookahead[] = {
 /*     0 */    41,   42,   43,   44,   45,   46,   47,   48,   49,   50,
 /*    10 */    51,   52,   53,   54,   18,   28,   23,   30,   22,   65,
 /*    20 */    66,   67,   68,   28,   23,   29,   19,   30,   32,   30,
 /*    30 */    37,   38,   93,   94,   37,   29,   37,   26,   37,   83,
 /*    40 */    33,   28,   31,   36,   93,   94,   50,   32,   93,   94,
 /*    50 */    63,   64,   65,   66,   67,   68,   93,   94,   62,   64,
 /*    60 */    65,   66,   67,   68,   93,   69,   70,   71,   72,   73,
 /*    70 */    74,   75,    5,   93,   94,    8,   63,   64,   65,   66,
 /*    80 */    67,   68,   76,   77,   78,   18,   80,   23,   30,   23,
 /*    90 */    30,   24,   25,   88,   81,   37,   28,   37,   93,   78,
 /*   100 */    79,   34,   35,   22,   28,   62,   39,   40,   78,   79,
 /*   110 */    23,   62,   30,   32,   30,   93,   94,   74,   23,   37,
 /*   120 */     0,   37,   55,   74,   37,   61,   85,   86,   59,   60,
 /*   130 */    30,   63,   64,   65,   66,   67,   68,   37,   19,   63,
 /*   140 */    64,   65,   66,   67,   68,   88,   30,   30,    0,   81,
 /*   150 */    93,   56,   21,   37,   37,   85,   86,   33,   33,   32,
 /*   160 */    36,   36,   32,   23,   32,   21,   23,   22,   62,   84,
 /*   170 */    37,   22,   62,   38,    0,   23,   23,    0,   32,   20,
 /*   180 */    92,   91,   28,   32,   58,   23,   90,    0,   22,   87,
 /*   190 */    32,   57,   56,   38,   23,   32,   21,    0,   33,   32,
 /*   200 */    22,   32,   32,   23,   93,   29,   84,    0,   23,    0,
 /*   210 */    29,   32,    0,   29,   86,   27,   23,    7,   89,   29,
 /*   220 */     6,   84,    0,   32,   29,   23,   32,   80,   29,   79,
 /*   230 */    22,    0,   93,   22,   93,   93,   38,   32,   23,    0,
 /*   240 */    22,    0,   29,   27,    0,   84,   23,   30,   29,   93,
 /*   250 */    93,   79,   32,   93,   33,   29,   32,   28,    0,   77,
 /*   260 */    32,   26,   32,   38,   32,   29,   23,    0,   22,   95,
 /*   270 */    87,   23,   30,   95,   95,   95,   95,   95,   95,   93,
 /*   280 */    95,   95,   93,   95,   95,   93,   95,   95,   95,   93,
};
#define YY_SHIFT_USE_DFLT (-47)
static short yy_shift_ofst[] = {
 /*     0 */    67,  119,   81,  159,  175,  178,  180,  207,  -47,  -47,
 /*    10 */     7,   81,  193,  222,  194,  202,  231,  211,  198,  205,
 /*    20 */   215,  239,   11,  216,   81,  229,  235,  236,  246,  242,
 /*    30 */   248,  244,  188,   81,  154,   15,   66,  120,  125,  130,
 /*    40 */   140,  148,  145,  133,  149,  135,  146,  153,  177,  124,
 /*    50 */   151,  162,  187,  166,  155,  158,  171,  197,  165,  169,
 /*    60 */   176,  179,  117,  185,  209,  179,  -47,  -41,  -47,  -47,
 /*    70 */   -47,  -47,  -47,  -47,  -47,  -47,  -47,  -47,  -47,  -47,
 /*    80 */   -47,  -47,  -47,  221,  220,  226,  179,  116,  223,  258,
 /*    90 */   225,  232,   95,  243,  267,   -4,   76,   -4,   -4,  -13,
 /*   100 */   -47,   -4,  -47,    6,   -4,  -47,  -47,  230,  -47,   49,
 /*   110 */   228,  -47,  -47,   43,  224,  -47,  -47,  -47,  182,   30,
 /*   120 */   172,  -47,  -47,  -47,  219,  218,  217,  -47,   60,  -47,
 /*   130 */    -4,  213,   -4,   58,  -47,   76,   -4,  -47,   -4,  -47,
 /*   140 */    -4,  -46,   -4,  -46,   -4,   -5,   76,  208,  -47,   21,
 /*   150 */   150,  -47,  -47,  199,   -4,  100,  -47,  147,  195,   -4,
 /*   160 */    82,  -47,  -47,  -47,  214,  191,  190,   -4,   84,  210,
 /*   170 */   184,    1,  212,  181,   -4,   -3,  -47,   -4,   -1,  -47,
 /*   180 */    -4,   68,  170,  -47,   13,  167,  -47,   -4,   -7,  163,
 /*   190 */   136,  134,  126,   64,  152,  174,  110,   87,  106,  143,
 /*   200 */   144,  132,   69,  -47,  -47,  131,  127,   -4,   76,  241,
};
#define YY_REDUCE_USE_DFLT (-62)
static short yy_reduce_ofst[] = {
 /*     0 */   -44,  -62,   85,  -62,  -62,  -62,  -62,  -62,  -62,  -62,
 /*    10 */   -62,  137,  -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,
 /*    20 */   -62,  -62,  -62,  -62,  161,  -62,  -62,  -62,  -62,  -62,
 /*    30 */   -62,  -62,  -62,  122,  -62,  -62,  -62,  -62,  -62,  -62,
 /*    40 */   -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,
 /*    50 */   -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,
 /*    60 */   -62,   70,  -62,  -62,  -62,  128,  -62,  -62,  -62,  -62,
 /*    70 */   -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,
 /*    80 */   -62,  -62,  -62,  -62,  -62,  -62,   41,  -62,  -62,  -62,
 /*    90 */   -62,  -62,  183,  -62,  -62,  189,  -62,  196,  192,  -62,
 /*   100 */   -62,  186,  -62,  -62,   22,  -62,  -62,  -62,  -62,  -62,
 /*   110 */   -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,
 /*   120 */   -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,
 /*   130 */   160,  -62,  -20,  -62,  -62,  -62,  157,  -62,  156,  -62,
 /*   140 */   142,  -62,  141,  -62,  139,  -62,  -62,  -62,  -62,  -62,
 /*   150 */   -62,  -62,  -62,  -62,  -37,  -62,  -62,  -62,  -62,  -45,
 /*   160 */   -62,  -62,  -62,  -62,  -62,  -62,  -62,   57,  -62,  -62,
 /*   170 */   129,  -62,  -62,  -62,  -49,  -62,  -62,  -61,  -62,  -62,
 /*   180 */   111,  -62,  -62,  -62,  -62,  -62,  -62,    5,  -62,  -62,
 /*   190 */   102,   96,   90,   88,  -62,  -62,  -62,  -62,  -62,  -62,
 /*   200 */   -62,  -62,  -62,  -62,  -62,  -62,  -62,  -29,  -62,  -62,
};
static YYACTIONTYPE yy_default[] = {
 /*     0 */   298,  298,  298,  298,  298,  298,  298,  298,  214,  215,
 /*    10 */   298,  298,  298,  298,  298,  298,  298,  298,  298,  298,
 /*    20 */   298,  298,  298,  298,  298,  298,  298,  298,  298,  298,
 /*    30 */   298,  298,  298,  298,  298,  298,  298,  298,  298,  298,
 /*    40 */   298,  298,  298,  298,  298,  298,  298,  298,  298,  298,
 /*    50 */   298,  298,  298,  298,  298,  298,  298,  298,  298,  298,
 /*    60 */   298,  298,  298,  298,  298,  298,  224,  298,  226,  227,
 /*    70 */   228,  229,  230,  231,  232,  233,  234,  235,  236,  237,
 /*    80 */   238,  239,  225,  298,  298,  298,  298,  298,  298,  298,
 /*    90 */   298,  298,  298,  298,  298,  298,  245,  298,  298,  298,
 /*   100 */   258,  298,  259,  260,  291,  261,  262,  263,  271,  298,
 /*   110 */   264,  272,  267,  298,  265,  273,  268,  266,  269,  298,
 /*   120 */   298,  287,  288,  270,  298,  298,  298,  274,  298,  275,
 /*   130 */   298,  298,  291,  298,  276,  292,  298,  279,  298,  280,
 /*   140 */   298,  281,  298,  282,  298,  283,  293,  298,  284,  298,
 /*   150 */   298,  285,  286,  298,  291,  298,  289,  298,  298,  291,
 /*   160 */   298,  290,  278,  277,  298,  298,  298,  298,  298,  298,
 /*   170 */   298,  298,  298,  298,  291,  298,  257,  291,  298,  256,
 /*   180 */   298,  296,  298,  297,  294,  298,  295,  298,  298,  298,
 /*   190 */   244,  246,  249,  298,  298,  298,  298,  298,  298,  298,
 /*   200 */   298,  298,  250,  251,  252,  298,  298,  247,  248,  298,
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
  "ALTER",         "BOOL_TYPE",     "TINYINT_TYPE",  "SMALLINT_TYPE",
  "INT_TYPE",      "BIGINT_TYPE",   "FLOAT_TYPE",    "DOUBLE_TYPE", 
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
 /*  17 */ "cre_column ::= ID TINYINT_TYPE",
 /*  18 */ "cre_column ::= ID SMALLINT_TYPE",
 /*  19 */ "cre_column ::= ID INT_TYPE",
 /*  20 */ "cre_column ::= ID BIGINT_TYPE",
 /*  21 */ "cre_column ::= ID FLOAT_TYPE",
 /*  22 */ "cre_column ::= ID DOUBLE_TYPE",
 /*  23 */ "cre_column ::= ID STRING_TYPE",
 /*  24 */ "cre_column ::= ID BLOB_TYPE",
 /*  25 */ "cre_column ::= ID DATETIME_TYPE",
 /*  26 */ "cre_column ::= ID REAL2_TYPE",
 /*  27 */ "cre_column ::= ID REAL3_TYPE",
 /*  28 */ "cre_column ::= ID REAL4_TYPE",
 /*  29 */ "cre_column ::= ID REAL6_TYPE",
 /*  30 */ "cmd ::= DELETE FROM ID where_opt SEMI",
 /*  31 */ "cmd ::= INSERT INTO ID LP target_list RP VALUES record_list SEMI",
 /*  32 */ "cmd ::= SELECT target_list FROM ID where_opt groupby_opt orderby_opt limit_opt SEMI",
 /*  33 */ "cmd ::= SELECT target_list SEMI",
 /*  34 */ "where_opt ::=",
 /*  35 */ "where_opt ::= WHERE expr_val",
 /*  36 */ "groupby_opt ::=",
 /*  37 */ "groupby_opt ::= GROUP BY ID",
 /*  38 */ "groupby_opt ::= GROUP BY ID expr_val",
 /*  39 */ "orderby_opt ::=",
 /*  40 */ "orderby_opt ::= ORDER BY ID",
 /*  41 */ "orderby_opt ::= ORDER BY ID ASC",
 /*  42 */ "orderby_opt ::= ORDER BY ID DESC",
 /*  43 */ "limit_opt ::=",
 /*  44 */ "limit_opt ::= LIMIT INTEGER",
 /*  45 */ "limit_opt ::= LIMIT INTEGER COMMA INTEGER",
 /*  46 */ "record_list ::= LP expr_val_list RP",
 /*  47 */ "record_list ::= record_list COMMA LP expr_val_list RP",
 /*  48 */ "expr_val ::= LP expr_val RP",
 /*  49 */ "expr_val ::= STAR",
 /*  50 */ "expr_val ::= ID",
 /*  51 */ "expr_val ::= TRUE",
 /*  52 */ "expr_val ::= FALSE",
 /*  53 */ "expr_val ::= INTEGER",
 /*  54 */ "expr_val ::= PLUS INTEGER",
 /*  55 */ "expr_val ::= MINUS INTEGER",
 /*  56 */ "expr_val ::= DOUBLE",
 /*  57 */ "expr_val ::= PLUS DOUBLE",
 /*  58 */ "expr_val ::= MINUS DOUBLE",
 /*  59 */ "expr_val ::= STRING",
 /*  60 */ "expr_val ::= BLOB",
 /*  61 */ "expr_val ::= INTEGER ID",
 /*  62 */ "expr_val ::= PLUS INTEGER ID",
 /*  63 */ "expr_val ::= MINUS INTEGER ID",
 /*  64 */ "expr_val ::= DATETIME_TYPE LP STRING RP",
 /*  65 */ "expr_val ::= ID LP expr_val_list RP",
 /*  66 */ "expr_val ::= ADD LP expr_val_list RP",
 /*  67 */ "expr_val ::= expr_val LT expr_val",
 /*  68 */ "expr_val ::= expr_val LE expr_val",
 /*  69 */ "expr_val ::= expr_val GT expr_val",
 /*  70 */ "expr_val ::= expr_val GE expr_val",
 /*  71 */ "expr_val ::= expr_val EQ expr_val",
 /*  72 */ "expr_val ::= expr_val NE expr_val",
 /*  73 */ "expr_val ::= expr_val AND expr_val",
 /*  74 */ "expr_val ::= ID LIKE STRING",
 /*  75 */ "expr_val ::= ID IS NOT NULL",
 /*  76 */ "expr_val ::= ID IS NULL",
 /*  77 */ "expr_val ::= STRING IS NOT NULL",
 /*  78 */ "expr_val ::= STRING IS NULL",
 /*  79 */ "expr_val ::= ID IN LP expr_val_list RP",
 /*  80 */ "expr_val ::= ID NOT IN LP expr_val_list RP",
 /*  81 */ "expr_val_list ::=",
 /*  82 */ "expr_val_list ::= expr_val",
 /*  83 */ "expr_val_list ::= expr_val_list COMMA expr_val",
 /*  84 */ "target_list ::= expr_val",
 /*  85 */ "target_list ::= expr_val AS ID",
 /*  86 */ "target_list ::= target_list COMMA expr_val",
 /*  87 */ "target_list ::= target_list COMMA expr_val AS ID",
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
    case 85:
#line 113 "parse.y"
{ ColumnList::FreeColumnList((yypminor->yy167)); }
#line 569 "parse.c"
      break;
    case 86:
#line 115 "parse.y"
{ ColumnItem::FreeColumnItem((yypminor->yy114)); }
#line 574 "parse.c"
      break;
    case 87:
#line 170 "parse.y"
{ ExprValue::FreeExprValue((yypminor->yy137)); }
#line 579 "parse.c"
      break;
    case 88:
#line 256 "parse.y"
{ TargetList::FreeTargetList((yypminor->yy120)); }
#line 584 "parse.c"
      break;
    case 89:
#line 202 "parse.y"
{ RecordList::FreeRecordList((yypminor->yy180)); }
#line 589 "parse.c"
      break;
    case 90:
#line 176 "parse.y"
{ delete ((yypminor->yy73)); }
#line 594 "parse.c"
      break;
    case 91:
#line 183 "parse.y"
{ delete ((yypminor->yy177)); }
#line 599 "parse.c"
      break;
    case 92:
#line 191 "parse.y"
{ delete ((yypminor->yy133)); }
#line 604 "parse.c"
      break;
    case 93:
#line 210 "parse.y"
{ ExprValue::FreeExprValue((yypminor->yy137)); }
#line 609 "parse.c"
      break;
    case 94:
#line 249 "parse.y"
{ ExprValueList::FreeExprValueList((yypminor->yy161)); }
#line 614 "parse.c"
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
  { 83, 7 },
  { 83, 4 },
  { 83, 10 },
  { 83, 7 },
  { 84, 1 },
  { 84, 1 },
  { 83, 4 },
  { 83, 4 },
  { 83, 4 },
  { 83, 8 },
  { 83, 6 },
  { 83, 6 },
  { 83, 7 },
  { 83, 7 },
  { 85, 3 },
  { 85, 1 },
  { 86, 2 },
  { 86, 2 },
  { 86, 2 },
  { 86, 2 },
  { 86, 2 },
  { 86, 2 },
  { 86, 2 },
  { 86, 2 },
  { 86, 2 },
  { 86, 2 },
  { 86, 2 },
  { 86, 2 },
  { 86, 2 },
  { 86, 2 },
  { 83, 5 },
  { 83, 9 },
  { 83, 9 },
  { 83, 3 },
  { 87, 0 },
  { 87, 2 },
  { 90, 0 },
  { 90, 3 },
  { 90, 4 },
  { 91, 0 },
  { 91, 3 },
  { 91, 4 },
  { 91, 4 },
  { 92, 0 },
  { 92, 2 },
  { 92, 4 },
  { 89, 3 },
  { 89, 5 },
  { 93, 3 },
  { 93, 1 },
  { 93, 1 },
  { 93, 1 },
  { 93, 1 },
  { 93, 1 },
  { 93, 2 },
  { 93, 2 },
  { 93, 1 },
  { 93, 2 },
  { 93, 2 },
  { 93, 1 },
  { 93, 1 },
  { 93, 2 },
  { 93, 3 },
  { 93, 3 },
  { 93, 4 },
  { 93, 4 },
  { 93, 4 },
  { 93, 3 },
  { 93, 3 },
  { 93, 3 },
  { 93, 3 },
  { 93, 3 },
  { 93, 3 },
  { 93, 3 },
  { 93, 3 },
  { 93, 4 },
  { 93, 3 },
  { 93, 4 },
  { 93, 3 },
  { 93, 5 },
  { 93, 6 },
  { 94, 0 },
  { 94, 1 },
  { 94, 3 },
  { 88, 1 },
  { 88, 3 },
  { 88, 3 },
  { 88, 5 },
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
  pdbAddUser(pParse, &yymsp[-4].minor.yy68, &yymsp[-1].minor.yy0);
}
#line 919 "parse.c"
        /* No destructor defined for ADD */
        /* No destructor defined for USER */
        /* No destructor defined for IDENTIFIED */
        /* No destructor defined for BY */
        /* No destructor defined for SEMI */
        break;
      case 1:
#line 57 "parse.y"
{
  pdbDropUser(pParse, &yymsp[-1].minor.yy68);
}
#line 931 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for USER */
        /* No destructor defined for SEMI */
        break;
      case 2:
#line 62 "parse.y"
{
  pdbChangePwd(pParse, &yymsp[-6].minor.yy68, &yymsp[-2].minor.yy0); 
}
#line 941 "parse.c"
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
  pdbChangeRole(pParse, &yymsp[-3].minor.yy68, &yymsp[-1].minor.yy0);
}
#line 956 "parse.c"
        /* No destructor defined for SET */
        /* No destructor defined for ROLE */
        /* No destructor defined for FOR */
        /* No destructor defined for EQ */
        /* No destructor defined for SEMI */
        break;
      case 4:
#line 71 "parse.y"
{ yygotominor.yy68 = yymsp[0].minor.yy0; }
#line 966 "parse.c"
        break;
      case 5:
#line 72 "parse.y"
{ yygotominor.yy68 = yymsp[0].minor.yy0; }
#line 971 "parse.c"
        break;
      case 6:
#line 75 "parse.y"
{
  pdbDropTable(pParse, &yymsp[-1].minor.yy0);
}
#line 978 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 7:
#line 79 "parse.y"
{
  pdbAttachTable(pParse, &yymsp[-1].minor.yy0);
}
#line 988 "parse.c"
        /* No destructor defined for ATTACH */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 8:
#line 83 "parse.y"
{
  pdbDetachTable(pParse, &yymsp[-1].minor.yy0);
}
#line 998 "parse.c"
        /* No destructor defined for DETACH */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 9:
#line 89 "parse.y"
{
  pdbAttachFile(pParse, &yymsp[-1].minor.yy0, &yymsp[-5].minor.yy0, &yymsp[-3].minor.yy0);
}
#line 1008 "parse.c"
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
#line 1020 "parse.c"
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
#line 1031 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for DATAFILE */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 12:
#line 104 "parse.y"
{
  pdbCreateTable(pParse, &yymsp[-4].minor.yy0, yymsp[-2].minor.yy167);
}
#line 1042 "parse.c"
        /* No destructor defined for CREATE */
        /* No destructor defined for TABLE */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for SEMI */
        break;
      case 13:
#line 108 "parse.y"
{
  pdbAlterTable(pParse, &yymsp[-4].minor.yy0, yymsp[-2].minor.yy167);
}
#line 1054 "parse.c"
        /* No destructor defined for ALTER */
        /* No destructor defined for TABLE */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for SEMI */
        break;
      case 14:
#line 117 "parse.y"
{
  yygotominor.yy167 = ColumnList::AppendColumnItem(yymsp[-2].minor.yy167, yymsp[0].minor.yy114);
}
#line 1066 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 15:
#line 120 "parse.y"
{
  yygotominor.yy167 = ColumnList::AppendColumnItem(nullptr, yymsp[0].minor.yy114);
}
#line 1074 "parse.c"
        break;
      case 16:
#line 124 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_BOOL); }
#line 1079 "parse.c"
        /* No destructor defined for BOOL_TYPE */
        break;
      case 17:
#line 125 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_INT8); }
#line 1085 "parse.c"
        /* No destructor defined for TINYINT_TYPE */
        break;
      case 18:
#line 126 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_INT16); }
#line 1091 "parse.c"
        /* No destructor defined for SMALLINT_TYPE */
        break;
      case 19:
#line 127 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_INT32); }
#line 1097 "parse.c"
        /* No destructor defined for INT_TYPE */
        break;
      case 20:
#line 128 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_INT64); }
#line 1103 "parse.c"
        /* No destructor defined for BIGINT_TYPE */
        break;
      case 21:
#line 129 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_FLOAT); }
#line 1109 "parse.c"
        /* No destructor defined for FLOAT_TYPE */
        break;
      case 22:
#line 130 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_DOUBLE); }
#line 1115 "parse.c"
        /* No destructor defined for DOUBLE_TYPE */
        break;
      case 23:
#line 131 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_STRING); }
#line 1121 "parse.c"
        /* No destructor defined for STRING_TYPE */
        break;
      case 24:
#line 132 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_BLOB); }
#line 1127 "parse.c"
        /* No destructor defined for BLOB_TYPE */
        break;
      case 25:
#line 133 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_DATETIME); }
#line 1133 "parse.c"
        /* No destructor defined for DATETIME_TYPE */
        break;
      case 26:
#line 134 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL2); }
#line 1139 "parse.c"
        /* No destructor defined for REAL2_TYPE */
        break;
      case 27:
#line 135 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL3); }
#line 1145 "parse.c"
        /* No destructor defined for REAL3_TYPE */
        break;
      case 28:
#line 136 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL4); }
#line 1151 "parse.c"
        /* No destructor defined for REAL4_TYPE */
        break;
      case 29:
#line 137 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL6); }
#line 1157 "parse.c"
        /* No destructor defined for REAL6_TYPE */
        break;
      case 30:
#line 143 "parse.y"
{
  pdbDelete(pParse, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy137);
}
#line 1165 "parse.c"
        /* No destructor defined for DELETE */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 31:
#line 149 "parse.y"
{
  pdbInsert(pParse, &yymsp[-6].minor.yy0, yymsp[-4].minor.yy120, yymsp[-1].minor.yy180);
}
#line 1175 "parse.c"
        /* No destructor defined for INSERT */
        /* No destructor defined for INTO */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for VALUES */
        /* No destructor defined for SEMI */
        break;
      case 32:
#line 155 "parse.y"
{
  pdbSelect(pParse, yymsp[-7].minor.yy120, &yymsp[-5].minor.yy0, yymsp[-4].minor.yy137, yymsp[-3].minor.yy73, yymsp[-2].minor.yy177, yymsp[-1].minor.yy133);
}
#line 1188 "parse.c"
        /* No destructor defined for SELECT */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 33:
#line 165 "parse.y"
{
  pdbSelect(pParse, yymsp[-1].minor.yy120, nullptr, nullptr, nullptr, nullptr, nullptr);
}
#line 1198 "parse.c"
        /* No destructor defined for SELECT */
        /* No destructor defined for SEMI */
        break;
      case 34:
#line 172 "parse.y"
{ yygotominor.yy137 = nullptr; }
#line 1205 "parse.c"
        break;
      case 35:
#line 173 "parse.y"
{ yygotominor.yy137 = yymsp[0].minor.yy137; }
#line 1210 "parse.c"
        /* No destructor defined for WHERE */
        break;
      case 36:
#line 178 "parse.y"
{ yygotominor.yy73 = nullptr; }
#line 1216 "parse.c"
        break;
      case 37:
#line 179 "parse.y"
{ yygotominor.yy73 = new GroupOpt(&yymsp[0].minor.yy0); }
#line 1221 "parse.c"
        /* No destructor defined for GROUP */
        /* No destructor defined for BY */
        break;
      case 38:
#line 180 "parse.y"
{ yygotominor.yy73 = new GroupOpt(&yymsp[-1].minor.yy0, yymsp[0].minor.yy137); }
#line 1228 "parse.c"
        /* No destructor defined for GROUP */
        /* No destructor defined for BY */
        break;
      case 39:
#line 185 "parse.y"
{ yygotominor.yy177 = nullptr; }
#line 1235 "parse.c"
        break;
      case 40:
#line 186 "parse.y"
{ yygotominor.yy177 = new OrderByOpt(&yymsp[0].minor.yy0, true); }
#line 1240 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        break;
      case 41:
#line 187 "parse.y"
{ yygotominor.yy177 = new OrderByOpt(&yymsp[-1].minor.yy0, true); }
#line 1247 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        /* No destructor defined for ASC */
        break;
      case 42:
#line 188 "parse.y"
{ yygotominor.yy177 = new OrderByOpt(&yymsp[-1].minor.yy0, false); }
#line 1255 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        /* No destructor defined for DESC */
        break;
      case 43:
#line 193 "parse.y"
{ yygotominor.yy133 = nullptr; }
#line 1263 "parse.c"
        break;
      case 44:
#line 194 "parse.y"
{ yygotominor.yy133 = new LimitOpt(&yymsp[0].minor.yy0); }
#line 1268 "parse.c"
        /* No destructor defined for LIMIT */
        break;
      case 45:
#line 195 "parse.y"
{ yygotominor.yy133 = new LimitOpt(&yymsp[-2].minor.yy0, &yymsp[0].minor.yy0); }
#line 1274 "parse.c"
        /* No destructor defined for LIMIT */
        /* No destructor defined for COMMA */
        break;
      case 46:
#line 205 "parse.y"
{ yygotominor.yy180 = RecordList::AppendRecordList(nullptr, yymsp[-1].minor.yy161); }
#line 1281 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 47:
#line 207 "parse.y"
{ yygotominor.yy180 = RecordList::AppendRecordList(yymsp[-4].minor.yy180, yymsp[-1].minor.yy161); }
#line 1288 "parse.c"
        /* No destructor defined for COMMA */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 48:
#line 212 "parse.y"
{ yygotominor.yy137 = yymsp[-1].minor.yy137; }
#line 1296 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 49:
#line 213 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeStarValue(); }
#line 1303 "parse.c"
        /* No destructor defined for STAR */
        break;
      case 50:
#line 214 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeID(&yymsp[0].minor.yy0); }
#line 1309 "parse.c"
        break;
      case 51:
#line 215 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeBoolValue(true); }
#line 1314 "parse.c"
        /* No destructor defined for TRUE */
        break;
      case 52:
#line 216 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeBoolValue(false); }
#line 1320 "parse.c"
        /* No destructor defined for FALSE */
        break;
      case 53:
#line 217 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIntValue(false, &yymsp[0].minor.yy0); }
#line 1326 "parse.c"
        break;
      case 54:
#line 218 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIntValue(false, &yymsp[0].minor.yy0); }
#line 1331 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 55:
#line 219 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIntValue(true, &yymsp[0].minor.yy0); }
#line 1337 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 56:
#line 220 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeDoubleValue(false, &yymsp[0].minor.yy0); }
#line 1343 "parse.c"
        break;
      case 57:
#line 221 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeDoubleValue(false, &yymsp[0].minor.yy0); }
#line 1348 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 58:
#line 222 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeDoubleValue(true, &yymsp[0].minor.yy0); }
#line 1354 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 59:
#line 223 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeStringValue(&yymsp[0].minor.yy0); }
#line 1360 "parse.c"
        break;
      case 60:
#line 224 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeBlobValue(&yymsp[0].minor.yy0); }
#line 1365 "parse.c"
        break;
      case 61:
#line 225 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeTimeValue(false, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1370 "parse.c"
        break;
      case 62:
#line 226 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeTimeValue(false, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1375 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 63:
#line 227 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeTimeValue(true, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1381 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 64:
#line 228 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeDateTime(&yymsp[-1].minor.yy0); }
#line 1387 "parse.c"
        /* No destructor defined for DATETIME_TYPE */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 65:
#line 230 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeFunction(&yymsp[-3].minor.yy0, yymsp[-1].minor.yy161); }
#line 1395 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 66:
#line 231 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeFunction(&yymsp[-3].minor.yy0, yymsp[-1].minor.yy161); }
#line 1402 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 67:
#line 233 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeCompare(TK_LT, yymsp[-2].minor.yy137, yymsp[0].minor.yy137); }
#line 1409 "parse.c"
        /* No destructor defined for LT */
        break;
      case 68:
#line 234 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeCompare(TK_LE, yymsp[-2].minor.yy137, yymsp[0].minor.yy137); }
#line 1415 "parse.c"
        /* No destructor defined for LE */
        break;
      case 69:
#line 235 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeCompare(TK_GT, yymsp[-2].minor.yy137, yymsp[0].minor.yy137); }
#line 1421 "parse.c"
        /* No destructor defined for GT */
        break;
      case 70:
#line 236 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeCompare(TK_GE, yymsp[-2].minor.yy137, yymsp[0].minor.yy137); }
#line 1427 "parse.c"
        /* No destructor defined for GE */
        break;
      case 71:
#line 237 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeCompare(TK_EQ, yymsp[-2].minor.yy137, yymsp[0].minor.yy137); }
#line 1433 "parse.c"
        /* No destructor defined for EQ */
        break;
      case 72:
#line 238 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeCompare(TK_NE, yymsp[-2].minor.yy137, yymsp[0].minor.yy137); }
#line 1439 "parse.c"
        /* No destructor defined for NE */
        break;
      case 73:
#line 239 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeCompare(TK_AND, yymsp[-2].minor.yy137, yymsp[0].minor.yy137); }
#line 1445 "parse.c"
        /* No destructor defined for AND */
        break;
      case 74:
#line 240 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeLike(&yymsp[-2].minor.yy0, &yymsp[0].minor.yy0); }
#line 1451 "parse.c"
        /* No destructor defined for LIKE */
        break;
      case 75:
#line 241 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIsNotNull(&yymsp[-3].minor.yy0); }
#line 1457 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NOT */
        /* No destructor defined for NULL */
        break;
      case 76:
#line 242 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIsNull(&yymsp[-2].minor.yy0); }
#line 1465 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NULL */
        break;
      case 77:
#line 243 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIsNotNull(&yymsp[-3].minor.yy0); }
#line 1472 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NOT */
        /* No destructor defined for NULL */
        break;
      case 78:
#line 244 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIsNull(&yymsp[-2].minor.yy0); }
#line 1480 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NULL */
        break;
      case 79:
#line 245 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIn(&yymsp[-4].minor.yy0, yymsp[-1].minor.yy161); }
#line 1487 "parse.c"
        /* No destructor defined for IN */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 80:
#line 246 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeNotIn(&yymsp[-5].minor.yy0, yymsp[-1].minor.yy161); }
#line 1495 "parse.c"
        /* No destructor defined for NOT */
        /* No destructor defined for IN */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 81:
#line 251 "parse.y"
{ yygotominor.yy161 = nullptr; }
#line 1504 "parse.c"
        break;
      case 82:
#line 252 "parse.y"
{ yygotominor.yy161 = ExprValueList::AppendExprValue(nullptr, yymsp[0].minor.yy137); }
#line 1509 "parse.c"
        break;
      case 83:
#line 253 "parse.y"
{ yygotominor.yy161 = ExprValueList::AppendExprValue(yymsp[-2].minor.yy161, yymsp[0].minor.yy137); }
#line 1514 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 84:
#line 258 "parse.y"
{ yygotominor.yy120 = TargetList::AppendExprValue(nullptr, yymsp[0].minor.yy137, nullptr); }
#line 1520 "parse.c"
        break;
      case 85:
#line 259 "parse.y"
{ yygotominor.yy120 = TargetList::AppendExprValue(nullptr, yymsp[-2].minor.yy137, &yymsp[0].minor.yy0); }
#line 1525 "parse.c"
        /* No destructor defined for AS */
        break;
      case 86:
#line 260 "parse.y"
{ yygotominor.yy120 = TargetList::AppendExprValue(yymsp[-2].minor.yy120, yymsp[0].minor.yy137, nullptr); }
#line 1531 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 87:
#line 261 "parse.y"
{ yygotominor.yy120 = TargetList::AppendExprValue(yymsp[-4].minor.yy120, yymsp[-2].minor.yy137, &yymsp[0].minor.yy0); }
#line 1537 "parse.c"
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

#line 1586 "parse.c"
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
