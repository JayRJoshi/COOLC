#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"
#include <map>
#include <set>
#include <utility>
#include <vector>

#define TRUE 1
#define FALSE 0

class ClassTable;
typedef ClassTable *ClassTableP;


// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.
using std::map;
using std::set;
using std::vector;

class ClassTable {
private:
  map<Symbol, Classp> classmap;
  set<Symbol> basic_classes;
  map<Symbol, map<Symbol, Methodp> > methodmap;
  map<Symbol, vector<Symbol> > intree;
  
  bool has_main;
  int semant_errors;
  void install_basic_classes();
  void install_other_classes();
  void install_methodmap();
  void has_cycle();
  ostream& error_stream;
  
public:
  ClassTable(Classes);
  Classes classes;
  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);
  void add_basic_class(Class_);
  bool is_basic(Symbol);
  bool is_basic(Class_);
  bool add_class(Class_);
  Class_ get_class(Symbol);
  bool has_class(Symbol);
  bool has_class(Class_);
  void add_child(Symbol par, Symbol child);
  bool add_method(Symbol cname, Feature f);
  bool has_method(Symbol cname, Feature f);
  vector<Symbol> &get_childs(Classp);
  bool subtype(Symbol child, Symbol par, Symbol curr_class);
  Symbol lcu(Symbol, Symbol, Symbol);
  Methodp get_method(Symbol cname, Symbol mname, Symbol curr_class);
  bool valid_signature(Symbol par, Methodp f2);
  bool valid_signature(Formals fml, Classp c);
};


#endif

