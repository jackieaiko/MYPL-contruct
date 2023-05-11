//----------------------------------------------------------------------
// FILE: ast_parser.cpp
// DATE: CPSC 326, Spring 2023
// AUTH:
// DESC:
//----------------------------------------------------------------------

#include "ast_parser.h"
#include "iostream"

using namespace std;

ASTParser::ASTParser(const Lexer &a_lexer)
    : lexer{a_lexer}
{
}

void ASTParser::advance()
{
  curr_token = lexer.next_token();
}

void ASTParser::eat(TokenType t, const string &msg)
{
  if (!match(t))
    error(msg);
  advance();
}

bool ASTParser::match(TokenType t)
{
  return curr_token.type() == t;
}

bool ASTParser::match(initializer_list<TokenType> types)
{
  for (auto t : types)
    if (match(t))
      return true;
  return false;
}

void ASTParser::error(const string &msg)
{
  string s = msg + " found '" + curr_token.lexeme() + "' ";
  s += "at line " + to_string(curr_token.line()) + ", ";
  s += "column " + to_string(curr_token.column());
  throw MyPLException::ParserError(s);
}

bool ASTParser::bin_op()
{
  return match({TokenType::PLUS, TokenType::MINUS, TokenType::TIMES,
                TokenType::DIVIDE, TokenType::AND, TokenType::OR, TokenType::EQUAL,
                TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQ,
                TokenType::GREATER_EQ, TokenType::NOT_EQUAL});
}

Program ASTParser::parse()
{
  Program p;
  advance();
  while (!match(TokenType::EOS))
  {
    if (match(TokenType::STRUCT))
      struct_def(p);
    else
      fun_def(p);
  }
  eat(TokenType::EOS, "expecting end-of-file");
  return p;
}

void ASTParser::struct_def(Program &p)
{
  StructDef s;
  eat(TokenType::STRUCT, "expecting struct statement");
  s.struct_name = curr_token;
  eat(TokenType::ID, "expecting id");
  eat(TokenType::LBRACE, "expecting lbrace");
  fields(s);
  eat(TokenType::RBRACE, "expecting rbrace");
  p.struct_defs.push_back(s);
}

void ASTParser::fun_def(Program &p)
{
  FunDef f;
  DataType d;
  if (match(TokenType::VOID_TYPE))
  {
    d.type_name = "void";
    f.return_type = d;
    eat(TokenType::VOID_TYPE, "expecting void");
  }
  else
  {
    data_type(d);
    f.return_type = d;
  }

  f.fun_name = curr_token;
  eat(TokenType::ID, "expecting id");
  eat(TokenType::LPAREN, "expecting lparen");
  params(f);
  eat(TokenType::RPAREN, "expecting rparen");
  eat(TokenType::LBRACE, "expecting lbrace");
  while (!match(TokenType::RBRACE))
  {
    stmt(f.stmts);
  }
  eat(TokenType::RBRACE, "expecting rbrace");
  p.fun_defs.push_back(f);
}

void ASTParser::fields(StructDef &s)
{
  if (!match(TokenType::RBRACE))
  {
    VarDef v;
    DataType d;

    data_type(d);
    v.var_name = curr_token;
    v.data_type = d;

    eat(TokenType::ID, "expecting id");
    s.fields.push_back(v);
    while (!match(TokenType::RBRACE))
    {
      eat(TokenType::COMMA, "expecting comma");
      data_type(d);

      v.var_name = curr_token;
      v.data_type = d;
      
      eat(TokenType::ID, "expecting id");
      s.fields.push_back(v);
    }
  }
}

void ASTParser::params(FunDef &f)
{
  if (!match(TokenType::RPAREN))
  {
    DataType d;
    VarDef v;
    
    data_type(d);
    v.var_name = curr_token;
    v.data_type = d;

    eat(TokenType::ID, "expecting id");
    f.params.push_back(v);
    while (match(TokenType::COMMA))
    {
      eat(TokenType::COMMA, "expecting comma");
      data_type(d);
      
      v.var_name = curr_token;
      v.data_type = d;

      eat(TokenType::ID, "expecting id");
      f.params.push_back(v);
    }
  }
}


void ASTParser::data_type(DataType& d) 
{
  if(match(TokenType::ID)) {
    d.type_name = curr_token.lexeme();
    advance();
  }
  else if(match(TokenType::ARRAY)) {
    d.is_array = true;
    advance();

    if(match(TokenType::ID)) {
      d.type_name = curr_token.lexeme();
      advance();
    }
    else{
      d.type_name = curr_token.lexeme();
      base_type();
    }
  }
  else {
    d.type_name = curr_token.lexeme();
    base_type();
  }
}


void ASTParser::base_type() 
{
  if(match({TokenType::INT_TYPE, TokenType::DOUBLE_TYPE, TokenType::BOOL_TYPE, TokenType::CHAR_TYPE, TokenType::STRING_TYPE})) {
    advance();
  }
}


void ASTParser::stmt(std::vector<std::shared_ptr<Stmt>>& s) 
{
  if(match(TokenType::SWITCH)) {
    SwitchStmt i;
    switch_stmt(i);
    s.push_back(std::make_shared<SwitchStmt>(i));
  }
  else if(match(TokenType::IF)) {
    IfStmt i;
    if_stmt(i);
    s.push_back(std::make_shared<IfStmt>(i));
  }
  else if(match(TokenType::WHILE)) {
    WhileStmt w;
    while_stmt(w);
    s.push_back(std::make_shared<WhileStmt>(w));
  }
  else if(match(TokenType::FOR)) {
    ForStmt f;
    for_stmt(f);
    s.push_back(std::make_shared<ForStmt>(f));
  }
  else if(match(TokenType::RETURN)) {
    ReturnStmt r;
    ret_stmt(r);
    s.push_back(std::make_shared<ReturnStmt>(r));
  }
  else if(match({TokenType::INT_TYPE, TokenType::DOUBLE_TYPE, TokenType::BOOL_TYPE, TokenType::CHAR_TYPE, TokenType::STRING_TYPE, TokenType::ARRAY})) {
    VarDeclStmt v;
    vdecl_stmt(v);
    s.push_back(std::make_shared<VarDeclStmt>(v));
  }
  else if(match(TokenType::ID)) {
    Token t = curr_token;
    advance();
    if(match(TokenType::LPAREN)){
      CallExpr c;
      c.fun_name = t;
      call_expr(c);
      s.push_back(std::make_shared<CallExpr>(c));
    }
    else if(match(TokenType::ID)) {
      VarDeclStmt v;
      v.var_def.data_type.type_name = t.lexeme();
      vdecl_stmt(v);
      s.push_back(std::make_shared<VarDeclStmt>(v));
    }
    else {
      AssignStmt a;
      VarRef l;
      l.var_name = t;
      a.lvalue.push_back(l);
      assign_stmt(a);
      s.push_back(std::make_shared<AssignStmt>(a));
    }
  }
}


void ASTParser::vdecl_stmt(VarDeclStmt& v) 
{
  if(!match(TokenType::ID)) {
    data_type(v.var_def.data_type);
  }
  v.var_def.var_name = curr_token;
  eat(TokenType::ID, "Expecting ID");
  eat(TokenType::ASSIGN, "Expecting ASSIGN");
  expr(v.expr);
}

void ASTParser::assign_stmt(AssignStmt& a) {
  lvalue(a.lvalue);
  eat(TokenType::ASSIGN, "expecting assign");
  expr(a.expr);
}


void ASTParser::lvalue(std::vector<VarRef>& p) 
{
  while(match(TokenType::DOT) || match(TokenType::LBRACKET)) {
    if(match(TokenType::DOT)) {
      eat(TokenType::DOT, "expecting dot");
      VarRef r;
      r.var_name = curr_token;
      eat(TokenType::ID, "Expecting ID");  
      p.push_back(r);
    }
    else if(match(TokenType::LBRACKET)) {
      VarRef r;
      r = p.back();
      p.pop_back();
      eat(TokenType::LBRACKET, "expecting lbracket");
      Expr e;
      expr(e);
      r.array_expr = e;
      eat(TokenType::RBRACKET, "expecting rbracket");
      p.push_back(r);
    }
  }
}


void ASTParser::if_stmt(IfStmt& i)
{
  eat(TokenType::IF, "expecting if");
  eat(TokenType::LPAREN, "expecting lparen");
  BasicIf b;
  expr(b.condition);
  eat(TokenType::RPAREN, "expecting rparen");
  eat(TokenType::LBRACE, "expecting lbrace");

  while(!match(TokenType::RBRACE))
  {
    stmt(b.stmts);
  }

  eat(TokenType::RBRACE, "expecting rbrace");
  i.if_part = b;
  if_stmt_tail(i);
}


void ASTParser::if_stmt_tail(IfStmt& i)
{
  if(match(TokenType::ELSEIF))
  {
    BasicIf b;
    eat(TokenType::ELSEIF, "expecting elseif");
    eat(TokenType::LPAREN, "expecting lparen");
    expr(b.condition);
    eat(TokenType::RPAREN, "expecting rparen");
    eat(TokenType::LBRACE, "expecting lbrace");

    while(!match(TokenType::RBRACE)) {
      stmt(b.stmts);
    }

    eat(TokenType::RBRACE, "expecting rbrace");
    i.else_ifs.push_back(b);
    if_stmt_tail(i);
  }
  else if(match(TokenType::ELSE)) 
  {
    eat(TokenType::ELSE, "expecting else");
    eat(TokenType::LBRACE, "expecting lbrace");
    while(!match(TokenType::RBRACE)) {
      stmt(i.else_stmts);
    }
    eat(TokenType::RBRACE, "expecting rbrace");
  }
}


void ASTParser::while_stmt(WhileStmt& w){
  eat(TokenType::WHILE, "expecting while");
  eat(TokenType::LPAREN, "expecting lparen");
  expr(w.condition);
  eat(TokenType::RPAREN, "expecting rparen");
  eat(TokenType::LBRACE, "expecting lbrace");

  while(!match(TokenType::RBRACE))
  {
    stmt(w.stmts);
  }
  eat(TokenType::RBRACE, "expecting rbrace");
}


void ASTParser::for_stmt(ForStmt& f)
{
  eat(TokenType::FOR, "expecting for");
  eat(TokenType::LPAREN, "expecting lparen");

  vdecl_stmt(f.var_decl);

  eat(TokenType::SEMICOLON, "expecting semicolon");
  expr(f.condition);
  eat(TokenType::SEMICOLON, "expecting semicolon");

  VarRef v;
  v.var_name = curr_token;
  f.assign_stmt.lvalue.push_back(v);

  eat(TokenType::ID, "expecting id");
  assign_stmt(f.assign_stmt);
  eat(TokenType::RPAREN, "expecting rparen");
  eat(TokenType::LBRACE, "expecting lbrace");

  while(!match(TokenType::RBRACE))
  {
    stmt(f.stmts);
  }
  eat(TokenType::RBRACE, "expecting rbrace");
}


void ASTParser::call_expr(CallExpr& c)
{
  eat(TokenType::LPAREN, "expecting lparen");
  if(!match(TokenType::RPAREN))
  {
    Expr e;
    expr(e);
    c.args.push_back(e);

    while(match(TokenType::COMMA)) {
      eat(TokenType::COMMA, "expecting comma");
      Expr e2;
      expr(e2);
      c.args.push_back(e2);
    }
  }
  eat(TokenType::RPAREN, "expecting rparen");
}


void ASTParser::ret_stmt(ReturnStmt& r)
{
  eat(TokenType::RETURN, "expecting return");
  expr(r.expr);
}


void ASTParser::expr(Expr& e)
{
  if(match(TokenType::NOT))
  {
    e.negated = true;
    eat(TokenType::NOT, "expecting not");
    expr(e);
  }
  else if(match(TokenType::LPAREN))
  {
    eat(TokenType::LPAREN, "expecting lparen");
    ComplexTerm c;
    expr(c.expr);
    eat(TokenType::RPAREN, "expecting rparen");
    e.first = std::make_shared<ComplexTerm>(c);
  } 
  else
  {
    SimpleTerm s;
    rvalue(s.rvalue);
    e.first = std::make_shared<SimpleTerm>(s);
  }

  if(bin_op())
  {
    e.op = curr_token;
    advance();
    Expr r;
    expr(r);
    e.rest = std::make_shared<Expr>(r);
  }
}


void ASTParser::rvalue(std::shared_ptr<RValue>& r)
{
  if(match(TokenType::NULL_VAL))
  {
    SimpleRValue s;
    s.value = curr_token;
    r = std::make_shared<SimpleRValue>(s);
    advance();
  }
  else if(match(TokenType::NEW))
  {
    NewRValue n;
    new_rvalue(n);
    r = std::make_shared<NewRValue>(n);
  }
  else if(match(TokenType::ID))
  {
    Token t = curr_token;
    advance();
    if(match(TokenType::LPAREN)){
      CallExpr c;
      c.fun_name = t;
      call_expr(c);
      r = std::make_shared<CallExpr>(c);
    }
    else 
    {
      VarRValue a;
      VarRef v;
      v.var_name = t;
      a.path.push_back(v);
      var_rvalue(a.path);
      r = std::make_shared<VarRValue>(a);
    }
  }
  else
  {
    SimpleRValue s;
    base_rvalue(s);
    r = std::make_shared<SimpleRValue>(s);
  }
}


void ASTParser::new_rvalue(NewRValue& n)
{
  eat(TokenType::NEW, "expecting new");
  if(match(TokenType::ID)) {
    n.type = curr_token;
    eat(TokenType::ID, "expecting id");

    if(match(TokenType::LBRACKET)) {
      eat(TokenType::LBRACKET, "expecting lbracket");
      Expr e;
      expr(e);
      n.array_expr = e;

      eat(TokenType::RBRACKET, "expecting rbracket");
    }
  }
  else {
    n.type = curr_token;
    base_type();

    eat(TokenType::LBRACKET, "expecting lbracket");
    Expr e;
    expr(e);
    n.array_expr = e;
    eat(TokenType::RBRACKET, "expecting rbracket");
  }
}


void ASTParser::base_rvalue(SimpleRValue& s)
{
  if(match({TokenType::INT_VAL, TokenType::DOUBLE_VAL, TokenType::BOOL_VAL, TokenType::CHAR_VAL, TokenType::STRING_VAL})) {
    s.value = curr_token;
    advance();
  }
}


void ASTParser::var_rvalue(std::vector<VarRef>& p)
{
  while (match(TokenType::DOT) || match(TokenType::LBRACKET)) 
  {
    if(match(TokenType::DOT)) {
      VarRef v;
      eat(TokenType::DOT, "expecting dot");
      v.var_name = curr_token;
      eat(TokenType::ID, "expecting id");
      p.push_back(v);
    }
    else if (match(TokenType::LBRACKET)) {
      VarRef v;
      v = p.back();
      p.pop_back();
      eat(TokenType::LBRACKET, "expecting lbracket");
      Expr e;
      expr(e);
      v.array_expr = e;
      eat(TokenType::RBRACKET, "expecting rbracket");
      p.push_back(v);
    }
  }
}




void ASTParser::switch_stmt(SwitchStmt& s)
{
  eat(TokenType::SWITCH, "expecting switch");
  eat(TokenType::LPAREN, "expecting lparen");
  base_rvalue(s.switch_expr);
  eat(TokenType::RPAREN, "expecting rparen");
  eat(TokenType::LBRACE, "expecting lbrace");

  case_stmt(s);
  eat(TokenType::RBRACE, "expecting rbrace");
}

void ASTParser::case_stmt(SwitchStmt& s)
{
  if (match(TokenType::CASE)) {
    CaseStmt c;
    eat(TokenType::CASE, "expecting case");
    base_rvalue(c.const_expr);
    eat(TokenType::COLON, "expecting colon");

    while (!match({TokenType::RBRACE, TokenType::DEFAULT, TokenType::BREAK, TokenType::CASE}))
    {
      stmt(c.stmts);
    }

    if (match(TokenType::BREAK)) {
      eat(TokenType::BREAK, "expecting break");
    }
    s.cases.push_back(c);
    
    if (match(TokenType::CASE)) {
      case_stmt(s);
    }
  }

  if (match(TokenType::DEFAULT)) {
    eat(TokenType::DEFAULT, "expecting while");
    eat(TokenType::COLON, "expecting colon");
    while(!match(TokenType::RBRACE))
    {
      stmt(s.defaults);
    }
  }
}