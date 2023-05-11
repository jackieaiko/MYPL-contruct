//----------------------------------------------------------------------
// FILE: code_generator.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: 
// DESC: 
//----------------------------------------------------------------------

#include <iostream>             // for debugging
#include "code_generator.h"

using namespace std;


// helper function to replace all occurrences of old string with new
void replace_all(string& s, const string& old_str, const string& new_str)
{
  while (s.find(old_str) != string::npos)
    s.replace(s.find(old_str), old_str.size(), new_str);
}


CodeGenerator::CodeGenerator(VM& vm)
  : vm(vm)
{
}


void CodeGenerator::visit(Program& p)
{
  for (auto& struct_def : p.struct_defs)
    struct_def.accept(*this);
  for (auto& fun_def : p.fun_defs)
    fun_def.accept(*this);
}


void CodeGenerator::visit(FunDef& f)
{
  VMFrameInfo new_frame;
  new_frame.function_name = f.fun_name.lexeme();
  new_frame.arg_count = f.params.size();
  curr_frame = new_frame;
  var_table.push_environment();

  for(int i = 0; i < f.params.size(); i++) {
    curr_frame.instructions.push_back(VMInstr::STORE(i));
    var_table.add(f.params[i].var_name.lexeme());
  }

  for(auto s : f.stmts) {
    s->accept(*this);
  }

  if(curr_frame.instructions.size() == 0 || curr_frame.instructions.back().opcode() != OpCode::RET) {
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
    curr_frame.instructions.push_back(VMInstr::RET());
  }

  var_table.pop_environment();
  vm.add(curr_frame);
}


void CodeGenerator::visit(StructDef& s)
{
  struct_defs[s.struct_name.lexeme()] = s;
}


void CodeGenerator::visit(ReturnStmt& s)
{
  s.expr.accept(*this);
  curr_frame.instructions.push_back(VMInstr::RET());
}


void CodeGenerator::visit(WhileStmt& s)
{
  int index = curr_frame.instructions.size();
  s.condition.accept(*this);

  int jmp_index = curr_frame.instructions.size();
  curr_frame.instructions.push_back(VMInstr::JMPF(-1));
  var_table.push_environment();

  for(auto st : s.stmts) {
    st->accept(*this);
  }

  var_table.pop_environment();

  curr_frame.instructions.push_back(VMInstr::JMP(index));
  curr_frame.instructions.push_back(VMInstr::NOP());
  int curr_index = curr_frame.instructions.size() - 1;
  curr_frame.instructions.at(jmp_index).set_operand(curr_index);
}


void CodeGenerator::visit(ForStmt& s)
{
  var_table.push_environment();
  s.var_decl.accept(*this);

  int index = curr_frame.instructions.size();

  s.condition.accept(*this);

  int jmpf_index = curr_frame.instructions.size();
  curr_frame.instructions.push_back(VMInstr::JMPF(-1));

  var_table.push_environment();
  for(auto st : s.stmts) {
    st->accept(*this);
  }
  var_table.pop_environment();

  s.assign_stmt.accept(*this);

  var_table.pop_environment();

  curr_frame.instructions.push_back(VMInstr::JMP(index));
  curr_frame.instructions.push_back(VMInstr::NOP());
  int curr_index = curr_frame.instructions.size() - 1;
  curr_frame.instructions.at(jmpf_index).set_operand(curr_index);
}


void CodeGenerator::visit(IfStmt& s)
{
  vector<int> jmp;
  vector<int> jmpf;

  s.if_part.condition.accept(*this);
  jmpf.push_back(curr_frame.instructions.size());
  curr_frame.instructions.push_back(VMInstr::JMPF(-1));

  var_table.push_environment();
  for(auto& st: s.if_part.stmts) {
    st->accept(*this);
  }
  var_table.pop_environment();

  jmp.push_back(curr_frame.instructions.size());
  curr_frame.instructions.push_back(VMInstr::JMP(-1));

  jmpf.push_back(curr_frame.instructions.size());
  curr_frame.instructions.push_back(VMInstr::NOP());
  curr_frame.instructions.at(jmpf[0]).set_operand(jmpf[1]);

  int counter = 0;
  for(auto ei : s.else_ifs){
    ei.condition.accept(*this);
    counter += 2;

    jmpf.push_back(curr_frame.instructions.size());
    curr_frame.instructions.push_back(VMInstr::JMPF(-1));

    var_table.push_environment();
    for(auto& el: ei.stmts) {
      el->accept(*this);
    }
    var_table.pop_environment();

    jmp.push_back(curr_frame.instructions.size());
    curr_frame.instructions.push_back(VMInstr::JMP(-1));

    jmpf.push_back(curr_frame.instructions.size());
    curr_frame.instructions.push_back(VMInstr::NOP());
    curr_frame.instructions.at(jmpf[counter]).set_operand(jmpf[counter + 1]);
  }

  for(auto e : s.else_stmts){
    e->accept(*this);
  }

  int index = curr_frame.instructions.size();
  curr_frame.instructions.push_back(VMInstr::NOP());

  for(auto i : jmp){
    curr_frame.instructions.at(i).set_operand(index);
  }
}


void CodeGenerator::visit(VarDeclStmt& s)
{
  s.expr.accept(*this);
  var_table.add(s.var_def.var_name.lexeme());
  curr_frame.instructions.push_back(VMInstr::STORE(var_table.get(s.var_def.var_name.lexeme())));
}


void CodeGenerator::visit(AssignStmt& s)
{
  curr_frame.instructions.push_back(VMInstr::LOAD(var_table.get(s.lvalue.at(0).var_name.lexeme())));
  
  for(int i = 0; i < s.lvalue.size(); ++i) {
    if(i > 0) {
      curr_frame.instructions.push_back(VMInstr::GETF(s.lvalue[i].var_name.lexeme()));
    }

    if(s.lvalue[i].array_expr.has_value()) {
      s.lvalue[i].array_expr->accept(*this);
      curr_frame.instructions.push_back(VMInstr::GETI());
    }
  }

  curr_frame.instructions.pop_back();
  s.expr.accept(*this);

  if(s.lvalue.size() > 1 && s.lvalue.back().array_expr == nullopt) {
    curr_frame.instructions.push_back(VMInstr::SETF(s.lvalue.back().var_name.lexeme()));
  }
  else if(s.lvalue.back().array_expr != nullopt) {
    curr_frame.instructions.push_back(VMInstr::SETI());
  }
  else {
    curr_frame.instructions.push_back(VMInstr::STORE(var_table.get(s.lvalue.back().var_name.lexeme())));
  }
}


void CodeGenerator::visit(CallExpr& e)
{
  for(auto e : e.args) {
    e.accept(*this);
  }
  
  if(e.fun_name.lexeme() == "print")
    curr_frame.instructions.push_back(VMInstr::WRITE());
  else if(e.fun_name.lexeme() == "input")
    curr_frame.instructions.push_back(VMInstr::READ());
  else if(e.fun_name.lexeme() == "get")
    curr_frame.instructions.push_back(VMInstr::GETC());
  else if(e.fun_name.lexeme() == "length")
    curr_frame.instructions.push_back(VMInstr::SLEN());
  else if(e.fun_name.lexeme() == "to_int")
    curr_frame.instructions.push_back(VMInstr::TOINT());
  else if(e.fun_name.lexeme() == "to_double")
    curr_frame.instructions.push_back(VMInstr::TODBL());
  else if(e.fun_name.lexeme() == "to_string")
    curr_frame.instructions.push_back(VMInstr::TOSTR());
  else if(e.fun_name.lexeme() == "concat")
    curr_frame.instructions.push_back(VMInstr::CONCAT());
  else 
    curr_frame.instructions.push_back(VMInstr::CALL(e.fun_name.lexeme()));
}


void CodeGenerator::visit(Expr& e)
{
  e.first->accept(*this);

  if(e.op.has_value()) {
    e.rest->accept(*this);

    if(e.op->lexeme() == "+") 
      curr_frame.instructions.push_back(VMInstr::ADD());
    else if(e.op->lexeme() == "-") 
      curr_frame.instructions.push_back(VMInstr::SUB());
    else if(e.op->lexeme() == "*") 
      curr_frame.instructions.push_back(VMInstr::MUL());
    else if(e.op->lexeme() == "/") 
      curr_frame.instructions.push_back(VMInstr::DIV());
    else if(e.op->lexeme() == "and") 
      curr_frame.instructions.push_back(VMInstr::AND());
    else if(e.op->lexeme() == "or") 
      curr_frame.instructions.push_back(VMInstr::OR());
    else if(e.op->lexeme() == "<=") 
      curr_frame.instructions.push_back(VMInstr::CMPLE());
    else if(e.op->lexeme() == "<") 
      curr_frame.instructions.push_back(VMInstr::CMPLT());
    else if(e.op->lexeme() == ">=") 
      curr_frame.instructions.push_back(VMInstr::CMPGE());
    else if(e.op->lexeme() == ">") 
      curr_frame.instructions.push_back(VMInstr::CMPGT());
    else if(e.op->lexeme() == "==") 
      curr_frame.instructions.push_back(VMInstr::CMPEQ());
    else if(e.op->lexeme() == "!=") 
      curr_frame.instructions.push_back(VMInstr::CMPNE());
  }
  if(e.negated == true) {
    curr_frame.instructions.push_back(VMInstr::NOT());
  }
}


void CodeGenerator::visit(SimpleTerm& t)
{
  t.rvalue->accept(*this);
}
 

void CodeGenerator::visit(ComplexTerm& t)
{
  t.expr.accept(*this);
}

void CodeGenerator::visit(SimpleRValue& v)
{
  if(v.value.type() == TokenType::INT_VAL) {
    int new_val = stoi(v.value.lexeme());
    curr_frame.instructions.push_back(VMInstr::PUSH(new_val));
  }
  else if(v.value.type() == TokenType::DOUBLE_VAL) {
    double new_val = stod(v.value.lexeme());
    curr_frame.instructions.push_back(VMInstr::PUSH(new_val));
  }
  else if (v.value.type() == TokenType::NULL_VAL) {
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
  }
  else if(v.value.type() == TokenType::BOOL_VAL) {
    if(v.value.lexeme() == "true") {
      curr_frame.instructions.push_back(VMInstr::PUSH(true));
    }
    else {
      curr_frame.instructions.push_back(VMInstr::PUSH(false));
    }
  }
  else if(v.value.type() == TokenType::STRING_VAL) {
    string s = v.value.lexeme();
    replace_all(s, "\\n", "\n");
    replace_all(s, "\\t ", "\t");
    curr_frame.instructions.push_back(VMInstr::PUSH(s));
  }
  else if(v.value.type() == TokenType::CHAR_VAL) {
    string s = v.value.lexeme();
    replace_all(s, "\\n", "\n");
    replace_all(s, "\\t", "\t");
    curr_frame.instructions.push_back(VMInstr::PUSH(s));
  }
}

void CodeGenerator::visit(NewRValue& v)
{
  if(v.array_expr.has_value()) {
    v.array_expr->accept(*this);

    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
    curr_frame.instructions.push_back(VMInstr::ALLOCA());
  }
  else {
    curr_frame.instructions.push_back(VMInstr::ALLOCS());

    for(auto& f : struct_defs[v.type.lexeme()].fields) {
      curr_frame.instructions.push_back(VMInstr::DUP());
      curr_frame.instructions.push_back(VMInstr::ADDF(f.var_name.lexeme()));
      curr_frame.instructions.push_back(VMInstr::DUP());
      curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
      curr_frame.instructions.push_back(VMInstr::SETF(f.var_name.lexeme()));
    }
  }
}


void CodeGenerator::visit(VarRValue& v)
{
  curr_frame.instructions.push_back(VMInstr::LOAD(var_table.get(v.path.at(0).var_name.lexeme())));

  for(int i = 0; i < v.path.size(); i++) {
    if(i > 0) {
      curr_frame.instructions.push_back(VMInstr::GETF(v.path[i].var_name.lexeme()));
    }

    if(v.path[i].array_expr.has_value()) {
      v.path[i].array_expr->accept(*this);
      curr_frame.instructions.push_back(VMInstr::GETI());
    }
  }
}
    



void CodeGenerator::visit(SwitchStmt& s) {
    vector<int> jmp;
    vector<int> jmpf;

    s.switch_expr.accept(*this);
    var_table.add(s.switch_expr.value.lexeme());
    curr_frame.instructions.push_back(VMInstr::STORE(var_table.get(s.switch_expr.value.lexeme())));

    int counter = 0;
    for(auto b : s.cases) {
      b.const_expr.accept(*this);
      curr_frame.instructions.push_back(VMInstr::LOAD(var_table.get(s.switch_expr.value.lexeme())));
      curr_frame.instructions.push_back(VMInstr::CMPEQ());
      

      jmpf.push_back(curr_frame.instructions.size());
      curr_frame.instructions.push_back(VMInstr::JMPF(-1));

      var_table.push_environment();
      for(auto st : b.stmts) {
        st->accept(*this);
      }
      var_table.pop_environment();

      jmp.push_back(curr_frame.instructions.size());
      curr_frame.instructions.push_back(VMInstr::JMP(-1));

      jmpf.push_back(curr_frame.instructions.size());
      curr_frame.instructions.push_back(VMInstr::NOP());
      curr_frame.instructions.at(jmpf[counter]).set_operand(jmpf[counter + 1]);
      counter += 2;
    }

    for(auto st : s.defaults) {
      st->accept(*this);
    }

    int index = curr_frame.instructions.size();
    curr_frame.instructions.push_back(VMInstr::NOP());

    for(auto i : jmp){
      curr_frame.instructions.at(i).set_operand(index);
    }

}

