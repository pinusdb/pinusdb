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
#define YYNSTATE 206
#define YYNRULE 87
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
 /*    10 */    78,   79,   80,   81,  127,  136,   11,  100,  118,  132,
 /*    20 */   134,   97,  101,  136,  205,   98,  120,  122,  103,   23,
 /*    30 */    14,  110,  114,   17,   32,  104,  131,  124,  176,  185,
 /*    40 */   130,  136,   50,  112,  116,   53,    8,  126,  131,  129,
 /*    50 */   140,  138,  132,  134,   97,  101,    9,  125,  107,  138,
 /*    60 */   132,  134,   97,  101,  126,  102,  105,  106,  109,  113,
 /*    70 */   117,  123,  160,  199,  200,   90,  140,  138,  132,  134,
 /*    80 */    97,  101,  143,  145,  153,    1,  149,   87,   82,  146,
 /*    90 */   148,   10,   22,  152,  181,  188,  136,  131,  151,  201,
 /*   100 */   126,   38,   49,  157,  136,   63,   58,   83,  131,  156,
 /*   110 */   126,  250,   65,   62,   82,  168,  249,  165,  164,  131,
 /*   120 */   171,  172,  183,  180,  176,  194,  131,  174,  126,  169,
 /*   130 */   240,  140,  138,  132,  134,   97,  101,   36,  175,  140,
 /*   140 */   138,  132,  134,   97,  101,  126,  184,   39,   88,  178,
 /*   150 */    42,  180,   37,  209,  192,   65,  187,   95,   35,   40,
 /*   160 */   186,   41,   34,   95,  213,  189,   43,  182,   44,   33,
 /*   170 */    45,  179,   46,    3,  177,   47,    4,   48,  215,  208,
 /*   180 */    31,   51,  190,  191,   52,   30,  214,    5,  170,   54,
 /*   190 */    55,  237,  238,   57,   56,  167,  216,  173,   59,   29,
 /*   200 */   166,   60,   61,  193,   67,   28,   27,   64,  218,  161,
 /*   210 */   163,   66,  195,   26,  162,  147,  155,   16,  251,    6,
 /*   220 */   154,   25,  144,  206,    7,  141,   24,  150,  197,  217,
 /*   230 */   212,  202,   21,  128,  198,    2,  219,   20,  139,  137,
 /*   240 */   135,  196,  121,  203,  133,  115,  111,  108,  119,   85,
 /*   250 */    19,  142,   84,   86,   18,  207,   89,   13,   91,   12,
 /*   260 */    92,  239,   94,  236,  184,  184,  184,  204,  184,   15,
 /*   270 */   184,  184,  184,  184,  184,  184,  184,   93,  184,  184,
 /*   280 */   184,  158,   99,  184,  184,  184,  184,  184,  184,  184,
 /*   290 */   184,  184,  184,  184,  184,  184,  184,  184,  184,  159,
 /*   300 */   184,  184,  184,  184,  294,  184,  184,  184,  184,  184,
 /*   310 */   184,   96,
};
static YYCODETYPE yy_lookahead[] = {
 /*     0 */    41,   42,   43,   44,   45,   46,   47,   48,   49,   50,
 /*    10 */    51,   52,   53,   54,   18,   28,   19,   30,   22,   65,
 /*    20 */    66,   67,   68,   28,   23,   29,   78,   79,   32,   26,
 /*    30 */    33,   62,   62,   36,   31,   29,   93,   94,   37,   38,
 /*    40 */    30,   28,   33,   74,   74,   36,   22,   37,   93,   94,
 /*    50 */    63,   64,   65,   66,   67,   68,   32,   30,   62,   64,
 /*    60 */    65,   66,   67,   68,   37,   69,   70,   71,   72,   73,
 /*    70 */    74,   75,    5,   59,   60,    8,   63,   64,   65,   66,
 /*    80 */    67,   68,   76,   77,   78,   18,   80,   85,   86,   78,
 /*    90 */    79,   24,   25,   30,   81,   90,   28,   93,   94,   57,
 /*   100 */    37,   34,   35,   30,   28,   30,   39,   40,   93,   94,
 /*   110 */    37,   23,   37,   85,   86,   23,   23,   30,   88,   93,
 /*   120 */    94,   30,   55,   93,   37,   37,   93,   94,   37,   37,
 /*   130 */    23,   63,   64,   65,   66,   67,   68,   32,   30,   63,
 /*   140 */    64,   65,   66,   67,   68,   37,   88,   33,   30,   81,
 /*   150 */    36,   93,   23,    0,   61,   37,   87,   56,   28,   32,
 /*   160 */    32,   23,   84,   56,    0,   91,   22,   32,   37,   27,
 /*   170 */    22,   32,   38,   84,   93,   32,   20,   23,    0,    0,
 /*   180 */    23,   32,   92,   23,   23,   30,    0,   21,   29,   22,
 /*   190 */    38,    0,    0,   23,   32,   89,    0,   29,   33,   22,
 /*   200 */     7,   32,   29,   62,   32,   29,   26,   23,    0,    6,
 /*   210 */    29,   86,   62,   28,   32,   79,   29,   23,   23,   22,
 /*   220 */    80,   84,   22,    0,   23,   93,   27,   29,   21,    0,
 /*   230 */     0,   21,   23,   29,   32,   19,    0,   32,   93,   93,
 /*   240 */    93,   58,   79,   32,   93,   32,   32,   32,   77,   32,
 /*   250 */    38,   93,   33,   29,   22,    0,   23,   23,   38,   84,
 /*   260 */    32,    0,   23,    0,   95,   95,   95,   93,   95,   32,
 /*   270 */    95,   95,   95,   95,   95,   95,   95,   87,   95,   95,
 /*   280 */    95,   93,   93,   95,   95,   95,   95,   95,   95,   95,
 /*   290 */    95,   95,   95,   95,   95,   95,   95,   95,   95,   93,
 /*   300 */    95,   95,   95,   95,   83,   95,   95,   95,   95,   95,
 /*   310 */    95,   93,
};
#define YY_SHIFT_USE_DFLT (-53)
static short yy_shift_ofst[] = {
 /*     0 */    67,  216,   24,  156,  166,  197,  201,  223,  -53,  -53,
 /*    10 */    -3,   24,  234,  255,  237,  194,  230,  232,  212,  205,
 /*    20 */   209,  229,    3,  199,   24,  185,  180,  176,  177,  155,
 /*    30 */   157,  179,  142,   24,  130,  105,  129,  153,  114,  127,
 /*    40 */   138,  164,  144,  131,  148,  134,  143,  154,  178,    9,
 /*    50 */   149,  161,  186,  167,  152,  162,  170,  196,  165,  169,
 /*    60 */   173,  172,   75,  184,  208,  172,  -53,  -41,  -53,  -53,
 /*    70 */   -53,  -53,  -53,  -53,  -53,  -53,  -53,  -53,  -53,  -53,
 /*    80 */   -53,  -53,  -53,  219,  217,  224,  172,  118,  233,  236,
 /*    90 */   220,  228,  107,  239,  263,   -4,   76,   -4,   -4,  -13,
 /*   100 */   -53,   -4,  -53,    6,   -4,  -53,  -53,  215,  -53,  -31,
 /*   110 */   214,  -53,  -53,  -30,  213,  -53,  -53,  -53,  171,  -52,
 /*   120 */   163,  -53,  -53,  -53,   27,  -53,   -4,  204,   -4,   10,
 /*   130 */   -53,   76,   -4,  -53,   -4,  -53,   -4,  -46,   -4,  -46,
 /*   140 */    -4,   -5,   76,  200,  -53,   11,  136,  -53,  -53,  198,
 /*   150 */    -4,   63,  -53,  140,  187,   -4,   73,  -53,  -53,  -53,
 /*   160 */   203,  182,  181,   -4,   87,  193,  168,   92,  191,  159,
 /*   170 */    -4,   91,  -53,   -4,  108,  -53,   -4,   68,  139,  -53,
 /*   180 */    13,  135,  -53,   -4,    1,  128,  101,   42,  183,   93,
 /*   190 */   160,  192,  141,   88,  150,  195,  207,  202,   14,  -53,
 /*   200 */   -53,  210,  211,   -4,   76,  261,
};
#define YY_REDUCE_USE_DFLT (-58)
static short yy_reduce_ofst[] = {
 /*     0 */   221,  -58,   89,  -58,  -58,  -58,  -58,  -58,  -58,  -58,
 /*    10 */   -58,  175,  -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,
 /*    20 */   -58,  -58,  -58,  -58,  137,  -58,  -58,  -58,  -58,  -58,
 /*    30 */   -58,  -58,  -58,   78,  -58,  -58,  -58,  -58,  -58,  -58,
 /*    40 */   -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,
 /*    50 */   -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,
 /*    60 */   -58,   28,  -58,  -58,  -58,  125,  -58,  -58,  -58,  -58,
 /*    70 */   -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,
 /*    80 */   -58,  -58,  -58,  -58,  -58,  -58,    2,  -58,  -58,  -58,
 /*    90 */   -58,  -58,  190,  -58,  -58,  218,  -58,  206,  189,  -58,
 /*   100 */   -58,  188,  -58,  -58,  -57,  -58,  -58,  -58,  -58,  -58,
 /*   110 */   -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,
 /*   120 */   -58,  -58,  -58,  -58,  -58,  -58,  158,  -58,  -45,  -58,
 /*   130 */   -58,  -58,  151,  -58,  147,  -58,  146,  -58,  145,  -58,
 /*   140 */   132,  -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,
 /*   150 */     4,  -58,  -58,  -58,  -58,   15,  -58,  -58,  -58,  -58,
 /*   160 */   -58,  -58,  -58,   30,  -58,  -58,  106,  -58,  -58,  -58,
 /*   170 */    26,  -58,  -58,   33,  -58,  -58,   81,  -58,  -58,  -58,
 /*   180 */   -58,  -58,  -58,   58,  -58,  -58,   69,    5,   74,   90,
 /*   190 */   -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,  -58,
 /*   200 */   -58,  -58,  -58,  174,  -58,  -58,
};
static YYACTIONTYPE yy_default[] = {
 /*     0 */   293,  293,  293,  293,  293,  293,  293,  293,  210,  211,
 /*    10 */   293,  293,  293,  293,  293,  293,  293,  293,  293,  293,
 /*    20 */   293,  293,  293,  293,  293,  293,  293,  293,  293,  293,
 /*    30 */   293,  293,  293,  293,  293,  293,  293,  293,  293,  293,
 /*    40 */   293,  293,  293,  293,  293,  293,  293,  293,  293,  293,
 /*    50 */   293,  293,  293,  293,  293,  293,  293,  293,  293,  293,
 /*    60 */   293,  293,  293,  293,  293,  293,  220,  293,  222,  223,
 /*    70 */   224,  225,  226,  227,  228,  229,  230,  231,  232,  233,
 /*    80 */   234,  235,  221,  293,  293,  293,  293,  293,  293,  293,
 /*    90 */   293,  293,  293,  293,  293,  293,  241,  293,  293,  293,
 /*   100 */   254,  293,  255,  256,  286,  257,  258,  259,  267,  293,
 /*   110 */   260,  268,  263,  293,  261,  269,  264,  262,  265,  293,
 /*   120 */   293,  282,  283,  266,  293,  270,  293,  293,  286,  293,
 /*   130 */   271,  287,  293,  274,  293,  275,  293,  276,  293,  277,
 /*   140 */   293,  278,  288,  293,  279,  293,  293,  280,  281,  293,
 /*   150 */   286,  293,  284,  293,  293,  286,  293,  285,  273,  272,
 /*   160 */   293,  293,  293,  293,  293,  293,  293,  293,  293,  293,
 /*   170 */   286,  293,  253,  286,  293,  252,  293,  291,  293,  292,
 /*   180 */   289,  293,  290,  293,  293,  293,  240,  242,  245,  293,
 /*   190 */   293,  293,  293,  293,  293,  293,  293,  293,  246,  247,
 /*   200 */   248,  293,  293,  243,  244,  293,
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
 /*  64 */ "expr_val ::= ID LP expr_val_list RP",
 /*  65 */ "expr_val ::= ADD LP expr_val_list RP",
 /*  66 */ "expr_val ::= expr_val LT expr_val",
 /*  67 */ "expr_val ::= expr_val LE expr_val",
 /*  68 */ "expr_val ::= expr_val GT expr_val",
 /*  69 */ "expr_val ::= expr_val GE expr_val",
 /*  70 */ "expr_val ::= expr_val EQ expr_val",
 /*  71 */ "expr_val ::= expr_val NE expr_val",
 /*  72 */ "expr_val ::= expr_val AND expr_val",
 /*  73 */ "expr_val ::= ID LIKE STRING",
 /*  74 */ "expr_val ::= ID IS NOT NULL",
 /*  75 */ "expr_val ::= ID IS NULL",
 /*  76 */ "expr_val ::= STRING IS NOT NULL",
 /*  77 */ "expr_val ::= STRING IS NULL",
 /*  78 */ "expr_val ::= ID IN LP expr_val_list RP",
 /*  79 */ "expr_val ::= ID NOT IN LP expr_val_list RP",
 /*  80 */ "expr_val_list ::=",
 /*  81 */ "expr_val_list ::= expr_val",
 /*  82 */ "expr_val_list ::= expr_val_list COMMA expr_val",
 /*  83 */ "target_list ::= expr_val",
 /*  84 */ "target_list ::= expr_val AS ID",
 /*  85 */ "target_list ::= target_list COMMA expr_val",
 /*  86 */ "target_list ::= target_list COMMA expr_val AS ID",
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
#line 574 "parse.c"
      break;
    case 86:
#line 115 "parse.y"
{ ColumnItem::FreeColumnItem((yypminor->yy114)); }
#line 579 "parse.c"
      break;
    case 87:
#line 170 "parse.y"
{ ExprValue::FreeExprValue((yypminor->yy137)); }
#line 584 "parse.c"
      break;
    case 88:
#line 255 "parse.y"
{ TargetList::FreeTargetList((yypminor->yy120)); }
#line 589 "parse.c"
      break;
    case 89:
#line 202 "parse.y"
{ RecordList::FreeRecordList((yypminor->yy180)); }
#line 594 "parse.c"
      break;
    case 90:
#line 176 "parse.y"
{ delete ((yypminor->yy73)); }
#line 599 "parse.c"
      break;
    case 91:
#line 183 "parse.y"
{ delete ((yypminor->yy177)); }
#line 604 "parse.c"
      break;
    case 92:
#line 191 "parse.y"
{ delete ((yypminor->yy133)); }
#line 609 "parse.c"
      break;
    case 93:
#line 210 "parse.y"
{ ExprValue::FreeExprValue((yypminor->yy137)); }
#line 614 "parse.c"
      break;
    case 94:
#line 248 "parse.y"
{ ExprValueList::FreeExprValueList((yypminor->yy161)); }
#line 619 "parse.c"
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
#line 923 "parse.c"
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
#line 935 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for USER */
        /* No destructor defined for SEMI */
        break;
      case 2:
#line 62 "parse.y"
{
  pdbChangePwd(pParse, &yymsp[-6].minor.yy68, &yymsp[-2].minor.yy0); 
}
#line 945 "parse.c"
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
#line 960 "parse.c"
        /* No destructor defined for SET */
        /* No destructor defined for ROLE */
        /* No destructor defined for FOR */
        /* No destructor defined for EQ */
        /* No destructor defined for SEMI */
        break;
      case 4:
#line 71 "parse.y"
{ yygotominor.yy68 = yymsp[0].minor.yy0; }
#line 970 "parse.c"
        break;
      case 5:
#line 72 "parse.y"
{ yygotominor.yy68 = yymsp[0].minor.yy0; }
#line 975 "parse.c"
        break;
      case 6:
#line 75 "parse.y"
{
  pdbDropTable(pParse, &yymsp[-1].minor.yy0);
}
#line 982 "parse.c"
        /* No destructor defined for DROP */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 7:
#line 79 "parse.y"
{
  pdbAttachTable(pParse, &yymsp[-1].minor.yy0);
}
#line 992 "parse.c"
        /* No destructor defined for ATTACH */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 8:
#line 83 "parse.y"
{
  pdbDetachTable(pParse, &yymsp[-1].minor.yy0);
}
#line 1002 "parse.c"
        /* No destructor defined for DETACH */
        /* No destructor defined for TABLE */
        /* No destructor defined for SEMI */
        break;
      case 9:
#line 89 "parse.y"
{
  pdbAttachFile(pParse, &yymsp[-1].minor.yy0, &yymsp[-5].minor.yy0, &yymsp[-3].minor.yy0);
}
#line 1012 "parse.c"
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
#line 1024 "parse.c"
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
#line 1035 "parse.c"
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
#line 1046 "parse.c"
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
#line 1058 "parse.c"
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
#line 1070 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 15:
#line 120 "parse.y"
{
  yygotominor.yy167 = ColumnList::AppendColumnItem(nullptr, yymsp[0].minor.yy114);
}
#line 1078 "parse.c"
        break;
      case 16:
#line 124 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_BOOL); }
#line 1083 "parse.c"
        /* No destructor defined for BOOL_TYPE */
        break;
      case 17:
#line 125 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_INT8); }
#line 1089 "parse.c"
        /* No destructor defined for TINYINT_TYPE */
        break;
      case 18:
#line 126 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_INT16); }
#line 1095 "parse.c"
        /* No destructor defined for SMALLINT_TYPE */
        break;
      case 19:
#line 127 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_INT32); }
#line 1101 "parse.c"
        /* No destructor defined for INT_TYPE */
        break;
      case 20:
#line 128 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_INT64); }
#line 1107 "parse.c"
        /* No destructor defined for BIGINT_TYPE */
        break;
      case 21:
#line 129 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_FLOAT); }
#line 1113 "parse.c"
        /* No destructor defined for FLOAT_TYPE */
        break;
      case 22:
#line 130 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_DOUBLE); }
#line 1119 "parse.c"
        /* No destructor defined for DOUBLE_TYPE */
        break;
      case 23:
#line 131 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_STRING); }
#line 1125 "parse.c"
        /* No destructor defined for STRING_TYPE */
        break;
      case 24:
#line 132 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_BLOB); }
#line 1131 "parse.c"
        /* No destructor defined for BLOB_TYPE */
        break;
      case 25:
#line 133 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_DATETIME); }
#line 1137 "parse.c"
        /* No destructor defined for DATETIME_TYPE */
        break;
      case 26:
#line 134 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL2); }
#line 1143 "parse.c"
        /* No destructor defined for REAL2_TYPE */
        break;
      case 27:
#line 135 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL3); }
#line 1149 "parse.c"
        /* No destructor defined for REAL3_TYPE */
        break;
      case 28:
#line 136 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL4); }
#line 1155 "parse.c"
        /* No destructor defined for REAL4_TYPE */
        break;
      case 29:
#line 137 "parse.y"
{ yygotominor.yy114 = ColumnItem::MakeColumnItem(&yymsp[-1].minor.yy0, PDB_FIELD_TYPE::TYPE_REAL6); }
#line 1161 "parse.c"
        /* No destructor defined for REAL6_TYPE */
        break;
      case 30:
#line 143 "parse.y"
{
  pdbDelete(pParse, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy137);
}
#line 1169 "parse.c"
        /* No destructor defined for DELETE */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 31:
#line 149 "parse.y"
{
  pdbInsert(pParse, &yymsp[-6].minor.yy0, yymsp[-4].minor.yy120, yymsp[-1].minor.yy180);
}
#line 1179 "parse.c"
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
#line 1192 "parse.c"
        /* No destructor defined for SELECT */
        /* No destructor defined for FROM */
        /* No destructor defined for SEMI */
        break;
      case 33:
#line 165 "parse.y"
{
  pdbSelect(pParse, yymsp[-1].minor.yy120, nullptr, nullptr, nullptr, nullptr, nullptr);
}
#line 1202 "parse.c"
        /* No destructor defined for SELECT */
        /* No destructor defined for SEMI */
        break;
      case 34:
#line 172 "parse.y"
{ yygotominor.yy137 = nullptr; }
#line 1209 "parse.c"
        break;
      case 35:
#line 173 "parse.y"
{ yygotominor.yy137 = yymsp[0].minor.yy137; }
#line 1214 "parse.c"
        /* No destructor defined for WHERE */
        break;
      case 36:
#line 178 "parse.y"
{ yygotominor.yy73 = nullptr; }
#line 1220 "parse.c"
        break;
      case 37:
#line 179 "parse.y"
{ yygotominor.yy73 = new GroupOpt(&yymsp[0].minor.yy0); }
#line 1225 "parse.c"
        /* No destructor defined for GROUP */
        /* No destructor defined for BY */
        break;
      case 38:
#line 180 "parse.y"
{ yygotominor.yy73 = new GroupOpt(&yymsp[-1].minor.yy0, yymsp[0].minor.yy137); }
#line 1232 "parse.c"
        /* No destructor defined for GROUP */
        /* No destructor defined for BY */
        break;
      case 39:
#line 185 "parse.y"
{ yygotominor.yy177 = nullptr; }
#line 1239 "parse.c"
        break;
      case 40:
#line 186 "parse.y"
{ yygotominor.yy177 = new OrderByOpt(&yymsp[0].minor.yy0, true); }
#line 1244 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        break;
      case 41:
#line 187 "parse.y"
{ yygotominor.yy177 = new OrderByOpt(&yymsp[-1].minor.yy0, true); }
#line 1251 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        /* No destructor defined for ASC */
        break;
      case 42:
#line 188 "parse.y"
{ yygotominor.yy177 = new OrderByOpt(&yymsp[-1].minor.yy0, false); }
#line 1259 "parse.c"
        /* No destructor defined for ORDER */
        /* No destructor defined for BY */
        /* No destructor defined for DESC */
        break;
      case 43:
#line 193 "parse.y"
{ yygotominor.yy133 = nullptr; }
#line 1267 "parse.c"
        break;
      case 44:
#line 194 "parse.y"
{ yygotominor.yy133 = new LimitOpt(&yymsp[0].minor.yy0); }
#line 1272 "parse.c"
        /* No destructor defined for LIMIT */
        break;
      case 45:
#line 195 "parse.y"
{ yygotominor.yy133 = new LimitOpt(&yymsp[-2].minor.yy0, &yymsp[0].minor.yy0); }
#line 1278 "parse.c"
        /* No destructor defined for LIMIT */
        /* No destructor defined for COMMA */
        break;
      case 46:
#line 205 "parse.y"
{ yygotominor.yy180 = RecordList::AppendRecordList(nullptr, yymsp[-1].minor.yy161); }
#line 1285 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 47:
#line 207 "parse.y"
{ yygotominor.yy180 = RecordList::AppendRecordList(yymsp[-4].minor.yy180, yymsp[-1].minor.yy161); }
#line 1292 "parse.c"
        /* No destructor defined for COMMA */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 48:
#line 212 "parse.y"
{ yygotominor.yy137 = yymsp[-1].minor.yy137; }
#line 1300 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 49:
#line 213 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeStarValue(); }
#line 1307 "parse.c"
        /* No destructor defined for STAR */
        break;
      case 50:
#line 214 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeID(&yymsp[0].minor.yy0); }
#line 1313 "parse.c"
        break;
      case 51:
#line 215 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeBoolValue(true); }
#line 1318 "parse.c"
        /* No destructor defined for TRUE */
        break;
      case 52:
#line 216 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeBoolValue(false); }
#line 1324 "parse.c"
        /* No destructor defined for FALSE */
        break;
      case 53:
#line 217 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIntValue(false, &yymsp[0].minor.yy0); }
#line 1330 "parse.c"
        break;
      case 54:
#line 218 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIntValue(false, &yymsp[0].minor.yy0); }
#line 1335 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 55:
#line 219 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIntValue(true, &yymsp[0].minor.yy0); }
#line 1341 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 56:
#line 220 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeDoubleValue(false, &yymsp[0].minor.yy0); }
#line 1347 "parse.c"
        break;
      case 57:
#line 221 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeDoubleValue(false, &yymsp[0].minor.yy0); }
#line 1352 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 58:
#line 222 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeDoubleValue(true, &yymsp[0].minor.yy0); }
#line 1358 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 59:
#line 223 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeStringValue(&yymsp[0].minor.yy0); }
#line 1364 "parse.c"
        break;
      case 60:
#line 224 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeBlobValue(&yymsp[0].minor.yy0); }
#line 1369 "parse.c"
        break;
      case 61:
#line 225 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeTimeValue(false, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1374 "parse.c"
        break;
      case 62:
#line 226 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeTimeValue(false, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1379 "parse.c"
        /* No destructor defined for PLUS */
        break;
      case 63:
#line 227 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeTimeValue(true, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0); }
#line 1385 "parse.c"
        /* No destructor defined for MINUS */
        break;
      case 64:
#line 229 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeFunction(&yymsp[-3].minor.yy0, yymsp[-1].minor.yy161); }
#line 1391 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 65:
#line 230 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeFunction(&yymsp[-3].minor.yy0, yymsp[-1].minor.yy161); }
#line 1398 "parse.c"
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 66:
#line 232 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeCompare(TK_LT, yymsp[-2].minor.yy137, yymsp[0].minor.yy137); }
#line 1405 "parse.c"
        /* No destructor defined for LT */
        break;
      case 67:
#line 233 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeCompare(TK_LE, yymsp[-2].minor.yy137, yymsp[0].minor.yy137); }
#line 1411 "parse.c"
        /* No destructor defined for LE */
        break;
      case 68:
#line 234 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeCompare(TK_GT, yymsp[-2].minor.yy137, yymsp[0].minor.yy137); }
#line 1417 "parse.c"
        /* No destructor defined for GT */
        break;
      case 69:
#line 235 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeCompare(TK_GE, yymsp[-2].minor.yy137, yymsp[0].minor.yy137); }
#line 1423 "parse.c"
        /* No destructor defined for GE */
        break;
      case 70:
#line 236 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeCompare(TK_EQ, yymsp[-2].minor.yy137, yymsp[0].minor.yy137); }
#line 1429 "parse.c"
        /* No destructor defined for EQ */
        break;
      case 71:
#line 237 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeCompare(TK_NE, yymsp[-2].minor.yy137, yymsp[0].minor.yy137); }
#line 1435 "parse.c"
        /* No destructor defined for NE */
        break;
      case 72:
#line 238 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeCompare(TK_AND, yymsp[-2].minor.yy137, yymsp[0].minor.yy137); }
#line 1441 "parse.c"
        /* No destructor defined for AND */
        break;
      case 73:
#line 239 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeLike(&yymsp[-2].minor.yy0, &yymsp[0].minor.yy0); }
#line 1447 "parse.c"
        /* No destructor defined for LIKE */
        break;
      case 74:
#line 240 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIsNotNull(&yymsp[-3].minor.yy0); }
#line 1453 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NOT */
        /* No destructor defined for NULL */
        break;
      case 75:
#line 241 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIsNull(&yymsp[-2].minor.yy0); }
#line 1461 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NULL */
        break;
      case 76:
#line 242 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIsNotNull(&yymsp[-3].minor.yy0); }
#line 1468 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NOT */
        /* No destructor defined for NULL */
        break;
      case 77:
#line 243 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIsNull(&yymsp[-2].minor.yy0); }
#line 1476 "parse.c"
        /* No destructor defined for IS */
        /* No destructor defined for NULL */
        break;
      case 78:
#line 244 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeIn(&yymsp[-4].minor.yy0, yymsp[-1].minor.yy161); }
#line 1483 "parse.c"
        /* No destructor defined for IN */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 79:
#line 245 "parse.y"
{ yygotominor.yy137 = ExprValue::MakeNotIn(&yymsp[-5].minor.yy0, yymsp[-1].minor.yy161); }
#line 1491 "parse.c"
        /* No destructor defined for NOT */
        /* No destructor defined for IN */
        /* No destructor defined for LP */
        /* No destructor defined for RP */
        break;
      case 80:
#line 250 "parse.y"
{ yygotominor.yy161 = nullptr; }
#line 1500 "parse.c"
        break;
      case 81:
#line 251 "parse.y"
{ yygotominor.yy161 = ExprValueList::AppendExprValue(nullptr, yymsp[0].minor.yy137); }
#line 1505 "parse.c"
        break;
      case 82:
#line 252 "parse.y"
{ yygotominor.yy161 = ExprValueList::AppendExprValue(yymsp[-2].minor.yy161, yymsp[0].minor.yy137); }
#line 1510 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 83:
#line 257 "parse.y"
{ yygotominor.yy120 = TargetList::AppendExprValue(nullptr, yymsp[0].minor.yy137, nullptr); }
#line 1516 "parse.c"
        break;
      case 84:
#line 258 "parse.y"
{ yygotominor.yy120 = TargetList::AppendExprValue(nullptr, yymsp[-2].minor.yy137, &yymsp[0].minor.yy0); }
#line 1521 "parse.c"
        /* No destructor defined for AS */
        break;
      case 85:
#line 259 "parse.y"
{ yygotominor.yy120 = TargetList::AppendExprValue(yymsp[-2].minor.yy120, yymsp[0].minor.yy137, nullptr); }
#line 1527 "parse.c"
        /* No destructor defined for COMMA */
        break;
      case 86:
#line 260 "parse.y"
{ yygotominor.yy120 = TargetList::AppendExprValue(yymsp[-4].minor.yy120, yymsp[-2].minor.yy137, &yymsp[0].minor.yy0); }
#line 1533 "parse.c"
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

#line 1582 "parse.c"
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
