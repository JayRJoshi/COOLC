/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */

int comment_depth = 0;

bool has_exceed_length(int c) {
	if(strlen(string_buf) + c >= MAX_STR_CONST) {
		yylval.error_msg = "String constant too long";
		return true;
	}
	return false;
}

%}


%x SINGLE_LINE_COMMENT COMMENT STR STRERR

%option yylineno

/*
 * Define names for regular expressions here.
 */



DARROW			=>
ASSIGN			<-
LE				<=
DIGIT			[0-9]
UPPER			[A-Z]
LOWER			[a-z]
ALPHABET		{LOWER}|{UPPER}
WHITESPACE		[ \n\f\r\t\v]
INT_CONST		[0-9]+
VALIDLIT		({ALPHABET}|{DIGIT}|"_")*
OBJECTID		{LOWER}{VALIDLIT}
TYPEID			{UPPER}{VALIDLIT}

/* Keywords */

CLASS			(?i:class)
ELSE			(?i:else)
FALSE			f(?i:alse)
FI				(?i:fi)
IF				(?i:if)
IN				(?i:in)
INHERITS		(?i:inherits)
ISVOID			(?i:isvoid)
LET				(?i:let)
LOOP			(?i:loop)
POOL			(?i:pool)
THEN			(?i:then)
WHILE			(?i:while)
CASE			(?i:case)
ESAC			(?i:esac)
NEW				(?i:new)
OF				(?i:of)
NOT				(?i:not)
TRUE			t(?i:rue)


%%

 /* Single Line Comments */

--				{ BEGIN(SINGLE_LINE_COMMENT); }
<SINGLE_LINE_COMMENT>.	{ }
<SINGLE_LINE_COMMENT>"\n" { BEGIN(INITIAL); }

 /*
  *  Nested comments
  */

"(*"	{ BEGIN(COMMENT); comment_depth = 1; }
"*)"	{ curr_lineno = yylineno; yylval.error_msg = "Unmatched *)"; return ERROR; }

<COMMENT>{
	<<EOF>> {
		curr_lineno = yylineno;
		yylval.error_msg = "EOF in comment";
		BEGIN(INITIAL);
		return ERROR;
	}
	"(*" {
		comment_depth++;
	}
	"*)" { 
		if(!--comment_depth) {
			BEGIN(INITIAL);
		}
	}
	. {}
	"\n" {}
}
 /*
  *  The multiple-character operators.
  */
{DARROW}		{ curr_lineno = yylineno; return DARROW; }
{ASSIGN}		{ curr_lineno = yylineno; return ASSIGN; }
{LE}			{ curr_lineno = yylineno; return LE; }

{CLASS}			{ curr_lineno = yylineno; return CLASS; }
{ELSE}			{ curr_lineno = yylineno; return ELSE; }
{FALSE}			{ curr_lineno = yylineno; yylval.boolean = false; return BOOL_CONST; }
{FI}			{ curr_lineno = yylineno; return FI; }
{IF}			{ curr_lineno = yylineno; return IF; }
{IN}			{ curr_lineno = yylineno; return IN; }
{INHERITS}		{ curr_lineno = yylineno; return INHERITS; }
{ISVOID}		{ curr_lineno = yylineno; return ISVOID; }
{LET}			{ curr_lineno = yylineno; return LET; }
{LOOP}			{ curr_lineno = yylineno; return LOOP; }
{POOL}			{ curr_lineno = yylineno; return POOL; }
{THEN}			{ curr_lineno = yylineno; return THEN; }
{WHILE}			{ curr_lineno = yylineno; return WHILE; }
{CASE}			{ curr_lineno = yylineno; return CASE; }
{ESAC}			{ curr_lineno = yylineno; return ESAC; }
{NEW}			{ curr_lineno = yylineno; return NEW; }
{OF}			{ curr_lineno = yylineno; return OF; }
{NOT}			{ curr_lineno = yylineno; return NOT; }
{TRUE}			{ curr_lineno = yylineno; yylval.boolean = true; return BOOL_CONST; }

[-;.:@,+*/<=(){}~]	{ curr_lineno = yylineno; return int(*yytext); }
{WHITESPACE} 		{ }

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */

"\""			{ strcpy(string_buf, ""); BEGIN(STR); }

<STR>{
	"\""	{ curr_lineno = yylineno; yylval.symbol = stringtable.add_string(string_buf); BEGIN(INITIAL); return STR_CONST; }
	"\\b"	{ curr_lineno = yylineno; if(has_exceed_length(1)) { BEGIN(STRERR); return ERROR; } strcat(string_buf, "\b"); }
	"\\t"	{ curr_lineno = yylineno; if(has_exceed_length(1)) { BEGIN(STRERR); return ERROR; } strcat(string_buf, "\t"); }
	"\\n"	{ curr_lineno = yylineno; if(has_exceed_length(1)) { BEGIN(STRERR); return ERROR; } strcat(string_buf, "\n"); }
	"\\f"	{ curr_lineno = yylineno; if(has_exceed_length(1)) { BEGIN(STRERR); return ERROR; } strcat(string_buf, "\f"); }
	"\\"\x00 { curr_lineno = yylineno; BEGIN(STRERR); yylval.error_msg = "String contains escaped null character."; return ERROR; }
	"\\".	{ curr_lineno = yylineno; if(has_exceed_length(1)) { BEGIN(STRERR); return ERROR; } strcat(string_buf, yytext+1); }
	"\\"\n	{ curr_lineno = yylineno; if(has_exceed_length(1)) { BEGIN(STRERR); return ERROR; } strcat(string_buf, "\n"); }
	"\n"	{ curr_lineno = yylineno; yylval.error_msg = "Unterminated string constant"; BEGIN(INITIAL); return ERROR; }
	\x00	{ curr_lineno = yylineno; yylval.error_msg = "String contains null character"; BEGIN(STRERR); return ERROR; }
	.		{ curr_lineno = yylineno; if(has_exceed_length(1)) { BEGIN(STRERR); return ERROR; } strcat(string_buf, yytext); }	
	<<EOF>>	{ curr_lineno = yylineno; yylval.error_msg = "EOF in string constant"; BEGIN(INITIAL); return ERROR; }
}

<STRERR>{
	"\\\n" 	{}
	"\""	{ BEGIN(INITIAL); }
	"\n"	{ BEGIN(INITIAL); }
	<<EOF>> { curr_lineno = yylineno; yylval.error_msg = "EOF in string constant"; BEGIN(INITIAL); return ERROR; }
	.		{ }
}

 /* IDs */
 
{INT_CONST}		{ curr_lineno = yylineno; yylval.symbol = inttable.add_string(yytext); return INT_CONST; }
{OBJECTID}	{ curr_lineno = yylineno; yylval.symbol = stringtable.add_string(yytext); return OBJECTID; }
{TYPEID}	{ curr_lineno = yylineno; yylval.symbol = stringtable.add_string(yytext); return TYPEID; }

 /* Anything else */
.		{ curr_lineno = yylineno; yylval.error_msg = yytext; return ERROR; }

%%
