#include <assert.h>
#include <stdio.h>
#include <cstdlib>
#include "emit.h"
#include "cool-tree.h"
#include "symtab.h"
#include <stack>
#include <utility>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>

enum Basicness     {Basic, NotBasic};
#define TRUE 1
#define FALSE 0
typedef SymtabEntry <Symbol, Entry > methodclass;
typedef SymtabEntry <Symbol, Entry > attrclass;

class CgenClassTable;
typedef CgenClassTable *CgenClassTableP;

class CgenNode;
typedef CgenNode *CgenNodeP;

template <class SYM, class DAT>
class SpecializedTable;
class Env;

class CgenClassTable : public SymbolTable<Symbol,CgenNode> {
private:
   List<CgenNode> *nds;
   CgenNodeP root;
   int label;
   std::map<CgenNodeP, int> subtree_nodes;
// The following methods emit code for
// constants and global declarations.

   void code_global_data();
   void code_global_text();
   void code_bools(int);
   void code_select_gc();
   void code_constants();
   void code_class_nameTab();
   void code_objTab();
   void code_dispTab();
   void code_protObj();
   void code_class_init();
   void code_method_def();
   
   void install_constants();
   void install_features(CgenNodeP);
// The following creates an inheritance graph from
// a list of classes.  The graph is implemented as
// a tree of `CgenNode', and class names are placed
// in the base class symbol table.

   void install_basic_classes();
   void install_class(CgenNodeP nd);
   void install_classes(Classes cs);
   void build_inheritance_tree();
   void set_relations(CgenNodeP nd);
	void rebuild_list(CgenNodeP);
	int build_hierarchy_dfs(CgenNodeP);	
public:
    ostream& str;
	Env *env;
	std::map<CgenNodeP, std::pair<int, int> > mp;
	CgenClassTable(Classes, ostream& str);
	void code();
	int ind(Symbol s) {
		CgenNodeP nd = probe(s);
		int c = 0;
		for(List< CgenNode > *l = nds; l; l = l->tl()) {
			CgenNodeP se = l->hd();
			if(se == nd) return c;
			c++;
		}
		return -1;
	}
	
	CgenNodeP getclassnode(int i) {
		int c = 0;
		for(List<CgenNode> *l = nds; l; l = l->tl()) {
			CgenNodeP nd = l->hd();
			if(c == i) return nd;
		}
		return NULL;
	}
	
	int get_label() { return label++; }
	
	int dispatch_ind(Symbol c, Symbol m);
};


class CgenNode : public class__class {
private: 
   CgenNodeP parentnd;                        // Parent of class
   List<CgenNode> *children;                  // Children of class
   Basicness basic_status;                    // `Basic' if class is basic
                                              // `NotBasic' otherwise
   
public:
   CgenNode(Class_ c,
            Basicness bstatus,
            CgenClassTableP class_table);

   SpecializedTable< Symbol, Entry > *methodtable;     // All the methods applicable for this class
   SpecializedTable< Symbol, Entry > *attrtable;
   void add_child(CgenNodeP child);
   List<CgenNode> *get_children() { return children; }
   void set_parentnd(CgenNodeP p);
   CgenNodeP get_parentnd() { return parentnd; }
   int basic() { return (basic_status == Basic); }
};

class BoolConst 
{
 private: 
  int val;
 public:
  BoolConst(int);
  void code_def(ostream&, int boolclasstag);
  void code_ref(ostream&) const;
};

class Env : public SymbolTable<Symbol, std::string> {
public:
	CgenNodeP curr_class;
	int stack_disp;
	Env() {
		curr_class = NULL;
		stack_disp = 0;
	}
	
	void push() {
		stack_disp -= 4;
	}
	
	void pop() {
		stack_disp += 4;
	}
	
	int top() {
		return stack_disp;
	}
};

/*
 Besides symtab specific tasks, this table maintain list of (key, val)
 */

template <class SYM, class DAT>
class SpecializedTable : public SymbolTable<SYM, DAT> {
	typedef SymtabEntry<SYM,DAT> ScopeEntry;
public:
	List< ScopeEntry > *list = NULL;
	List<ScopeEntry> *get_list() {
		return list;
	}
	
	int ind(SYM s) {
		int c = 0;
		for(List< ScopeEntry > *l = list; l; l = l->tl()) {
			ScopeEntry *se = l->hd();
			if(se->get_id() == s) return c;
			c++;
		}
		return -1;
	}
	
	int len() {
		return list_length(list);
	}
	
	ScopeEntry * add(SYM s, DAT *i) {
		ScopeEntry *se = this->addid(s, i);
			list = new List<ScopeEntry> (se, list);
			return se;
	}
	
	ScopeEntry * update(SYM s, DAT *i) {
		ScopeEntry *se = this->addid(s, i);
		for(List <ScopeEntry> *l = list; l; l = l->tl()) {
			ScopeEntry *se = l->hd();
			//TODO: possible memory leak
			if(se->get_id()) {
				ScopeEntry *sen = new ScopeEntry(se->hd()->get_id(), i);
				l->hd() = sen;
				return;
			}
		}
		list = new List<ScopeEntry> (se, list);
		return se;
	}
};
