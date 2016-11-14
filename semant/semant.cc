

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stack>
#include <queue>
#include <map>
#include <set>
#include "utilities.h"

using std::map;
using std::set;
using std::vector;
using std::queue;
using std::stack;

#include "semant.h"


extern int semant_debug;
extern char *curr_filename;
typedef unsigned int uint_t;
void printset(set<Symbol>&);
void fatal_error();
//#define CDEBUG
//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol 
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg         = idtable.add_string("arg");
    arg2        = idtable.add_string("arg2");
    Bool        = idtable.add_string("Bool");
    concat      = idtable.add_string("concat");
    cool_abort  = idtable.add_string("abort");
    copy        = idtable.add_string("copy");
    Int         = idtable.add_string("Int");
    in_int      = idtable.add_string("in_int");
    in_string   = idtable.add_string("in_string");
    IO          = idtable.add_string("IO");
    length      = idtable.add_string("length");
    Main        = idtable.add_string("Main");
    main_meth   = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any 
    //   user-defined class.
    No_class    = idtable.add_string("_no_class");
    No_type     = idtable.add_string("_no_type");
    Object      = idtable.add_string("Object");
    out_int     = idtable.add_string("out_int");
    out_string  = idtable.add_string("out_string");
    prim_slot   = idtable.add_string("_prim_slot");
    self        = idtable.add_string("self");
    SELF_TYPE   = idtable.add_string("SELF_TYPE");
    Str         = idtable.add_string("String");
    str_field   = idtable.add_string("_str_field");
    substr      = idtable.add_string("substr");
    type_name   = idtable.add_string("type_name");
    val         = idtable.add_string("_val");
}



ClassTable::ClassTable(Classes classes_) : semant_errors(0) , error_stream(cerr) {

    /* Fill this in */
#ifdef CDEBUG
    cout << "Entered in class table constructor" << endl;
#endif
	has_main = false;
    classes = classes_;
	install_basic_classes();
	install_other_classes();
	has_cycle();
	install_methodmap();
}

void ClassTable::add_basic_class(Class_ c) {
	add_class(c);
	basic_classes.insert(((Classp)c)->get_name()).second;
}

bool ClassTable::add_class(Class_ cp) {
	Classp c = (Classp) cp;
	if(has_class(cp)) return false;
	classmap[c->get_name()] = (Classp) c;
	return true;
}

bool ClassTable::has_class(Class_ c) {
	return classmap.find(((Classp) c)->get_name()) != classmap.end();
}

bool ClassTable::has_class(Symbol c) {
	return classmap.find(c) != classmap.end();
}

void ClassTable::add_child(Symbol par, Symbol child) {
	intree[par].push_back(child);
}

void ClassTable::has_cycle() {
	stack<Symbol> st;
	set<Symbol> s;
	st.push(No_class);
	while(!st.empty()) {
		Symbol c = st.top();
		
		// following just can't happen...
		/*if(s.find(c) != s.end()) {
			semant_error(classmap[c]) << " Inheritance Tree has a cycle " << endl;
			fatal_error();
		}*/
		
		s.insert(c);
		//printset(s);
		st.pop();
		
		for(uint_t i=0; i<intree[c].size(); i++) {
			Symbol ch = intree[c][i];
			//cout << " has_cycle: " << c << " " << ch << endl;
			st.push(ch);
		}
	}
	
	bool outcasted_classes = false;
	for(map<Symbol, vector<Symbol> >::iterator it = intree.begin(); it!=intree.end(); it++) {
		if(s.find(it->first) == s.end()) { semant_error(classmap[it->first]) 
		 << " Out of tree class spotted: " << it->first << endl;
			outcasted_classes = true;
		}
	}
	if(outcasted_classes) fatal_error();
}

bool ClassTable::valid_signature(Symbol par, Methodp f2) {
	Methodp f1 = methodmap[par][f2->get_name()];
	
	Formals fmls1 = f1->get_formals(), fmls2 = f2->get_formals();
	if(fmls1->len() != fmls2->len() || f1->get_return_type() != f2->get_return_type()) {
		return false;
	}
	for(int i=0; fmls1->more(i); i++) {
		if(((Formalp)fmls1->nth(i))->get_type_decl() != ((Formalp)fmls2->nth(i))->get_type_decl()) return false;
	}
	return true;
}

bool ClassTable::valid_signature(Formals fmls, Classp c) {
	std::set<Symbol> s;
	bool has_errors = false;
	for(int i=0; fmls->more(i); i++) {
		Formalp fml = (Formalp) fmls->nth(i);
		Symbol name = fml->get_name();
		if(name == self) {
			semant_error(c) << "self as parameter" << endl;
			has_errors = true;
		}
		if(s.find(name) != s.end()) {
			semant_error(c) << "variable " << name << " is already declared" << endl;
			has_errors = true;
		}
		if(fml->get_type_decl() == SELF_TYPE) {
			semant_error(c) << "Parameter SELF_TYPE not allowed" << endl;
			has_errors = true;
		}
		if(!has_class(fml->get_type_decl())) {
			semant_error(c) << "Class " << fml->get_type_decl() << " is undeclared" << endl;
			has_errors = true;
		}
		s.insert(name);
	}
	if(has_errors) return false;
	return true;
}

bool ClassTable::add_method(Symbol cname, Feature fn) {
#ifdef CDEBUG 
cout << "inside add_method " << cname << " " << ((Methodp)fn)->get_name() << endl; 
#endif
	Methodp f = (Methodp) fn;
	Classp c = classmap[cname];
	Symbol par = c->get_parent();
	
	if(has_class(par) && has_method(par, fn)) {
		if(!valid_signature(par, f)) {
			return false;
		}
	}
	methodmap[cname][f->get_name()] = f;
	return true;
}
	
bool ClassTable::has_method(Symbol cname, Feature fn) {
	Methodp f = (Methodp) fn;
	if(methodmap.find(cname) == methodmap.end() ||
		methodmap[cname].find(f->get_name()) == methodmap[cname].end()) return false;
	return true;
}

Class_ ClassTable::get_class(Symbol s) {
	return classmap[s];
}

void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
   // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");
    
    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.
    
    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
	class_(Object, 
	       No_class,
	       append_Features(
			       append_Features(
					       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
					       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
			       single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	       filename);

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
	class_(IO, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       single_Features(method(out_string, single_Formals(formal(arg, Str)),
										      SELF_TYPE, no_expr())),
							       single_Features(method(out_int, single_Formals(formal(arg, Int)),
										      SELF_TYPE, no_expr()))),
					       single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
			       single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	       filename);  

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
	class_(Int, 
	       Object,
	       single_Features(attr(val, prim_slot, no_expr())),
	       filename);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
	class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
	class_(Str, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       append_Features(
									       single_Features(attr(val, Int, no_expr())),
									       single_Features(attr(str_field, prim_slot, no_expr()))),
							       single_Features(method(length, nil_Formals(), Int, no_expr()))),
					       single_Features(method(concat, 
								      single_Formals(formal(arg, Str)),
								      Str, 
								      no_expr()))),
			       single_Features(method(substr, 
						      append_Formals(single_Formals(formal(arg, Int)), 
								     single_Formals(formal(arg2, Int))),
						      Str, 
						      no_expr()))),
	       filename);
	       
	      	add_basic_class(Object_class);
	      	add_basic_class(IO_class);
	      	add_basic_class(Int_class);
	      	add_basic_class(Bool_class);
	      	add_basic_class(Str_class);
	      	
	      	
	      	add_child(No_class, Object);
	      	add_child(Object, IO);
	      	add_child(Object, Int);
	      	add_child(Object, Str);
	      	add_child(Object, Bool);
	      	
#ifdef CDEBUG
    cout << "Exited from install basic classes" << endl;
#endif

}

void ClassTable::install_other_classes() {
	Class_ c;
#ifdef CDEBUG
    cout << "Entered in insatll other classes constructor" << endl;
#endif

	for(int i=0; classes->more(i); i++) {
		c =  classes->nth(i);
		Symbol cname = ((Classp) c)->get_name();
		if(!add_class(c)) {
			semant_error(c) << "Redefinition of class " << cname << endl;
			fatal_error();
		}
		if(is_basic(c)) {
			semant_error(c) << "Redefinition of Basic class " << cname << endl;
			fatal_error();
		}
		
		if(cname == SELF_TYPE) {
			semant_error(c) << "Class name can't be SELF_NAME" << endl;
			fatal_error();
		}
		
		if(cname == Main) {
			has_main = true;
		}
	}

	if(!has_main) {semant_error() << "Class Main is not defined." << endl; }
	
	for(int i=0; classes->more(i); i++) {
		Classp c = (Classp) classes->nth(i);
		Symbol cp = c->get_parent();
		if(!has_class(cp)) {
			semant_error(c) << "Parent class " << cp << " of " << c->get_name()
			<< " is not defined " << endl;
			fatal_error();
		}
		if(cp == Int || cp == Str || cp == Bool || cp == SELF_TYPE) {
			semant_error(c) << "Class " << c->get_name() << " Inherits from class "
			<< cp << endl;
			fatal_error("Compilation halted due to static semantic errors.");
		}
		add_child(cp, c->get_name());
	}
}

void ClassTable::install_methodmap() {
	queue<Symbol> q;
	q.push(Object);
	bool has_main_meth = false;
	Symbol c;
	Classp cl;
	Class_ ch;
	while(!q.empty()) {
		c = q.front();
		cl = classmap[c];
		q.pop();
		
		Features fs = classmap[c]->get_features();
		std::set<Symbol> methods;
		for(int i=0; fs->more(i); i++) {
			Feature f = fs->nth(i);
			if(f->is_type_method()) {
				Methodp m = (Methodp) f;
				if(methods.find(m->get_name()) != methods.end()){
					semant_error(cl) << "Method " << m->get_name() << " is already defined" << endl;
					continue;
				}
				
				if(!add_method(c, f)) {
					semant_error(cl) << "Inhertited method " << c << " :" << m->get_name() << " has conflicts." << endl;
					continue;
				}
				
				if(m->get_name() == main_meth && c == Main) {
					has_main_meth = true;
					if(m->get_formals()->len() > 0){
						semant_error(cl) << "main method inside Main class has arguments!" << endl;
					}
					continue;
				}
				
				if(!valid_signature(m->get_formals(), cl)) continue;
				methods.insert(m->get_name());
			}
		}
		for(uint_t i=0; i<intree[c].size(); i++) {
			Symbol ch = intree[c][i];
			q.push(ch);
			for(map<Symbol, Methodp>::iterator it=methodmap[c].begin(); it!=methodmap[c].end(); it++) {
				add_method(ch, it->second); 
			}
		}

	}
	
	if(has_main && !has_main_meth) {
		semant_error(cl) << "No main method inside Main class" << endl;
	}
}

bool ClassTable::is_basic(Symbol c) {
	return basic_classes.find(c) != basic_classes.end();
}

bool ClassTable::is_basic(Class_ c) {
	return basic_classes.find(((Classp) c)->get_name()) != basic_classes.end();
}

bool ClassTable::subtype(Symbol child, Symbol par, Symbol curr_class) {
	#ifdef CDEBUG 
		cout << "Inside subtype:" << child <<  "  " << par << endl; 
		#endif
	if(child == No_class) return true;
	if(par == SELF_TYPE && child != SELF_TYPE) return false;
	if(par == SELF_TYPE) par = curr_class;
	if(child == SELF_TYPE) child = curr_class;
	 
	if(!has_class(child) || !has_class(par)) {
		#ifdef CDEBUG 
		cout << "One of " << child <<  " Or " << par << " is not defined." << endl; 
		#endif
		return false;
	}
	
	Classp ch = (Classp) get_class(child);
	Classp p = (Classp) get_class(par);
	while(true) {
	#ifdef CDEBUG 
		cout << "child is " << ch->get_name() << endl; 
	#endif
		if(ch == p) return true;
		if(ch->get_parent() == No_class) break;
		ch = classmap[ch->get_parent()];
	}
	#ifdef CDEBUG 
	 cout << "exiting subtype " << child << " " << par << endl; 
	#endif
	return false;
}

Symbol ClassTable::lcu(Symbol t1, Symbol t2, Symbol curr_class) {
	while(!subtype(t1, t2, curr_class)) {
		t2 = classmap[t2]->get_parent();
	}
	return t2;
}

vector<Symbol> &ClassTable::get_childs(Classp c) {
	return intree[c->get_name()];
}

Methodp ClassTable::get_method(Symbol cname, Symbol mname, Symbol curr_class) {
#ifdef CDEBUG 
cout << "inside get_method " << cname << " " << mname << endl; 
#endif

	if(cname == SELF_TYPE) cname = curr_class;
	if(methodmap[cname].find(mname) != methodmap[cname].end()) 
		return methodmap[cname][mname];
	return (Methodp) NULL;
}

Symbol program_class::traverse(TypeEnv *t) {
	#ifdef CDEBUG 
	cout << "Program class traverse "<< endl; 
	#endif
	t->ct->get_class(Object)->traverse(t);
	return Object;
}

Symbol class__class::traverse(TypeEnv *t) {
	t->enterscope();
	t->set_curr_class(this->get_name());
	Features fs = this->get_features();
#ifdef CDEBUG 
cout << "Class class traverse " << get_name() << endl; 
#endif
	//set_type(Object);
	for(int i=0; fs->more(i); i++) {
		Feature f = fs->nth(i);
		if(!f->is_type_method()) {
			Symbol fname = ((Attrp)f)->get_name();
			Symbol type = ((Attrp)f)->get_type_decl();
			if(fname == self) {
				t->semant_error() << "self can't be initialised." << endl;
				return Object;
			}
			else if(t->probe(f)) {
				t->semant_error() << "Redefinition of id " << fname << endl;
				return Object;
			}
			else if(t->hasid(f)) {
				t->semant_error() << "Inherited attribute " << get_name() << " is defined again" << endl;
				return Object;
			}
			else {
#ifdef CDEBUG 
cout << "id added" << fname << " " << type << endl; 
#endif
			 t->addid(fname, type);
			 }
		}
	}
	
	for(int i=0; fs->more(i); i++) {
		fs->nth(i)->traverse(t);
	}
	
	#ifdef CDEBUG
	cout << "Class " << get_name() << endl;
	t->dump_table();
	#endif
	vector<Symbol> &childs = t->ct->get_childs(this);
	for(uint_t i=0; i<childs.size(); i++) {
		Classp ch = (Classp) t->ct->get_class(childs[i]);
		ch->traverse(t);
	}

	t->exitscope();
	return Object;
}

Symbol method_class::traverse(TypeEnv *t) {
#ifdef CDEBUG
cout << "method class traverse " << get_name() << endl; 
#endif
	t->enterscope();
	Symbol rt = this->get_return_type();
	
	// Formals have been type verified already
	Formals fmls = this->get_formals();
	for(int i=0; fmls->more(i); i++) {
		#ifdef CDEBUG 
		cout << "Inside formals" << endl; 
		#endif
		Formalp fml = (Formalp) fmls->nth(i);
		fml->traverse(t);
	}
	
	Symbol type = get_expr()->traverse(t);
	
	if(!t->ct->subtype(type, rt, t->get_curr_class())) {
		// Note that following error also results in case one of the class names is not defined...
		t->semant_error() << "Method " << this->get_name() 
		 << " has conflicting return-type ( " << rt << " ) and expr-type \
 (" << type << ") " << endl;
		return Object;
	}
	
	t->exitscope();
	return rt;
}
Symbol attr_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "attr class traverse " << get_name() << endl; 
#endif
	Symbol type = get_init()->traverse(t);
	if(!t->ct->subtype(type, this->get_type_decl(), t->get_curr_class())) {
		t->semant_error() << "Conflicting types: init (" << type<< ") and \
type declared (" << this->get_type_decl() << ")" << endl;
		return Object;
	}
	return get_type_decl();
}

Symbol formal_class::traverse(TypeEnv *t) {
#ifdef CDEBUG
cout << "Formal class traverse " << get_name() << endl; 
#endif
	if(this->get_type_decl() == SELF_TYPE) {
		t->semant_error() << "SELF_TYPE as parameter type is not supported" << endl;
		return Object;	
	}
	if(this->get_name() == self) {
		t->semant_error() << "objectid can't be named self" << endl;
		return Object;
	}
#ifdef CDEBUG 
cout << "id added" << get_name() << " " << get_type_decl() << endl; 
#endif
	t->addid(this->get_name(), this->get_type_decl());
	return this->get_type_decl();
}

Symbol branch_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "Branch class traverse " << get_name() << endl; 
#endif
	t->enterscope();
#ifdef CDEBUG 
cout << "id added" << get_name() << " " << get_type_decl() << endl; 
#endif
	t->addid(get_name(), get_type_decl());
	Symbol type = get_expr()->traverse(t);
	if(!t->ct->subtype(type, get_type_decl(), t->get_curr_class())) {
		t->semant_error() << " Conflicting types inside branch " << type << " " << get_type_decl() << endl;
		return Object;
	}
	t->exitscope();
#ifdef CDEBUG 
cout << "Branch class ends " << get_name() << " " << type << endl; 
#endif
	return type;
}

Symbol assign_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "assign class traverse " << get_name() << endl; 
#endif

	if(!t->hasid(get_name())) {
		t->semant_error() << "Id " << get_name() << " is not defined." << endl;
		set_type(Object);
		return Object;
	}
	Symbol t1 = get_expr()->traverse(t);
	Symbol t2 = t->getid(get_name());
	if(!t->ct->subtype(t1,t2,t->get_curr_class())) {
		t->semant_error() << " Type conflicts." << t1 << " " << t2 <<  " " << get_name() << endl;
		set_type(Object);
	}
	set_type(t1);
#ifdef CDEBUG 
cout << "assign class traverse ends" << get_name() << " " << get_type() << endl; 
#endif
	return t1;
}

Symbol static_dispatch_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "Static dispatch class traverse " << get_name() << endl; 
#endif
	Symbol type = get_expr()->traverse(t);
	this->set_type(Object);
	if(!t->ct->subtype(type, get_type_name(), t->get_curr_class())) {
		t->semant_error() << "Type conflict between expr type ("  << type << " ) " 
		<<  " and type declared (" << get_type_name() << ")" << endl;
		set_type(Object);
		return Object;
	}
	
	Methodp mp = t->ct->get_method(this->get_type_name(), this->get_name(), t->get_curr_class());
	if(!mp) {
		t->semant_error() << "No method " << get_type_name() << " " << this->get_name() << endl;
		set_type(Object);
		return Object;
	}
	if(!t->valid_signature(this->get_actual(), mp)) {
		t->semant_error() << "Signatures are not valid " << endl;
		set_type(Object);
		return Object;
	}
	
	if(mp->get_return_type() == SELF_TYPE) {
		set_type(type);
	} else {
		set_type(mp->get_return_type());
	}
#ifdef CDEBUG 
cout << "static dispatch class traverse ends" << " " << get_type() << endl; 
#endif

	return get_type();
}

Symbol dispatch_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "dispatch class traverse " << get_name() << endl; 
#endif
	Symbol type = get_expr()->traverse(t);

	Methodp mp = t->ct->get_method(type, get_name(), t->get_curr_class());
#ifdef CDEBUG 
cout << "Outside " << endl; 
#endif
	if(!mp) {
		t->semant_error() << "No method " << type << " " << get_name() << endl;
		set_type(Object);
		return Object;
	}
	if(!t->valid_signature(this->get_actual(), mp)) {
		#ifdef CDEBUG 
		 cout << "signatures not valid " << endl; 
		#endif
		t->semant_error() << "Signatures are not valid " << endl;
		set_type(Object);
		return Object;
	}
#ifdef CDEBUG 
cout << "herer  " << type << " " << mp->get_return_type() << " " << get_name() << endl; 
#endif

	if(mp->get_return_type() == SELF_TYPE) {
		set_type(type);
	} else {
		set_type(mp->get_return_type());
	}
#ifdef CDEBUG 
cout << "dispatch traverse ends" <<  " " << get_type() << endl; 
#endif
	return get_type();
}

Symbol cond_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "cond class traverse " << endl; 
#endif
	Symbol t1 = get_pred()->traverse(t);
	if(t1 != Bool) {
		t->semant_error() << "Type of predicate is not bool" << endl;
		set_type(Object);
		return Object;
	}
	
	Symbol t2 = get_then_exp()->traverse(t);
	Symbol t3 = get_else_exp()->traverse(t);
	set_type(t->lcu(t2,t3, t->get_curr_class()));

#ifdef CDEBUG 
cout << "cond class traverse ends" << endl; 
#endif
	return get_type();
	
}
Symbol loop_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "loop class traverse " << endl; 
#endif
	t->enterscope();
	Symbol t1 = get_pred()->traverse(t);
	set_type(Object);
	if(t1 != Bool) {
		t->semant_error() << "Type of predicate is not bool" << endl;
		return Object;
	}
	get_body()->traverse(t);
	t->exitscope();
	return Object;
}

Symbol typcase_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "Case class traverse " << endl; 
#endif
	get_expr()->traverse(t);
	Cases cs = get_cases();
	std::set<Symbol> st;
	Symbol type = NULL;
	for(int i=0; cs->more(i); i++) {
		Branchp c = (Branchp) cs->nth(i);
		Symbol ctype = c->traverse(t);
		Symbol dec_type = c->get_type_decl();
		if(st.find(dec_type) != st.end()) {
			t->semant_error() << "Case branch with type " << dec_type  << " already declared." << endl;
			set_type(Object);
			return Object;
		}
		st.insert(dec_type);

#ifdef CDEBUG 
cout << "Branch inside going for lcu " << ctype << endl; 
#endif
		if(!type) type = ctype;
		else type = t->lcu(type, ctype, t->get_curr_class());
#ifdef CDEBUG 
cout << "Branch internal " << type << endl; 
#endif
	}
	set_type(type);
	return type;
}

Symbol block_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "block class traverse " << endl; 
#endif
	Expressions exprs = get_body();
	Symbol type;
	for(int i=0; exprs->more(i); i++) {
		type = exprs->nth(i)->traverse(t);
	}
	set_type(type);
	return type;
}

Symbol let_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "let class traverse " << get_identifier() << endl; 
#endif
	t->enterscope();
	Symbol type = get_type_decl();
	if(get_identifier() == self) {
		t->semant_error() << "self is being declared." << endl;
		set_type(Object);
		return Object;
	}
	if(get_type_decl() == SELF_TYPE) {
		type = t->get_curr_class();
	}
#ifdef CDEBUG 
cout << "id added" << get_identifier() << " " << get_type_decl() << endl; 
#endif	
	t->addid(get_identifier(), get_type_decl());
	Symbol type2 = get_init()->traverse(t);
	if(!t->ct->subtype(type2, type,t->get_curr_class())) {
		t->semant_error() << "let expr e1 (" << type <<"), e2 (" << type2 << ")." << endl; 
		set_type(Object);
		return Object;
	}
	
	type = get_body()->traverse(t);
	t->exitscope();
	set_type(type);
#ifdef CDEBUG 
cout << "let class traverse ends" << get_identifier() << " " << get_type() << endl; 
#endif
	return type;
}

Symbol plus_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "plus class traverse " << endl; 
#endif
	Symbol type = e1->traverse(t);
	if(type != Int) {
		t->semant_error() << "Plus requires Both operands to be Int." << endl;
		set_type(Object);
		return Object;
	}
	type = e2->traverse(t);
	if(type != Int) {
		set_type(Object);
		t->semant_error() << "Plus requires Both operands to be Int." << endl;
		return Object;
	}
	set_type(Int);
	return Int; 
}

Symbol sub_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "sub class traverse "  << endl; 
#endif
	Symbol type = e1->traverse(t);
	if(type != Int) {
		t->semant_error() << "Sub requires Both operands to be Int." << endl;
		set_type(Object);
		return Object;
	}
	type = e2->traverse(t);
	if(type != Int) {
		set_type(Object);
		t->semant_error() << "Sub requires Both operands to be Int." << endl;
		return Object;
	}
	set_type(Int);
	return Int; 
}
Symbol mul_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "mul class traverse " << endl; 
#endif
	Symbol type = e1->traverse(t);
	if(type != Int) {
		t->semant_error() << "Mul requires Both operands to be Int." << endl;
		set_type(Object);
		return Object;
	}
	type = e2->traverse(t);
	if(type != Int) {
		set_type(Object);
		t->semant_error() << "Mul requires Both operands to be Int." << endl;
		return Object;
	}
	set_type(Int);
	return Int; 
}
Symbol divide_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "divide class traverse " << endl; 
#endif
	Symbol type = e1->traverse(t);
	if(type != Int) {
		t->semant_error() << "Divide requires Both operands to be Int." << endl;
		set_type(Object);
		return Object;
	}
	type = e2->traverse(t);
	if(type != Int) {
		set_type(Object);
		t->semant_error() << "Divide requires Both operands to be Int." << endl;
		return Object;
	}
	set_type(Int);
	return Int; 
}

Symbol neg_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "neg class traverse " << endl; 
#endif
	if(e1->traverse(t) != Int) {
		t->semant_error() << "Neg requires Int operator." << endl;
		set_type(Object);
		return Object;
	}
	set_type(Int);
	return Int;
}

Symbol lt_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "lt class traverse " << endl; 
#endif
	Symbol type1 = e1->traverse(t);
	Symbol type2 = e2->traverse(t);
	if(type1 != Int || type2 != Int) {
		t->semant_error() << "LT requires both operands to be Int." << type1 << " " << type2 << endl;
		set_type(Object);
		return Object;
	}
	set_type(Bool);
	return Bool;
}
Symbol eq_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "eq class traverse " << endl; 
#endif
	Symbol t1 = e1->traverse(t);
	Symbol t2 = e2->traverse(t);
	if(t1 == Int || t1 == Str || t1 == Bool || t2 == Int || t2 == Str || t2 == Bool) {
		if(t1!=t2) {
			t->semant_error() << "Both operands must have same type." << t1 << " " << t2 << endl;
			set_type(Object);
			return Object;
		}
	}

	set_type(Bool);
	return Bool;
}

Symbol leq_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "leq class traverse " << endl; 
#endif
	Symbol type1 = e1->traverse(t);
	Symbol type2 = e2->traverse(t);
	if(type1 != Int || type2 != Int) {
		t->semant_error() << "LE requires both operands to be Int." << type1 << " " << type2 << endl;
		set_type(Object);
		return Object;
	}
	set_type(Bool);
	return Bool;
}

Symbol comp_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "comp class traverse " << endl; 
#endif
	Symbol type = e1->traverse(t);
	if(type != Bool) {
		t->semant_error() << " 'Not' must have Bool operand." << endl;
		set_type(Object);
		return Object;
	}
	set_type(Bool);
	return Bool;
}

Symbol int_const_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "int const class traverse " << endl; 
#endif
	set_type(Int);
	return Int;
}
Symbol bool_const_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "bool const class traverse " << endl; 
#endif
	set_type(Bool);
	return Bool;
}

Symbol string_const_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "string class traverse" << endl; 
#endif
	set_type(Str);
	return Str;
}

Symbol new__class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "new class traverse " << get_type_name() << endl; 
#endif
	Symbol type = get_type_name();
	if(get_type_name() == SELF_TYPE) {
		set_type(SELF_TYPE);
		return SELF_TYPE;
	}
	if(!t->ct->has_class(type)) {
		t->semant_error() << "Class " << type << " is not defined." << endl;
		set_type(Object);
		return Object;
	}
	set_type(type);
#ifdef CDEBUG 
cout << "new class traverse ends " << get_type_name() << " " << get_type() << endl; 
#endif
	return type;
}

Symbol isvoid_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "isvoid class traverse " << endl; 
#endif
	e1->traverse(t);
	set_type(Bool);
	return Bool;
}

Symbol no_expr_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "no expr class traverse " << endl; 
#endif
	return No_class;
}

Symbol object_class::traverse(TypeEnv *t) {
#ifdef CDEBUG 
cout << "object class traverse " << get_name() << endl; 
#endif
	if(get_name() == self) {
		set_type(SELF_TYPE);
		return SELF_TYPE;
	}
	if(!t->hasid(get_name())) {
		t->semant_error() << "Name " << get_name() << " is not declared." << endl;
		set_type(Object);
		return Object;
	}
	Symbol type = t->getid(get_name());
	if(type == NULL) { 
		t->semant_error() << "Variable named " << get_name() << " doesn't  exists."<< endl;
		set_type(Object);
		return Object; 
	}
#ifdef CDEBUG 
cout << "object class ends" << get_name() << " " << type << endl; 
#endif
	set_type(type);
	return type;
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{                                                             
    return semant_error(c->get_filename(),c);
}    

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
} 



/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant()
{
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
    ClassTable *classtable = new ClassTable(classes);
	TypeEnv *t = new TypeEnv(classtable);
	this->traverse(t);
    /* some semantic analysis code may go here */
#ifdef CDEBUG
 cout << "Program class " << endl; 
#endif

    if (classtable->errors()) {
	cerr << "Compilation halted due to static semantic errors." << endl;
	exit(1);
    }
}

//Utility functions

void printset(set<Symbol> &s) {
	cout << "[";
	for(set<Symbol>::iterator it = s.begin(); it!=s.end(); it++) {
		cout << *it << ",";
	}
	cout << "]" << endl;
}

void fatal_error() {
	fatal_error("Compilation halted due to static semantic errors.");
}
