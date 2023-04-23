//----------------------------------------------------------------------
// FILE: lexer.cpp
// DATE: CPSC 326, Spring 2023
// NAME: 
// DESC: 
//----------------------------------------------------------------------

#include "lexer.h"

using namespace std;


Lexer::Lexer(istream& input_stream)
  : input {input_stream}, column {0}, line {1}
{}


char Lexer::read()
{
  ++column;
  return input.get();
}


char Lexer::peek()
{
  return input.peek();
}


void Lexer::error(const string& msg, int line, int column) const
{
  throw MyPLException::LexerError(msg + " at line " + to_string(line) +
                                  ", column " + to_string(column));
}


Token Lexer::next_token()
{ 
  //check for whitespace, tabs, newlines and comments
  while((isspace(input.peek()))||(input.peek() == '#')){
    if(peek() == '#'){
      while((input.peek() != '\n')&&(input.peek() != EOF)){
        read();
      }
    }
    else{
      if(peek() == '\n'){
        ++line;
        column = 0;
      }
      else{
        ++column;
      }
      input.get();
    }
  }
  
  //check for EOF
  if(peek() == EOF){

    ++column;
    return Token(TokenType::EOS,"end-of-stream",line,column);
  }

  //check for single characters and two characters
  if(peek() == '.'){
    read();
    return Token(TokenType::DOT,".",line,column); 
  }
  if(peek() == ','){
    read();
    return Token(TokenType::COMMA,",",line,column); 
  }
  if(peek() == ';'){
    read();
    return Token(TokenType::SEMICOLON,";",line,column); 
  }
  if(peek() == '('){
    read();
    return Token(TokenType::LPAREN,"(",line,column); 
  }
  if(peek() == ')'){
    read();
    return Token(TokenType::RPAREN,")",line,column); 
  }
  if(peek() == '{'){
    read();
    return Token(TokenType::LBRACE,"{",line,column); 
  }
  if(peek() == '}'){
    read();
    return Token(TokenType::RBRACE,"}",line,column); 
  }
  if(peek() == '['){
    read();
    return Token(TokenType::LBRACKET,"[",line,column); 
  }
  if(peek() == ']'){
    read();
    return Token(TokenType::RBRACKET,"]",line,column); 
  }
  if(peek() == '+'){
    read();
    return Token(TokenType::PLUS,"+",line,column); 
  }
  if(peek() == '-'){
    read();
    return Token(TokenType::MINUS,"-",line,column); 
  }
  if(peek() == '*'){
    read();
    return Token(TokenType::TIMES,"*",line,column); 
  }
  if(peek() == '/'){
    read();
    return Token(TokenType::DIVIDE,"/",line,column); 
  }
  if(peek() == '<'){
    read();
    if(peek() == '='){
      read();
      return Token(TokenType::LESS_EQ, "<=", line,column-1);
    }
    return Token(TokenType::LESS,"<",line,column); 
  }
  if(peek() == '>'){
    read();
    if(peek() == '='){
      read();
      return Token(TokenType::GREATER_EQ, ">=", line,column-1);
    }
    return Token(TokenType::GREATER,">",line,column); 
  }
  if(peek() == '!'){
    read();
    if(peek() != '='){
      string s(1,peek());
      error("expecting '!=' found '!" + s + "'", line,column);
    }
    else{
      read();
      return Token(TokenType::NOT_EQUAL,"!=",line,column-1);
    }
  }
  if(peek() == '='){
    read();
    if(input.peek() == '='){
      read();
      return Token(TokenType::EQUAL, "==", line,column-1);
    }
    return Token(TokenType::ASSIGN,"=",line,column); 
  }
  // check for characters
  if(peek() == '\''){
    string char_val = "";
    bool slashes = false;
    int num_chars = 0;
    string additional = "";
    read();
    if(peek() == '\n' ){
      read();
      error("found end-of-line in character",line,column);      
    }
    while((peek() != '\'')&&(peek() != EOF)){
      if(peek() == '\\' ){
        char_val += "\\";
        slashes = true;
        --num_chars;
      }
      ++num_chars;
      if(num_chars > 1){
        string s(1,peek());
        additional += s;
      }
      string s(1,peek());
      read();
      char_val += s;
    }
    if(peek() != '\''){
      ++column;
      error("found end-of-file in character",line,column);
    }
    if(num_chars > 1){
      error("expecting ' found " + additional, line,column);
    }
    read();
    if(num_chars == 0){
      error("empty character", line,column);
    }
    if(slashes){
      return Token(TokenType::CHAR_VAL,char_val,line,column-3);
    }
    return Token(TokenType::CHAR_VAL,char_val,line,column-2);
  }
  if(peek() == '\"'){
    string string_val = "";
    bool slashes = false;
    int num_chars = 0;
    read();
    while((peek() != '\"')&&(peek() != EOF)){
      if(peek() == '\n' ){
        read();
        error("found end-of-line in string",line,column);      
      }
      if(peek() == '\\' ){
        string_val += "\\";
        slashes = true;
        --num_chars;
        --column;
      }
      ++num_chars;
      if((peek() != '\\')&&(peek() != '\"')){
        string s(1,peek());
        string_val += s;
      }
      read();
    }
    if(peek() != '\"'){
      ++column;
      error("found end-of-file in string",line,column-num_chars);
    }
    read();
    ++num_chars;
    return Token(TokenType::STRING_VAL,string_val,line,column-num_chars);
  }

  //check for integer or double
  if(isdigit(peek())){
    int dot = 0; 
    string before = "";
    string after = "";
    bool dont_continue = false;
    while((dot <= 1)&&(!isspace(peek()))&&(peek() != EOF)){
      if(peek() == '.'){
        if(dot == 0){
          dot += 1;
        }else{
          break;
        }
      }
      if((isalpha(peek()))&&(dot == 1)&&(after == "")){
        read();
        error("missing digit in '" + before + ".'", line,column);
      }
      if((isdigit(peek()))&&(dot == 0)){
        ++column;
        int i = input.peek() - '0';
        before += to_string(i);
      }else if((isdigit(peek()))&&(dot == 1)){
        ++column;
        int i = peek() - '0';
        after += to_string(i);
      }else if(peek() != '.'){
        break;
      }
      else{
        ++column;
      }
      input.get();
    }
    if((before[0] == '0' )&&(before.length() > 1 )&&(dot == 0 )){
      error("leading zero in number", line, column-before.length() +1);
    }
    if(dot == 0){
      return Token(TokenType::INT_VAL,before,line,column-before.length()+1);
    }else{
      if((dot == 1)&&(after.length() == 0)){
        error("missing digit in '" + before + ".'", line,column);
      }
      return Token(TokenType::DOUBLE_VAL,before + "." + after,line,column-after.length()-before.length());
    }
  }

  //check for reserved words
  if(isalpha(peek())){
    string val = "";
    bool has_digit = false;
    //check if it forms null or new
    while((!isspace(peek()))&&(peek() != EOF)&&(peek() != '\n')){
      if(!isalpha(peek())&&(peek() != '_')&&(!isdigit(peek()))){
        break;
      }
      if(isalpha(peek())){
        val += peek();
      }
      else{
        val += peek();
        has_digit = true;
      }
      read();
    }
    //identifier
    if(has_digit){
      return Token(TokenType::ID, val, line, column-val.length()+1);
    }
    //reserved words null, new, not
    if(val == "null"){
      return Token(TokenType::NULL_VAL, "null", line, column-val.length()+1);
    }
    if(val == "new"){
      return Token(TokenType::NEW, "new", line, column-val.length()+1);
    }
    if(val == "not"){
      return Token(TokenType::NOT, "not", line, column-val.length()+1);
    }
    //reserved words true false if else
    if(val == "if"){
      return Token(TokenType::IF,"if", line, column-val.length()+1);
    }
    if(val == "or"){
      return Token(TokenType::OR, "or", line, column-val.length()+1);
    }
    if((val == "true")||(val =="false")){
      return Token(TokenType::BOOL_VAL, val, line, column-val.length()+1);
    }
    if(val == "elseif"){
      return Token(TokenType::ELSEIF, "elseif", line, column-val.length()+1);
    }
    if(val == "else"){
      return Token(TokenType::ELSE, "else", line, column-val.length()+1);
    }
    if(val == "int"){
      return Token(TokenType::INT_TYPE, "int", line, column-val.length()+1);
    }
    if(val == "double"){
      return Token(TokenType::DOUBLE_TYPE, "double", line, column-val.length()+1);
    }
    if(val == "char"){
      return Token(TokenType::CHAR_TYPE, "char", line, column-val.length()+1);
    }
    if(val == "string"){
      return Token(TokenType::STRING_TYPE, "string", line, column-val.length()+1);
    }
    if(val == "bool"){
      return Token(TokenType::BOOL_TYPE, "bool", line, column-val.length()+1);
    }
    if(val == "void"){
      return Token(TokenType::VOID_TYPE, "void", line, column-val.length()+1);
    }
    if(val == "and"){
      return Token(TokenType::AND, "and", line, column-val.length()+1);
    }
    if(val == "for"){
      return Token(TokenType::FOR, "for", line, column-val.length()+1);
    }
    if(val == "or"){
      return Token(TokenType::OR, "or", line, column-val.length()+1);
    }
    if(val == "while"){
      return Token(TokenType::WHILE, "while", line, column-val.length()+1);
    }
    if(val == "struct"){
      return Token(TokenType::STRUCT, "struct", line, column-val.length()+1);
    }
    if(val == "array"){
      return Token(TokenType::ARRAY, "array", line, column-val.length()+1);
    }
    if(val == "return"){
      return Token(TokenType::RETURN, "return", line, column-val.length()+1);
    }
    return Token(TokenType::ID, val, line, column-val.length()+1);
  }

  string s(1,peek());
  read();
  error("unexpected character '" + s + "'", line,column);
  return Token(TokenType::ID, s, line, column);

}
  

