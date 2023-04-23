//----------------------------------------------------------------------
// FILE: vm.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: 
// DESC: 
//----------------------------------------------------------------------

#include <iostream>
#include "vm.h"
#include "mypl_exception.h"
#include <iostream>

using namespace std;


void VM::error(string msg) const
{
  throw MyPLException::VMError(msg);
}


void VM::error(string msg, const VMFrame& frame) const
{
  int pc = frame.pc - 1;
  VMInstr instr = frame.info.instructions[pc];
  string name = frame.info.function_name;
  msg += " (in " + name + " at " + to_string(pc) + ": " +
    to_string(instr) + ")";
  throw MyPLException::VMError(msg);
}


string to_string(const VM& vm)
{
  string s = "";
  for (const auto& entry : vm.frame_info) {
    const string& name = entry.first;
    s += "\nFrame '" + name + "'\n";
    const VMFrameInfo& frame = entry.second;
    for (int i = 0; i < frame.instructions.size(); ++i) {
      VMInstr instr = frame.instructions[i];
      s += "  " + to_string(i) + ": " + to_string(instr) + "\n"; 
    }
  }
  return s;
}


void VM::add(const VMFrameInfo& frame)
{
  frame_info[frame.function_name] = frame;
}


void VM::run(bool DEBUG)
{
  // grab the "main" frame if it exists
  if (!frame_info.contains("main"))
    error("No 'main' function");
  shared_ptr<VMFrame> frame = make_shared<VMFrame>();
  frame->info = frame_info["main"];
  call_stack.push(frame);

  // run loop (keep going until we run out of instructions)
  while (!call_stack.empty() and frame->pc < frame->info.instructions.size()) {

    // get the next instruction
    VMInstr& instr = frame->info.instructions[frame->pc];

    // increment the program counter
    ++frame->pc;

    // for debugging
    if (DEBUG) {
      // TODO
      cerr << endl << endl;
      cerr << "\t FRAME.........: " << frame->info.function_name << endl;
      cerr << "\t PC............: " << (frame->pc - 1) << endl;
      cerr << "\t INSTR.........: " << to_string(instr) << endl;
      cerr << "\t NEXT OPERAND..: ";
      if (!frame->operand_stack.empty())
        cerr << to_string(frame->operand_stack.top()) << endl;
      else
        cerr << "empty" << endl;
      cerr << "\t NEXT FUNCTION.: ";
      if (!call_stack.empty())
        cerr << call_stack.top()->info.function_name << endl;
      else
        cerr << "empty" << endl;
    }

    //----------------------------------------------------------------------
    // Literals and Variables
    //----------------------------------------------------------------------

    if (instr.opcode() == OpCode::PUSH) {
      frame->operand_stack.push(instr.operand().value());
    }

    else if (instr.opcode() == OpCode::POP) {
      frame->operand_stack.pop();
    }

    // TODO: Finish LOAD and STORE

    else if(instr.opcode() == OpCode::LOAD) {
      VMValue x = frame->variables.at(get<int>(instr.operand().value()));
      frame->operand_stack.push(x);
    }
    else if(instr.opcode() == OpCode::STORE) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();

      if(get<int>(instr.operand().value()) >= frame->variables.size()) {
        frame->variables.push_back(x);
      } else {
        frame->variables[get<int>(instr.operand().value())] = x;
      }

    }
    
    //----------------------------------------------------------------------
    // Operations
    //----------------------------------------------------------------------

    else if (instr.opcode() == OpCode::ADD) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(add(y, x));
    }

    // TODO: Finish SUB, MUL, DIV, AND, OR, NOT, COMPLT, COMPLE,
    // CMPGT, CMPGE, CMPEQ, CMPNE

    else if (instr.opcode() == OpCode::SUB) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(sub(y, x));
    }
    else if (instr.opcode() == OpCode::MUL) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(mul(y, x));
    }
    else if (instr.opcode() == OpCode::DIV) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(div(y, x));
    }
    else if (instr.opcode() == OpCode::AND) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(get<bool>(y) && get<bool>(x));
    }
    else if (instr.opcode() == OpCode::OR) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(get<bool>(y) || get<bool>(x));
    }
    else if (instr.opcode() == OpCode::NOT) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      frame->operand_stack.push(!get<bool>(x));
    }
    else if (instr.opcode() == OpCode::CMPLT) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(lt(y, x));
    }
    else if (instr.opcode() == OpCode::CMPLE) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(le(y, x));
    }
    else if (instr.opcode() == OpCode::CMPGT) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(gt(y, x));
    }
    else if (instr.opcode() == OpCode::CMPGE) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(ge(y, x));
    }
    else if (instr.opcode() == OpCode::CMPEQ) {
      VMValue x = frame->operand_stack.top();
      //ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      //ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(eq(y,x));
    }
    else if (instr.opcode() == OpCode::CMPNE) {
      VMValue x = frame->operand_stack.top();
      //ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      //ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(!get<bool>(eq(y, x)));
    }

    
    
    //----------------------------------------------------------------------
    // Branching
    //----------------------------------------------------------------------

    // TODO: Finish JMP and JMPF
    else if (instr.opcode() == OpCode::JMP) {
      frame->pc = get<int>(instr.operand().value());
    }
    else if (instr.opcode() == OpCode::JMPF) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      if(get<bool>(x) == false) {
        frame->pc = get<int>(instr.operand().value());
      }
    }
    
    //----------------------------------------------------------------------
    // Functions
    //----------------------------------------------------------------------


    // TODO: Finish CALL, RET

    else if(instr.opcode() == OpCode::CALL)
    {
      string func_name = get<string>(instr.operand().value());
      shared_ptr<VMFrame> new_frame = make_shared<VMFrame>();
      new_frame->info = frame_info[func_name];

      call_stack.push(new_frame);

      for(int i = 0; i < frame_info[func_name].arg_count; i++) {
        VMValue v = frame->operand_stack.top();
        new_frame->operand_stack.push(v);
        frame->operand_stack.pop();
      }
      
      frame = new_frame;
    }

    else if(instr.opcode() == OpCode::RET)
    {
      // 1. Pop the return value off the current frame's operand stack
      VMValue v = frame->operand_stack.top();
      frame->operand_stack.pop();

      // 2. Pop the frame off the stack
      call_stack.pop();

      if(!call_stack.empty())
      {
        frame = call_stack.top();
        frame->operand_stack.push(v);
      }

    }

    
    //----------------------------------------------------------------------
    // Built in functions
    //----------------------------------------------------------------------


    else if (instr.opcode() == OpCode::WRITE) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      cout << to_string(x);
    }

    else if (instr.opcode() == OpCode::READ) {
      string val = "";
      getline(cin, val);
      frame->operand_stack.push(val);
    }

    // TODO: Finish SLEN, ALEN, GETC, TODBL, TOSTR, CONCAT

    else if(instr.opcode() == OpCode::SLEN)
    {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      string x_str = get<string>(x);
      int size = x_str.size();
      frame->operand_stack.push(size);
    }
    else if(instr.opcode() == OpCode::ALEN)
    {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      int size = array_heap[get<int>(x)].size();
      frame->operand_stack.push(size);
    }

    else if(instr.opcode() == OpCode::TODBL)
    {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      double x_dub;

      if (holds_alternative<int>(x)) 
        x_dub = static_cast<double>(get<int>(x));

      else if (holds_alternative<std::string>(x)) {
        try {
          x_dub = stod(get<std::string>(x));
        }
        catch(const std::exception& e) {
          string msg = "cannot convert string to double";
          error(msg, *frame);
        }
      }

      frame->operand_stack.push(x_dub);
    }
    else if (instr.opcode() == OpCode::TOINT) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      int x_dub;

      if (holds_alternative<double>(x)) 
        x_dub = static_cast<int>(get<double>(x));

      else if (holds_alternative<std::string>(x)) {
        try {
          x_dub = stoi(get<std::string>(x));
        }
        catch(const std::exception& e) {
          string msg = "cannot convert string to int";
          error(msg, *frame);
        }
      }

      frame->operand_stack.push(x_dub);
    }
    else if (instr.opcode() == OpCode::TOSTR) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      //string x_dub = to_string(get<string>(x));
      frame->operand_stack.push(to_string(x));
    }
    else if(instr.opcode() == OpCode::CONCAT)
    {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(get<string>(y) + get<string>(x));
    }
    else if(instr.opcode() == OpCode::GETC)
    {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();

      string x_str = get<string>(x);
      int size = x_str.size();
      if(get<int>(y) >= size) {
        string msg = "out-of-bounds string index";
        error(msg, *frame);
      }
      else if(get<int>(y) < 0) {
        string msg = "out-of-bounds string index";
        error(msg, *frame);
      }
      else {
        string character;
        character.push_back(x_str[get<int>(y)]);
        frame->operand_stack.push(character);
      }
    }
    
    //----------------------------------------------------------------------
    // heap
    //----------------------------------------------------------------------

    // TODO: Finish ALLOCS, ALLOCA, ADDF, SETF, GETF, SETI, GETI
    else if(instr.opcode() == OpCode::ALLOCS)
    {
      struct_heap[next_obj_id] = {};
      frame->operand_stack.push(next_obj_id);
      ++next_obj_id;
    }
    else if(instr.opcode() == OpCode::ALLOCA)
    {
      VMValue val = frame->operand_stack.top();
      frame->operand_stack.pop();
      int size = get<int>(frame->operand_stack.top());
      frame->operand_stack.pop();
      array_heap[next_obj_id] = vector<VMValue>(size, val);
      frame->operand_stack.push(next_obj_id);
      ++next_obj_id;
    }
    else if(instr.opcode() == OpCode::ADDF)
    {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      int i = get<int>(x);
      struct_heap[i][get<string>(instr.operand().value())];
    }

    else if(instr.opcode() == OpCode::SETF)
    {
      VMValue x = frame->operand_stack.top();
      //ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();

      int i = get<int>(y);
      struct_heap[i][get<string>(instr.operand().value())] = x;
    }

    else if(instr.opcode() == OpCode::GETF)
    {
      VMValue x = frame->operand_stack.top();
      //ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      int i = get<int>(x);
      frame->operand_stack.push(struct_heap[i][get<string>(instr.operand().value())]);
    }

    else if(instr.opcode() == OpCode::SETI)
    {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();

      VMValue z = frame->operand_stack.top();
      ensure_not_null(*frame, z);
      frame->operand_stack.pop();

      int size = array_heap[get<int>(z)].size();
      if(get<int>(y) >= size) {
        string msg = "out-of-bounds array index";
        error(msg, *frame);
      }
      else if(get<int>(y) < 0) {
        string msg = "out-of-bounds array index";
        error(msg, *frame);
      }
      else {
        array_heap[get<int>(z)].at(get<int>(y)) = x;
      }
    }

    else if(instr.opcode() == OpCode::GETI)
    {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();

      int size = array_heap[get<int>(y)].size();
      if(get<int>(x) >= size) {
        string msg = "out-of-bounds array index";
        error(msg, *frame);
      }
      else if(get<int>(x) < 0) {
        string msg = "out-of-bounds array index";
        error(msg, *frame);
      }
      else {
        frame->operand_stack.push(array_heap[get<int>(y)][get<int>(x)]);
      }
    }

    //----------------------------------------------------------------------
    // special
    //----------------------------------------------------------------------

    
    else if (instr.opcode() == OpCode::DUP) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(x);
      frame->operand_stack.push(x);      
    }

    else if (instr.opcode() == OpCode::NOP) {
      // do nothing
    }
    
    else {
      error("unsupported operation " + to_string(instr));
    }
  }
}


void VM::ensure_not_null(const VMFrame& f, const VMValue& x) const
{
  if (holds_alternative<nullptr_t>(x))
    error("null reference", f);
}


VMValue VM::add(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) + get<int>(y);
  else
    return get<double>(x) + get<double>(y);
}

// TODO: Finish the rest of the following arithmetic operators

VMValue VM::sub(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) - get<int>(y);
  else
    return get<double>(x) - get<double>(y);
}

VMValue VM::mul(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) * get<int>(y);
  else
    return get<double>(x) * get<double>(y);
}

VMValue VM::div(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) / get<int>(y);
  else
    return get<double>(x) / get<double>(y);
}


VMValue VM::eq(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) == get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) == get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) == get<string>(y);
  else
    return get<bool>(x) == get<bool>(y);
}

// TODO: Finish the rest of the comparison operators

VMValue VM::lt(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) < get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) < get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) < get<string>(y);
  else
    return get<bool>(x) < get<bool>(y);
}

VMValue VM::le(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) <= get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) <= get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) <= get<string>(y);
  else
    return get<bool>(x) <= get<bool>(y);
}

VMValue VM::gt(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) > get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) > get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) > get<string>(y);
  else
    return get<bool>(x) > get<bool>(y);
}

VMValue VM::ge(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) >= get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) >= get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) >= get<string>(y);
  else
    return get<bool>(x) >= get<bool>(y);
}

