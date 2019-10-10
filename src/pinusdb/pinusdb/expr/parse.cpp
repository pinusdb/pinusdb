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
#define YYNOCODE 99
#define YYACTIONTYPE unsigned short int
#define pdbParseTOKENTYPE Token
typedef union {
  pdbParseTOKENTYPE yy0;
  GroupOpt* yy11;
  Token yy14;
  LimitOpt* yy17;
  OrderByOpt* yy41;
  ExprList* yy70;
  ColumnList* yy97;
  ExprItem* yy98;
  RecordList* yy114;
  ColumnItem* yy124;
  int yy197;
} YYMINORTYPE;
#define YYSTACKDEPTH 100
#define pdbParseARG_SDECL SQLParser *pParse;
#define pdbParseARG_PDECL ,SQLParser *pParse
#define pdbParseARG_FETCH SQLParser *pParse = yypParser->pParse
#define pdbParseARG_STORE yypParser->pParse = pParse
#define YYNSTATE 217
#define YYNRULE 88
#define YYERRORSYMBOL 80
#define YYERRSYMDT yy197
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
 /*     0 */   135,  110,   68,   69,   70,   71,   72,   73,   74,   75,
 /*    10 */    76,   77,   34,  107,   92,  157,  119,  185,  115,  113,
 /*    20 */   114,   35,  260,   94,  116,  115,  113,  219,  128,  181,
 /*    30 */   111,  121,  124,  216,  137,  131,  133,   95,  129,  139,
 /*    40 */   141,  152,  163,  145,  269,   86,  107,  179,  196,  180,
 /*    50 */   112,   99,   23,   91,   36,    1,  109,   32,  119,  118,
 /*    60 */   113,   10,   22,   97,   98,  100,  101,  104,   11,  105,
 /*    70 */   108,   38,   49,  121,  124,   50,   58,   79,   53,  114,
 /*    80 */   203,  114,   14,  106,   99,   17,  127,  189,  194,  111,
 /*    90 */   158,  111,    8,  117,   84,   37,   97,   98,  100,  101,
 /*   100 */   104,   65,    9,  108,  142,  144,   83,   78,  183,  112,
 /*   110 */   192,  112,  162,  157,  151,  147,  148,  119,  156,  119,
 /*   120 */   210,  211,   63,  149,  160,  149,   62,   78,  102,   65,
 /*   130 */   168,  167,  121,  124,  121,  124,  193,  179,  195,  184,
 /*   140 */   151,  174,  103,  193,  175,  178,  151,  177,  270,  186,
 /*   150 */    39,  149,  149,   42,  151,  155,  117,  171,  187,  220,
 /*   160 */    31,  188,  205,  182,   40,   30,   41,  224,  206,  190,
 /*   170 */    43,  172,   29,   44,  191,   45,   46,  271,  173,   47,
 /*   180 */   244,   48,  208,  226,  170,   28,  176,   51,  169,   52,
 /*   190 */    27,  225,   54,   26,   25,   56,   57,  166,   55,  164,
 /*   200 */   227,  165,  161,  159,   59,   61,  209,   60,  217,   67,
 /*   210 */   197,   24,   64,  228,  229,  198,   91,   66,   21,  154,
 /*   220 */     7,    6,  153,  150,  213,  146,   20,  212,  199,    4,
 /*   230 */   214,    5,  223,   18,    3,  140,  126,  138,   80,   19,
 /*   240 */   136,  134,  132,  207,  123,  125,  122,   81,   82,  143,
 /*   250 */   120,   85,   16,  230,   87,   15,   88,  202,  218,   90,
 /*   260 */   110,  243,  246,  245,   13,    2,   33,  188,   93,  188,
 /*   270 */   188,  188,  188,  188,  188,  188,  188,  200,  130,  188,
 /*   280 */   188,  204,  201,  188,  188,  188,  188,   89,  188,  188,
 /*   290 */   188,  188,  188,  188,  188,  188,  188,  188,  188,   12,
 /*   300 */   215,  306,  188,  188,  188,  188,  188,  188,  188,  188,
 /*   310 */   188,  188,  188,  188,  188,  188,  188,  188,   96,
};
static YYCODETYPE yy_lookahead[] = {
 /*     0 */    28,   29,   41,   42,   43,   44,   45,   46,   47,   48,
 /*    10 */    49,   50,   82,   22,   95,   96,   60,   92,   93,   94,
 /*    20 */    22,   28,   23,   32,   92,   93,   94,    0,   30,   32,
 /*    30 */    32,   75,   76,   23,   62,   63,   64,   65,   66,   67,
 /*    40 */    68,   69,    5,   71,   23,    8,   22,   37,   38,   52,
 /*    50 */    52,   60,   26,   54,   32,   18,   32,   31,   60,   93,
 /*    60 */    94,   24,   25,   72,   73,   74,   75,   76,   19,   60,
 /*    70 */    79,   34,   35,   75,   76,   33,   39,   40,   36,   22,
 /*    80 */    59,   22,   33,   74,   60,   36,   30,   30,   51,   32,
 /*    90 */    28,   32,   22,   37,   30,   23,   72,   73,   74,   75,
 /*   100 */    76,   37,   32,   79,   69,   70,   83,   84,   32,   52,
 /*   110 */    91,   52,   95,   96,   96,   97,   30,   60,   30,   60,
 /*   120 */    57,   58,   30,   37,   62,   37,   83,   84,   60,   37,
 /*   130 */    30,   86,   75,   76,   75,   76,   91,   37,   86,   29,
 /*   140 */    96,   97,   74,   91,   30,   30,   96,   97,   23,   30,
 /*   150 */    33,   37,   37,   36,   96,   97,   37,   23,   53,    0,
 /*   160 */    23,   32,   37,   53,   32,   30,   23,    0,   60,   53,
 /*   170 */    22,   37,   22,   37,   32,   22,   38,   23,   29,   32,
 /*   180 */     0,   23,   21,    0,   87,   29,   29,   32,    7,   23,
 /*   190 */    26,    0,   22,   28,   82,   32,   23,   29,   38,    6,
 /*   200 */     0,   32,   96,   96,   33,   29,   32,   32,    0,   32,
 /*   210 */    32,   27,   23,    0,    0,   85,   54,   84,   23,   29,
 /*   220 */    23,   22,   71,   96,   21,   29,   32,   55,   88,   20,
 /*   230 */    32,   21,    0,   22,   82,   96,   32,   96,   33,   38,
 /*   240 */    96,   96,   96,   56,   32,   60,   60,   32,   29,   70,
 /*   250 */    32,   23,   23,    0,   38,   32,   32,   23,    0,   23,
 /*   260 */    29,    0,    0,    0,   23,   19,   27,   98,   61,   98,
 /*   270 */    98,   98,   98,   98,   98,   98,   98,   89,   96,   98,
 /*   280 */    98,   60,   90,   98,   98,   98,   98,   85,   98,   98,
 /*   290 */    98,   98,   98,   98,   98,   98,   98,   98,   98,   82,
 /*   300 */    94,   81,   98,   98,   98,   98,   98,   98,   98,   98,
 /*   310 */    98,   98,   98,   98,   98,   98,   98,   98,   96,
};
#define YY_SHIFT_USE_DFLT (-45)
static short yy_shift_ofst[] = {
 /*     0 */    37,  246,   70,  209,  210,  199,  197,  208,  -45,  -45,
 /*    10 */    49,   70,  241,  258,  223,  229,  232,  211,  201,  194,
 /*    20 */   195,  213,   26,  184,   70,  165,  164,  156,  150,  135,
 /*    30 */   137,   27,  239,   70,   -7,   22,   72,  159,  117,  132,
 /*    40 */   143,  167,  148,  136,  153,  138,  147,  158,  183,   42,
 /*    50 */   155,  166,  191,  170,  160,  163,  173,  200,  171,  175,
 /*    60 */   176,  177,   92,  189,  214,  177,  -45,  -39,  -45,  -45,
 /*    70 */   -45,  -45,  -45,  -45,  -45,  -45,  -45,  -45,  -45,  205,
 /*    80 */   215,  219,  177,   64,  228,  253,  216,  224,   -1,  236,
 /*    90 */   261,   -9,  207,   -9,  -28,   24,  -45,  -45,  -45,  -45,
 /*   100 */   -45,   68,  -45,  -45,    9,  -45,  -45,  -45,  -45,  231,
 /*   110 */    -2,  -45,  -45,  -45,  -45,  -45,   56,   59,  -45,  218,
 /*   120 */   -45,  186,  212,  -45,  185,  204,  -45,  -45,  -45,   24,
 /*   130 */   -45,   24,  -45,   24,  -45,   24,  -45,   24,  -45,   24,
 /*   140 */   -45,   35,  179,  -45,  -45,  196,   24,   86,  -45,   24,
 /*   150 */   -45,  -45,  151,  190,   24,   88,  -45,   62,   24,  -45,
 /*   160 */    24,  -45,  -45,  193,  169,  168,   -3,  100,  181,  157,
 /*   170 */   134,  180,  149,   24,  114,  -45,   24,  115,  -45,   -3,
 /*   180 */   -45,  110,   76,  -45,   57,  119,  105,  129,  -45,  116,
 /*   190 */   142,  -45,  -45,  -45,   -3,   10,  178,  162,  172,  187,
 /*   200 */    21,  234,  263,  221,  125,  108,  154,  161,  174,   63,
 /*   210 */   -45,  -45,  203,  198,  -44,  -45,  262,
};
#define YY_REDUCE_USE_DFLT (-82)
static short yy_reduce_ofst[] = {
 /*     0 */   220,  -82,  152,  -82,  -82,  -82,  -82,  -82,  -82,  -82,
 /*    10 */   -82,  217,  -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,
 /*    20 */   -82,  -82,  -82,  -82,  112,  -82,  -82,  -82,  -82,  -82,
 /*    30 */   -82,  -82,  -82,  -70,  -82,  -82,  -82,  -82,  -82,  -82,
 /*    40 */   -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,
 /*    50 */   -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,
 /*    60 */   -82,   43,  -82,  -82,  -82,  133,  -82,  -82,  -82,  -82,
 /*    70 */   -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,
 /*    80 */   -82,  -82,   23,  -82,  -82,  -82,  -82,  -82,  202,  -82,
 /*    90 */   -82,  -81,  -82,   17,  -82,  222,  -82,  -82,  -82,  -82,
 /*   100 */   -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,
 /*   110 */   -68,  -82,  -82,  -82,  -82,  -82,  -82,  -34,  -82,  -82,
 /*   120 */   -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,  182,
 /*   130 */   -82,  146,  -82,  145,  -82,  144,  -82,  141,  -82,  139,
 /*   140 */   -82,  -82,  -82,  -82,  -82,  -82,   18,  -82,  -82,  127,
 /*   150 */   -82,  -82,  -82,  -82,   58,  -82,  -82,  -82,  107,  -82,
 /*   160 */   106,  -82,  -82,  -82,  -82,  -82,   45,  -82,  -82,   97,
 /*   170 */   -82,  -82,  -82,   44,  -82,  -82,   50,  -82,  -82,   19,
 /*   180 */   -82,  -82,  -82,  -82,  -75,  -82,  -82,  -82,  -82,  -82,
 /*   190 */   -82,  -82,  -82,  -82,   52,  -82,  -82,  130,  140,  188,
 /*   200 */   192,  -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,  -82,
 /*   210 */   -82,  -82,  -82,  -82,  206,  -82,  -82,
};
static YYACTIONTYPE yy_default[] = {
 /*     0 */   305,  305,  305,  305,  305,  305,  305,  305,  221,  222,
 /*    10 */   305,  305,  305,  305,  305,  305,  305,  305,  305,  305,
 /*    20 */   305,  305,  305,  305,  305,  305,  305,  305,  305,  305,
 /*    30 */   305,  305,  305,  305,  305,  305,  305,  305,  305,  305,
 /*    40 */   305,  305,  305,  305,  305,  305,  305,  305,  305,  305,
 /*    50 */   305,  305,  305,  305,  305,  305,  305,  305,  305,  305,
 /*    60 */   305,  305,  305,  305,  305,  305,  231,  305,  233,  234,
 /*    70 */   235,  236,  237,  238,  239,  240,  241,  242,  232,  305,
 /*    80 */   305,  305,  305,  305,  305,  305,  305,  305,  305,  305,
 /*    90 */   305,  305,  261,  305,  305,  305,  272,  286,  287,  288,
 /*   100 */   289,  305,  290,  291,  305,  292,  293,  294,  295,  305,
 /*   110 */   305,  254,  255,  256,  257,  258,  305,  305,  259,  305,
 /*   120 */   298,  305,  305,  299,  305,  305,  300,  296,  297,  305,
 /*   130 */   273,  305,  274,  305,  275,  305,  276,  305,  277,  305,
 /*   140 */   278,  305,  305,  279,  280,  305,  305,  305,  283,  305,
 /*   150 */   302,  301,  305,  305,  305,  305,  284,  305,  305,  281,
 /*   160 */   305,  282,  285,  305,  305,  305,  305,  305,  305,  305,
 /*   170 */   305,  305,  305,  305,  305,  304,  305,  305,  303,  305,
 /*   180 */   247,  248,  305,  249,  305,  305,  305,  305,  250,  305,
 /*   190 */   305,  251,  253,  252,  305,  305,  305,  260,  262,  265,
 /*   200 */   305,  305,  305,  305,  305,  305,  305,  305,  305,  266,
 /*   210 */   267,  268,  305,  305,  263,  264,  305,
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
  "STAR",          "AS",            "WHERE",         "GROUP",       
  "ORDER",         "ASC",           "DESC",          "LIMIT",       
  "INTEGER",       "AND",           "NE",            "GT",          
  "GE",            "LT",            "LE",            "LIKE",        
  "IS",            "NOT",           "NULL",          "IN",          
  "TRUE",          "FALSE",         "DOUBLE",        "PLUS",        
  "MINUS",         "UINTEGER",      "UDOUBLE",       "BLOB",        
  "error",         "cmd",           "username",      "cre_columnlist",
  "cre_column",    "where_opt",     "target_list",   "record_list", 
  "groupby_opt",   "orderby_opt",   "limit_opt",     "target_item", 
  "arg_list",      "arg_item",      "timeval",       "condi_expr",  
  "userval",       "userval_list",
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
 /*  30 */ "target_item ::= STAR",
 /*  31 */ "target_item ::= ID",
 /*  32 */ "target_item ::= ID AS ID",
 /*  33 */ "target_item ::= ID LP arg_list RP AS ID",
 /*  34 */ "target_item ::= ID LP RP AS ID",
 /*  35 */ "target_list ::= target_item",
 /*  36 */ "target_list ::= target_list COMMA target_item",
 /*  37 */ "arg_item ::= ID",
 /*  38 */ "arg_item ::= STAR",
 /*  39 */ "arg_item ::= timeval",
 /*  40 */ "arg_item ::= STRING",
 /*  41 */ "arg_list ::= arg_item",
 /*  42 */ "arg_list ::= arg_list COMMA arg_item",
 /*  43 */ "where_opt ::=",
 /*  44 */ "where_opt ::= WHERE condi_expr",
 /*  45 */ "groupby_opt ::=",
 /*  46 */ "groupby_opt ::= GROUP BY ID",
 /*  47 */ "groupby_opt ::= GROUP BY ID timeval",
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
 /*  62 */ "condi_expr ::= ID IS NOT NULL",
 /*  63 */ "condi_expr ::= ID IS NULL",
 /*  64 */ "condi_expr ::= userval EQ userval",
 /*  65 */ "condi_expr ::= userval NE userval",
 /*  66 */ "condi_expr ::= ID IN LP userval_list RP",
 /*  67 */ "condi_expr ::= ID NOT IN LP userval_list RP",
 /*  68 */ "condi_expr ::= condi_expr AND condi_expr",
 /*  69 */ "userval ::= TRUE",
 /*  70 */ "userval ::= FALSE",
 /*  71 */ "userval ::= INTEGER",
 /*  72 */ "userval ::= DOUBLE",
 /*  73 */ "userval ::= PLUS INTEGER",
 /*  74 */ "userval ::= PLUS DOUBLE",
 /*  75 */ "userval ::= MINUS INTEGER",
 /*  76 */ "userval ::= MINUS DOUBLE",
 /*  77 */ "userval ::= STRING",
 /*  78 */ "userval ::= BLOB",
 /*  79 */ "userval ::= ID LP arg_list RP",
 /*  80 */ "userval ::= ID LP RP",
 /*  81 */ "timeval ::= INTEGER ID",
 /*  82 */ "timeval ::= PLUS INTEGER ID",
 /*  83 */ "timeval ::= MINUS INTEGER ID",
 /*  84 */ "userval_list ::= userval",
 /*  85 */ "userval_list ::= userval_list COMMA userval",
 /*  86 */ "record_list ::= LP userval_list RP",
 /*  87 */ "record_list ::= record_list COMMA LP userval_list RP",
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
    case 83:
#line 109 "parse.y"
{ ColumnList::FreeColumnList((yypminor->yy97)); }
#line 574 "parse.c"
      break;
    case 84:
#line 111 "parse.y"
{ ColumnItem::FreeColumnItem((yypminor->yy124)); }
#line 579 "parse.c"
      break;
    case 85:
#line 190 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy98)); }
#line 584 "parse.c"
      break;
    case 86:
#line 157 "parse.y"
{ ExprList::FreeExprList((yypminor->yy70)); }
#line 589 "parse.c"
      break;
    case 87:
#line 275 "parse.y"
{ RecordList::FreeRecordList((yypminor->yy114)); }
#line 594 "parse.c"
      break;
    case 88:
#line 196 "parse.y"
{ delete ((yypminor->yy11)); }
#line 599 "parse.c"
      break;
    case 89:
#line 203 "parse.y"
{ delete ((yypminor->yy41)); }
#line 604 "parse.c"
      break;
    case 90:
#line 211 "parse.y"
{ delete ((yypminor->yy17)); }
#line 609 "parse.c"
      break;
    case 91:
#line 159 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy98)); }
#line 614 "parse.c"
      break;
    case 92:
#line 177 "parse.y"
{ ExprList::FreeExprList((yypminor->yy70)); }
#line 619 "parse.c"
      break;
    case 93:
#line 175 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy98)); }
#line 624 "parse.c"
      break;
    case 94:
#line 260 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy98));}
#line 629 "parse.c"
      break;
    case 95:
#line 224 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy98)); }
#line 634 "parse.c"
      break;
    case 96:
#line 244 "parse.y"
{ ExprItem::FreeExprItem((yypminor->yy98)); }
#line 639 "parse.c"
      break;
    case 97:
#line 267 "parse.y"
{ ExprList::FreeExprList((yypminor->yy70)); }
#line 644 "parse.c"
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
  { 81, 7 },
  { 81, 4 },
  { 81, 10 },
  { 81, 7 },
  { 82, 1 },
  { 82, 1 },
  { 81, 4 },
  { 81, 4 },
  { 81, 4 },
  { 81, 8 },
  { 81, 6 },
  { 81, 6 },
  { 81, 7 },
  { 81, 7 },
  { 83, 3 },
  { 83, 1 },
  { 84, 2 },
  { 84, 2 },
  { 84, 2 },
  { 84, 2 },
  { 84, 2 },
  { 84, 2 },
  { 84, 2 },
  { 84, 2 },
  { 84, 2 },
  { 84, 2 },
  { 81, 5 },
  { 81, 9 },
  { 81, 9 },
  { 81, 3 },
  { 91, 1 },
  { 91, 1 },
  { 91, 3 },
  { 91, 6 },
  { 91, 5 },
  { 86, 1 },
  { 86, 3 },
  { 93, 1 },
  { 93, 1 },
  { 93, 1 },
  { 93, 1 },
  { 92, 1 },
  { 92, 3 },
  { 85, 0 },
  { 85, 2 },
  { 88, 0 },
  { 88, 3 },
  { 88, 4 },
  { 89, 0 },
  { 89, 3 },
  { 89, 4 },
  { 89, 4 },
  { 90, 0 },
  { 90, 2 },
  { 90, 4 },
  { 95, 3 },
  { 95, 3 },
  { 95, 3 },
  { 95, 3 },
  { 95, 3 },
  { 95, 3 },
  { 95, 3 },
  { 95, 4 },
  { 95, 3 },
  { 95, 3 },
  { 95, 3 },
  { 95, 5 },
  { 95, 6 },
  { 95, 3 },
  { 96, 1 },
  { 96, 1 },
  { 96, 1 },
  { 96, 1 },
  { 96, 2 },
  { 96, 2 },
  { 96, 2 },
  { 96, 2 },
  { 96, 1 },
  { 96, 1 },
  { 96, 4 },
  { 96, 3 },
  { 94, 2 },
  { 94, 3 },
  { 94, 3 },
  { 97, 1 },
  { 97, 3 },
  { 87, 3 },
  { 87, 5 },
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
  pdbAddUser(pParse, &yymsp[-4].minor.yy14, &yymsp[-1].minor.yy0);
}
#line 949 "parse.c"
        /* No destructor defined for ADD */
        /* No destructor defined for USER */
        /* No destructor defined for IDENTIFIED */
        /* No destructor defined for BY */
        /* No destructor defined for SEMI */
        break;
      case 1:
#line 53 "parse.y"
{
  pdbDropUser(pParse, &yymsp[-1].minor.yy14);
}
#line 961 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for USER */
        /* No destructor defined for SEMI */
        break;
      case 2:
#line 58 "parse.y"
{
  pdbChangePwd(pParse, &yymsp[-6].minor.yy14, &yymsp[-2].minor.yy0); 
}
#line 971 "parse.c"
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
  pdbChangeRole(pParse, &yymsp[-3].minor.yy14, &yymsp[-1].minor.yy0);
}
#line 986 "parse.c"
        /* No destructor defined for SET */
        /* No destructor defined for ROLE */
        /* No destructor defined for FOR */
        /* No destructor defined for EQ */
        /* No destructor defined for SEMI */
        break;
      case 4:
#line 67 "parse.y"
{ yygotominor.yy14 = yymsp[0].minor.yy0; }
#line 996 "parse.c"
        break;
      case 5:
#line 68 "parse.y"
{ yygotominor.yy14 = yymsp[0].minor.yy0; }
#line 1001 "parse.c"
        break;
      case 6:
#line 71 "parse.y"
{
  pdbDropTable(pParse, &yymsp[-1].minor.yy0);
}
#line 1008 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 7:
#line 75 "parse.y"
{
  pdbAttachTable(pParse, &yymsp[-1].minor.yy0);
}
#line 1018 "parse.c"
        /* No destructor defined for ATTACH */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 8:
#line 79 "parse.y"
{
  pdbDetachTable(pParse, &yymsp[-1].minor.yy0);
}
#line 1028 "parse.c"
        /* No destructor defined for DETACH */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 9:
#line 85 "parse.y"
{
  pdbAttachFile(pParse, &yymsp[-1].minor.yy0, &yymsp[-5].minor.yy0, &yymsp[-3].minor.yy0);
}
#line 1038 "parse.c"
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
#line 1050 "parse.c"
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
#line 1061 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for DATAFILE */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 12:
#line 100 "parse.y"
{
  pdbCreateTable(pParse, &yymsp[-4].minor.yy0, yymsp[-2].minor.yy97);
}
#line 1072 "parse.c"
        /* No destructor defined for CREATE */
        /* No destructor defined for TABLE */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for SEMI */
        break;
      case 13:
#line 104 "parse.y"
{
  pdbAlterTable(pParse, &yymsp[-4].minor.yy0, yymsp[-2].minor.yy97);
}
#line 1084 "parse.c"
        /* No destructor defined for ALTER */
        /* No destructor defined for TABLE */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for SEMI */
        break;
      case 14:
#line 113 "parse.y"
{
  yygotominor.yy97 = ColumnList::AppendColumnItem(yymsp[-2].minor.yy97, yymsp[0].minor.yy124);
}
#line 1096 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 15:
#line 116 "parse.y"
{
  yygotominor.yy97 = ColumnList::AppendColumnItem(nullptr, yymsp[0].minor.yy124);
}
#line 1104 "parse.c"
        break;
      case 16:
#line 120 "parse.y"
{ yygotominor.yy124 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_BOOL); }
#line 1109 "parse.c"
        /* No destructor defined for BOOL_TYPE */
        break;
      case 17:
#line 121 "parse.y"
{ yygotominor.yy124 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_INT64); }
#line 1115 "parse.c"
        /* No destructor defined for BIGINT_TYPE */
        break;
      case 18:
#line 122 "parse.y"
{ yygotominor.yy124 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_DOUBLE); }
#line 1121 "parse.c"
        /* No destructor defined for DOUBLE_TYPE */
        break;
      case 19:
#line 123 "parse.y"
{ yygotominor.yy124 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_STRING); }
#line 1127 "parse.c"
        /* No destructor defined for STRING_TYPE */
        break;
      case 20:
#line 124 "parse.y"
{ yygotominor.yy124 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_BLOB); }
#line 1133 "parse.c"
        /* No destructor defined for BLOB_TYPE */
        break;
      case 21:
#line 125 "parse.y"
{ yygotominor.yy124 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_DATETIME); }
#line 1139 "parse.c"
        /* No destructor defined for DATETIME_TYPE */
        break;
      case 22:
#line 126 "parse.y"
{ yygotominor.yy124 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL2); }
#line 1145 "parse.c"
        /* No destructor defined for REAL2_TYPE */
        break;
      case 23:
#line 127 "parse.y"
{ yygotominor.yy124 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL3); }
#line 1151 "parse.c"
        /* No destructor defined for REAL3_TYPE */
        break;
      case 24:
#line 128 "parse.y"
{ yygotominor.yy124 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL4); }
#line 1157 "parse.c"
        /* No destructor defined for REAL4_TYPE */
        break;
      case 25:
#line 129 "parse.y"
{ yygotominor.yy124 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL6); }
#line 1163 "parse.c"
        /* No destructor defined for REAL6_TYPE */
        break;
      case 26:
#line 135 "parse.y"
{
  pdbDelete(pParse, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy98);
}
#line 1171 "parse.c"
        /* No destructor defined for DELETE */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 27:
#line 141 "parse.y"
{
  pdbInsert(pParse, &yymsp[-6].minor.yy0, yymsp[-4].minor.yy70, yymsp[-1].minor.yy114);
}
#line 1181 "parse.c"
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
  pdbSelect(pParse, yymsp[-7].minor.yy70, &yymsp[-5].minor.yy0, yymsp[-4].minor.yy98, yymsp[-3].minor.yy11, yymsp[-2].minor.yy41, yymsp[-1].minor.yy17);
}
#line 1194 "parse.c"
        /* No destructor defined for SELECT */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 29:
#line 152 "parse.y"
{
  pdbSelect(pParse, yymsp[-1].minor.yy70, nullptr, nullptr, nullptr, nullptr, nullptr);
}
#line 1204 "parse.c"
        /* No destructor defined for SELECT */
        /* No destructor defined for SEMI */
        break;
      case 30:
#line 161 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_STAR, &yymsp[0].minor.yy0); }
#line 1211 "parse.c"
        break;
      case 31:
#line 162 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_ID, &yymsp[0].minor.yy0); }
#line 1216 "parse.c"
        break;
      case 32:
#line 163 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_ID, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy0); }
#line 1221 "parse.c"
        /* No destructor defined for AS */
        break;
      case 33:
#line 164 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeFunction(TK_FUNCTION, &yymsp[-5].minor.yy0, yymsp[-3].minor.yy70, &yymsp[0].minor.yy0); }
#line 1227 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for AS */
        break;
      case 34:
#line 165 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeFunction(TK_FUNCTION, &yymsp[-4].minor.yy0, nullptr, &yymsp[0].minor.yy0); }
#line 1235 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        /* No destructor defined for AS */
        break;
      case 35:
#line 167 "parse.y"
{
  yygotominor.yy70 = ExprList::AppendExprItem(nullptr, yymsp[0].minor.yy98);
}
#line 1245 "parse.c"
        break;
      case 36:
#line 170 "parse.y"
{
  yygotominor.yy70 = ExprList::AppendExprItem(yymsp[-2].minor.yy70, yymsp[0].minor.yy98);
}
#line 1252 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 37:
#line 179 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_ID, &yymsp[0].minor.yy0); }
#line 1258 "parse.c"
        break;
      case 38:
#line 180 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_STAR, &yymsp[0].minor.yy0); }
#line 1263 "parse.c"
        break;
      case 39:
#line 181 "parse.y"
{ yygotominor.yy98 = yymsp[0].minor.yy98; }
#line 1268 "parse.c"
        break;
      case 40:
#line 182 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_STRING, &yymsp[0].minor.yy0); }
#line 1273 "parse.c"
        break;
      case 41:
#line 185 "parse.y"
{ yygotominor.yy70 = ExprList::AppendExprItem(nullptr, yymsp[0].minor.yy98); }
#line 1278 "parse.c"
        break;
      case 42:
#line 187 "parse.y"
{ yygotominor.yy70 = ExprList::AppendExprItem(yymsp[-2].minor.yy70, yymsp[0].minor.yy98); }
#line 1283 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 43:
#line 192 "parse.y"
{ yygotominor.yy98 = nullptr; }
#line 1289 "parse.c"
        break;
      case 44:
#line 193 "parse.y"
{ yygotominor.yy98 = yymsp[0].minor.yy98; }
#line 1294 "parse.c"
        /* No destructor defined for WHERE */
        break;
      case 45:
#line 198 "parse.y"
{ yygotominor.yy11 = nullptr; }
#line 1300 "parse.c"
        break;
      case 46:
#line 199 "parse.y"
{ yygotominor.yy11 = new GroupOpt(&yymsp[0].minor.yy0); }
#line 1305 "parse.c"
        /* No destructor defined for GROUP */
        /* No destructor defined for BY */
        break;
      case 47:
#line 200 "parse.y"
{ yygotominor.yy11 = new GroupOpt(&yymsp[-1].minor.yy0, yymsp[0].minor.yy98); }
#line 1312 "parse.c"
        /* No destructor defined for GROUP */
        /* No destructor defined for BY */
        break;
      case 48:
#line 205 "parse.y"
{ yygotominor.yy41 = nullptr; }
#line 1319 "parse.c"
        break;
      case 49:
#line 206 "parse.y"
{ yygotominor.yy41 = new OrderByOpt(&yymsp[0].minor.yy0, true); }
#line 1324 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        break;
      case 50:
#line 207 "parse.y"
{ yygotominor.yy41 = new OrderByOpt(&yymsp[-1].minor.yy0, true); }
#line 1331 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        /* No destructor defined for ASC */
        break;
      case 51:
#line 208 "parse.y"
{ yygotominor.yy41 = new OrderByOpt(&yymsp[-1].minor.yy0, false); }
#line 1339 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        /* No destructor defined for DESC */
        break;
      case 52:
#line 213 "parse.y"
{ yygotominor.yy17 = nullptr; }
#line 1347 "parse.c"
        break;
      case 53:
#line 214 "parse.y"
{ yygotominor.yy17 = new LimitOpt(&yymsp[0].minor.yy0); }
#line 1352 "parse.c"
        /* No destructor defined for LIMIT */
        break;
      case 54:
#line 215 "parse.y"
{ yygotominor.yy17 = new LimitOpt(&yymsp[-2].minor.yy0, &yymsp[0].minor.yy0); }
#line 1358 "parse.c"
        /* No destructor defined for LIMIT */
        /* No destructor defined for COMMA */
        break;
      case 55:
#line 226 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeCondition(TK_LT, &yymsp[-2].minor.yy0, yymsp[0].minor.yy98); }
#line 1365 "parse.c"
        /* No destructor defined for LT */
        break;
      case 56:
#line 227 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeCondition(TK_LE, &yymsp[-2].minor.yy0, yymsp[0].minor.yy98); }
#line 1371 "parse.c"
        /* No destructor defined for LE */
        break;
      case 57:
#line 228 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeCondition(TK_GT, &yymsp[-2].minor.yy0, yymsp[0].minor.yy98); }
#line 1377 "parse.c"
        /* No destructor defined for GT */
        break;
      case 58:
#line 229 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeCondition(TK_GE, &yymsp[-2].minor.yy0, yymsp[0].minor.yy98); }
#line 1383 "parse.c"
        /* No destructor defined for GE */
        break;
      case 59:
#line 230 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeCondition(TK_EQ, &yymsp[-2].minor.yy0, yymsp[0].minor.yy98); }
#line 1389 "parse.c"
        /* No destructor defined for EQ */
        break;
      case 60:
#line 231 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeCondition(TK_NE, &yymsp[-2].minor.yy0, yymsp[0].minor.yy98); }
#line 1395 "parse.c"
        /* No destructor defined for NE */
        break;
      case 61:
#line 232 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeCondition(TK_LIKE, &yymsp[-2].minor.yy0, yymsp[0].minor.yy98); }
#line 1401 "parse.c"
        /* No destructor defined for LIKE */
        break;
      case 62:
#line 233 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeCondition(TK_ISNOTNULL, &yymsp[-3].minor.yy0, nullptr); }
#line 1407 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NOT */
        /* No destructor defined for NULL */
        break;
      case 63:
#line 234 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeCondition(TK_ISNULL, &yymsp[-2].minor.yy0, nullptr); }
#line 1415 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NULL */
        break;
      case 64:
#line 235 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeCondition(TK_EQ, yymsp[-2].minor.yy98, yymsp[0].minor.yy98); }
#line 1422 "parse.c"
        /* No destructor defined for EQ */
        break;
      case 65:
#line 236 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeCondition(TK_NE, yymsp[-2].minor.yy98, yymsp[0].minor.yy98); }
#line 1428 "parse.c"
        /* No destructor defined for NE */
        break;
      case 66:
#line 237 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeFuncCondition(TK_IN, &yymsp[-4].minor.yy0, yymsp[-1].minor.yy70); }
#line 1434 "parse.c"
        /* No destructor defined for IN */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 67:
#line 238 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeFuncCondition(TK_NOTIN, &yymsp[-5].minor.yy0, yymsp[-1].minor.yy70); }
#line 1442 "parse.c"
        /* No destructor defined for NOT */
        /* No destructor defined for IN */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 68:
#line 241 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeCondition(TK_AND, yymsp[-2].minor.yy98, yymsp[0].minor.yy98); }
#line 1451 "parse.c"
        /* No destructor defined for AND */
        break;
      case 69:
#line 246 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_TRUE, &yymsp[0].minor.yy0); }
#line 1457 "parse.c"
        break;
      case 70:
#line 247 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_FALSE, &yymsp[0].minor.yy0); }
#line 1462 "parse.c"
        break;
      case 71:
#line 248 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_INTEGER, &yymsp[0].minor.yy0); }
#line 1467 "parse.c"
        break;
      case 72:
#line 249 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_DOUBLE, &yymsp[0].minor.yy0); }
#line 1472 "parse.c"
        break;
      case 73:
#line 250 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_INTEGER, &yymsp[0].minor.yy0); }
#line 1477 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 74:
#line 251 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_DOUBLE, &yymsp[0].minor.yy0); }
#line 1483 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 75:
#line 252 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_UINTEGER, &yymsp[0].minor.yy0); }
#line 1489 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 76:
#line 253 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_UDOUBLE, &yymsp[0].minor.yy0); }
#line 1495 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 77:
#line 254 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_STRING, &yymsp[0].minor.yy0); }
#line 1501 "parse.c"
        break;
      case 78:
#line 255 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeValue(TK_BLOB, &yymsp[0].minor.yy0); }
#line 1506 "parse.c"
        break;
      case 79:
#line 256 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeFunction(TK_FUNCTION, &yymsp[-3].minor.yy0, yymsp[-1].minor.yy70, nullptr); }
#line 1511 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 80:
#line 257 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeFunction(TK_FUNCTION, &yymsp[-2].minor.yy0, nullptr, nullptr); }
#line 1518 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 81:
#line 262 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeTimeVal(true, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1525 "parse.c"
        break;
      case 82:
#line 263 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeTimeVal(true, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1530 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 83:
#line 264 "parse.y"
{ yygotominor.yy98 = ExprItem::MakeTimeVal(false, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1536 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 84:
#line 270 "parse.y"
{ yygotominor.yy70 = ExprList::AppendExprItem(nullptr, yymsp[0].minor.yy98); }
#line 1542 "parse.c"
        break;
      case 85:
#line 272 "parse.y"
{ yygotominor.yy70 = ExprList::AppendExprItem(yymsp[-2].minor.yy70, yymsp[0].minor.yy98); }
#line 1547 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 86:
#line 278 "parse.y"
{ yygotominor.yy114 = RecordList::AppendRecordList(nullptr, yymsp[-1].minor.yy70); }
#line 1553 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 87:
#line 280 "parse.y"
{ yygotominor.yy114 = RecordList::AppendRecordList(yymsp[-4].minor.yy114, yymsp[-1].minor.yy70); }
#line 1560 "parse.c"
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

#line 1610 "parse.c"
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
