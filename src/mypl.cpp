//----------------------------------------------------------------------
// FILE: mypl.cpp
// DATE: Spring 2023
// AUTH: Jackie Ramsey
// DESC: create a basic skeleton for mypl interpreter program
//----------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include "token.h"
#include "lexer.h"
#include "simple_parser.h"
#include "ast_parser.h"
#include "print_visitor.h"
#include "ast.h"
#include "semantic_checker.h"
#include "vm.h"
#include "code_generator.h"

using namespace std;


void usage() {
  cout << "Usage: ./mypl [option] [script-file]" << endl;
  cout << "Options:" << endl;
  cout << "  --help prints this message" << endl;
  cout << "  --lex displays token information" << endl;
  cout << "  --parse checks for syntax errors" << endl;
  cout << "  --print pretty prints program" << endl;
  cout << "  --check statically checks program" << endl;
  cout << "  --ir print intermediate (code) representation" << endl;
}

int main(int argc, char* argv[])
{

  if(argc == 1) {
    // case: ./mypl
    cout << "[Normal Mode]" << endl;
    istream* input = &cin;
    try {
      Lexer lexer(*input);
      ASTParser parser(lexer);
      Program p = parser.parse();
      SemanticChecker t;
      p.accept(t);
      VM vm;
      CodeGenerator g(vm);
      p.accept(g);
      vm.run();
    } catch (MyPLException& ex) {
      cerr << ex.what() << endl;
    }
  }

  else if(argc == 2) {
    istream* input = &cin;

    if(string(argv[1]) == "--help") {
      usage();
    }

    else if(string(argv[1]) == "--lex") {
      try {
          Lexer lexer(*input);
          Token t = lexer.next_token();
          cout << to_string(t) << endl;
          while (t.type() != TokenType::EOS) {
            t = lexer.next_token();
            cout << to_string(t) << endl;
          }
        } catch (MyPLException& ex) {
          cerr << ex.what() << endl;
        }
    }

    else if(string(argv[1]) == "--parse") {
      try {
        Lexer lexer(*input);
        SimpleParser parser(lexer);
        parser.parse();
      } catch (MyPLException& ex) {
        cerr << ex.what() << endl;
      }

    }
    else if(string(argv[1]) == "--print") {
      try {
          Lexer lexer(*input);
          ASTParser parser(lexer);
          Program p = parser.parse();
          PrintVisitor v(cout);
          p.accept(v);
        } catch (MyPLException& ex) {
        cerr << ex.what() << endl;
      }
    }
    else if(string(argv[1]) == "--check") {
      try {
        Lexer lexer(*input);
        ASTParser parser(lexer);
        Program p = parser.parse();
        SemanticChecker v;
        p.accept(v);
      } catch (MyPLException& ex) {
        cerr << ex.what() << endl;
      }
    }
    else if(string(argv[1]) == "--ir") {
      try {
        Lexer lexer(*input);
        ASTParser parser(lexer);
        Program p = parser.parse();
        SemanticChecker t;
        p.accept(t);
        VM vm;
        CodeGenerator g(vm);
        p.accept(g);
        cout << to_string(vm) << endl;
      } catch (MyPLException& ex) {
        cerr << ex.what() << endl;
      }

    }
    else {
    //   // case: invalid mode or file
      input = new ifstream(argv[1]);
      cout << "[Normal Mode]" << endl;
      try {
        Lexer lexer(*input);
        ASTParser parser(lexer);
        Program p = parser.parse();
        SemanticChecker t;
        p.accept(t);
        VM vm;
        CodeGenerator g(vm);
        p.accept(g);
        vm.run();
      } catch (MyPLException& ex) {
        cerr << ex.what() << endl;
      }
    }
  }

  else if(argc == 3) {
    istream* input = &cin;
    input = new ifstream(argv[2]);

    // case: invalid file
    if(input->fail()) {
      cout << "ERROR: Unable to open file '" << string(argv[2]) << "'" << endl;
    }
    
    else if(string(argv[1]) == "--lex") {
      try {
          Lexer lexer(*input);
          Token t = lexer.next_token();
          cout << to_string(t) << endl;
          while (t.type() != TokenType::EOS) {
            t = lexer.next_token();
            cout << to_string(t) << endl;
          }
        } catch (MyPLException& ex) {
          cerr << ex.what() << endl;
        }
    }
    else if(string(argv[1]) == "--parse") {
      try {
        Lexer lexer(*input);
        SimpleParser parser(lexer);
        parser.parse();
      } catch (MyPLException& ex) {
        cerr << ex.what() << endl;
      }
    }
    else if(string(argv[1]) == "--print") {
      try {
          Lexer lexer(*input);
          ASTParser parser(lexer);
          Program p = parser.parse();
          PrintVisitor v(cout);
          p.accept(v);
        } catch (MyPLException& ex) {
        cerr << ex.what() << endl;
      }
    }
    else if(string(argv[1]) == "--check") {
      try {
        Lexer lexer(*input);
        ASTParser parser(lexer);
        Program p = parser.parse();
        SemanticChecker v;
        p.accept(v);
      } catch (MyPLException& ex) {
        cerr << ex.what() << endl;
      }
    }
    else if(string(argv[1]) == "--ir") {
      try {
        Lexer lexer(*input);
        ASTParser parser(lexer);
        Program p = parser.parse();
        SemanticChecker t;
        p.accept(t);
        VM vm;
        CodeGenerator g(vm);
        p.accept(g);
        cout << to_string(vm) << endl;
      } catch (MyPLException& ex) {
        cerr << ex.what() << endl;
      }
    }
    // case: invalid mode
    else {
      cout << "ERROR: Unable to open file '" << string(argv[1]) << "'" << endl;
      return 1;
    }
    delete(input);
  }
  else {
    // case: too many parameters
    usage();
  }

}

