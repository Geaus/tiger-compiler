%filenames = "scanner"

 /*
  * Please don't modify the lines above.
  */

 /* You can add lex definitions here. */
 /* TODO: Put your lab2 code here */

%x COMMENT STR IGNORE

%%

 /*
  * Below is examples, which you can wipe out
  * and write regular expressions and actions of your own.
  *
  * All the tokens:
  *   Parser::ID
  *   Parser::STRING
  *   Parser::INT
  *   Parser::COMMA
  *   Parser::COLON
  *   Parser::SEMICOLON
  *   Parser::LPAREN
  *   Parser::RPAREN
  *   Parser::LBRACK
  *   Parser::RBRACK
  *   Parser::LBRACE
  *   Parser::RBRACE
  *   Parser::DOT
  *   Parser::PLUS
  *   Parser::MINUS
  *   Parser::TIMES
  *   Parser::DIVIDE
  *   Parser::EQ
  *   Parser::NEQ
  *   Parser::LT
  *   Parser::LE
  *   Parser::GT
  *   Parser::GE
  *   Parser::AND
  *   Parser::OR
  *   Parser::ASSIGN
  *   Parser::ARRAY
  *   Parser::IF
  *   Parser::THEN
  *   Parser::ELSE
  *   Parser::WHILE
  *   Parser::FOR
  *   Parser::TO
  *   Parser::DO
  *   Parser::LET
  *   Parser::IN
  *   Parser::END
  *   Parser::OF
  *   Parser::BREAK
  *   Parser::NIL
  *   Parser::FUNCTION
  *   Parser::VAR
  *   Parser::TYPE
  */

/*  ID  INT  STRING   */
[a-zA-Z][_a-zA-Z0-9]* {adjust();return Parser::ID}
[0-9]+                {adjust();return Parser::INT}

/*     */
","                   {adjust();return Parser::COMMA}
":"                   {adjust();return Parser::COLON}
";"                   {adjust();return Parser::SEMICOLON}
"("                   {adjust();return Parser::LPAREN}
")"                   {adjust();return Parser::RPAREN}
"{"                   {adjust();return Parser::LBRACK}
"}"                   {adjust();return Parser::LBRACK}
"["                   {adjust();return Parser::LBRACE}
"]"                   {adjust();return Parser::RBRACE}
"."                   {adjust();return Parser::DOT}
"+"                   {adjust();return Parser::PLUS}
"-"                   {adjust();return Parser::MINUS}
"*"                   {adjust();return Parser::TIMES}
"/"                   {adjust();return Parser::DIVIDE}
"="                   {adjust();return Parser::EQ}
"<>"                  {adjust();return Parser::NEQ}
"<"                   {adjust();return Parser::LT}
"<="                  {adjust();return Parser::LE}
">"                   {adjust();return Parser::GT}
">="                  {adjust();return Parser::GE}
"&"                   {adjust();return Parser::AND}
"|"                   {adjust();return Parser::OR}
":="                  {adjust();return Parser::ASSIGN}

/* reserved words */
/* TODO: Put your lab2 code here */
"array"              {adjust(); return Parser::ARRAY;}
"if"                 {adjust(); return Parser::IF;}



 /*
  * skip white space chars.
  * space, tabs and LF
  */
[ \t]+ {adjust();}
\n {adjust(); errormsg_->Newline();}

 /* illegal input */
. {adjust(); errormsg_->Error(errormsg_->tok_pos_, "illegal token");}
