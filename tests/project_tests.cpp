//----------------------------------------------------------------------
// FILE: code_generate_tests.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: S. Bowers
// DESC: Basic code generator tests
//----------------------------------------------------------------------



#include <iostream>
#include <sstream>
#include <string>
#include <gtest/gtest.h>
#include "mypl_exception.h"
#include "lexer.h"
#include "simple_parser.h"
#include "ast_parser.h"
#include "vm.h"
#include "code_generator.h"

using namespace std;


streambuf* stream_buffer;


void change_cout(stringstream& out)
{
  stream_buffer = cout.rdbuf();
  cout.rdbuf(out.rdbuf());
}

void restore_cout()
{
  cout.rdbuf(stream_buffer);
}

string build_string(initializer_list<string> strs)
{
  string result = "";
  for (string s : strs)
    result += s + "\n";
  return result;
}


//----------------------------------------------------------------------
// token.cpp Tests
//----------------------------------------------------------------------

TEST(BasicTokenTest, TokenCreation) {
  Token token1(TokenType::SWITCH, "switch", 0, 0);
  Token token2(TokenType::CASE, "case", 1, 1);
  Token token3(TokenType::BREAK, "break", 2, 2);
  Token token4(TokenType::DEFAULT, "default", 3, 3);
  Token token5(TokenType::COLON, "colon", 4, 4);
}

TEST(BasicTokenTest, TypeGivenIsReturned) {
  Token token1(TokenType::SWITCH, "switch", 0, 0);
  Token token2(TokenType::CASE, "case", 1, 1);
  Token token3(TokenType::BREAK, "break", 2, 2);
  Token token4(TokenType::DEFAULT, "default", 3, 3);
  Token token5(TokenType::COLON, "colon", 4, 4);
  ASSERT_EQ(TokenType::SWITCH, token1.type());
  ASSERT_EQ(TokenType::CASE, token2.type());
  ASSERT_EQ(TokenType::BREAK, token3.type());
  ASSERT_EQ(TokenType::DEFAULT, token4.type());
  ASSERT_EQ(TokenType::COLON, token5.type());
}

TEST(BasicTokenTest, LexemeGivenIsReturned) {
  Token token1(TokenType::SWITCH, "switch", 0, 0);
  Token token2(TokenType::CASE, "case", 1, 1);
  Token token3(TokenType::BREAK, "break", 2, 2);
  Token token4(TokenType::DEFAULT, "default", 3, 3);
  Token token5(TokenType::COLON, "colon", 4, 4);
  ASSERT_EQ("switch", token1.lexeme());
  ASSERT_EQ("case", token2.lexeme());
  ASSERT_EQ("break", token3.lexeme());
  ASSERT_EQ("default", token4.lexeme());
  ASSERT_EQ("colon", token5.lexeme());
}

TEST(BasicTokenTest, ColumnGivenIsReturned) {
  Token token1(TokenType::SWITCH, "switch", 0, 10);
  Token token2(TokenType::CASE, "case", 0, 15);
  Token token3(TokenType::BREAK, "break", 0, 20);
  Token token4(TokenType::DEFAULT, "default", 0, 30);
  Token token5(TokenType::COLON, "colon", 0, 40);
  ASSERT_EQ(10, token1.column());
  ASSERT_EQ(15, token2.column());
  ASSERT_EQ(20, token3.column());
  ASSERT_EQ(30, token4.column());
  ASSERT_EQ(40, token5.column());
}

TEST(BasicTokenTest, LineGivenIsReturned) {
  Token token1(TokenType::SWITCH, "switch", 10, 0);
  Token token2(TokenType::CASE, "case", 15, 0);
  Token token3(TokenType::BREAK, "break", 20, 0);
  Token token4(TokenType::DEFAULT, "default", 30, 0);
  Token token5(TokenType::COLON, "colon", 40, 0);
  ASSERT_EQ(10, token1.line());
  ASSERT_EQ(15, token2.line());
  ASSERT_EQ(20, token3.line());
  ASSERT_EQ(30, token4.line());
  ASSERT_EQ(40, token5.line());
}

TEST(BasicTokenTest, CorrectlyPrints) {
  Token token1(TokenType::SWITCH, "switch", 0, 0);
  Token token2(TokenType::CASE, "case", 1, 1);
  Token token3(TokenType::BREAK, "break", 2, 2);
  Token token4(TokenType::DEFAULT, "default", 3, 3);
  Token token5(TokenType::COLON, "colon", 4, 4);
  ASSERT_EQ("0, 0: SWITCH 'switch'", to_string(token1));
  ASSERT_EQ("1, 1: CASE 'case'", to_string(token2));
  ASSERT_EQ("2, 2: BREAK 'break'", to_string(token3));
  ASSERT_EQ("3, 3: DEFAULT 'default'", to_string(token4));
  ASSERT_EQ("4, 4: COLON 'colon'", to_string(token5));
}


//----------------------------------------------------------------------
// lexer.cpp Tests
//----------------------------------------------------------------------

TEST(BasicLexerTest, EmptyInput) {
  stringstream in;
  Lexer lexer(in);
  Token t = lexer.next_token();
  ASSERT_EQ(TokenType::EOS, t.type());
  ASSERT_EQ(1, t.line());
  ASSERT_EQ(1, t.column());
}

TEST(BasicLexerTest, SwitchWords) {
  stringstream in("switch case break default :");
  Lexer lexer(in);
  Token t = lexer.next_token();
  ASSERT_EQ(TokenType::SWITCH, t.type());
  ASSERT_EQ("switch", t.lexeme());
  ASSERT_EQ(1, t.line());
  ASSERT_EQ(1, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::CASE, t.type());
  ASSERT_EQ("case", t.lexeme());
  ASSERT_EQ(1, t.line());
  ASSERT_EQ(8, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::BREAK, t.type());
  ASSERT_EQ("break", t.lexeme());
  ASSERT_EQ(1, t.line());
  ASSERT_EQ(13, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::DEFAULT, t.type());
  ASSERT_EQ("default", t.lexeme());
  ASSERT_EQ(1, t.line());
  ASSERT_EQ(19, t.column()); 
  t = lexer.next_token();
  ASSERT_EQ(TokenType::COLON, t.type());
  ASSERT_EQ(":", t.lexeme());
  ASSERT_EQ(1, t.line());
  ASSERT_EQ(27, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::EOS, t.type());
}



//----------------------------------------------------------------------
// simple_parser.cpp Tests
//----------------------------------------------------------------------

// good
TEST(BasicSimpleParserTests, EmptySwitch) {
  stringstream in(build_string({
        "void my_fun() {",
        "  switch(0) {",
        "  }",
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicSimpleParserTests, SwitchEmptyCase) {
  stringstream in(build_string({
        "void my_fun() {",
        "  switch(0) {",
        "    case(0):",
        "    case(1):",
        "    case(2):",
        "    case(3):",
        "  }",
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicSimpleParserTests, SwitchEmptyDefault) {
  stringstream in(build_string({
        "void my_fun() {",
        "  switch(0) {",
        "    case(0):",
        "    case(1):",
        "    case(2):",
        "    default:",
        "  }",
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicSimpleParserTests, SwitchCaseNoBreaks) {
  stringstream in(build_string({
        "void my_fun() {",
        "  switch(0) {",
        "    case(0):",
        "      int i = 0",
        "    case(1):",
        "      int i = 1",
        "    case(2):",
        "      int i = 2",
        "    default:",
        "      int i = 3",
        "  }",
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicSimpleParserTests, SwitchCaseBreaks) {
  stringstream in(build_string({
        "void my_fun() {",
        "  switch(0) {",
        "    case(0):",
        "      int i = 0",
        "      break",
        "    case(1):",
        "      int i = 1",
        "      break",
        "    case(2):",
        "      int i = 2",
        "      break",
        "    default:",
        "      int i = 3",
        "  }",
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicSimpleParserTests, SwitchNoDefault) {
  stringstream in(build_string({
        "void my_fun() {",
        "  switch(0) {",
        "    case(0):",
        "      int i = 0",
        "      break",
        "    case(1):",
        "      int i = 1",
        "      break",
        "    case(2):",
        "      int i = 2",
        "  }",
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicSimpleParserTests, SwitchCharType) {
  stringstream in(build_string({
        "void my_fun() {",
        "  switch('a') {",
        "    case('a'):",
        "      int i = 0",
        "      break",
        "    case('b'):",
        "      int i = 1",
        "      break",
        "    case('c'):",
        "      int i = 2",
        "      break",
        "    default:",
        "      int i = 3",
        "      int j = 8",
        "  }",
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}

TEST(BasicSimpleParserTests, SwitchCombo) {
  stringstream in(build_string({
        "void my_fun() {",
        "  switch('a') {",
        "    case('a'):",
        "      int i = 0",
        "      int j = 2",
        "      break",
        "    case('b'):",
        "      int i = 1",
        "    case('c'):",
        "      int i = 2",
        "      int j = 4",
        "      break",
        "  }",
        "}"
      }));
  SimpleParser(Lexer(in)).parse();  
}

// bad
TEST(BasicSimpleParserTests, MissingParen) {
  stringstream in(build_string({
        "void my_fun() {",
        "  switch('a') {",
        "    case'a'):",
        "      int i = 0",
        "      int j = 2",
        "      break",
        "  }",
        "}"
      }));
  try {
    SimpleParser(Lexer(in)).parse();
    FAIL();
  }
  catch(MyPLException& e) {
    string msg = e.what();
    ASSERT_EQ("Parser Error: ", msg.substr(0, 14));
  }
}

TEST(BasicSimpleParserTests, UndefinedConstantExpr) {
  stringstream in(build_string({
        "void my_fun() {",
        "  switch('a') {",
        "    case(a):",
        "      int i = 0",
        "      int j = 2",
        "      break",
        "  }",
        "}"
      }));
  try {
    SimpleParser(Lexer(in)).parse();
    FAIL();
  }
  catch(MyPLException& e) {
    string msg = e.what();
    ASSERT_EQ("Parser Error: ", msg.substr(0, 14));
  }
}

TEST(BasicSimpleParserTests, DefaultContainsBreak) {
  stringstream in(build_string({
        "void my_fun() {",
        "  switch('a') {",
        "    case('a'):",
        "      int i = 0",
        "      int j = 2",
        "      break",
        "    default:",
        "      int i = 3",
        "      break",
        "  }",
        "}"
      }));
  try {
    SimpleParser(Lexer(in)).parse();
    FAIL();
  }
  catch(MyPLException& e) {
    string msg = e.what();
    ASSERT_EQ("Parser Error: ", msg.substr(0, 14));
  }
}


//----------------------------------------------------------------------
// ast_parser.cpp Tests
//----------------------------------------------------------------------

TEST(BasicASTParserTests, SwitchEmpty) {
  stringstream in(build_string({
        "void main() {", 
        "  switch('a') {",
        "  }",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  SwitchStmt& s = (SwitchStmt&)*p.fun_defs[0].stmts[0];
  ASSERT_EQ(0, s.cases.size());
  ASSERT_EQ(0, s.defaults.size());
}

TEST(BasicASTParserTests, SwitchOneCase) {
  stringstream in(build_string({
        "void main() {", 
        "  switch('a') {",
        "    case('a'):",
        "      int i = 0",
        "      int j = 2",
        "      break",
        "  }",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  SwitchStmt& s = (SwitchStmt&)*p.fun_defs[0].stmts[0];
  ASSERT_EQ(1, s.cases.size());
  ASSERT_EQ(0, s.defaults.size());
}

TEST(BasicASTParserTests, SwitchTwoCase) {
  stringstream in(build_string({
        "void main() {", 
        "  switch('a') {",
        "    case('a'):",
        "      int i = 0",
        "      int j = 2",
        "      break",
        "    case('b'):",
        "      int i = 0",
        "      int j = 2",
        "      break",
        "  }",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  SwitchStmt& s = (SwitchStmt&)*p.fun_defs[0].stmts[0];
  ASSERT_EQ(2, s.cases.size());
  ASSERT_EQ(0, s.defaults.size());
}

TEST(BasicASTParserTests, SwitchNoBreak) {
  stringstream in(build_string({
        "void main() {", 
        "  switch('a') {",
        "    case('a'):",
        "      int i = 0",
        "      int j = 2",
        "    case('b'):",
        "      int i = 0",
        "      int j = 2",
        "  }",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  SwitchStmt& s = (SwitchStmt&)*p.fun_defs[0].stmts[0];
  ASSERT_EQ(2, s.cases.size());
  ASSERT_EQ(2, s.cases[0].stmts.size());
  ASSERT_EQ(0, s.defaults.size());
}

TEST(BasicASTParserTests, SwitchDefaultOnly) {
  stringstream in(build_string({
        "void main() {", 
        "  switch('a') {",
        "    default:",
        "      int i = 3",
        "  }",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  SwitchStmt& s = (SwitchStmt&)*p.fun_defs[0].stmts[0];
  ASSERT_EQ(0, s.cases.size());
  ASSERT_EQ(1, s.defaults.size());
}

TEST(BasicASTParserTests, SwitchCombos) {
  stringstream in(build_string({
        "void main() {", 
        "  switch('a') {",
        "    case('a'):",
        "      int i = 0",
        "      int j = 2",
        "      char yellow = 'y'",
        "    case('b'):",
        "      int i = 0",
        "      int j = 2",
        "    default:",
        "      int i = 3",
        "      int l = 21",
        "  }",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  SwitchStmt& s = (SwitchStmt&)*p.fun_defs[0].stmts[0];
  ASSERT_EQ(2, s.cases.size());
  ASSERT_EQ(3, s.cases[0].stmts.size());
  ASSERT_EQ(2, s.cases[1].stmts.size());
  ASSERT_EQ(2, s.defaults.size());
}

//----------------------------------------------------------------------
// print_visitor.cpp Tests
//----------------------------------------------------------------------















//----------------------------------------------------------------------
// main
//----------------------------------------------------------------------

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

