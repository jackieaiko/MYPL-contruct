//----------------------------------------------------------------------
// FILE: print_visitor.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: 
// DESC: 
//----------------------------------------------------------------------

#include "print_visitor.h"
#include <iostream>

using namespace std;


PrintVisitor::PrintVisitor(ostream& output)
  : out(output)
{
}


void PrintVisitor::inc_indent()
{
  indent += INDENT_AMT;
}


void PrintVisitor::dec_indent()
{
  indent -= INDENT_AMT;
}


void PrintVisitor::print_indent()
{
  out << string(indent, ' ');
}


void PrintVisitor::visit(Program& p)
{
  for (auto struct_def : p.struct_defs)
    struct_def.accept(*this);
  for (auto fun_def : p.fun_defs)
    fun_def.accept(*this);
}

// TODO: Finish the visitor functions
void PrintVisitor::visit(FunDef& f) {
  inc_indent();
  this->out << f.return_type.type_name << " " << f.fun_name.lexeme() << "(";

  for(int i = 0; i < f.params.size(); i++) {
    this->out << f.params.at(i).data_type.type_name << " " << f.params.at(i).var_name.lexeme();
    if(i < f.params.size() - 1) {
      this->out << ", ";
    }
  }
  this->out << ")" << " " << "{" << endl;
  
  for(int i = 0; i < f.stmts.size(); i++) {
    print_indent();
    f.stmts.at(i)->accept(*this);
    this->out << endl;
  }
  dec_indent();
  this->out << endl << "}" << endl;
  
}

void PrintVisitor::visit(StructDef& s) {
  print_indent();
  this->out << "struct " << s.struct_name.lexeme();
  this->out << " " << "{" << endl;
  inc_indent();
  
  for(int i = 0; i < s.fields.size(); i++) {
    print_indent();
    this->out << s.fields.at(i).data_type.type_name << " " << s.fields.at(i).var_name.lexeme();
    if(i < s.fields.size() - 1) {
      this->out << "," << endl;
    }
    else {
      this->out << endl;
    }
  }
  dec_indent();
  this->out << "}" << endl;
}

void PrintVisitor::visit(ReturnStmt& s) {
  this->out << "return ";
  s.expr.accept(*this);
}

void PrintVisitor::visit(WhileStmt& s) {
  this->out << "while (";
  s.condition.accept(*this);
  this->out << ") {" << endl;
  inc_indent();

  for(auto s : s.stmts) {
    print_indent();
    s->accept(*this);
    this->out << endl;
  }
  dec_indent();
  print_indent();
  this->out << "}";
}

void PrintVisitor::visit(ForStmt& s) {
  this->out << "for (";
  // s.var_decl.accept(*this);
  this->out << "; ";
  s.condition.accept(*this);
  this->out << "; ";
  // s.assign_stmt.accept(*this);
  this->out << ") {" << endl;

  inc_indent();

  for(auto s : s.stmts) {
    print_indent();
    s->accept(*this);
    this->out << endl;
  }
  dec_indent();
  print_indent();
  this->out << "}" ;

}

void PrintVisitor::visit(IfStmt& s) {
  this->out << "if (";
  s.if_part.condition.accept(*this);
  
  this->out << ") {" << endl;
  inc_indent();

  for(auto s : s.if_part.stmts) {
    print_indent();
    s->accept(*this);
    this->out << endl;
  }

  dec_indent();
  print_indent();
  this->out << "}";

  for(auto b : s.else_ifs) {
    this->out << endl;
    print_indent();
    this->out << "elseif (";
    b.condition.accept(*this);
    this->out << ") {" << endl;
    inc_indent();

    for(auto s : b.stmts) {
      print_indent();
      s->accept(*this);
      this->out << endl;
    }
    dec_indent();
    print_indent();
    this->out << "}";
  }

  if(s.else_stmts.size() > 0) {
    this->out << endl;
    print_indent();
    this->out << "else {" << endl;
    inc_indent();

    for(auto s : s.else_stmts) {
      print_indent();
      s->accept(*this);
      this->out << endl;
    }
    dec_indent();
    print_indent();
    this->out << "}";
  }
}

void PrintVisitor::visit(VarDeclStmt& s) {
  this->out << s.var_def.data_type.type_name << " " << s.var_def.var_name.lexeme() << " = ";
  s.expr.accept(*this);
}

void PrintVisitor::visit(AssignStmt& s) {
  int i = 0;
  for(auto l : s.lvalue) {
    this->out << l.var_name.lexeme();
    i++;
    if(i > 0 && i < s.lvalue.size())
      this->out << ".";
  }
  this->out << " = ";
  s.expr.accept(*this);
}

void PrintVisitor::visit(CallExpr& e) {
  this->out << e.fun_name.lexeme() << "(";
  for(int i = 0; i < e.args.size() - 1; i++) {
    e.args[i].accept(*this);
    this->out << ", ";
  }
  e.args[e.args.size() - 1].accept(*this);
  this->out << ")";
}

void PrintVisitor::visit(Expr& e) {
  if(e.negated == true) {
    this->out << "not (";
    e.first->accept(*this);
    this->out << ")";
  }
  else {
    e.first->accept(*this);
    if(e.op.has_value())
    {
      this->out << " " << e.op->lexeme() << " ";
    }
  }
  if(e.rest)
  {
    e.rest->accept(*this);
  }
}

void PrintVisitor::visit(SimpleTerm& t) {
  t.rvalue->accept(*this);
}

void PrintVisitor::visit(ComplexTerm& t) {
  this->out << "(";
  t.expr.accept(*this);
  this->out << ")";
}

void PrintVisitor::visit(SimpleRValue& v) {
  if(v.value.type() == TokenType::STRING_VAL) {
    this->out << "\"" << v.value.lexeme() << "\"";
  }
  else if(v.value.type() == TokenType::CHAR_VAL) {
    this->out << "'" << v.value.lexeme() << "'";
    return;
  }
  else {
    this->out << v.value.lexeme();
  }
}

void PrintVisitor::visit(NewRValue& v) {
  this->out << "new " << v.type.lexeme();
}

void PrintVisitor::visit(VarRValue& v) {
  for(int i = 0; i < v.path.size(); i++) {
    if(i > 0) {
      this->out << ".";
    }
    this->out << v.path[i].var_name.lexeme();
  }
}



void PrintVisitor::visit(SwitchStmt& s) {
  this->out << "switch (";
  // switch val
  s.switch_expr.accept(*this);
  
  this->out << ") {" << endl;
  inc_indent();

  for(auto b : s.cases) {
    this->out << endl;
    print_indent();
    this->out << "case (";
    // case val
    b.const_expr.accept(*this);
    
    this->out << ") :" << endl;
    inc_indent();

    for(auto s : b.stmts) {
      print_indent();
      s->accept(*this);
      this->out << endl;
    }

    // break statements

    dec_indent();
    print_indent();
  }

  if(s.defaults.size() > 0) {
    this->out << endl;
    print_indent();
    this->out << "default :" << endl;
    inc_indent();

    for(auto s : s.defaults) {
      print_indent();
      s->accept(*this);
      this->out << endl;
    }
    dec_indent();
    print_indent();
  }
  dec_indent();
  print_indent();
  this->out << "}";
}
