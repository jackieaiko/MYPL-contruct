//----------------------------------------------------------------------
// FILE: semantic_checker.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: 
// DESC: 
//----------------------------------------------------------------------

#include <unordered_set>
#include "mypl_exception.h"
#include "semantic_checker.h"
#include <iostream>


using namespace std;

// hash table of names of the base data types and built-in functions
const unordered_set<string> BASE_TYPES {"int", "double", "char", "string", "bool"};
const unordered_set<string> BUILT_INS {"print", "input", "to_string",  "to_int",
  "to_double", "length", "get", "concat"};


// helper functions

optional<VarDef> SemanticChecker::get_field(const StructDef& struct_def,
                                            const string& field_name)
{
  for (const VarDef& var_def : struct_def.fields)
    if (var_def.var_name.lexeme() == field_name)
      return var_def;
  return nullopt;
}


void SemanticChecker::error(const string& msg, const Token& token)
{
  string s = msg;
  s += " near line " + to_string(token.line()) + ", ";
  s += "column " + to_string(token.column());
  throw MyPLException::StaticError(s);
}


void SemanticChecker::error(const string& msg)
{
  throw MyPLException::StaticError(msg);
}


// visitor functions


void SemanticChecker::visit(Program& p)
{
  // record each struct def
  for (StructDef& d : p.struct_defs) {
    string name = d.struct_name.lexeme();
    if (struct_defs.contains(name))
      error("multiple definitions of '" + name + "'", d.struct_name);
    struct_defs[name] = d;
  }
  // record each function def (need a main function)
  bool found_main = false;
  for (FunDef& f : p.fun_defs) {
    string name = f.fun_name.lexeme();
    if (BUILT_INS.contains(name))
      error("redefining built-in function '" + name + "'", f.fun_name);
    if (fun_defs.contains(name))
      error("multiple definitions of '" + name + "'", f.fun_name);
    if (name == "main") {
      if (f.return_type.type_name != "void")
        error("main function must have void type", f.fun_name);
      if (f.params.size() != 0)
        error("main function cannot have parameters", f.params[0].var_name);
      found_main = true;
    }
    fun_defs[name] = f;
  }
  if (!found_main)
    error("program missing main function");
  // check each struct
  for (StructDef& d : p.struct_defs)
    d.accept(*this);
  // check each function
  for (FunDef& d : p.fun_defs)
    d.accept(*this);
}


void SemanticChecker::visit(SimpleRValue& v)
{
  if (v.value.type() == TokenType::INT_VAL)
    curr_type = DataType {false, "int"};
  else if (v.value.type() == TokenType::DOUBLE_VAL)
    curr_type = DataType {false, "double"};    
  else if (v.value.type() == TokenType::CHAR_VAL)
    curr_type = DataType {false, "char"};    
  else if (v.value.type() == TokenType::STRING_VAL)
    curr_type = DataType {false, "string"};    
  else if (v.value.type() == TokenType::BOOL_VAL)
    curr_type = DataType {false, "bool"};    
  else if (v.value.type() == TokenType::NULL_VAL)
    curr_type = DataType {false, "void"};    
}
 

// // TODO: Implement the rest of the visitor functions (stubbed out below)

void SemanticChecker::visit(FunDef& f)
{
  symbol_table.push_environment();
  DataType ret_type = f.return_type;

  if(BASE_TYPES.contains(ret_type.type_name) || ret_type.type_name == "void") {
    symbol_table.add("return", ret_type);
  }
  else if(struct_defs.count(ret_type.type_name) == 0) {
    error("undefined return type");
  }
  
  symbol_table.add("return", ret_type);

  for(auto p : f.params) {
    if(symbol_table.name_exists_in_curr_env(p.var_name.lexeme())) {
      error("params has same name as function");
    }

    string p_type = p.data_type.type_name;
    if(!BASE_TYPES.contains(p_type)) {
      if(struct_defs.count(p.data_type.type_name) == 0) {
        error("undefined struct");
      }
    }

    symbol_table.add(p.var_name.lexeme(), p.data_type);
  }

  for(auto s : f.stmts) {
      s->accept(*this);
  }

  symbol_table.pop_environment();
}


void SemanticChecker::visit(StructDef& s)
{
  symbol_table.push_environment();

  for(auto f : s.fields) {
    if(symbol_table.name_exists_in_curr_env(f.var_name.lexeme())) {
      error("fields has same name as struct");
    }

    string f_type = f.data_type.type_name;
    if(!BASE_TYPES.contains(f_type) && f_type != "void") {
      if(struct_defs.count(f.data_type.type_name) == 0) {
        error("undefined struct");
      }
    }

    symbol_table.add(f.var_name.lexeme(), f.data_type);
  }

  symbol_table.pop_environment();
}


void SemanticChecker::visit(ReturnStmt& s)
{
  s.expr.accept(*this);
  if(symbol_table.get("return")->type_name != curr_type.type_name 
    && curr_type.type_name != "void") {
    error("invalid return type");
  }
}


void SemanticChecker::visit(WhileStmt& s)
{
  symbol_table.push_environment();
  s.condition.accept(*this);
  
  if(curr_type.type_name != "bool") {
    error("condition is not type bool");
  }

  for(auto st : s.stmts) {
    st->accept(*this);
  }

  symbol_table.pop_environment();
}


void SemanticChecker::visit(ForStmt& s)
{
  symbol_table.push_environment();
  s.var_decl.accept(*this);
  s.condition.accept(*this);

  if((curr_type.type_name != "bool") || (curr_type.is_array)) {
    error("assignment is not type int");
  }

  s.assign_stmt.accept(*this);
  for(auto s : s.stmts) {
    s->accept(*this);
  }

  symbol_table.pop_environment();
}


void SemanticChecker::visit(IfStmt& s)
{
  symbol_table.push_environment();
  s.if_part.condition.accept(*this);

  if(curr_type.type_name != "bool" || curr_type.is_array) {
    error("condition is not type bool");
  }

  for(auto st : s.if_part.stmts) {
    st->accept(*this);
  }

  if(s.else_ifs.size() > 0) {
    for(auto b : s.else_ifs) {
      symbol_table.push_environment();
      b.condition.accept(*this);

      if(curr_type.type_name != "bool") {
        error("condition is not type bool");
      }
      
      symbol_table.pop_environment();
    }
  }

  symbol_table.push_environment();
  if(s.else_stmts.size() > 0) {
    for(auto st : s.else_stmts) {
      st->accept(*this);
    }
  }
  
  symbol_table.pop_environment();
  symbol_table.pop_environment();
}


void SemanticChecker::visit(VarDeclStmt& s)
{
  string v = s.var_def.var_name.lexeme();
  if(symbol_table.name_exists_in_curr_env(v)) {
    error("var is already declared");
  }

  symbol_table.add(s.var_def.var_name.lexeme(), s.var_def.data_type);
  s.expr.accept(*this);

  if(curr_type.type_name != s.var_def.data_type.type_name) {
    if(curr_type.type_name != "void") {
      error("type mismatch");
    }
  }
}


void SemanticChecker::visit(AssignStmt& s)
{
  s.expr.accept(*this);
  DataType rhs = curr_type;

  if(s.lvalue.size() < 2) {
    DataType lval = *symbol_table.get(s.lvalue[0].var_name.lexeme());

    if(curr_type.type_name != lval.type_name) {
      error("mismatched type");
    }
  }

  string var_name = s.lvalue[0].var_name.lexeme();
  if(symbol_table.name_exists(var_name)) {
    curr_type = DataType{symbol_table.get(var_name)->is_array, symbol_table.get(var_name).value().type_name};
    
    if(struct_defs.contains(curr_type.type_name)) {
      for(int i = 1; i < s.lvalue.size(); i++) {
        var_name = s.lvalue[i].var_name.lexeme();
        VarDef field = get_field(struct_defs[curr_type.type_name], var_name).value();
        curr_type = {field.data_type.is_array, field.data_type.type_name};
      }
    } 

    if(curr_type.type_name != rhs.type_name) {
      error("mismatched type");
    }
  }
  else
  {
    error("use before def");
  }
}


void SemanticChecker::visit(CallExpr& e)
{
  string name = e.fun_name.lexeme();

  if(name == "print") {
    if(e.args.size() != 1) {
      error("too many args");
    }
    e.args.at(0).accept(*this);

    if((!BASE_TYPES.contains(curr_type.type_name)) || (curr_type.is_array)){
      error("Cannot print object of type "+ curr_type.type_name);
    }
    curr_type = DataType {false, "void"};
  }

  else if(name == "input") {
    if(e.args.size() != 0) {
      error("too many args");
    }
    curr_type = DataType{false, "string"};
  }

  else if(name == "to_string") {
    if(e.args.size() != 1) {
      error("too many args");
    }
    
    if(curr_type.is_array) {
      error("cannot convert array to string");
    }
    e.args.at(0).accept(*this);
    
    if(curr_type.type_name == "string" || (curr_type.type_name == "void") || (curr_type.type_name == "bool")) {
      error("cannot convert type to string type");
    }
    curr_type = DataType{false, "string"};
  }

  else if(name == "to_int") {
    if(e.args.size() != 1) {
      error("too many args");
    }
    
    if(curr_type.is_array) {
      error("cannot convert array to int");
    }
    e.args.at(0).accept(*this);

    if(curr_type.type_name == "int" || (curr_type.type_name == "void") || 
    (curr_type.type_name == "bool") || (curr_type.type_name == "char")) {
      error("cannot convert type to int");
    }
    curr_type = DataType{false, "int"};
  }

  else if(name == "to_double") {
    if(e.args.size() != 1) {
      error("too many args");
    }

    if(curr_type.is_array) {
      error("cannot convert array to double");
    }
    e.args.at(0).accept(*this);

    if(curr_type.type_name == "double" || (curr_type.type_name == "void") || 
    (curr_type.type_name == "bool") || (curr_type.type_name == "char")) {
      error("cannot convert type to double");
    }
    curr_type = DataType{false, "double"};
  }

  else if(name == "length") {
    if(e.args.size() != 1) {
      error("too many args");
    }
    e.args.at(0).accept(*this);

    if(curr_type.type_name != "string") {
      if(curr_type.is_array == false) {
        error("invalid parameter", e.first_token());
      }
    }
    curr_type = DataType{false, "int"};
  }

  else if(name == "get") {
    if(e.args.size() != 2) {
      error("incorrect nnumber of args");
    }
    e.args.at(0).accept(*this);

    if(curr_type.type_name != "int") {
      error("arg type mst be int");
    }
    else {
      e.args.at(1).accept(*this);
      if((curr_type.type_name == "string") || (curr_type.type_name == "char")) {
        curr_type = DataType{false, "char"};
      }
      else {
        error("incorrect type in get");
      }
    }
  }

  else if(name == "concat") {
    if(e.args.size() != 2) {
      error("incorrect number of args");
    }
    e.args.at(0).accept(*this);

    if(curr_type.type_name != "string") {
      error("incorrect type");
    }
    else {
      e.args.at(1).accept(*this);
      if((curr_type.type_name != "string")) {
        error("incorrect type");
      }
      else {
        curr_type = DataType{false, "string"};
      }
    }
  }

  else {
    if(fun_defs.contains(name)) {
      FunDef f = fun_defs[name];

      if(e.args.size() != f.params.size()) {
        error("wrong number of params");
      }

      for(int i = 0; i < e.args.size(); i++) {
        DataType params = f.params[i].data_type;
        e.args[i].accept(*this);

        if(curr_type.type_name != params.type_name || curr_type.is_array != params.is_array) {
          if(curr_type.type_name != "void") {
            error("mismatched type");
          }
        }
      }
      curr_type = DataType{f.return_type.is_array, f.return_type.type_name};
    }
    else {
      error("function used before defined");
    }
  }

}


void SemanticChecker::visit(Expr& e)
{
  e.first->accept(*this);
  
  if(e.op.has_value()) {
    e.rest->accept(*this);
    
    if((curr_type.type_name == "void") && (BASE_TYPES.contains(curr_type.type_name))) {
      error("can't be used in expression");
    }

    string lhs = curr_type.type_name;
    string rhs = curr_type.type_name;

    if((e.op->lexeme() == "==") || (e.op->lexeme() == "!=")) {
      if((lhs != rhs) && (lhs != "void") && (rhs != "void")) {
        error("lhs and rhs incompatible");
      }
      curr_type.type_name = "bool";
    }

    else if((e.op->lexeme() == "<") || (e.op->lexeme() == ">") || 
            (e.op->lexeme() == "<=") || (e.op->lexeme() == ">=")) {
      if(lhs != rhs) {
        error("lhs and rhs incompatible");
      }
      if(lhs == "bool") {
        error("lhs incompatible with operator");
      }
      curr_type.type_name = "bool";
    }

    else if((e.op->lexeme() == "+") || (e.op->lexeme() == "/") || (e.op->lexeme() == "-")) {
      if(lhs != rhs) {
        error("lhs and rhs incompatible");
      }
      if(lhs != "int" && lhs != "double") {
        error("lhs incompatible with operator");
      }
      curr_type.type_name = lhs;
    }
  }

}


void SemanticChecker::visit(SimpleTerm& t)
{
  t.rvalue->accept(*this);
} 


void SemanticChecker::visit(ComplexTerm& t)
{
  t.expr.accept(*this);
}


void SemanticChecker::visit(NewRValue& v)
{  
  if(v.type.type() == TokenType::INT_TYPE) {
    curr_type = DataType{true, "int"};
  }
  else if(v.type.type() == TokenType::DOUBLE_TYPE) {
    curr_type = DataType{true, "double"};
  }
  else if(v.type.type() == TokenType::BOOL_TYPE) {
    curr_type = DataType{true, "bool"};
  }
  else if(v.type.type() == TokenType::CHAR_TYPE) {
    curr_type = DataType{true, "char"};
  }
  else if(v.type.type() == TokenType::STRING_TYPE) {
    curr_type = DataType{true, "string"};
  }
  else {
    if(struct_defs.count(v.type.lexeme()) == 0) {
      error("type undefined");
    }
    else {
      curr_type = DataType{false, v.type.lexeme()};
    }
  }
}


void SemanticChecker::visit(VarRValue& v)
{
  string var_name = v.path[0].var_name.lexeme();
  if(!symbol_table.name_exists(var_name)) {
    to_string(symbol_table);
    error("use before def", v.path[0].var_name);
  }

  curr_type = DataType{symbol_table.get(var_name)->is_array, symbol_table.get(var_name).value().type_name};

  if(struct_defs.contains(curr_type.type_name)) {
    for(int i = 1; i < v.path.size(); i++) {
      string var_name2 = v.path[i].var_name.lexeme();
      VarDef field = get_field(struct_defs[curr_type.type_name], var_name2).value();
      curr_type = {field.data_type.is_array, field.data_type.type_name};
    }
  }
}    



void SemanticChecker::visit(SwitchStmt& s) {

}

void SemanticChecker::visit(CaseStmt& s) {

}

void SemanticChecker::visit(DefaultStmt& s) {

}