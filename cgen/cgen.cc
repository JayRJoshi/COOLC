
//**************************************************************
//
// Code generator SKELETON
//
// Read the comments carefully. Make sure to
//    initialize the base class tags in
//       `CgenClassTable::CgenClassTable'
//
//    Add the label for the dispatch tables to
//       `IntEntry::code_def'
//       `StringEntry::code_def'
//       `BoolConst::code_def'
//
//    Add code to emit everyting else that is needed
//       in `CgenClassTable::code'
//
//
// The files as provided will produce code to begin the code
// segments, declare globals, and emit constants.  You must
// fill in the rest.
//
//**************************************************************

#include "cgen.h"
#include "cgen_gc.h"

extern void emit_string_constant(ostream& str, char *s);
extern int cgen_debug;

//
// Three symbols from the semantic analyzer (semant.cc) are used.
// If e : No_type, then no code is generated for e.
// Special code is generated for new SELF_TYPE.
// The name "self" also generates code different from other references.
//
//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
Symbol 
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

static char *gc_init_names[] =
  { "_NoGC_Init", "_GenGC_Init", "_ScnGC_Init" };
static char *gc_collect_names[] =
  { "_NoGC_Collect", "_GenGC_Collect", "_ScnGC_Collect" };


//  BoolConst is a class that implements code generation for operations
//  on the two booleans, which are given global names here.
BoolConst falsebool(FALSE);
BoolConst truebool(TRUE);

//*********************************************************
//
// Define method for code generation
//
// This is the method called by the compiler driver
// `cgtest.cc'. cgen takes an `ostream' to which the assembly will be
// emmitted, and it passes this and the class list of the
// code generator tree to the constructor for `CgenClassTable'.
// That constructor performs all of the work of the code
// generator.
//
//*********************************************************

void program_class::cgen(ostream &os) 
{
  // spim wants comments to start with '#'
  os << "# start of generated code\n";

  initialize_constants();
  CgenClassTable *ct = new CgenClassTable(classes,os);

  os << "\n# end of generated code\n";
}

static std::string i2s(int i) {
	std::ostringstream ss;
	ss << i;
	return ss.str();
}
//////////////////////////////////////////////////////////////////////////////
//
//  emit_* procedures
//
//  emit_X  writes code for operation "X" to the output stream.
//  There is an emit_X for each opcode X, as well as emit_ functions
//  for generating names according to the naming conventions (see emit.h)
//  and calls to support functions defined in the trap handler.
//
//  Register names and addresses are passed as strings.  See `emit.h'
//  for symbolic names you can use to refer to the strings.
//
//////////////////////////////////////////////////////////////////////////////

static void emit_load(char *dest_reg, int offset, char *source_reg, ostream& s)
{
  s << LW << dest_reg << " " << offset * WORD_SIZE << "(" << source_reg << ")" 
    << endl;
}

static void emit_store(char *source_reg, int offset, char *dest_reg, ostream& s)
{
  s << SW << source_reg << " " << offset * WORD_SIZE << "(" << dest_reg << ")"
      << endl;
}

static void emit_load_imm(char *dest_reg, int val, ostream& s)
{ s << LI << dest_reg << " " << val << endl; }

static void emit_load_address(char *dest_reg, char *address, ostream& s)
{ s << LA << dest_reg << " " << address << endl; }

static void emit_partial_load_address(char *dest_reg, ostream& s)
{ s << LA << dest_reg << " "; }

static void emit_load_bool(char *dest, const BoolConst& b, ostream& s)
{
  emit_partial_load_address(dest,s);
  b.code_ref(s);
  s << endl;
}

static void emit_load_string(char *dest, StringEntry *str, ostream& s)
{
  emit_partial_load_address(dest,s);
  str->code_ref(s);
  s << endl;
}

static void emit_load_int(char *dest, IntEntry *i, ostream& s)
{
  emit_partial_load_address(dest,s);
  i->code_ref(s);
  s << endl;
}

static void emit_move(char *dest_reg, char *source_reg, ostream& s)
{ s << MOVE << dest_reg << " " << source_reg << endl; }

static void emit_neg(char *dest, char *src1, ostream& s)
{ s << NEG << dest << " " << src1 << endl; }

static void emit_add(char *dest, char *src1, char *src2, ostream& s)
{ s << ADD << dest << " " << src1 << " " << src2 << endl; }

static void emit_addu(char *dest, char *src1, char *src2, ostream& s)
{ s << ADDU << dest << " " << src1 << " " << src2 << endl; }

static void emit_addiu(char *dest, char *src1, int imm, ostream& s)
{ s << ADDIU << dest << " " << src1 << " " << imm << endl; }

static void emit_div(char *dest, char *src1, char *src2, ostream& s)
{ s << DIV << dest << " " << src1 << " " << src2 << endl; }

static void emit_mul(char *dest, char *src1, char *src2, ostream& s)
{ s << MUL << dest << " " << src1 << " " << src2 << endl; }

static void emit_sub(char *dest, char *src1, char *src2, ostream& s)
{ s << SUB << dest << " " << src1 << " " << src2 << endl; }

static void emit_sll(char *dest, char *src1, int num, ostream& s)
{ s << SLL << dest << " " << src1 << " " << num << endl; }

static void emit_jalr(char *dest, ostream& s)
{ s << JALR << "\t" << dest << endl; }

static void emit_jal(char *address,ostream &s)
{ s << JAL << address << endl; }

static void emit_return(ostream& s)
{ s << RET << endl; }

static void emit_gc_assign(ostream& s)
{ s << JAL << "_GenGC_Assign" << endl; }

static void emit_disptable_ref(Symbol sym, ostream& s)
{  s << sym << DISPTAB_SUFFIX; }

static void emit_init_ref(Symbol sym, ostream& s)
{ s << sym << CLASSINIT_SUFFIX; }

static void emit_label_ref(int l, ostream &s)
{ s << "label" << l; }

static void emit_protobj_ref(Symbol sym, ostream& s)
{ s << sym << PROTOBJ_SUFFIX; }

static void emit_method_ref(Symbol classname, Symbol methodname, ostream& s)
{ s << classname << METHOD_SEP << methodname; }

static void emit_label_def(int l, ostream &s)
{
  emit_label_ref(l,s);
  s << ":" << endl;
}

static void emit_beqz(char *source, int label, ostream &s)
{
  s << BEQZ << source << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_beq(char *src1, char *src2, int label, ostream &s)
{
  s << BEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bne(char *src1, char *src2, int label, ostream &s)
{
  s << BNE << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bleq(char *src1, char *src2, int label, ostream &s)
{
  s << BLEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blt(char *src1, char *src2, int label, ostream &s)
{
  s << BLT << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blti(char *src1, int imm, int label, ostream &s)
{
  s << BLT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bgti(char *src1, int imm, int label, ostream &s)
{
  s << BGT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_branch(int l, ostream& s)
{
  s << BRANCH;
  emit_label_ref(l,s);
  s << endl;
}

//
// Push a register on the stack. The stack grows towards smaller addresses.
//
static void emit_push(char *reg, ostream& str)
{
  emit_store(reg,0,SP,str);
  emit_addiu(SP,SP,-4,str);
}

static void emit_pop(ostream& str) {
	emit_addiu(SP, SP, 4, str);
}

static void emit_pop(char *ref, ostream &str) {
	emit_pop(str);
	emit_load(ref, 0, SP, str);
}
//
// Fetch the integer value in an Int object.
// Emits code to fetch the integer value of the Integer object pointed
// to by register source into the register dest
//
static void emit_fetch_int(char *dest, char *source, ostream& s)
{ emit_load(dest, DEFAULT_OBJFIELDS, source, s); }

//
// Emits code to store the integer value contained in register source
// into the Integer object pointed to by dest.
//
static void emit_store_int(char *source, char *dest, ostream& s)
{ emit_store(source, DEFAULT_OBJFIELDS, dest, s); }


static void emit_test_collector(ostream &s)
{
  emit_push(ACC, s);
  emit_move(ACC, SP, s); // stack end
  emit_move(A1, ZERO, s); // allocate nothing
  s << JAL << gc_collect_names[cgen_Memmgr] << endl;
  emit_addiu(SP,SP,4,s);
  emit_load(ACC,0,SP,s);
}

static void emit_gc_check(char *source, ostream &s)
{
  if (source != (char*)A1) emit_move(A1, source, s);
  s << JAL << "_gc_check" << endl;
}


///////////////////////////////////////////////////////////////////////////////
//
// coding strings, ints, and booleans
//
// Cool has three kinds of constants: strings, ints, and booleans.
// This section defines code generation for each type.
//
// All string constants are listed in the global "stringtable" and have
// type StringEntry.  StringEntry methods are defined both for String
// constant definitions and references.
//
// All integer constants are listed in the global "inttable" and have
// type IntEntry.  IntEntry methods are defined for Int
// constant definitions and references.
//
// Since there are only two Bool values, there is no need for a table.
// The two booleans are represented by instances of the class BoolConst,
// which defines the definition and reference methods for Bools.
//
///////////////////////////////////////////////////////////////////////////////

//
// Strings
//
void StringEntry::code_ref(ostream& s)
{
   s << STRCONST_PREFIX << index;
}

//
// Emit code for a constant String.
// You should fill in the code naming the dispatch table.
//

void StringEntry::code_def(ostream& s, int stringclasstag)
{
  IntEntryP lensym = inttable.add_int(len);

  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s  << LABEL                                             // label
      << WORD << stringclasstag << endl                                 // tag
      << WORD << (DEFAULT_OBJFIELDS + STRING_SLOTS + (len+4)/4) << endl // size
      << WORD << "String_dispTab";


 /***** Add dispatch information for class String ******/

      s << endl;                                              // dispatch table
      s << WORD;  lensym->code_ref(s);  s << endl;            // string length
  emit_string_constant(s,str);                                // ascii string
  s << ALIGN;                                                 // align to word
}

//
// StrTable::code_string
// Generate a string object definition for every string constant in the 
// stringtable.
//
void StrTable::code_string_table(ostream& s, int stringclasstag)
{  
  for (List<StringEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,stringclasstag);
}

//
// Ints
//
void IntEntry::code_ref(ostream &s)
{
  s << INTCONST_PREFIX << index;
}

//
// Emit code for a constant Integer.
// You should fill in the code naming the dispatch table.
//

void IntEntry::code_def(ostream &s, int intclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                // label
      << WORD << intclasstag << endl                      // class tag
      << WORD << (DEFAULT_OBJFIELDS + INT_SLOTS) << endl  // object size
      << WORD << "Int_dispTab";

 /***** Add dispatch information for class Int ******/

      s << endl;                                          // dispatch table
      s << WORD << str << endl;                           // integer value
}


//
// IntTable::code_string_table
// Generate an Int object definition for every Int constant in the
// inttable.
//
void IntTable::code_string_table(ostream &s, int intclasstag)
{
  for (List<IntEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,intclasstag);
}


//
// Bools
//
BoolConst::BoolConst(int i) : val(i) { assert(i == 0 || i == 1); }

void BoolConst::code_ref(ostream& s) const
{
  s << BOOLCONST_PREFIX << val;
}
  
//
// Emit code for a constant Bool.
// You should fill in the code naming the dispatch table.
//

void BoolConst::code_def(ostream& s, int boolclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                  // label
      << WORD << boolclasstag << endl                       // class tag
      << WORD << (DEFAULT_OBJFIELDS + BOOL_SLOTS) << endl   // object size
      << WORD << "Bool_dispTab";

 /***** Add dispatch information for class Bool ******/

      s << endl;                                            // dispatch table
      s << WORD << val << endl;                             // value (0 or 1)
}

//////////////////////////////////////////////////////////////////////////////
//
//  CgenClassTable methods
//
//////////////////////////////////////////////////////////////////////////////

//***************************************************
//
//  Emit code to start the .data segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_data()
{
  Symbol main    = idtable.lookup_string(MAINNAME);
  Symbol string  = idtable.lookup_string(STRINGNAME);
  Symbol integer = idtable.lookup_string(INTNAME);
  Symbol boolc   = idtable.lookup_string(BOOLNAME);

  str << "\t.data\n" << ALIGN;
  //
  // The following global names must be defined first.
  //
  str << GLOBAL << CLASSNAMETAB << endl;
  str << GLOBAL; emit_protobj_ref(main,str);    str << endl;
  str << GLOBAL; emit_protobj_ref(integer,str); str << endl;
  str << GLOBAL; emit_protobj_ref(string,str);  str << endl;
  str << GLOBAL; falsebool.code_ref(str);  str << endl;
  str << GLOBAL; truebool.code_ref(str);   str << endl;
  str << GLOBAL << INTTAG << endl;
  str << GLOBAL << BOOLTAG << endl;
  str << GLOBAL << STRINGTAG << endl;

}


//***************************************************
//
//  Emit code to start the .text segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_text()
{
  str << GLOBAL << HEAP_START << endl
      << HEAP_START << LABEL 
      << WORD << 0 << endl
      << "\t.text" << endl
      << GLOBAL;
  emit_init_ref(idtable.add_string("Main"), str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Int"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("String"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Bool"),str);
  str << endl << GLOBAL;
  emit_method_ref(idtable.add_string("Main"), idtable.add_string("main"), str);
  str << endl;
}

void CgenClassTable::code_bools(int boolclasstag)
{
  falsebool.code_def(str,boolclasstag);
  truebool.code_def(str,boolclasstag);
}

void CgenClassTable::code_select_gc()
{
  //
  // Generate GC choice constants (pointers to GC functions)
  //
  str << GLOBAL << "_MemMgr_INITIALIZER" << endl;
  str << "_MemMgr_INITIALIZER:" << endl;
  str << WORD << gc_init_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_COLLECTOR" << endl;
  str << "_MemMgr_COLLECTOR:" << endl;
  str << WORD << gc_collect_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_TEST" << endl;
  str << "_MemMgr_TEST:" << endl;
  str << WORD << (cgen_Memmgr_Test == GC_TEST) << endl;
}


//********************************************************
//
// Emit code to reserve space for and initialize all of
// the constants.  Class names should have been added to
// the string table (in the supplied code, is is done
// during the construction of the inheritance graph), and
// code for emitting string constants as a side effect adds
// the string's length to the integer table.  The constants
// are emmitted by running through the stringtable and inttable
// and producing code for each entry.
//
//********************************************************

void CgenClassTable::install_constants() {
	for(List<CgenNode> *l = nds; l; l = l->tl()) {
		CgenNodeP nd = l->hd();
		nd->get_constants();
	}
}

void CgenClassTable::code_constants()
{
  //
  // Add constants that are required by the code generator.
  //
  
  stringtable.add_string("");
  inttable.add_string("0");

  install_constants();
  
  str << INTTAG << LABEL
      << WORD << ind(Int) << endl;
  str << BOOLTAG << LABEL 
      << WORD << ind(Bool) << endl;
  str << STRINGTAG << LABEL 
      << WORD << ind(Str) << endl;
  
  stringtable.code_string_table(str,ind(Str));
  inttable.code_string_table(str,ind(Int));
  code_bools(ind(Bool));
}

int CgenClassTable::build_hierarchy_dfs(CgenNodeP n) {
	List<CgenNode> *l = n->get_children();
	int nodes = 1;
	for(; l; l = l->tl()) {
		CgenNodeP nd = l->hd();
		nodes += build_hierarchy_dfs(nd);
	}
	nds = new List<CgenNode>(n, nds);
	subtree_nodes[n] = nodes - 1;
	return nodes;
}

void CgenClassTable::rebuild_list(CgenNodeP n) {
	nds = NULL;
	build_hierarchy_dfs(n);
	int c = 0;
	for(List<CgenNode> *l = nds; l; l = l->tl()) {
		CgenNodeP nd = l->hd();
		mp[nd] = std::make_pair(c,c + subtree_nodes[nd]);
		c++;
	}
}

CgenClassTable::CgenClassTable(Classes classes, ostream& s) : nds(NULL) , str(s)
{
	label = 0;
	env = new Env();
	env->enterscope();
   enterscope();
   if (cgen_debug) cout << "Building CgenClassTable" << endl;
   install_basic_classes();
   install_classes(classes);
   build_inheritance_tree();
   root = probe(Object);
   rebuild_list(root);
   code();
   exitscope();
   env->exitscope();
}

void CgenClassTable::install_basic_classes()
{

// The tree package uses these globals to annotate the classes built below.
  //curr_lineno  = 0;
  Symbol filename = stringtable.add_string("<basic class>");

//
// A few special class names are installed in the lookup table but not
// the class list.  Thus, these classes exist, but are not part of the
// inheritance hierarchy.
// No_class serves as the parent of Object and the other special classes.
// SELF_TYPE is the self class; it cannot be redefined or inherited.
// prim_slot is a class known to the code generator.
//
  addid(No_class,
	new CgenNode(class_(No_class,No_class,nil_Features(),filename),
			    Basic,this));
  addid(SELF_TYPE,
	new CgenNode(class_(SELF_TYPE,No_class,nil_Features(),filename),
			    Basic,this));
  addid(prim_slot,
	new CgenNode(class_(prim_slot,No_class,nil_Features(),filename),
			    Basic,this));

// 
// The Object class has no parent class. Its methods are
//        cool_abort() : Object    aborts the program
//        type_name() : Str        returns a string representation of class name
//        copy() : SELF_TYPE       returns a copy of the object
//
// There is no need for method bodies in the basic classes---these
// are already built in to the runtime system.
//
  install_class(
   new CgenNode(
    class_(Object, 
	   No_class,
	   append_Features(
           append_Features(
           single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
           single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
           single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	   filename),
    Basic,this));

// 
// The IO class inherits from Object. Its methods are
//        out_string(Str) : SELF_TYPE          writes a string to the output
//        out_int(Int) : SELF_TYPE               "    an int    "  "     "
//        in_string() : Str                    reads a string from the input
//        in_int() : Int                         "   an int     "  "     "
//
   install_class(
    new CgenNode(
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
	   filename),	    
    Basic,this));

//
// The Int class has no methods and only a single attribute, the
// "val" for the integer. 
//
   install_class(
    new CgenNode(
     class_(Int, 
	    Object,
            single_Features(attr(val, prim_slot, no_expr())),
	    filename),
     Basic,this));

//
// Bool also has only the "val" slot.
//
    install_class(
     new CgenNode(
      class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename),
      Basic,this));

//
// The class Str has a number of slots and operations:
//       val                                  ???
//       str_field                            the string itself
//       length() : Int                       length of the string
//       concat(arg: Str) : Str               string concatenation
//       substr(arg: Int, arg2: Int): Str     substring
//       
   install_class(
    new CgenNode(
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
	     filename),
        Basic,this));

}

// CgenClassTable::install_class
// CgenClassTable::install_classes
//
// install_classes enters a list of classes in the symbol table.
//
void CgenClassTable::install_class(CgenNodeP nd)
{
  Symbol name = nd->get_name();

  if (probe(name))
    {
      return;
    }

  // The class name is legal, so add it to the list of classes
  // and the symbol table.
  nds = new List<CgenNode>(nd,nds);
  addid(name,nd);
}

void CgenClassTable::install_classes(Classes cs)
{
  for(int i = cs->first(); cs->more(i); i = cs->next(i))
    install_class(new CgenNode(cs->nth(i),NotBasic,this));
}

//
// CgenClassTable::build_inheritance_tree
//
void CgenClassTable::build_inheritance_tree()
{
  for(List<CgenNode> *l = nds; l; l = l->tl())
      set_relations(l->hd());
}

//
// CgenClassTable::set_relations
//
// Takes a CgenNode and locates its, and its parent's, inheritance nodes
// via the class table.  Parent and child pointers are added as appropriate.
//
void CgenClassTable::set_relations(CgenNodeP nd)
{
  CgenNode *parent_node = probe(nd->get_parent());
  nd->set_parentnd(parent_node);
  parent_node->add_child(nd);
}

void CgenNode::add_child(CgenNodeP n)
{
  children = new List<CgenNode>(n,children);
}

void CgenNode::set_parentnd(CgenNodeP p)
{
  assert(parentnd == NULL);
  assert(p != NULL);
  parentnd = p;
}

void CgenClassTable::code_class_nameTab() {
	str	 << CLASSNAMETAB << LABEL;
	
	for(List<CgenNode> *l = nds; l; l = l->tl()) {
		Symbol name = l->hd()->name;
		char *s = (char *) name->get_string();
		StringEntry *se = (StringEntry *) stringtable.lookup_string(s);
		str << WORD; se->code_ref(str);
		str << endl;
	}
}

void CgenClassTable::code_objTab() {
	str << CLASSOBJTAB << LABEL;
	
	for(List<CgenNode> *l = nds; l; l = l->tl()) {
		Symbol name = l->hd()->name;
		char *s = (char *) name->get_string();
		str << WORD << s << "_protObj" << endl;
		str << WORD << s << "_init" << endl;
	}
}

void CgenClassTable::install_features(CgenNodeP n) {
	Features fs = n->features;
	
	SpecializedTable<Symbol, Entry> *partab = n->get_parentnd()->methodtable;
	
	//attribute order for Str is defined.
	std::stack< std::pair <Symbol, Symbol> > st1; 
	
	for(int i=0; fs->more(i); i++) {
		Feature f = fs->nth(i);
		if(f->is_method) {
			MethodP mth = (MethodP) f;
			methodclass *at = n->methodtable->addid(mth->name, n->name);
			if(!partab->lookup(mth->name))
				n->methodtable->list = new List<methodclass>(at, n->methodtable->list);
		}
		else {
			AttrP atr = (AttrP) f;
			st1.push(std::make_pair(atr->name, atr->type_decl));
			//n->attrtable->add(atr->name, atr->type_decl);
		}
	}
	
	while(!st1.empty()) {
		std::pair<Symbol, Symbol> en = st1.top();
		st1.pop();
		n->attrtable->add(en.first, en.second);
	}
	// This quick fix (usage of stack) tries to convert stack like traversal of list to queue like
	// is needed because dispatch table entry have to be in particular order to
	// dispatch methods correctly (inherited class's method name address in disptab 
	// have to be same as method name's address in parent class's disptab). 
	// It is due to dynamic nature of dispatch. For more, see dispacth class's code method.
	
	List< methodclass > *methods = n->get_parentnd()->methodtable->get_list();
	std::stack< std::pair <Symbol, Symbol> > st;
	
	for(; methods; methods = methods->tl()) {
		methodclass *mc= methods->hd();
		st.push(std::make_pair(mc->get_id(), mc->get_info()));
	}
	
	while(!st.empty()) {
		std::pair<Symbol, Symbol> en = st.top();
		st.pop();
		if(!n->methodtable->lookup(en.first)) n->methodtable->add(en.first, en.second);
		else n->methodtable->add(en.first, n->name);
	}
	
	List< attrclass > *attrs = n->get_parentnd()->attrtable->get_list();
	
	for(; attrs; attrs = attrs->tl()) {
		attrclass *ac = attrs->hd();
		st1.push(std::make_pair(ac->get_id(), ac->get_info()));
	}
	
	while(!st1.empty()) {
		std::pair<Symbol, Symbol> en = st1.top();
		st1.pop();
	
	
		n->attrtable->add(en.first, en.second);
	}
	
	for(List<CgenNode> *l = n->get_children(); l; l=l->tl()) {
		CgenNodeP child = l->hd();
		install_features(child);
	}
}

void CgenClassTable::code_dispTab() {
	for(List<CgenNode> *l = nds; l; l = l->tl()) {
		CgenNodeP nd = l->hd();
		str << nd->name << DISPTAB_SUFFIX << LABEL;
		List< methodclass > *methods = nd->methodtable->get_list();
		for(; methods; methods = methods->tl()) {
			methodclass *mc = methods->hd();
			Symbol methodname = mc->get_id();
			Symbol classname = mc->get_info();
			str << WORD << classname <<"."<< methodname << endl;
		}
	}
}
static void default_value(ostream& str, CgenNodeP nd) {
		Symbol s = nd->name;
		if(nd->basic()) {
			if(s == Str) {
				StringEntry *empty = (StringEntry *) stringtable.add_string("");
				empty->code_ref(str);
			}
			else if(s == Int) {
				IntEntry *empty = (IntEntry *) inttable.add_int(0);
				empty->code_ref(str);
			}
			else if(s == Bool) {
				BoolConst(0).code_ref(str);
			}
			else {
				str << 0;
			}
		}
		else str << 0;
}
void CgenClassTable::code_protObj() {
	int c = 0;
	for(List<CgenNode> *l = nds; l; l = l->tl()) {
		CgenNodeP nd = l->hd();
		int len = nd->attrtable->len();
		str << WORD << "-1" << endl;
		str << nd->name << PROTOBJ_SUFFIX << LABEL;
		str << WORD << c << endl;				//classtag
		str << WORD << ( 3 + len ) << endl;	//size
		str << WORD << nd->name << DISPTAB_SUFFIX << endl;
		
		List < attrclass > *list = nd->attrtable->get_list();
		for(; list; list = list->tl()) {
			attrclass *ac = list->hd();
			CgenNodeP node = probe(ac->get_info());
			
			str << WORD; default_value(str, node);
			str << endl;
		}
		
		c++;
	}
}

void CgenClassTable::code_class_init() {
	for(List<CgenNode> *l = nds; l; l = l->tl()) {
		CgenNodeP nd = l->hd();
		env->curr_class = nd;
		str << nd->name << "_init" << LABEL;
		emit_push(FP, str);
		emit_push(SELF, str);
		emit_push(RA, str);
		emit_addiu(FP, SP, 4, str);
		
		emit_move(SELF, ACC, str);
		
		env->enterscope();
		
		std::string parent_init = nd->get_parentnd()->name->get_string() + std::string("_init");
		if(parent_init != "_no_class_init") emit_jal((char *)parent_init.c_str(), str);
		Features fs = nd->features;
		
		List<attrclass> *lt = nd->attrtable->get_list();
		int c = 12;
		for(; lt; lt = lt->tl()) {
			attrclass *ac = lt->hd();
			std::string *tmp = new std::string(i2s(c) + "($s0)");
			env->addid(ac->get_id(), tmp);
			c += 4;
		}
		
		/*for(int j=0; fs->more(j); j++) {
			Feature f = fs->nth(j);
			if(!f->is_method) {
			//cout << " dsd  " << endl;
				Symbol atr = ((AttrP) f)->name;
				int ind = nd->attrtable->ind(atr);
				std::string *tmp = new std::string(i2s(4*ind + 12) + "($s0)");
				env->addid(atr, tmp);
			}
		}*/
		
		for(int j=0; fs->more(j); j++) {
			Feature f = fs->nth(j);
			if(!f->is_method) {
				AttrP atr = (AttrP) f;
				int ind = nd->attrtable->ind(atr->name);
				if(!atr->init->type) continue;
				atr->init->code(this);
				emit_store(ACC, 3+ind, SELF, str);
			}
		}
		
		
		
		env->exitscope();
		emit_move(ACC, SELF, str);
		
		emit_pop(RA, str);
		emit_pop(SELF, str);
		emit_pop(FP, str);
		
		str << RET << endl;
	}
}

int CgenClassTable::dispatch_ind(Symbol c, Symbol m) {
	return probe(c)->methodtable->ind(m);
}

void CgenClassTable::code_method_def() {
	for(List<CgenNode> *l = nds; l; l = l->tl()) {
		CgenNodeP nd = l->hd();
		nd->code(this);	 // adds attibutes to symtab 
		Features fs = nd->features;
		for(int j=0; fs->more(j); j++) {
			Feature f = fs->nth(j);
			if(f->is_method) {
				((MethodP)f)->code(this);
			}
		}
	}
}



void CgenClassTable::code()
{
  if (cgen_debug) cout << "coding global data" << endl;
  code_global_data();

  if (cgen_debug) cout << "choosing gc" << endl;
  code_select_gc();

  if (cgen_debug) cout << "coding constants" << endl;
  code_constants();

//                 Add your code to emit
//                   - prototype objects
//                   - class_nameTab
//                   - dispatch tables
//

	code_class_nameTab();
	code_objTab();
	
	install_features(root);
	code_dispTab();
	
	code_protObj();
	
  if (cgen_debug) cout << "coding global text" << endl;
  code_global_text();

	code_class_init();
	
	code_method_def();
	
//                 Add your code to emit
//                   - object initializer
//                   - the class methods
//                   - etc...

}

///////////////////////////////////////////////////////////////////////
//
// CgenNode methods
//
///////////////////////////////////////////////////////////////////////

CgenNode::CgenNode(Class_ nd, Basicness bstatus, CgenClassTableP ct) :
   class__class((const class__class &) *nd),
   parentnd(NULL),
   children(NULL),
   basic_status(bstatus)
{ 
   methodtable = new SpecializedTable<Symbol, Entry>();
   attrtable = new SpecializedTable<Symbol, Entry>();
   methodtable->enterscope();
   attrtable->enterscope();
   stringtable.add_string(name->get_string());          // Add class name to string table
}

void class__class::get_constants() {
	Features fs = features;
	for(int i=0; fs->more(i); i++) {
		fs->nth(i)->get_constants();
	}
}

void method_class::get_constants() {
	expr->get_constants();
}

void attr_class::get_constants() {
	init->get_constants();
}

void formal_class::get_constants() {
}

void branch_class::get_constants() {
	expr->get_constants();
}

void assign_class::get_constants() {
	expr->get_constants();
}

void static_dispatch_class::get_constants() {
	expr->get_constants();
	Expressions es = actual;
	for(int i=0; es->more(i); i++) {
		es->nth(i)->get_constants();
	}
}

void dispatch_class::get_constants() {
	expr->get_constants();
	Expressions es = actual;
	for(int i=0; es->more(i); i++) {
		es->nth(i)->get_constants();
	}
}

void cond_class::get_constants() {
//	cout << "inside cond " << endl;
	pred->get_constants();
	then_exp->get_constants();
	else_exp->get_constants();
}

void loop_class::get_constants() {
	pred->get_constants();
	body->get_constants();
}

void typcase_class::get_constants() {
	expr->get_constants();
	for(int i=0; cases->more(i); i++) {
		cases->nth(i)->get_constants();
	}
}

void block_class::get_constants() {
	Expressions es = body;
	for(int i=0; es->more(i); i++) {
		es->nth(i)->get_constants();
	}
}

void let_class::get_constants() {
	init->get_constants();
	body->get_constants();
}

void plus_class::get_constants() {
	e1->get_constants();
	e2->get_constants();
}

void sub_class::get_constants() {
	e1->get_constants();
	e2->get_constants();
}

void mul_class::get_constants() {
	e1->get_constants();
	e2->get_constants();
}

void divide_class::get_constants() {
	e1->get_constants();
	e2->get_constants();
}

void neg_class::get_constants() {
	e1->get_constants();
}

void lt_class::get_constants() {
	e1->get_constants();
	e2->get_constants();
}

void eq_class::get_constants() {
	e1->get_constants();
	e2->get_constants();
}

void leq_class::get_constants() {
	e1->get_constants();
	e2->get_constants();
}

void comp_class::get_constants() {
	e1->get_constants();
}

void int_const_class::get_constants() {
	inttable.add_string(token->get_string());
}

void bool_const_class::get_constants() {
}

void string_const_class::get_constants() {
	stringtable.add_string(token->get_string());
}

void new__class::get_constants() {
}

void isvoid_class::get_constants() {
	e1->get_constants();
}

void no_expr_class::get_constants() {
}

void object_class::get_constants() {
}

//******************************************************************
//
//   Fill in the following methods to produce code for the
//   appropriate expression.  You may add or remove parameters
//   as you wish, but if you do, remember to change the parameters
//   of the declarations in `cool-tree.h'  Sample code for
//   constant integers, strings, and booleans are provided.
//
//*****************************************************************

using namespace std;
void method_class::code(CgenClassTable *ct) {
//	cout << "method class " << endl;
	ostream &s = ct->str;
	Env *env = ct->env;
	
	if(env->curr_class->basic()) return;	
	env->enterscope();
	
	int len = formals->len();
	
	for(int i=0; formals->more(i); i++) {
		formal_class *fml = (formal_class *) formals->nth(i);
		string *str = new string(i2s(8 + 4*(len - i)) + "($fp)");
		env->addid(fml->name, str);
	}
	
	s << env->curr_class->name << "." << this->name << LABEL;
	emit_push(FP, s);
	emit_push(SELF, s);
	emit_push(RA, s);
	
	emit_addiu(FP, SP, 4, s);
	emit_move(SELF, ACC, s);
	
	expr->code(ct);
	
	emit_pop(RA, s);
	emit_pop(SELF, s);
	emit_pop(FP, s);
	
	emit_addiu(SP, SP, 4*len, s);
	s << RET << endl;
	env->exitscope();
}

void class__class::code(CgenClassTable *ct) {
	//cout << "class class " << endl;
	ostream &s = ct->str;
	Env *env = ct->env;
	env->curr_class = ct->probe(this->name);
	
	List< attrclass > *l = env->curr_class->attrtable->get_list();

	for( ; l; l = l->tl()) {
		methodclass *ac = l->hd();
		Symbol atr = ac->get_id();
		Symbol cl = ac->get_info();
		int ind = env->curr_class->attrtable->ind(atr);
		string *tmp = new string(i2s(12 + 4*ind) + "($s0)");
		env->addid(atr, tmp);
	}
}

void assign_class::code(CgenClassTable *ct) {
	//cout << "assign class " << endl;
	ostream &s = ct->str;
	
	expr->code(ct);
	
	string str = *ct->env->lookup(name);
	s << SW << ACC << " " << str << endl;
}

void common_dispatch_proc(CgenClassTable *ct, Expression expr, Symbol name, Expressions actual, int line_number) {
	ostream &s = ct->str;
	//emit_push(RA, s);
	int len = actual->len();
	for(int i=0; i<len; i++) {
		actual->nth(i)->code(ct);
		emit_push(ACC, s);
	}
	expr->code(ct);
	int lab = ct->get_label();
	emit_bne(ACC, ZERO, lab, s);
	
//	if(!ct->env) cout << "ds ds a" << endl;
	//cout << ct->env->curr_class->name << endl;
	StringEntry *se = stringtable.lookup_string((char *)ct->env->curr_class->filename->get_string());
	s << LA << ACC << " "; se->code_ref(s); 
	s << endl;
	
	emit_load_imm(T1, line_number, s);
	emit_jal("_dispatch_abort", s);
	
	emit_label_def(lab, s);
}

void static_dispatch_class::code(CgenClassTable *ct) {
	//cout << "static dispatch class " << endl;
	ostream &s = ct->str;
	common_dispatch_proc(ct, expr, name, actual, line_number);
	string str = type_name->get_string() + string(DISPTAB_SUFFIX);
	int ind = ct->probe(type_name)->methodtable->ind(name);
	
	emit_load_address(T1, (char *)str.c_str(), s);
	emit_load(T1, ind, T1, s);
	emit_jalr(T1, s);
}

void dispatch_class::code(CgenClassTable *ct) {
	ostream &s = ct->str;
	//s << "dispatch class " << endl;
	common_dispatch_proc(ct, expr, name, actual, line_number);
	Symbol cl = expr->type;
	if(cl == SELF_TYPE) {
		cl = ct->env->curr_class->name;
	}
	//Order of methods in disptab is exploted here, 
	//It would not work, had COOL supported multiple inheritance.

	int ind = ct->probe(cl)->methodtable->ind(name);
	
	emit_load(T1, 2, ACC, s);
	emit_load(T1, ind, T1, s);
	emit_jalr(T1, s);
	
	//TODO: symtab updte, move class to S0
}

void cond_class::code(CgenClassTable *ct) {
	//cout << "cond clas" << endl;
	ostream &s = ct->str;
	
	pred->code(ct);
	int lab1 = ct->get_label();
	int lab2 = ct->get_label();
	
	emit_load(T1, 3, ACC, s);
	emit_beqz(T1, lab1, s);
	then_exp->code(ct);
	emit_branch(lab2, s);

	emit_label_def(lab1, s);
	else_exp->code(ct);

	emit_label_def(lab2, s);
}

void loop_class::code(CgenClassTable *ct) {
//	cout << "loop classs" << endl;
	ostream &s = ct->str;
	ct->env->enterscope();
	
	int lab1 = ct->get_label();
	int lab2 = ct->get_label();
	
	emit_label_def(lab1, s);
	pred->code(ct);
	emit_load(T1, 3, ACC, s);
	emit_beqz(T1, lab2, s);
	
	body->code(ct);
	emit_load(T1, 3, ACC, s);
	emit_branch(lab1, s);
	
	emit_label_def(lab2, s);
	emit_load_imm(ACC, 0, s);
	
	ct->env->exitscope();
}

struct info {
	int f, s;
	Expression ep;
	Symbol id;
};

bool cmp(const info &l, const info &r) {
	if(l.f != r.f) return l.f > r.f;
	return l.s < r.s;
}

void typcase_class::code(CgenClassTable *ct) {
	ostream &s = ct->str;
	Env *env = ct->env;
	env->enterscope();
	
	expr->code(ct);
	emit_push(ACC, s);
	env->push();
	
	vector< info > ci;
	int lab0 = ct->get_label();
	int lab = ct->get_label();
	emit_bne(ACC, ZERO, lab, s);
	StringEntry *se = stringtable.lookup_string((char *)ct->env->curr_class->filename->get_string());
	s << LA << ACC << " "; se->code_ref(s); 
	s << endl;
	
	emit_load_imm(T1, line_number, s);
	emit_jal("_case_abort2", s);	
	
	emit_label_def(lab, s);
	emit_load(T1, 0, ACC, s);

	vector<Expression> ep;
	for(int i=0; cases->more(i); i++) {
		branch_class *cs = (branch_class *)cases->nth(i);
		info t;
		pair<int, int> p = ct->mp[ct->probe(cs->type_decl)];
		t.f = p.first; t.s = p.second; t.ep = cs->expr; t.id = cs->name;
		ci.push_back(t);
	}
	
	sort(ci.begin(), ci.end(), cmp);

	for(int i=0; i<ci.size(); i++) {
		env->enterscope();
	
		lab = ct->get_label();
		emit_blti(T1, ci[i].f, lab, s);
		emit_bgti(T1, ci[i].s, lab, s);
		
		string *str = new string(i2s(env->top()) + "($fp)");
		env->addid(ci[i].id, str);
		
		ci[i].ep->code(ct);
		emit_branch(lab0, s);
		emit_label_def(lab, s);
	
		env->exitscope();
	}
	emit_jal("_case_abort", s);
	
	emit_label_def(lab0, s);
	emit_pop(s);
	env->pop();
	
	env->exitscope();
//	cout << "typecse class" << endl;
}

void block_class::code(CgenClassTable *ct) {
	ostream &s = ct->str;
	ct->env->enterscope();
//cout << "block class " << endl;
	for(int i=0; body->more(i); i++) {
		body->nth(i)->code(ct);
	}
	ct->env->exitscope();
}

void let_class::code(CgenClassTable *ct) {
	ostream &s = ct->str;
	Env *env = ct->env;
	env->enterscope();
	
	if(!init->type) {
		if(ct->probe(type_decl)->basic())
			new__class(type_decl).code(ct);
		else
			emit_load_imm(ACC, 0, s);
	}
	else init->code(ct);
	emit_push(ACC, s);
	env->push();
	
	string *str = new string(i2s(env->top()) + "($fp)");
	env->addid(identifier, str);
	
	body->code(ct);
	
	emit_pop(s);
	env->pop();
	env->exitscope();
//	cout << "let class " << endl;
}

void common_arith(Expression e1, Expression e2, CgenClassTable *ct) {
	ostream &s = ct->str;
	e1->code(ct);
	emit_load(T1, 3, ACC, s);
	emit_push(T1, s);			// stack stores just value field,
								// not whole Int object

	//s << " here .. " << endl;
	e2->code(ct);
	emit_jal("Object.copy", s);
	
	emit_load(T2, 3, ACC, s);
	emit_load(T1, 1, SP, s);
	emit_pop(s);
}
void plus_class::code(CgenClassTable *ct) {
	ostream &s = ct->str;
	//s << "plus class" << endl;
	common_arith(e1, e2, ct);
	emit_add(T1, T1, T2, s);
	emit_store(T1, 3, ACC, s);
}

void sub_class::code(CgenClassTable *ct) {
//	cout << "sub class " << endl;
	ostream &s = ct->str;
	common_arith(e1, e2, ct);
	emit_sub(T1, T1, T2, s);
	emit_store(T1, 3, ACC, s);
}

void mul_class::code(CgenClassTable *ct) {
//	cout <<"mul class"  << endl;
	ostream &s = ct->str;
	common_arith(e1, e2, ct);
	emit_mul(T1, T1, T2, s);
	emit_store(T1, 3, ACC, s);
}

void divide_class::code(CgenClassTable *ct) {
//	cout << "divide class " << endl;
	ostream &s = ct->str;
	common_arith(e1, e2, ct);
	emit_div(T1, T1, T2, s);
	emit_store(T1, 3, ACC, s);
}

void neg_class::code(CgenClassTable *ct) {
//	cout << "neg class" << endl;
	ostream &s = ct->str;
	e1->code(ct);
	emit_jal("Object.copy", s);
	emit_load(T1, 3, ACC, s);
	emit_neg(T2, T1, s);
	emit_store(T2, 3, ACC, s);
}

void common_comp(Expression e1, Expression e2, CgenClassTable *ct, int type) {
	ostream &s = ct->str;
	e1->code(ct);
	emit_load(T1, 3, ACC, s);
	emit_push(T1, s);

	e2->code(ct);
	
	emit_load(T2, 3, ACC, s);
	emit_load(T1, 1, SP, s);
	emit_pop(s);
	
	int lab = ct->get_label();
	emit_load_bool(ACC, BoolConst(1), s);
	if(!type) {
		emit_blt(T1, T2, lab, s);
	}
	else {
		emit_bleq(T1, T2, lab, s);
	}
	
	emit_load_bool(ACC, BoolConst(0), s);
	emit_label_def(lab, s);
}

void lt_class::code(CgenClassTable *ct) {
//	cout << "lt class" << endl;
	ostream &s = ct->str;
	common_comp(e1, e2, ct, 0);
}

void eq_class::code(CgenClassTable *ct) {
//	cout <<"eq class" << endl;
	ostream &s = ct->str;
	e1->code(ct);
	emit_move(T1, ACC, s);
	emit_push(T1, s);

	e2->code(ct);
	emit_move(T2, ACC, s);
	emit_load(T1, 1, SP, s);
	emit_pop(s);
	
	int lab = ct->get_label();
	emit_load_bool(ACC, BoolConst(1), s);
	emit_beq(T1, T2, lab, s);
	
	emit_load_bool(ACC, BoolConst(1), s);
	emit_load_bool(A1, BoolConst(0), s);
	emit_jal("equality_test", s);
	
	emit_label_def(lab, s);
}

void leq_class::code(CgenClassTable *ct) {
//	cout << "leq class" << endl;
	ostream &s = ct->str;
	common_comp(e1, e2, ct, 1);
}

void comp_class::code(CgenClassTable *ct) {
//	cout << "comp class" << endl;
	ostream &s = ct->str;
	
	e1->code(ct);
	emit_load(T1, 3, ACC, s);
	
	int lab = ct->get_label();
	emit_load_bool(ACC, BoolConst(1), s);
	emit_beqz(T1, lab, s);
	emit_load_bool(ACC, BoolConst(0), s);
	emit_label_def(lab, s);
}

void int_const_class::code(CgenClassTable *ct) {
//	cout << "int const class " << endl;
	ostream &s = ct->str;
  //
  // Need to be sure we have an IntEntry *, not an arbitrary Symbol
  //
  emit_load_int(ACC,inttable.lookup_string(token->get_string()),s);
}

void string_const_class::code(CgenClassTable *ct) {
//	cout << "string const clas" << endl;
	ostream &s = ct->str;
  emit_load_string(ACC,stringtable.lookup_string(token->get_string()),s);
}

void bool_const_class::code(CgenClassTable *ct) {
//	cout << "bool const clas " << endl;
	ostream &s = ct->str;
  emit_load_bool(ACC, BoolConst(val), s);
}

void new__class::code(CgenClassTable *ct) {
//	cout << " new class " << endl;
	ostream &s = ct->str;
	
	emit_load_address(T1, CLASSOBJTAB, s);
	if(type_name == SELF_TYPE) {
		emit_load(T2, 0, SELF, s);
		emit_sll(T2, T2, 3, s);
	}
	else {
		int ind = ct->ind(type_name);
		emit_load_imm(T2, ind*8,s );
	}
	emit_addu(T1, T1, T2, s);
	emit_move(S1, T1, s);
	emit_load(ACC, 0, T1, s);
	
	emit_jal("Object.copy", s);
	
	emit_load(T1, 1, S1, s);
	emit_jalr(T1, s);
}

void isvoid_class::code(CgenClassTable *ct) {
//	cout << "isvoid class " << endl;
	ostream &s = ct->str;
	e1->code(ct);
	emit_move(T1, ACC, s);
	emit_load_bool(ACC, BoolConst(1), s);
	int lab = ct->get_label();
	emit_beqz(T1, lab, s);
	emit_load_bool(ACC, BoolConst(0), s);
	emit_label_def(lab, s);
}

void no_expr_class::code(CgenClassTable *ct) {
//	cout << "no expr class " << endl;
	ostream &s = ct->str;
}

void object_class::code(CgenClassTable *ct) {
	ostream &s = ct->str;
	//s << "object class " << endl;
	Env *env = ct->env;
	if(name == self) {
		emit_move(ACC, SELF, s);
		return;
	}
//	cout << "not self " << endl;
	string *str = env->lookup(name);
	
	//s << str << endl;
	s << LW << ACC << " " << *str << endl;
}


