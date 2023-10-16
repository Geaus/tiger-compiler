%filenames parser
%scanner tiger/lex/scanner.h
%baseclass-preinclude tiger/absyn/absyn.h

 /*
  * Please don't modify the lines above.
  */

%union {
  int ival;
  std::string* sval;
  sym::Symbol *sym;
  absyn::Exp *exp;
  absyn::ExpList *explist;
  absyn::Var *var;
  absyn::DecList *declist;
  absyn::Dec *dec;
  absyn::EFieldList *efieldlist;
  absyn::EField *efield;
  absyn::NameAndTyList *tydeclist;
  absyn::NameAndTy *tydec;
  absyn::FieldList *fieldlist;
  absyn::Field *field;
  absyn::FunDecList *fundeclist;
  absyn::FunDec *fundec;
  absyn::Ty *ty;
  }

%token <sym> ID
%token <sval> STRING
%token <ival> INT

%token
  COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK
  LBRACE RBRACE DOT
  ASSIGN
  ARRAY IF THEN ELSE WHILE FOR TO DO LET IN END OF
  BREAK NIL
  FUNCTION VAR TYPE

 /* token priority */
 /* TODO: Put your lab3 code here */
 %left OR
 %left AND
 %nonassoc EQ NEQ LT LE GT GE
 %left PLUS MINUS
 %left TIMES DIVIDE
 %left UMINUS

%type <exp> exp expseq
%type <explist> actuals nonemptyactuals sequencing sequencing_exps
%type <var> lvalue one oneormore
%type <declist> decs decs_nonempty
%type <dec> decs_nonempty_s vardec
%type <efieldlist> rec rec_nonempty
%type <efield> rec_one
%type <tydeclist> tydec
%type <tydec> tydec_one
%type <fieldlist> tyfields tyfields_nonempty
%type <field> tyfield
%type <ty> ty
%type <fundeclist> fundec
%type <fundec> fundec_one

%start program

%%
program:  exp  {absyn_tree_ = std::make_unique<absyn::AbsynTree>($1);};

 /* TODO: Put your lab3 code here */
 exp:
   LPAREN RPAREN {$$ = new absyn::VoidExp(scanner_.GetTokPos());} |
   LPAREN exp RPAREN {$$ = $2;} |
   string {$$ = new absyn::StringExp(scanner_.GetTokPos(), $1);} |
   INT {$$ = new absyn::IntExp(scanner_.GetTokPos(), $1);} |
   NIL {$$ = new absyn::NilExp(scanner_.GetTokPos(), $1);} |
   lvalue {$$ = new absyn::VarExp(scanner_.GetTokPos(), $1); } |
   
   MINUS exp %prec UMINUS {$$ = new absyn::OpExp(scanner_.GetTokPos(),absyn::MINUS_OP,new absyn::IntExp(scanner_.GetTokPos(),0),$2);} |
   exp PLUS exp {$$ = new absyn::OpExp(scanner_.GetTokPos(),absyn::PLUS_OP, $1, $3); } |
   exp MINUS exp {$$ = new absyn::OpExp(scanner_.GetTokPos(),absyn::MINUS_OP, $1, $3);} |
   exp TIMES exp {$$ = new absyn::OpExp(scanner_.GetTokPos(),absyn::TIMES_OP, $1, $3);} |
   exp DIVIDE exp {$$ = new absyn::OpExp(scanner_.GetTokPos(),absyn::DIVIDE_OP, $1, $3);} |
   exp EQ exp {$$ = new absyn::OpExp(scanner_.GetTokPos(),absyn::EQ_OP, $1, $3);} |
   exp NEQ exp {$$ = new absyn::OpExp(scanner_.GetTokPos(),absyn::NEQ_OP, $1, $3);} |
   exp LT exp {$$ = new absyn::OpExp(scanner_.GetTokPos(),absyn::LT_OP, $1, $3);} |
   exp LE exp {$$ = new absyn::OpExp(scanner_.GetTokPos(),absyn::LE_OP, $1, $3);} |
   exp GT exp {$$ = new absyn::OpExp(scanner_.GetTokPos(),absyn::GT_OP, $1, $3);} |
   exp GE exp {$$ = new absyn::OpExp(scanner_.GetTokPos(),absyn::GE_OP, $1, $3);} |
   exp AND exp {$$ = new absyn::IfExp(scanner_.GetTokPos(), $1, $3, new absyn::IntExp(scanner_.GetTokPos(), 0));} |
   exp OR exp {$$ = new absyn::IfExp(scanner_.GetTokPos(), $1, new absyn::IntExp(scanner_.GetTokPos(), 1), $3);} |

   lvalue ASSIGN exp {$$ = new absyn::AssignExp(scanner_.GetTokPos(), $1, $3);} |
   ID LPAREN actuals RPAREN   {$$ = new absyn::CallExp(scanner_.GetTokPos(), $1, $3);} |
   LPAREN sequencing_exps RPAREN  {$$ = new absyn::SeqExp(scanner_.GetTokPos(), $2);} | 
   ID LBRACE rec RBRACE   {$$ = new absyn::RecordExp(scanner_.GetTokPos(), $1, $3);} |
   ID LBRACK exp RBRACK OF exp   {$$ = new absyn::ArrayExp(scanner_.GetTokPos(), $1, $3, $6);} |
   IF exp THEN exp           {$$ = new absyn::IfExp(scanner_.GetTokPos(), $2, $4, nullptr);} |
   IF exp THEN exp ELSE exp  {$$ = new absyn::IfExp(scanner_.GetTokPos(), $2, $4, $6);} |

   WHILE exp DO exp    {$$ = new absyn::WhileExp(scanner_.GetTokPos(), $2, $4);} |
   FOR ID ASSIGN exp TO exp DO exp  {$$ = new absyn::ForExp(scanner_.GetTokPos(), $2, $4, $6, $8);} |
   BREAK   {$$ = new absyn::BreakExp(scanner_.GetTokPos());} |
   LET decs IN expseq END   {$$ = new absyn::LetExp(scanner_.GetTokPos(), $2, $4);};

expseq:
  exp    {$$ = new absyn::SeqExp(scanner_.GetTokPos(), new absyn::ExpList($1));} |
  sequencing_exps    {$$ = new absyn::SeqExp(scanner_.GetTokPos(), $1);};

sequencing_exps:
  exp SEMICOLON exp    {$$ = (new absyn::ExpList($3)) -> Prepend($1);}|
  exp SEMICOLON sequencing_exps {$$ = $3 -> Prepend($1);};

actuals: {$$ = new absyn::ExpList();} |
  nonemptyactuals  {$$ = $1;};

nonemptyactuals: 
  exp   {$$ = new absyn::ExpList($1);} |
  exp COMMA nonemptyactuals  {$$ = $3 -> Prepend($1);};

lvalue:  
  ID  {$$ = new absyn::SimpleVar(scanner_.GetTokPos(), $1);} |  
  oneormore  {$$ = $1;};

one: 
  ID DOT ID   {$$ = new absyn::FieldVar(scanner_.GetTokPos(), new absyn::SimpleVar(scanner_.GetTokPos(), $1), $3);}|
  ID LBRACK exp RBRACK  {$$ = new absyn::SubscriptVar(scanner_.GetTokPos(), new absyn::SimpleVar(scanner_.GetTokPos(), $1), $3);};

oneormore: 
  one    {$$ = $1;} |
  oneormore DOT ID   {$$ = new absyn::FieldVar(scanner_.GetTokPos(), $1, $3);}  |
  oneormore LBRACK exp RBRACK  {$$ = new absyn::SubscriptVar(scanner_.GetTokPos(), $1, $3);};
