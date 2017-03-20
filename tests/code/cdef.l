%{
#include <string>
#include <vector>
#include <cassert>
#include <iostream>

  const int defined_return = 1;
  const char * fn_prefix = "_fn_";
  int is_def_int = 0;
  int bracket_count = 0;
  std::vector<std::string> parameters;
%}

KEYWORD ("if"|"while")
ID [A-Za-z_][A-Za-z0-9_]*
NONID [^A-Za-z0-9]
BLANK [\n\t\r ]+
COMMENT "//"[^\n]+
SKIP ({BLANK}|{COMMENT})+

%%
{NONID}"int"{SKIP}{ID}/{SKIP}*";" { printf("%cint32_t%s = 0", yytext[0], yytext+4); }
^"int"{SKIP}{ID}/{SKIP}*";"  { printf("int32_t%s = 0", yytext+3); }

{NONID}"def"{SKIP}"int"/{NONID}   { putchar(yytext[0]); printf("int32_t"); is_def_int = 1; }
^"def"{SKIP}"int"/{NONID}   { printf("int32_t"); is_def_int = 1; }

{NONID}"def"{SKIP}"void"/{NONID}   { is_def_int = 0; putchar(yytext[0]); printf("void"); }
^"def"{SKIP}"void"/{NONID}   { is_def_int = 0; printf("void"); }

{NONID}"int"/{NONID}  { putchar(yytext[0]); printf("int32_t"); }
^"int"/{NONID}  { printf("int32_t"); }

{NONID}{KEYWORD}/{SKIP}*"("   { ECHO; }
{NONID}{ID}/{SKIP}*"("  {printf("%c%s%s", yytext[0], fn_prefix, yytext+1); }


"{"   {bracket_count++; ECHO;}
"}"   { if(--bracket_count == 0 && is_def_int && defined_return)
          puts("\nreturn 0; // safe return");
        ECHO;
      }

"%"   { assert(!parameters.empty()); printf("%s", parameters.back().c_str()); parameters.pop_back(); }
. ECHO;

%%

#include <fstream>
#include <iostream>

int main(int argc, char ** argv){
  if(argc >= 2){
    freopen(argv[1], "r", stdin);
  }

  if(argc >= 3){
    std::ifstream f(argv[2]);
    std::string tok;
    while(f >> tok)
      parameters.push_back(tok);
  }

  puts("#include \"stdio.h\"");
  puts("#include \"stdint.h\"");
  puts("void _fn_print(int32_t x) { printf(\"%d\\n\", x); }");
  puts("");
  puts("/* BEGIN USER CODE */");
  yylex();
  puts("/* END USER CODE */");
  puts("");
  puts("int main(){ _fn_main(); }");
}
