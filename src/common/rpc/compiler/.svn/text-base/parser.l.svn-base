%{
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "parser.tab.hh"

extern "C" {
    int yywrap(void);
    int yylex(void);
    extern int lineno;
}

%}

chars         [a-zA-Z]
number        [0-9]
yconst        (const)
yin           (in)
yout          (out)
ytime         [1-9]({number})*
ybool         (bool)
ychar         (char)
ysigned       (signed)
yunsigned     (unsigned)
yshort        (short)
yint          (int)
ylong         (long)
yfloat        (float)
ydouble       (double)
yvoid         (void)
scopeop       (::)
ydash         (\-)

line_comment  \/\/[^\n]*
block_comment \/\*([^\*]|\*[^/]|\n|\r)*\*\/
comment_quote \"(.+?)\"
objectname    ({chars})+(\_|{chars}|{number})*

%%

cpp_quote         { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return cpp_quote; }
stub_cpp_quote    { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return stub_cpp_quote; }
proxy_cpp_quote   { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return proxy_cpp_quote; }

module            { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return module; }
interface         { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return interface; }
timeout           { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return timeout; }
amd               { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return amd; }
nestable          { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return nestable; }
callback          { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return callback; }

{ydash}           { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return ydash; }
{scopeop}         { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return scopeop; }
{ybool}           { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return ybool; }
{ychar}           { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return ychar; }
{ysigned}         { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return ysigned; }
{yunsigned}       { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return yunsigned; }
{yshort}          { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return yshort; }
{yint}            { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return yint; }
{ylong}           { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return ylong; }
{yfloat}          { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return yfloat; }
{ydouble}         { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return ydouble; }
{yvoid}           { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return yvoid; }

{yconst}          { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return yconst; }
{yin}             { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return yin; }
{yout}            { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return yout; }
{ytime}           { /*printf("%s\n", yytext);*/ yylval.ival = atoi(strdup(yytext)); return ytime; }
{line_comment}    { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return line_comment; }
{block_comment}   { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return block_comment; }
{objectname}      { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return objectname; }
{comment_quote}   { /*printf("%s\n", yytext);*/ yylval.sval = strdup(yytext); return comment_quote; }

[\t ] ;
[{]     { return '{'; }
[}]     { return '}'; }
[\[]    { return '['; }
[\]]    { return ']'; }
[\(]    { return '('; }
[\)]    { return ')'; }
[;]     { return ';'; }
[,]     { return ','; }
[\&]    { return '&'; }
[<]     { return '<'; }
[>]     { return '>'; }
[\*]    { return '*'; }
[\n]    { lineno++;   }

%%


int yywrap(void)
{
    return 1;
}

// remove warning
void (*unused_yyunput)(int c,char *buf_ptr) = &yyunput;

