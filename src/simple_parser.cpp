//----------------------------------------------------------------------
// FILE: simple_parser.cpp
// DATE: CPSC 326, Spring 2023
// AUTH:
// DESC:
//----------------------------------------------------------------------

#include "simple_parser.h"

#include <iostream>
using namespace std;

SimpleParser::SimpleParser(const Lexer &a_lexer)
    : lexer{a_lexer}
{
}

void SimpleParser::advance()
{
  curr_token = lexer.next_token();
}

void SimpleParser::eat(TokenType t, const std::string &msg)
{
  if (!match(t))
    error(msg);
  advance();
}

bool SimpleParser::match(TokenType t)
{
  return curr_token.type() == t;
}

bool SimpleParser::match(std::initializer_list<TokenType> types)
{
  for (auto t : types)
    if (match(t))
      return true;
  return false;
}

void SimpleParser::error(const std::string &msg)
{
  std::string s = msg + " found '" + curr_token.lexeme() + "' ";
  s += "at line " + std::to_string(curr_token.line()) + ", ";
  s += "column " + std::to_string(curr_token.column());
  throw MyPLException::ParserError(s);
}

bool SimpleParser::bin_op()
{
  return match({TokenType::PLUS, TokenType::MINUS, TokenType::TIMES,
                TokenType::DIVIDE, TokenType::AND, TokenType::OR, TokenType::EQUAL,
                TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQ,
                TokenType::GREATER_EQ, TokenType::NOT_EQUAL});
}

void SimpleParser::parse()
{
  advance();
  while (!match(TokenType::EOS))
  {
    if (match(TokenType::STRUCT))
      struct_def();
    else
      fun_def();
  }
  eat(TokenType::EOS, "expecting end-of-file");
}

void SimpleParser::struct_def()
{
  eat(TokenType::STRUCT, "expecting struct statement");
  eat(TokenType::ID, "expecting id");
  eat(TokenType::LBRACE, "expecting lbrace");
  fields();
  eat(TokenType::RBRACE, "expecting rbrace");
}

void SimpleParser::fun_def()
{
  if (match(TokenType::VOID_TYPE))
  {
    eat(TokenType::VOID_TYPE, "expecting void");
  }
  else
  {
    data_type();
  }

  eat(TokenType::ID, "expecting id");
  eat(TokenType::LPAREN, "expecting lparen");
  params();
  eat(TokenType::RPAREN, "expecting rparen");
  eat(TokenType::LBRACE, "expecting lbrace");
  while (!match(TokenType::RBRACE))
  {
    stmt();
  }
  eat(TokenType::RBRACE, "expecting rbrace");
}


void SimpleParser::fields()
{
  if(!match(TokenType::RBRACE)) {
    data_type();

    eat(TokenType::ID, "expecting id");
    while (match(TokenType::COMMA))
    {
      eat(TokenType::COMMA, "expecting comma");
      data_type();
      eat(TokenType::ID, "expecting id");
    }
  }
}

void SimpleParser::params()
{
  data_type();
  if (match(TokenType::ID))
  {
    eat(TokenType::ID, "expecting id");
    while (match(TokenType::COMMA))
    {
      eat(TokenType::COMMA, "expecting comma");
      data_type();
      eat(TokenType::ID, "expecting id");
    }
  }
}

void SimpleParser::data_type()
{
  if (base_type())
  {
    advance();
  }

  else if (match(TokenType::ID))
  {
    eat(TokenType::ID, "expecting id");
  }

  else if (match(TokenType::ARRAY))
  {
    eat(TokenType::ARRAY, "expecting array");
    if (base_type())
    {
      advance();
    }
    else if (match(TokenType::ID))
    {
      eat(TokenType::ID, "expecting id");
    }
  }
}

bool SimpleParser::base_type()
{
  return match({TokenType::INT_TYPE, TokenType::DOUBLE_TYPE, TokenType::BOOL_TYPE,
                TokenType::CHAR_TYPE, TokenType::STRING_TYPE});
}

void SimpleParser::stmt()
{
  //if match switch statement
  if (match(TokenType::SWITCH))
  {
    switch_stmt();
  }

  else if (match(TokenType::IF))
  {
    if_stmt();
  }
  else if (match(TokenType::WHILE))
  {
    while_stmt();
  }
  else if (match(TokenType::FOR))
  {
    for_stmt();
  }
  else if (match(TokenType::RETURN))
  {
    ret_stmt();
  }
  else if(match(TokenType::ID))
  {
    eat(TokenType::ID, "expecting id");
    if (match(TokenType::LPAREN))
    {
      call_expr();
    }
    else if(match(TokenType::ID)) {
      data_type();
      vdecl_stmt();
    }
    else {
      assign_stmt();
    }
  }
  else 
  {
    data_type();
    eat(TokenType::ID, "expecting id");
    vdecl_stmt();
  }
}

void SimpleParser::vdecl_stmt()
{
  eat(TokenType::ASSIGN, "expecting assign");
  expr();
}

void SimpleParser::assign_stmt()
{
  lvalue();
  eat(TokenType::ASSIGN, "expecting assign");
  expr();
}

void SimpleParser::lvalue()
{
  while (match(TokenType::DOT) || match(TokenType::LBRACKET))
  {

    if (match(TokenType::DOT))
    {
      eat(TokenType::DOT, "expecting dot");
      eat(TokenType::ID, "expecting id");
    }

    else if (match(TokenType::LBRACKET))
    {
      eat(TokenType::LBRACKET, "expecting lbracket");
      expr();
      eat(TokenType::RBRACKET, "expecting rbracket");
    }
  }
}

void SimpleParser::if_stmt()
{
  eat(TokenType::IF, "expecting if");
  eat(TokenType::LPAREN, "expecting lparen");
  expr();
  eat(TokenType::RPAREN, "expecting rparen");
  eat(TokenType::LBRACE, "expecting lbrace");
  while (!match(TokenType::RBRACE))
  {
    stmt();
  }
  eat(TokenType::RBRACE, "expecting rbrace");
  if_stmt_t();
}

void SimpleParser::if_stmt_t()
{
  if (match(TokenType::ELSEIF))
  {
    eat(TokenType::ELSEIF, "expecting elseif");
    eat(TokenType::LPAREN, "expecting lparen");
    expr();
    eat(TokenType::RPAREN, "expecting rparen");
    eat(TokenType::LBRACE, "expecting lbrace");

    while (!match(TokenType::RBRACE))
    {
      stmt();
    }
    eat(TokenType::RBRACE, "expecting rbrace");
    if_stmt_t();
  }
  else if (match(TokenType::ELSE))
  {
    eat(TokenType::ELSE, "expecting else");
    eat(TokenType::LBRACE, "expecting lbrace");
    while (!match(TokenType::RBRACE))
    {
      stmt();
    }
    eat(TokenType::RBRACE, "expecting rbrace");
  }
}

void SimpleParser::while_stmt()
{
  eat(TokenType::WHILE, "expecting while");
  eat(TokenType::LPAREN, "expecting lparen");
  expr();
  eat(TokenType::RPAREN, "expecting rparen");
  eat(TokenType::LBRACE, "expecting lbrace");
  while (!match(TokenType::RBRACE))
  {
    stmt();
  }
  eat(TokenType::RBRACE, "expecting rbrace");
}

void SimpleParser::for_stmt()
{
  eat(TokenType::FOR, "expecting for");
  eat(TokenType::LPAREN, "expecting lparen");

  data_type();
  eat(TokenType::ID, "expecting id");
  vdecl_stmt();;

  eat(TokenType::SEMICOLON, "expecting semicolon");
  expr();
  eat(TokenType::SEMICOLON, "expecting semicolon");

  eat(TokenType::ID, "expecting id");
  assign_stmt();
  eat(TokenType::RPAREN, "expecting rparen");
  eat(TokenType::LBRACE, "expecting lbrace");
  while (!match(TokenType::RBRACE))
  {
    stmt();
  }
  eat(TokenType::RBRACE, "expecting rbrace");
}

void SimpleParser::call_expr()
{
  eat(TokenType::LPAREN, "expecting lparen");
  if (!match(TokenType::RPAREN))
  {
    expr();
    while (match(TokenType::COMMA))
    {
      eat(TokenType::COMMA, "expecting comma");
      expr();
    }
  }
  eat(TokenType::RPAREN, "expecting rparen");
}

void SimpleParser::ret_stmt()
{
  eat(TokenType::RETURN, "expecting return");
  expr();
}

void SimpleParser::expr()
{
  if (match(TokenType::NOT))
  {
    eat(TokenType::NOT, "expecting not");
    expr();
  }
  else if (match(TokenType::LPAREN))
  {
    eat(TokenType::LPAREN, "expecting lparen");
    expr();
    eat(TokenType::RPAREN, "expecting rparen");
  }
  else
  {
    rvalue();
  }

  if (bin_op())
  {
    advance();
    expr();
  }
}

void SimpleParser::rvalue()
{
  if (base_rvalue())
  {
    advance();
  }
  else if (match(TokenType::NULL_VAL))
  {
    eat(TokenType::NULL_VAL, "expecting null");
  }
  else if (match(TokenType::NEW))
  {
    new_rvalue();
  }
  else
  {
    eat(TokenType::ID, "expecting id");
    if (match(TokenType::LPAREN))
    {
      call_expr();
    }
    else if (match(TokenType::DOT) || match(TokenType::LBRACKET))
    {
      var_rvalue();
    }
  }
}

void SimpleParser::new_rvalue()
{
  eat(TokenType::NEW, "expecting new");
  if (match(TokenType::ID))
  {
    eat(TokenType::ID, "expecting id");
    if (match(TokenType::LBRACKET))
    {
      eat(TokenType::LBRACKET, "expecting lbracket");
      expr();
      eat(TokenType::RBRACKET, "expecting rbracket");
    }
  }
  else if (base_type())
  {
    advance();
    eat(TokenType::LBRACKET, "expecting lbracket");
    expr();
    eat(TokenType::RBRACKET, "expecting rbracket");
  }
}

bool SimpleParser::base_rvalue()
{
  return match({TokenType::INT_VAL, TokenType::DOUBLE_VAL, TokenType::BOOL_VAL,
                TokenType::CHAR_VAL, TokenType::STRING_VAL});
}

void SimpleParser::var_rvalue()
{
  while (match(TokenType::DOT) || match(TokenType::LBRACKET))
  {
    if (match(TokenType::DOT))
    {
      eat(TokenType::DOT, "expecting dot");
      eat(TokenType::ID, "expecting id");
    }
    else if (match(TokenType::LBRACKET))
    {
      eat(TokenType::LBRACKET, "expecting lbracket");
      expr();
      eat(TokenType::RBRACKET, "expecting rbracket");
    }
  }
}


// void SimpleParser::switch_stmt()
// {
//   eat(TokenType::SWITCH, "expecting switch");
//   eat(TokenType::LPAREN, "expecting lparen");
//   if(base_rvalue()) {
//     advance();
//   }
//   // else {
//   //   eat(TokenType::ID, "expecting id"); // pre defined const expression?
//   // }
//   eat(TokenType::RPAREN, "expecting rparen");
//   eat(TokenType::LBRACE, "expecting lbrace");
//   while (!match({TokenType::RBRACE, TokenType::DEFAULT}))
//   {
//     case_stmt();
//   }

//   if(match(TokenType::DEFAULT)) {
//     default_stmt();
//   }

//   eat(TokenType::RBRACE, "expecting rbrace");
// }

// void SimpleParser::case_stmt()
// {
//   eat(TokenType::CASE, "expecting case");
//   eat(TokenType::LPAREN, "expecting lparen");
//   if(base_rvalue()) {
//     advance();
//   }
//   eat(TokenType::RPAREN, "expecting rparen");
//   eat(TokenType::COLON, "expecting colon");
//   while (!match({TokenType::RBRACE, TokenType::DEFAULT, TokenType::BREAK, TokenType::CASE}))
//   {
//     stmt();
//   }
//   if (match(TokenType::BREAK)) {
//     eat(TokenType::BREAK, "expecting break");
//   }
// }

// void SimpleParser::default_stmt()
// {
//   eat(TokenType::DEFAULT, "expecting default");
//   eat(TokenType::COLON, "expecting colon");
//   while (!match(TokenType::RBRACE))
//   {
//     stmt();
//   }
// }


void SimpleParser::switch_stmt()
{
  eat(TokenType::SWITCH, "expecting switch");
  eat(TokenType::LPAREN, "expecting lparen");
  if(base_rvalue()) {
    advance();
  }
  eat(TokenType::RPAREN, "expecting rparen");
  eat(TokenType::LBRACE, "expecting lbrace");
  while (!match({TokenType::RBRACE, TokenType::DEFAULT}))
  {
    case_stmt();
  }

  if(match(TokenType::DEFAULT)) {
    default_stmt();
  }

  eat(TokenType::RBRACE, "expecting rbrace");
}

void SimpleParser::case_stmt()
{
  eat(TokenType::CASE, "expecting case");
  eat(TokenType::LPAREN, "expecting lparen");
  if(base_rvalue()) {
    advance();
  }
  eat(TokenType::RPAREN, "expecting rparen");
  eat(TokenType::COLON, "expecting colon");
  while (!match({TokenType::RBRACE, TokenType::DEFAULT, TokenType::BREAK, TokenType::CASE}))
  {
    stmt();
  }
  if (match(TokenType::BREAK)) {
    eat(TokenType::BREAK, "expecting break");
  }
}

void SimpleParser::default_stmt()
{
  eat(TokenType::DEFAULT, "expecting default");
  eat(TokenType::COLON, "expecting colon");
  while (!match(TokenType::RBRACE))
  {
    stmt();
  }
}