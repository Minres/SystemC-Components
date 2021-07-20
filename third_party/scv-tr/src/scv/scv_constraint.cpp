//  -*- C++ -*- <this line is for emacs to recognize it as C++ code>
/*****************************************************************************
  Licensed to Accellera Systems Initiative Inc. (Accellera) 
  under one or more contributor license agreements.  See the 
  NOTICE file distributed with this work for additional 
  information regarding copyright ownership. Accellera licenses 
  this file to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at
 
    http://www.apache.org/licenses/LICENSE-2.0
 
  Unless required by applicable law or agreed to in writing,
  software distributed under the License is distributed on an
  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
  KIND, either express or implied.  See the License for the
  specific language governing permissions and limitations
  under the License.

 *****************************************************************************/

/*****************************************************************************

  scv_constraint.cpp -- The public interface for the constraint facility.

  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Torsten Maehne,
                               Universite Pierre et Marie Curie, 2013-04-20
  Description of Modification: Fix the for loop condition in
                               _scv_find_extensions() and _scv_copy_values()
                               to ensure that the i and j iterators are always
                               valid for dereferencing in the loop body.

 *****************************************************************************/
#include "scv/scv_util.h"
#include "scv/scv_constraint.h"

#include "cuddObj.hh"
#include "cudd.h"
#include "cuddInt.h"
#include "util.h"

#include <cfloat>

/////////////////////////////////////////////////////////////////
// Class : scv_constraint_base
//   - Implementation of the scv_constraint_base class methods
//   - scv_constraint_base is base class constraints
/////////////////////////////////////////////////////////////////

static int sDebugLevel = 0;
static void _scv_push_constraint(scv_constraint_base * c);

scv_constraint_base::scv_constraint_base() : 
  mode_(scv_extensions_if::RANDOM), scan_counter_(-1), ignore_(0)
{ 
  _scv_push_constraint(this); 
}

scv_constraint_base::~scv_constraint_base() {}

void scv_constraint_base::next() 
{ 
  uninitialize(); 
  scv_constraint_manager::set_value(this, true);
}

void scv_constraint_base::set_mode(scv_extensions_if::mode_t m) 
{
  if (m == scv_extensions_if::DISTRIBUTION) {
    _scv_message::message(_scv_message::CONSTRAINT_INVALID_MODE, 
      get_name());
  } else {
    if (mode_ == scv_extensions_if::SCAN) {
      scan_counter_ = -1;
      _scv_message::message(_scv_message::NOT_IMPLEMENTED_YET, "SCAN mode for complex constraints (will treat as RANDOM).");
      mode_ = scv_extensions_if::RANDOM;
    } else {
      mode_ = m;
    }
    std::list<scv_smart_ptr_if*>::iterator i;
    for (i= get_members().begin(); i != get_members().end(); ++i) {
      scv_extensions_if* e = (*i)->get_extensions_ptr();
      e->get_constraint_data()->set_ext_mode(m, 0, 0);
    }
  }
}

scv_extensions_if::mode_t scv_constraint_base::get_mode(void) const 
{
  return mode_;
}

void scv_constraint_base::set_random(scv_shared_ptr<scv_random> g) 
{
  gen_ = g;
  std::list<scv_smart_ptr_if*>::iterator i;
  for (i= get_members().begin(); i != get_members().end(); ++i) {
    scv_extensions_if* e = (*i)->get_extensions_ptr();
    e->set_random(g);
  }
}

scv_shared_ptr<scv_random> scv_constraint_base::get_random(void) 
{
  if (gen_.isNull())  {
    gen_ = new scv_random(get_name());
    scv_shared_ptr<scv_random> g(gen_);
    gen_ = g; 
  }
  return gen_;
}

void scv_constraint_base::get_members(std::list<scv_smart_ptr_if*>& vlist) 
{
  std::list<scv_smart_ptr_if*>::iterator i;
  for (i = pointers_.begin(); i != pointers_.end(); i++) {
    vlist.push_back(*i);
  }
}

std::list<scv_smart_ptr_if*>& scv_constraint_base::get_members(void) 
{
  return pointers_;
}

int scv_constraint_base::debug_ = scv_debug::INITIAL_DEBUG_LEVEL;

int scv_constraint_base::get_debug()
{
  return debug_;
}

const char *scv_constraint_base::get_name() const
{
  return name_.c_str();
}

const std::string& scv_constraint_base::get_name_string() const
{
  return name_;
}

const char *scv_constraint_base::kind() const
{
  static const char *name = "scv_constraint_base";
  return name;
}

void scv_constraint_base::print(ostream& o, int details, int indent) const
{
  int local_indent = indent;
  std::string spaces="";

  for (int i=0; i < indent; i++) spaces += " ";

  if (details == 0 || details == 1) {
    o << spaces << kind() << " Name: " << name_ << endl;
    o << spaces << "  Hard constraints: " << _hard_constraints << endl;
    o << spaces << "  Soft constraints: " << _soft_constraints << endl;
  }

  if (details == 0 || details == 2) {
    o << spaces << "  Number of elements: " << pointers_.size() << endl;
    o << spaces << "  Current value of elements: " << endl;
    std::list<scv_smart_ptr_if*>::const_iterator i;
    for (i= pointers_.begin(); i != pointers_.end(); ++i) {
      o << spaces << "    " << (*i)->get_name() << ":  " ;
      if ((*i)->get_extensions_ptr()->is_record() || 
          (*i)->get_extensions_ptr()->is_array()) {
        o << endl;
        local_indent = indent + 4;
      } else {
        local_indent = indent;
      }
      (*i)->print(o, details, local_indent);
    }
  }
}

void scv_constraint_base::set_debug(int debug)
{
  if ( debug_ == debug ) return;
  debug_ = debug;
  scv_debug::set_facility_level(scv_debug::RANDOMIZATION, debug);
}

void scv_constraint_base::show(int details, int indent) const
{
  print(scv_out, details, indent);
}

scv_expression scv_constraint_base::get_constraint() const 
{
  return scv_constraint_base::eh() && scv_constraint_base::ebh();
}

scv_expression scv_constraint_base::get_soft_constraint() const 
{
  return scv_constraint_base::es() && scv_constraint_base::ebs();
}

scv_constraint_base* scv_constraint_base::get_copy(scv_constraint_base * from) 
{
  assert(0);
  return NULL;
}

void scv_constraint_base::uninitialize() 
{
  std::list<scv_smart_ptr_if*>::iterator i;
  for (i= pointers_.begin(); i != pointers_.end(); ++i)
    (*i)->get_extensions_ptr()->uninitialize();
}

void scv_constraint_base::initialize() 
{
  scv_constraint_manager::set_value(this, true);
}

bddNodeT& scv_constraint_base::get_bdd() 
{
  static bddNodeT& bh = scv_constraint_manager::get_bdd(
    scv_constraint_base::get_constraint(), this, true);
  static bddNodeT& bs = scv_constraint_manager::get_bdd(
    scv_constraint_base::get_soft_constraint(), this, false);
  static bddNodeT& b = _scv_bdd_and(bh, bs, this);
  if (ignore_) {
    return bh;
  } else {
    return b;
  }
}

void scv_constraint_base::init_bdd() 
{
  return;
}

scv_expression& scv_constraint_base::eh() const
{ 
  static scv_expression e = true; 
  return e; 
}

scv_expression& scv_constraint_base::es() const 
{ 
  static scv_expression e = true; 
  return e; 
}

scv_expression& scv_constraint_base::ebh() const 
{ 
  static scv_expression e = true; 
  return e; 
}

scv_expression& scv_constraint_base::ebs() const 
{
  static scv_expression e = true; 
  return e; 
}

void scv_constraint_base::set_up_members(std::list<scv_smart_ptr_if*>& members) 
{
  pointers_ = members;
  scv_constraint_manager::reset();
  std::list<scv_smart_ptr_if*>::iterator i;
  for (i= pointers_.begin(); i != pointers_.end(); ++i) {
    (*i)->get_extensions_ptr()->set_constraint(this);
    //scv_constraint_manager::add_extension((*i)->get_extensions_ptr());
  }
}

void scv_constraint_base::ignore_soft_constraint(void) 
{
  ignore_ = true;
}

bool scv_constraint_base::is_ignored_soft_constraint(void) 
{
  return ignore_;
}

int scv_constraint_base::get_scan_counter(void) 
{
  return ++scan_counter_;
}

void scv_constraint_base::set_expression_string(const char * e, bool hard_constraint)  
{
  if (hard_constraint) {
    _hard_constraints = e;
  } else {
    _soft_constraints = e;
  }
}

/////////////////////////////////////////////////////////////////
// Class : scv_constraint_manager
//   - Implementation of scv_constraint_manager
//   - External interface for the _scv_constraint_manager class.
//   - Provides static methods to manipulate constraints and
//     generate random values
/////////////////////////////////////////////////////////////////

void _scv_remove(_scv_expr* e);
static void _scv_remove_data(_scv_expr* e); 
static void _scv_remove_if_bdd(_scv_expr* e); 
static bool _scv_is_avoid_duplicate(scv_constraint_base * c);
static bool _scv_is_scan(scv_constraint_base * c); 

_scv_constraint_manager* scv_constraint_manager::globalConstraintManagerP=NULL;
static _scv_expr* errorExprP;

bddNodeT& scv_constraint_manager::get_bdd(scv_expression e, scv_constraint_base* c, bool hard_constraint)
{
  return getConstraintManagerP()->get_bdd(e, c, hard_constraint);
}

void scv_constraint_manager::init_bdd(scv_expression e, scv_constraint_base* c, bool hard_constraint)
{
  getConstraintManagerP()->init_bdd(e, c, hard_constraint);
}

void scv_constraint_manager::init_maxvar(const scv_expression& he, const scv_expression& se)
{
  getConstraintManagerP()->countMaxVar(he);
  getConstraintManagerP()->countMaxVar(se);
}

void scv_constraint_manager::set_value(scv_extensions_if* s, bddNodeT *b, 
  _scv_constraint_data *cdata_)
{
  getConstraintManagerP()->assignRandomValue(s, b, cdata_);
}

void scv_constraint_manager::set_value(scv_constraint_base* c, bool simplify)
{
  getConstraintManagerP()->assignRandomValue(c, simplify);
}

void scv_constraint_manager::wrapup(scv_extensions_if* s, void *argP)
{
  getConstraintManagerP()->wrapup(s);
}

void scv_constraint_manager::reset(void)
{
  getConstraintManagerP()->reset();
}

void scv_constraint_manager::add_extension(scv_extensions_if *s)
{
  getConstraintManagerP()->add_extension(s);
}

_scv_constraint_manager* scv_constraint_manager::getConstraintManagerP(void)
{
  SCV_STARTUP();
  return globalConstraintManagerP;
}

////////////////////////////////////////////////////////////////////
// Class :  _scv_expr
//   - Implementation for the internal expression representation 
//   - Used in the BDD based constraint solver 
////////////////////////////////////////////////////////////////////

_scv_expr::_scv_expr()
{
  type = _scv_expr::EMPTY;
  bit_width = 0;
  vecsize = 0;
  sigLsb = 0;
  sigMsb = 0;
  value.uvalue = 0; 
  isVar = 0;
}

_scv_expr::_scv_expr(const _scv_expr& other)
{
  type = other.type;
  bit_width = other.bit_width;
  vecsize = other.vecsize;
  sigLsb = other.sigLsb;
  sigMsb = other.sigMsb;
  value = other.value;
  isVar = other.isVar;
  sc_data = other.sc_data;
}

_scv_expr::~_scv_expr() {}

_scv_expr& _scv_expr::operator=(const _scv_expr& rhs)
{
  type = rhs.type;
  bit_width = rhs.bit_width;
  vecsize = rhs.vecsize;
  sigLsb = rhs.sigLsb;
  sigMsb = rhs.sigMsb;
  value = rhs.value;
  isVar = rhs.isVar;
  sc_data = rhs.sc_data;

  return *this;
}

void _scv_expr::setType(ExprValueType t)
{
  type = t;
}

void _scv_expr::setBddNodeP(bddNodeT* b)
{
  value.bdd = b;
}

void _scv_expr::setIntValue(long long i)
{
  value.ivalue = i;
}

void _scv_expr::setBooleanValue(bool b)
{
  value.bvalue = b;
}

void _scv_expr::setUnsignedValue(unsigned long long u)
{
  value.uvalue = u;
}

void _scv_expr::setBigValue(sc_bv_base& v, int bw)
{
  scv_shared_ptr<sc_bv_base> tmp(new sc_bv_base(bw));
  sc_data = tmp;
  *sc_data = v;
}

void _scv_expr::setDoubleValue(double d)
{
  value.dvalue = d;
}

void _scv_expr::setStringValue(const char* s) 
{
  value.cvalue = new char[strlen(s)+1];
  strcpy(value.cvalue, s);
}

void _scv_expr::setBddVectorP(bddVectorT* v)
{
  value.bddvec = v;
}

void _scv_expr::setExtensionP(scv_extensions_if* s)
{
  value.ext = s;
}

void _scv_expr::setVecSize(int vsize)
{
  vecsize = vsize;
}

void _scv_expr::setBitWidth(int bw)
{
  bit_width = bw;
}

void _scv_expr::setSigWidth(int lsb, int msb)
{
  sigLsb = lsb;
  sigMsb = msb;
}

void _scv_expr::setSigMsb(int msb)
{
  sigMsb = msb;
}

void _scv_expr::setSigLsb(int lsb)
{
  sigLsb = lsb;
}

const _scv_expr::ExprValueType _scv_expr::get_type(void) const
{
  return type;
}

bddNodeT* _scv_expr::getBddNodeP(void) const
{
  return value.bdd;
}

long long _scv_expr::getIntValue(void) const
{
  return value.ivalue;  
}

unsigned long long _scv_expr::getUnsignedValue(void) const
{
  return value.uvalue;
}

sc_bv_base _scv_expr::getBigValue(void) const
{
  return *sc_data;
}

bool _scv_expr::getBoolValue(void) const
{
  return value.bvalue;
}

double _scv_expr::getDoubleValue(void) const
{
  return value.dvalue;
}

char* _scv_expr::getStringValue(void) const
{
  return value.cvalue;
}

bddVectorT* _scv_expr::getBddVectorP(void) const
{
  return value.bddvec;
}

scv_extensions_if* _scv_expr::getExtensionP(void) const
{
  return value.ext;
}

int _scv_expr::getVecSize(void) const
{
  if (type == BDDVECTOR || type == BDDVECTOR_SIGNED) {
    return vecsize;
  } else { 
    _scv_constraint_error::internalError(
      "Accessing vector size for non-vector expression");
    return 0;
  }
}

int _scv_expr::getBitWidth(void) const
{
  return bit_width;
}

int _scv_expr::getSigMsb(void) const
{
  if (type == BDDVECTOR || type == BDDVECTOR_SIGNED) {
    return sigMsb;
  } else {
    _scv_constraint_error::internalError(
      "Accessing lsb for non-vector expression.");
    return 0;
  }
}

int _scv_expr::getSigLsb(void) const
{
  if (type == BDDVECTOR || type == BDDVECTOR_SIGNED) {
    return sigLsb;
  } else {
    _scv_constraint_error::internalError(
      "Accessing lsb for non-vector expression.");
    return 0;
  }
}

///////////////////////////////////////////////////////////////
// _scv_constraint_manager: 
//  Class which implements the BDD constraint solver.
//  There is a single object of this type which is controlled
//  via the scv_constraint_manager class static methods
//  scv_constraint_manager provides external interface.
///////////////////////////////////////////////////////////////

_scv_constraint_manager::_scv_constraint_manager()
{
  _mgr = new Cudd();
  verboseLevel = 0;
  nthvar = 0;
  maxvar = 1;
  numBddVar = 0;
  maxNumBits = 0;
  eProb = new int;
  mode = scv_extensions_if::RANDOM;
  numBitsSigned = 8 * sizeof(int);
  oneNode = _mgr->bddOne().getNode();
  zeroNode = _mgr->bddZero().getNode();
  exprRepOne = new _scv_expr;
  exprRepZero = new _scv_expr;

  extHash = new _scv_open_table<_smartDataRecordT>(SIZE_HINT);
  countExtHash = new _scv_associative_array<scv_extensions_if*, int>
    ("countExtHash", 0);
  avoidDuplicateHash = new _scv_associative_array<scv_constraint_base*, 
    bddNodeT*> ("avoidDuplicateHash", 0);
  avoidDuplicateExtHash = new _scv_associative_array<scv_extensions_if*,
    bddNodeT*> ("avoidDuplicateExtHash", 0);
  nodeHashP = new _scv_open_table<int>(SIZE_HINT);
  nodeWeightHash = new _scv_associative_array<ddNodeT*, double> 
    ("nodeWeightHash", 0);
  wasVisited = new _scv_associative_array<ddNodeT*, int> 
    ("wasVisited", 0);
  enumVarList = new std::list<scv_extensions_if*>;
}

_scv_constraint_manager::~_scv_constraint_manager()
{
  delete eProb;
  delete extHash;
  delete countExtHash;
  delete avoidDuplicateHash;
  delete avoidDuplicateExtHash;
  delete nodeHashP;
  delete nodeWeightHash;
  delete wasVisited;
}

void _scv_constraint_manager::reset(void) 
{
  nthvar = 0;
  maxvar = 0;
  maxNumBits = 0;
  numBddVar = 0;
  countExtHash->clear();
  enumVarList->clear();
}

bddNodeT& _scv_constraint_manager::get_bdd(scv_expression e,  scv_constraint_base * c, bool hard_constraint)
{
  bddNodeT* b = NULL;
  _scv_expr t1 = getExpressionRep(e);
  _scv_expr t2 = getBasicEnumConstraint();
  _scv_expr t = exprAnd(t1, t2);
  _scv_remove_data(&t1);
  _scv_remove_data(&t2);
  if (isOverConstrained(t)) {
    _scv_constraint_error::cannotMeetConstraint(c->get_name());
    b = getExprRepOne().getBddNodeP();
  } else {
    b = t.getBddNodeP();
    check_sparse_var(c, b);
  }
  return *b;
}

void _scv_constraint_manager::add_sparse_var(scv_extensions_if* e, bddNodeT* b)
{
  int size;

  if (e->is_record()) {
    size = e->get_num_fields();
    for (int j=0; j<size; ++j) {
      scv_extensions_if *ef = e->get_field(j);
      add_sparse_var(ef, b);
    }
  } else if (e->is_array()) {
    size = e->get_array_size();
    for (int j=0; j<size; ++j) {
      scv_extensions_if *ef = e->get_array_elt(j);
      add_sparse_var(ef, b);
    }
  } else {
    int bitWidth=0;
    _smartDataRecordT * sDataP;

    bitWidth = e->get_bitwidth();
  
    if (verboseLevel > 3) {
      scv_out << "Extension: " << e << " BitWidth : " << bitWidth << " Has Extension: " << extHash->get(e, sDataP) << endl;
    }
  
    if (extHash->get(e, sDataP)) {
      if (bitWidth < maxNumBits) {
        for (int j = bitWidth; j < maxNumBits; j++) {
          *b = (*b &  (!_mgr->bddVar((maxvar*j)+sDataP->startIndex)));
          if (verboseLevel > 3) {
            scv_out << "  EXTRA FOR SIZE: " << ((maxvar*j)+sDataP->startIndex) << endl;
          }
        } 
      }
    }
  }
}

void _scv_constraint_manager::check_sparse_var(scv_constraint_base * c, bddNodeT* b) 
{
  std::list<scv_smart_ptr_if*>::iterator i;
  std::list<scv_smart_ptr_if*>& pointers_ = c->get_members();
  scv_extensions_if* e = NULL;

  for (i= pointers_.begin(); i != pointers_.end(); ++i) {
    e = (*i)->get_extensions_ptr();
    add_sparse_var(e, b);
  }
}

void _scv_constraint_manager::init_bdd(scv_expression e, scv_constraint_base* c, bool hard_constraint)
{
  c->set_expression_string(e.get_expression_string(), hard_constraint);
  initExpression(e);
}

_scv_expr _scv_constraint_manager::simplifyMember(scv_extensions_if* e, _scv_expr t1, scv_constraint_base* c, bool& remove_, bool& over_constraint) 
{
  int size;
  _scv_expr tmp;
  if (e->is_record()) {
    size = e->get_num_fields();
    for (int j=0; j<size; ++j) {
      scv_extensions_if *ef = e->get_field(j);
      t1 = simplifyMember(ef, t1, c, remove_, over_constraint);
      if (over_constraint) break;
    }
  } else if (e->is_array()) {
    size = e->get_array_size();
    for (int j=0; j<size; ++j) {
      scv_extensions_if *ef = e->get_array_elt(j);
      t1 = simplifyMember(ef, t1, c, remove_, over_constraint);
      if (over_constraint) break;
    }
  } else {
    bool tmp_remove = false;

    tmp = simplifyField(t1, e, tmp_remove);
    if (isOverConstrained(tmp)) { 
      if (!c->is_ignored_soft_constraint()) {
        _scv_constraint_error::ignoredLevel(c->get_name());
        c->ignore_soft_constraint();
        if (tmp_remove) {
          _scv_remove_data(&tmp);
        }
        tmp.setBddNodeP(simplifyConstraint(c, remove_ ));
      } else {
        _scv_constraint_error::cannotMeetConstraint(c->get_name());
        over_constraint = true;
      }
    }
    if (tmp_remove && (t1.getBddNodeP() != &c->get_bdd())) {
      _scv_remove_data(&t1);
    }
    remove_ = tmp_remove;
    t1 = tmp;
  }
  return t1;
}

bddNodeT* _scv_constraint_manager::simplifyConstraint(scv_constraint_base* c, bool& remove_, bddNodeT* b)
{ 
  _scv_expr t1;

  remove_ = false;
  bool over_constraint = false;

  if (b) {
    t1.setBddNodeP(b);
  } else {
    t1.setBddNodeP(&(c->get_bdd()));
  }
  t1.setType(_scv_expr::BDD);
  std::list<scv_smart_ptr_if*>::iterator i;
  for (i= c->get_members().begin(); i != c->get_members().end(); ++i) {
    scv_extensions_if* e = (*i)->get_extensions_ptr();
    t1 = simplifyMember(e, t1, c, remove_, over_constraint);
    if (over_constraint) break;
  }
  return t1.getBddNodeP();
}

_scv_expr _scv_constraint_manager::simplifyField(_scv_expr t1, scv_extensions_if* s, bool & remove_)
{
  _scv_expr expr;

   _smartDataRecordT * sDataP;

  if (!extHash->get(s, sDataP)) {
    return t1;
  } 
 
  if (s->is_initialized()) {
    switch(s->get_type()) {
    case scv_extensions_if::INTEGER: 
    case scv_extensions_if::ENUMERATION: 
    {
      int bitwidth = s->get_bitwidth();
      if (bitwidth <= 64) {
        long long value = s->get_integer();
        expr = exprEqual(createExprRep(s), createExprRep(value, bitwidth));
      } else {
        sc_bv_base value(bitwidth);
        s->get_value(value);
        expr = exprEqual(createExprRep(s), createExprRep(value, bitwidth));
      }
      t1 = exprAnd(t1, expr);
      _scv_remove_data(&expr);
      remove_ = true;
      break;
    }
    case scv_extensions_if::UNSIGNED:
    case scv_extensions_if::BIT_VECTOR:
    case scv_extensions_if::LOGIC_VECTOR:
    {
      int bitwidth = s->get_bitwidth();
      if (bitwidth <= 64) {
        unsigned long long value = s->get_unsigned();
        expr = exprEqual(createExprRep(s), createExprRep(value, bitwidth));
      } else {
        sc_bv_base value(bitwidth);
        s->get_value(value);
        expr = exprEqual(createExprRep(s), createExprRep(value, bitwidth));
      }
      t1 = exprAnd(t1, expr);
      _scv_remove_data(&expr);
      remove_ = true;
      break;
    }
    case scv_extensions_if::BOOLEAN:
    {
      bool value = s->get_bool();
      int bitwidth = s->get_bitwidth();
      expr = exprEqual(createExprRep(s), createExprRep(value, bitwidth));
      t1 = exprAnd(t1, expr);
      _scv_remove_data(&expr);
      remove_ = true;
      break;
    }
    default: {
        _scv_constraint_error::notImplementedYet(s->get_type_name());
        break;
      }
    }
  }
  return t1;
}

void _scv_constraint_manager::assignRandomValue(scv_extensions_if* s, bddNodeT * b, scv_shared_ptr<scv_random> g)
{
   _smartDataRecordT * sDataP;

  setRandomGen(g); 

  if (!extHash->get(s, sDataP)) {
    if (!s->is_initialized()) {
      s->next();
    }
  } else {
    int msize = getSizeOfBddVec(s);
    ValueIndexSizeType valueIndexMaxIndex = msize > 0 ? (msize-1)*sDataP->numvar + sDataP->startIndex : 0;
    if (valueIndexMaxIndex >= valueIndex.size()) {
      valueIndex.resize(valueIndexMaxIndex + 1);
    }
    for (int i=0; i<msize; i++) {
      valueIndex[i*sDataP->numvar+sDataP->startIndex] = 2;
    }
    getVectorFromBdd(b->getNode());
    setValue(s, sDataP->startIndex, msize, sDataP->numvar);
  }
}

void _scv_constraint_manager::assignRandomValue(scv_extensions_if* s, bddNodeT * b, _scv_constraint_data * cdata_)
{
   scv_shared_ptr<scv_random> g = cdata_->get_random(s);
   _smartDataRecordT * sDataP;

  setRandomGen(g); 

  if (!extHash->get(s, sDataP)) {
    if (!s->is_initialized()) {
      generate_value_range_constraint(s, cdata_);
    }  
  } else {
    int msize = getSizeOfBddVec(s);
    bool _avoid_duplicate = false;
    bool remove_ = false;
    bddNodeT* sb;
    _scv_expr t, expr, expr_n, expr_f;
  
    scv_constraint_base* c = cdata_->get_constraint();
  
    if (cdata_->is_scan_mode()) {
      mode = scv_extensions_if::SCAN;
      randomnext = cdata_->prev_val_.unsigned_;
      randomnext = randomnext << 1;
      cdata_->prev_val_.unsigned_ += cdata_->lb_scan_;
    } else if (cdata_->is_avoid_duplicate_mode()) {
      _avoid_duplicate = true;
      // Remove the previous bdd if it is not the constraint expression bdd 
      sb = avoidDuplicateHash->getValue(c);
      if (sb != &c->get_bdd()) {
        remove_ = true;
      }
      t.setBddNodeP(sb);
      t.setType(_scv_expr::BDD);
  
    } else {
      mode = scv_extensions_if::RANDOM;
    }
   
    ValueIndexSizeType valueIndexMaxIndex = msize > 0 ? (msize-1)*sDataP->numvar + sDataP->startIndex : 0;
    if (valueIndexMaxIndex >= valueIndex.size()) {
      valueIndex.resize(valueIndexMaxIndex + 1);
    }
    for (int i=0; i<msize; i++) {
      valueIndex[i*sDataP->numvar+sDataP->startIndex] = 2;
    }
  
    getVectorFromBdd(b->getNode());
  
    if (_avoid_duplicate) {
      expr = assignValueMember(s, _avoid_duplicate);
  
      expr_n = exprNot(expr);
      expr_f = exprAnd(t, expr_n);
      
      _scv_remove_data(&expr_n);
      _scv_remove_data(&expr);
      if (isOverConstrained(expr_f)) {
        avoidDuplicateHash->insert(c, 0); 
      } else {
        avoidDuplicateHash->insert(c, expr_f.getBddNodeP());
      }
      if (remove_) {
        _scv_remove_data(&t);
      }
    } else {
      setValue(s, sDataP->startIndex, msize, sDataP->numvar);
    }
  }
}

void _scv_constraint_manager::initMember(scv_extensions_if* e)
{
  _smartDataRecordT * sDataP=NULL;
  if (e->is_record()) {
    int size = e->get_num_fields();

    for (int j=0; j < size; j++) {
      scv_extensions_if* ef = e->get_field(j);
      initMember(ef);
    }
  } else if (e->is_array()) {
    int size = e->get_array_size();

    for (int j=0; j < size; j++) {
      scv_extensions_if* ef = e->get_array_elt(j);
      initMember(ef);
    }
  } else {
    _scv_constraint_data::gen_mode gen_mode = 
      e->get_constraint_data()->get_mode();

    if ( (gen_mode == _scv_constraint_data::DISTRIBUTION) ||
         (gen_mode == _scv_constraint_data::DISTRIBUTION_RANGE) ||
         (gen_mode == _scv_constraint_data::RANGE_CONSTRAINT)) {

        if (verboseLevel > 3) {
          scv_out << "  Doing Initialize on: " << e->get_name() << endl;
        }
        e->initialize();
    } 
    if (extHash->get(e, sDataP)) {

      if ( !(gen_mode == _scv_constraint_data::DISTRIBUTION) ||
           (gen_mode == _scv_constraint_data::DISTRIBUTION_RANGE) ||
           (gen_mode == _scv_constraint_data::RANGE_CONSTRAINT)) {
        int msize = getSizeOfBddVec(e);
        ValueIndexSizeType valueIndexMaxIndex = msize > 0 ? (msize-1)*sDataP->numvar + sDataP->startIndex : 0;
        if (valueIndexMaxIndex >= valueIndex.size()) {
          valueIndex.resize(valueIndexMaxIndex + 1);
        }
        for (int k=0; k<msize; k++) {
          valueIndex[k*sDataP->numvar+sDataP->startIndex] = 2;
        }
      }
    } 
  }
  return;
}

_scv_expr _scv_constraint_manager::assignValueMember(scv_extensions_if* e, bool _avoid_duplicate)
{
  _smartDataRecordT * sDataP=NULL;
  _scv_expr expr;
  _scv_expr tmp;
  _scv_expr tmp1;

  if (_avoid_duplicate) {
    expr = getExprRepOne();
  }

  if (e->is_record()) {
    int size = e->get_num_fields();

    for (int j=0; j < size; j++) {
      scv_extensions_if* ef = e->get_field(j);
      if (_avoid_duplicate) {
        tmp1 = expr;
      }
      tmp = assignValueMember(ef, _avoid_duplicate);
      if (_avoid_duplicate) {
        expr = exprAnd(expr, tmp);
        _scv_remove_data(&tmp); 
        _scv_remove_data(&tmp1); 
      }
    }
  } else if (e->is_array()) {
    int size = e->get_array_size();
    for (int j=0; j < size; j++) {
      scv_extensions_if* ef = e->get_array_elt(j);
      if (_avoid_duplicate) {
        tmp1 = expr;
      }
      tmp = assignValueMember(ef, _avoid_duplicate);
      if (_avoid_duplicate) {
        expr = exprAnd(expr, tmp);
        _scv_remove_data(&tmp); 
        _scv_remove_data(&tmp1); 
      }
    }
  } else {
    bool should_remove=false;

    if (extHash->get(e, sDataP)) {
      if (!e->is_initialized()) {
        int msize = getSizeOfBddVec(e);
        setValue(e, sDataP->startIndex, msize, sDataP->numvar);
        if (_avoid_duplicate) {
          tmp = simplifyField(expr, e, should_remove);
          _scv_remove_data(&expr);
          expr = tmp;
        }
      }
    } else {
      if (!e->is_initialized()) {
        e->next();
      } 
    }
  }
  return expr;
}

void _scv_constraint_manager::assignRandomValue(scv_constraint_base* c, bool simplify)
{
  std::list<scv_smart_ptr_if*>::iterator i;

  std::list<scv_smart_ptr_if*>& pointers_ = c->get_members();

  setRandomGen(c->get_random());

  for (i= pointers_.begin(); i != pointers_.end(); ++i) {
    scv_extensions_if* e = (*i)->get_extensions_ptr();
    initMember(e);
  }

  bool remove_ = false;
  bool _avoid_duplicate = false;
  _scv_expr t;
  bddNodeT* sb;
  _scv_expr expr; 
  _scv_expr tmp, tmp1;

  _avoid_duplicate = _scv_is_avoid_duplicate(c);
  if (_avoid_duplicate) {
    remove_ = true;
    sb = avoidDuplicateHash->getValue(c);
    if (!sb) {
      if (simplify) {
        sb = simplifyConstraint(c, remove_);
      } else {
        sb = &c->get_bdd();
        remove_ = false;
      }
    } 
  } else {
    if (simplify) {
      sb = simplifyConstraint(c, remove_);
    } else {
      sb = &c->get_bdd();
    }
  }
  t.setBddNodeP(sb);
  t.setType(_scv_expr::BDD);

  if (_scv_is_scan(c)) {
    mode = scv_extensions_if::SCAN;
    randomnext = (unsigned int)c->get_scan_counter();
    randomnext = randomnext << 1;
  } else {
    mode = scv_extensions_if::RANDOM;
  } 

  getVectorFromBdd(sb->getNode());
 
  if (_avoid_duplicate) { 
    expr = getExprRepOne();
  }

  for (i= pointers_.begin(); i != pointers_.end(); ++i) {
    scv_extensions_if* e = (*i)->get_extensions_ptr();
    _scv_constraint_data::gen_mode gen_mode = e->get_constraint_data()->get_mode();

    if ( !((gen_mode == _scv_constraint_data::DISTRIBUTION) ||
         (gen_mode == _scv_constraint_data::DISTRIBUTION_RANGE) ||
         (gen_mode == _scv_constraint_data::RANGE_CONSTRAINT)) ) {

      if (_avoid_duplicate) { 
        tmp1 = expr; 
      }
  
      tmp = assignValueMember(e, _avoid_duplicate);
      
      if (_avoid_duplicate) {
        expr = exprAnd(expr, tmp);
        _scv_remove_data(&tmp1);
        _scv_remove_data(&tmp);
      }
    }
  }

  if (_avoid_duplicate) {
    _scv_expr expr_n = exprNot(expr);
    _scv_expr expr_f = exprAnd(t, expr_n);

    _scv_remove_data(&expr_n);
    _scv_remove_data(&expr);
    if (isOverConstrained(expr_f)) {
      avoidDuplicateHash->insert(c, 0); 
      _scv_remove_data(&expr_f);
    } else {
      avoidDuplicateHash->insert(c, expr_f.getBddNodeP());
    }
  }

  if (remove_) {
    _scv_remove_data(&t);
  }

}

void _scv_constraint_manager::wrapup(scv_extensions_if* s)
{
  _smartDataRecordT* sDataP;

  if (s->is_record()) {
    int size = s->get_num_fields();
    for (int i=0; i<size; ++i) {
      scv_extensions_if* ef = s->get_field(i);
      wrapup(ef);
    }
  } else if (s->is_array()) { 
    int size = s->get_array_size();
    for (int i=0; i<size; ++i) {
      scv_extensions_if* ef = s->get_array_elt(i);
      wrapup(ef);
    }
  } else { 
    if (extHash->get(s, sDataP)) {
      extHash->remove(s);
    }
  }
}

void _scv_constraint_manager::add_extension(scv_extensions_if* s)
{
  if (s->is_record()) {
    int size = s->get_num_fields();
    for (int i=0; i<size; ++i) {
      add_extension(s->get_field(i));
    }
  } else if (s->is_array()) {
    int size = s->get_array_size();
    for (int i=0; i<size; ++i) {
      add_extension(s->get_array_elt(i));
    }
  } else {
    createExprRep(s);
  }
}

bool _scv_constraint_manager::has_complex_constraint(scv_extensions_if* s)

{
  _smartDataRecordT * sDataP = NULL;
  return extHash->get(s, sDataP); 
}

_scv_expr _scv_constraint_manager::getConstantExprRep(_scv_expr e, int signExtend) const
{
  unsigned binVal[1024];
  _scv_expr newElem;

  //int msize = getSizeOfBddVec(e);
  int msize = e.getBitWidth();
  int resultSize = msize;

  if (signExtend != -1 && msize != signExtend) {
    resultSize = (msize >= signExtend) ? msize : signExtend;
  }

  bddVectorT *vec = new bddVectorT(resultSize);
  bddVectorT& vecR = *vec;

  switch(e.get_type()) {
  case _scv_expr::UNSIGNED :  
  case _scv_expr::SC_BV_BASE :  
  case _scv_expr::UNSIGNED_64BIT :  {
    getVector(e, binVal);
    for (int i=0; i<resultSize; i++) {
      if (i >= msize) {
        vecR[i] = _mgr->bddZero();
      } else {
        if (binVal[i] == 0) {
          vecR[i] = _mgr->bddZero();
        } else if (binVal[i] == 1) {
          vecR[i] = _mgr->bddOne();
        }
      }
    }
    newElem.setType(_scv_expr::BDDVECTOR);
    newElem.setBddVectorP(vec);
    newElem.setVecSize(resultSize);
    break;
  }
  case _scv_expr::INT :  
  case _scv_expr::BOOLEAN : {
    getVector(e, binVal);
    for (int i=0; i<resultSize; i++) {
      if (i >= msize) {
        if (binVal[msize-1] == 1) {
          vecR[i] = _mgr->bddOne();
        } else {
          vecR[i] = _mgr->bddZero();
        }
      } else {
        if (binVal[i] == 0) {
          vecR[i] = _mgr->bddZero();
        } else if (binVal[i] == 1) {
          vecR[i] = _mgr->bddOne();
        }
      }
    }
    newElem.setType(_scv_expr::BDDVECTOR);
    newElem.setBddVectorP(vec);
    newElem.setVecSize(resultSize);
    break;
  }
  case _scv_expr::DOUBLE : 
  case _scv_expr::STRING : 
  case _scv_expr::RECORD : 
  case _scv_expr::ARRAY : {
    break;
  }
  default:
    _scv_constraint_error::internalError(
      "Unknown expression type- getConstantExprRep.\n");
    break;
  }
  return newElem;
}

void _scv_constraint_manager::countMaxVar(const scv_expression& e)
{
  switch (e.get_operator()) {
  case scv_expression::EMPTY:
    break;
  case scv_expression::INT_CONSTANT:
  case scv_expression::UNSIGNED_CONSTANT:
  case scv_expression::DOUBLE_CONSTANT:
  case scv_expression::BOOLEAN_CONSTANT:
    return;
  case scv_expression::EXTENSION:
    checkExprRep(e.get_extension());
    return;
  case scv_expression::PLUS:
  case scv_expression::MINUS:
  case scv_expression::MULTIPLY:
  case scv_expression::EQUAL:
  case scv_expression::NOT_EQUAL:
  case scv_expression::GREATER_THAN:
  case scv_expression::LESS_THAN:
  case scv_expression::GREATER_OR_EQUAL:
  case scv_expression::LESS_OR_EQUAL:
  case scv_expression::AND:
  case scv_expression::OR:
      countMaxVar(e.get_left());
      countMaxVar(e.get_right());
      return;
  case scv_expression::NOT:
      countMaxVar(e.get_left());
      return;
  default: 
    _scv_constraint_error::internalError(
      "init bdd for unknown operator type in countMaxVar\n");
     break;
  }
  return;
}

void _scv_constraint_manager::initExpression(const scv_expression& e)
{
  switch (e.get_operator()) {
  case scv_expression::EMPTY:
    break;
  case scv_expression::INT_CONSTANT:
  case scv_expression::UNSIGNED_CONSTANT:
  case scv_expression::DOUBLE_CONSTANT:
  case scv_expression::BOOLEAN_CONSTANT:
    return;
  case scv_expression::EXTENSION:
    createExprRep(e.get_extension());
    return;
  case scv_expression::PLUS:
  case scv_expression::MINUS:
  case scv_expression::MULTIPLY:
  case scv_expression::EQUAL:
  case scv_expression::NOT_EQUAL:
  case scv_expression::GREATER_THAN:
  case scv_expression::LESS_THAN:
  case scv_expression::GREATER_OR_EQUAL:
  case scv_expression::LESS_OR_EQUAL:
  case scv_expression::AND:
  case scv_expression::OR:
      initExpression(e.get_left());
      initExpression(e.get_right());
      return;
  case scv_expression::NOT:
      initExpression(e.get_left());
      return;
  default: 
    _scv_constraint_error::internalError(
      "init bdd for unknown operator type in initExpression\n");
     break;
  }
  return;
}

_scv_expr _scv_constraint_manager::getExpressionRep(const scv_expression& e)
{
  switch (e.get_operator()) {
  case scv_expression::EMPTY:
    _scv_constraint_error::internalError(
      "Generating internal BddRep for an EMPTY scv_expression");
    break;
  case scv_expression::INT_CONSTANT:
    return createExprRep(e.get_int_value(), e.get_bit_width());
  case scv_expression::UNSIGNED_CONSTANT:
    return createExprRep(e.get_unsigned_value(), e.get_bit_width());
  case scv_expression::SC_BIGINT_CONSTANT:
  case scv_expression::SC_BIGUINT_CONSTANT:
  case scv_expression::SC_BV_CONSTANT: {
    sc_bv_base val(e.get_bit_width());
    e.get_value(val);
    return createExprRep(val, e.get_bit_width());
  }
  case scv_expression::BOOLEAN_CONSTANT:
    return createExprRep(e.get_bool_value(), e.get_bit_width());
  case scv_expression::DOUBLE_CONSTANT:
    return createExprRep(e.get_double_value());
  case scv_expression::EXTENSION:
    return createExprRep(e.get_extension());
  case scv_expression::PLUS:
    {
      _scv_expr t1 = getExpressionRep(e.get_left());
      _scv_expr t2 = getExpressionRep(e.get_right());
      _scv_expr t3 = exprPlus(t1, t2);
      _scv_remove_if_bdd(&t1); 
      _scv_remove_if_bdd(&t2); 
      return t3;
    }
  case scv_expression::MINUS:
    {
      _scv_expr t1 = getExpressionRep(e.get_left());
      _scv_expr t2 = getExpressionRep(e.get_right());
      _scv_expr t3 = exprMinus(t1, t2);
      _scv_remove_if_bdd(&t1); 
      _scv_remove_if_bdd(&t2); 
      return t3;
    }
  case scv_expression::MULTIPLY:
    {
      _scv_expr t1 = getExpressionRep(e.get_left());
      _scv_expr t2 = getExpressionRep(e.get_right());
      _scv_expr t3 = exprMultiply(t1, t2);
      _scv_remove_if_bdd(&t1); 
      _scv_remove_if_bdd(&t2); 
      return t3;
    }
  case scv_expression::EQUAL:
    {
      _scv_expr t1 = getExpressionRep(e.get_left());
      _scv_expr t2 = getExpressionRep(e.get_right());
      _scv_expr t3 = exprEqual(t1, t2);
      _scv_remove_if_bdd(&t1); 
      _scv_remove_if_bdd(&t2); 
      return t3;
    }
  case scv_expression::NOT_EQUAL:
    {
      _scv_expr t1 = getExpressionRep(e.get_left());
      _scv_expr t2 = getExpressionRep(e.get_right());
      _scv_expr t3 = exprNEqual(t1, t2);
      _scv_remove_if_bdd(&t1); 
      _scv_remove_if_bdd(&t2); 
      return t3;
    }
  case scv_expression::GREATER_THAN:
    {
      _scv_expr t1 = getExpressionRep(e.get_left());
      _scv_expr t2 = getExpressionRep(e.get_right());
      _scv_expr t3 = exprGThan(t1, t2);
      _scv_remove_if_bdd(&t1); 
      _scv_remove_if_bdd(&t2); 
      return t3;
    }
  case scv_expression::LESS_THAN:
    {
      _scv_expr t1 = getExpressionRep(e.get_left());
      _scv_expr t2 = getExpressionRep(e.get_right());
      _scv_expr t3 = exprLThan(t1, t2);
      _scv_remove_if_bdd(&t1); 
      _scv_remove_if_bdd(&t2); 
      return t3;
    }
  case scv_expression::GREATER_OR_EQUAL:
    {
      _scv_expr t1 = getExpressionRep(e.get_left());
      _scv_expr t2 = getExpressionRep(e.get_right());
      _scv_expr t3 = exprGEq(t1, t2);
      _scv_remove_if_bdd(&t1); 
      _scv_remove_if_bdd(&t2); 
      return t3;
    }
  case scv_expression::LESS_OR_EQUAL:
    {
      _scv_expr t1 = getExpressionRep(e.get_left());
      _scv_expr t2 = getExpressionRep(e.get_right());
      _scv_expr t3 = exprLEq(t1, t2);
      _scv_remove_if_bdd(&t1); 
      _scv_remove_if_bdd(&t2); 
      return t3;
    }
  case scv_expression::AND:
    {
      _scv_expr t1 = getExpressionRep(e.get_left());
      _scv_expr t2 = getExpressionRep(e.get_right());
      _scv_expr t3 = exprAnd(t1, t2);
      _scv_remove_if_bdd(&t1); 
      _scv_remove_if_bdd(&t2); 
      return t3;
    }
  case scv_expression::OR:
    {
      _scv_expr t1 = getExpressionRep(e.get_left());
      _scv_expr t2 = getExpressionRep(e.get_right());
      _scv_expr t3 = exprOr(t1, t2);
      _scv_remove_if_bdd(&t1); 
      _scv_remove_if_bdd(&t2); 
      return t3;
    }
  case scv_expression::NOT:
    {
      _scv_expr t1 = getExpressionRep(e.get_left());
      _scv_expr t3 = exprNot(t1);
      _scv_remove_if_bdd(&t1); 
      return t3;
    }
  default: 
    _scv_constraint_error::internalError(
      "creating bdd for unknown operator type in getExpressionRep\n");
     break;
  }
  return *errorExprP;
}

_scv_expr _scv_constraint_manager::getExprRepZero(void)
{
  bddNodeT* internalZero = new bddNodeT;
  *internalZero = _mgr->bddZero();
  exprRepZero->setType(_scv_expr::BDD);
  exprRepZero->setBddNodeP(internalZero);
  return *exprRepZero;
}

_scv_expr _scv_constraint_manager::getExprRepOne(void)
{
  bddNodeT* internalOne = new bddNodeT;
  *internalOne = _mgr->bddOne();
  exprRepOne->setType(_scv_expr::BDD);
  exprRepOne->setBddNodeP(internalOne);
  return *exprRepOne;
}

_scv_expr _scv_constraint_manager::exprAnd(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem; 
  
  if (e1.isBdd() && e2.isBdd()) {
    bddNodeT *tmp = new bddNodeT;
    *tmp = (*(e1.getBddNodeP()) & *(e2.getBddNodeP())); 
    newElem.setType(_scv_expr::BDD);
    newElem.setBddNodeP(tmp);
  } else if (e1.isBddVector() && e2.isBddVector()) {
    newElem = _exprAnd(e1, e2);
  } else if (e1.isBdd() && e2.isBddVector()) {
    bddNodeT *tmp = new bddNodeT;
    _scv_expr tmpElem; 
    tmpElem = _exprAnd(e2, e2);
    *tmp = (*(e1.getBddNodeP()) & *tmpElem.getBddNodeP());
    newElem.setType(_scv_expr::BDD);
    newElem.setBddNodeP(tmp);
    _scv_remove_data(&tmpElem);
  } else if (e1.isBddVector() && e2.isBdd()) {
    bddNodeT *tmp = new bddNodeT;
    _scv_expr tmpElem; 
    tmpElem = _exprAnd(e1, e1);
    *tmp = (*(e2.getBddNodeP()) & *tmpElem.getBddNodeP());
    newElem.setType(_scv_expr::BDD);
    newElem.setBddNodeP(tmp);
    _scv_remove_data(&tmpElem);
  } else if (e1.isBddVector() && e2.isConstant()) {
    _scv_expr constElem = getConstantExprRep(e2);
    newElem = _exprAnd(e1, constElem);
    delete constElem.getBddVectorP();
  } else if (e1.isConstant() && e2.isBddVector()) {
    _scv_expr constElem = getConstantExprRep(e1);
    newElem = _exprAnd(constElem, e2);
    delete constElem.getBddVectorP();
  } else if (e1.isConstant() && e2.isConstant()) {
    _scv_expr constElem1 = getConstantExprRep(e1);
    _scv_expr constElem2 = getConstantExprRep(e2);
    newElem = _exprAnd(constElem1, constElem2);
    delete constElem1.getBddVectorP();
    delete constElem2.getBddVectorP();
  } else if (e1.isConstant() && e2.isBdd()) {
    newElem = e2;
    bddNodeT *tmp = new bddNodeT;
    *tmp = *e2.getBddNodeP();
    newElem.setBddNodeP(tmp);
  } else if (e1.isBdd() && e2.isConstant()) {
    newElem = e1;
    bddNodeT *tmp = new bddNodeT;
    *tmp = *e1.getBddNodeP();
    newElem.setBddNodeP(tmp);
  } else if (e1.isDouble() || e2.isDouble()) {
    _scv_constraint_error::ignoreDoubleConstraints();
    newElem = getExprRepOne();
  } else if (e1.isString() || e2.isString()) {
    _scv_constraint_error::ignoreDoubleConstraints();
    newElem = getExprRepOne();
  } else if (e1.isEmpty() || e2.isEmpty()) {
  } else {
    _scv_constraint_error::internalError(
                            "Trying exprAnd for unknown types\n");
  }
  return newElem;
}

_scv_expr _scv_constraint_manager::_exprAnd(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem;
  bddNodeT *tmp = new bddNodeT;
  bddNodeT tmpE1; 
  bddNodeT tmpE2;

  bddVectorT *e1Vec = e1.getBddVectorP();
  bddVectorT *e2Vec = e2.getBddVectorP();
  int e1size = e1.getVecSize();
  int e2size = e2.getVecSize();

  bddVectorT& e1VecR = *e1Vec;
  bddVectorT& e2VecR = *e2Vec;

  tmpE1 = _mgr->bddZero();
  tmpE2 = _mgr->bddZero();

  for (int i=0; i<e1size; ++i) {
    tmpE1 = tmpE1 | e1VecR[i];
  } 
  for (int i=0; i<e2size; ++i) {
    tmpE2 = tmpE2 | e2VecR[i];
  } 
  *tmp = (tmpE1 & tmpE2); 
  newElem.setType(_scv_expr::BDD);
  newElem.setBddNodeP(tmp);
  return newElem;
}

_scv_expr _scv_constraint_manager::exprOr(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem;

  if (e1.isBdd() && e2.isBdd()) {
    bddNodeT *tmp = new bddNodeT;
    *tmp = (*(e1.getBddNodeP()) | *(e2.getBddNodeP())); 
    newElem.setType(_scv_expr::BDD);
    newElem.setBddNodeP(tmp);
  } else if (e1.isBddVector() && e2.isBddVector()) {
    newElem = _exprOr(e1, e2);
  } else if (e1.isBdd() && e2.isBddVector()) {
    bddNodeT *tmp = new bddNodeT;
    _scv_expr tmpElem; 
    tmpElem = _exprOr(e2, e2);
    *tmp = (*(e1.getBddNodeP()) | *tmpElem.getBddNodeP());
    newElem.setType(_scv_expr::BDD);
    newElem.setBddNodeP(tmp);
    _scv_remove_data(&tmpElem);
  } else if (e1.isBddVector() && e2.isBdd()) {
    bddNodeT *tmp = new bddNodeT;
    _scv_expr tmpElem; 
    tmpElem = _exprOr(e1, e1);
    *tmp = (*(e2.getBddNodeP()) | *tmpElem.getBddNodeP());
    newElem.setType(_scv_expr::BDD);
    newElem.setBddNodeP(tmp);
    _scv_remove_data(&tmpElem);
  } else if (e1.isBddVector() && e2.isConstant()) {
    _scv_expr constElem = getConstantExprRep(e2);  
    newElem = _exprOr(e1, constElem);
    delete constElem.getBddVectorP();
  } else if (e1.isConstant() && e2.isBddVector()) {
    _scv_expr constElem = getConstantExprRep(e1);  
    newElem = _exprOr(constElem, e2);
    delete constElem.getBddVectorP();
  } else if (e1.isConstant() && e2.isConstant()) {
    _scv_expr constElem1 = getConstantExprRep(e1);  
    _scv_expr constElem2 = getConstantExprRep(e2);  
    newElem = _exprOr(constElem1, constElem2);
    delete constElem1.getBddVectorP();
    delete constElem2.getBddVectorP();
  } else if (e1.isDouble() || e2.isDouble()) {
    _scv_constraint_error::ignoreDoubleConstraints();
    newElem = getExprRepOne();
  } else if (e1.isString() || e2.isString()) {
    _scv_constraint_error::ignoreStringConstraints();
    newElem = getExprRepOne();
  } else if (e1.isEmpty() || e2.isEmpty()) {
  } else {
    _scv_constraint_error::internalError(
      "Trying exprOr for unknown types\n");
  }
  return newElem;
}

_scv_expr _scv_constraint_manager::_exprOr(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem;
  bddNodeT* tmp = new bddNodeT;
  bddNodeT tmpE1;
  bddNodeT tmpE2;

  int e1size = e1.getVecSize();
  int e2size = e2.getVecSize();

  bddVectorT *e1Vec = e1.getBddVectorP();
  bddVectorT *e2Vec = e2.getBddVectorP();
 
  bddVectorT& e1VecR = *e1Vec;
  bddVectorT& e2VecR = *e2Vec;

  tmpE1 = _mgr->bddZero();
  tmpE2 = _mgr->bddZero(); 
 
  for (int i=0; i<e1size; ++i) {
    tmpE1 = tmpE1 | e1VecR[i];
  } 
  for (int i=0; i<e2size; ++i) {
    tmpE2 = tmpE2 | e2VecR[i];
  } 

  *tmp = tmpE1 | tmpE2; 
  newElem.setType(_scv_expr::BDD);
  newElem.setBddNodeP(tmp);
  return newElem;
}

_scv_expr _scv_constraint_manager::exprNot(const _scv_expr& e)
{
  _scv_expr newElem;

  if (e.isBdd()) {
    bddNodeT *tmp = new bddNodeT;
    *tmp = !(*(e.getBddNodeP()));
    newElem.setType(_scv_expr::BDD);
    newElem.setBddNodeP(tmp);
  } else if (e.isBddVector()) {
    newElem = _exprNot(e);
  } else if (e.isConstant()) {
    _scv_expr constElem = getConstantExprRep(e);
    newElem = _exprNot(constElem);
    delete constElem.getBddVectorP();
  } else if (e.isDouble()) {
    _scv_constraint_error::ignoreDoubleConstraints();
    newElem = getExprRepOne();
  } else if (e.isString()) {
    _scv_constraint_error::ignoreStringConstraints();
    newElem = getExprRepOne();
  } else if (e.isEmpty()) {
  } else {
    _scv_constraint_error::internalError(
      "Trying exprNot for unknown types\n");
  }
  return newElem;
}

_scv_expr _scv_constraint_manager::_exprNot(const _scv_expr& e)
{
  _scv_expr newElem;
  int esize = e.getVecSize();

  bddVectorT *eVec = e.getBddVectorP();
  bddVectorT& eVecR = *eVec;
  bddVectorT *resVec = new bddVectorT(esize);
  bddVectorT& resVecR = *resVec;

  for (int i=0; i < esize; ++i) {
    resVecR[i] = !eVecR[i];
  }

  newElem.setType(e.get_type());
  newElem.setVecSize(esize);
  newElem.setBddVectorP(resVec);
  return newElem;
}

_scv_expr _scv_constraint_manager::exprPlus(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem;
  
  if (e1.isBdd() && e2.isBdd()) {
    bddNodeT *tmp = new bddNodeT();
    bddNodeT& e1bdd = *(e1.getBddNodeP());
    bddNodeT& e2bdd = *(e2.getBddNodeP());
    *tmp = e1bdd + e2bdd;
    newElem.setType(_scv_expr::BDD);
    newElem.setBddNodeP(tmp);
  } else if (e1.isBddVector()&& e2.isBddVector()) {
    newElem = _exprPlus(e1, e2);
  } else if (e1.isBddVector() && e2.isConstant()) {
    _scv_expr constElem = getConstantExprRep(e2, e1.getVecSize());
    newElem = _exprPlus(e1, constElem);
    delete constElem.getBddVectorP();
  } else if (e1.isConstant() && e2.isBddVector()) {
    _scv_expr constElem = getConstantExprRep(e1, e2.getVecSize());
    newElem = _exprPlus(constElem, e2); 
    delete constElem.getBddVectorP();
  } else if (e1.isConstant() && e2.isConstant()) {
    _scv_expr constElem1 = getConstantExprRep(e1);
    _scv_expr constElem2 = getConstantExprRep(e2);
    newElem = _exprPlus(constElem1, constElem2);
    delete constElem1.getBddVectorP();
    delete constElem2.getBddVectorP();
  } else if (e1.isDouble() || e2.isDouble()) {
    _scv_constraint_error::ignoreDoubleConstraints();
    newElem = getExprRepOne();
  } else if (e1.isString() || e2.isString()) {
    _scv_constraint_error::ignoreStringConstraints();
    newElem = getExprRepOne();
  } else if (e1.isEmpty() || e2.isEmpty()) {
  } else {
    _scv_constraint_error::internalError(
      "Trying exprPlus for unknown types\n");
  }
  return newElem;
}

_scv_expr _scv_constraint_manager::_exprPlus(const _scv_expr& e1, const _scv_expr& e2)
{
  _scv_expr newElem;

  int e1size = e1.getVecSize();
  int e2size = e2.getVecSize();

  int minBits = (e1size <= e2size) ? e1size : e2size;
  int maxBits = (e1size >= e2size) ? e1size : e2size;

  int resSize;
 
  resSize = maxBits+1;

  bddVectorT *vptr = new bddVectorT(resSize);
  bddVectorT& vecR = *vptr;

  bddVectorT *e1vec = e1.getBddVectorP();
  bddVectorT& e1vecR = *e1vec;
  bddVectorT *e2vec = e2.getBddVectorP();
  bddVectorT& e2vecR = *e2vec;

  bddNodeT carry = _mgr->bddZero();

  for (int i=0; i < minBits; ++i) {
    vecR[i] = carry ^ e1vecR[i] ^ e2vecR[i];
    carry = ((e1vecR[i] & e2vecR[i]) | (carry & (e1vecR[i] | e2vecR[i])));
  }

  for (int i=minBits; i <= e1size-1; ++i) {
    vecR[i] = carry ^ e1vecR[i] ^ _mgr->bddZero();
    carry = ( carry & e1vecR[i] );
  }
  for (int i=minBits; i <= e2size-1; ++i) {
    vecR[i] = carry ^ e2vecR[i] ^ _mgr->bddZero();
    carry = (carry & e2vecR[i]);
  }
  vecR[resSize-1] = carry;

  if (e1.isSigned() || e2.isSigned()) {
    newElem.setType(_scv_expr::BDDVECTOR_SIGNED);
  } else {
    newElem.setType(_scv_expr::BDDVECTOR);
  }

  newElem.setVecSize(resSize);
  newElem.setBddVectorP(vptr);

  return newElem;
}

_scv_expr _scv_constraint_manager::twosCompliment(const _scv_expr& e)
{
  _scv_expr newElem;

  if (e.isBddVector()) {
    _scv_expr tmp = exprNot(e);
    _scv_expr tmpConstant = createExprRep((unsigned long long)1, numBitsSigned);

    _scv_expr t1 = exprPlus(tmp, tmpConstant);
    t1.setVecSize(tmp.getVecSize());
    _scv_remove_data(&tmp);
    _scv_remove_data(&tmpConstant);
    return t1; 
  } else {
    _scv_constraint_error::internalError(
                            "Trying twoscompliment for unknown types\n");
  }
  return newElem; 
}

_scv_expr _scv_constraint_manager::exprMinus(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem;

  if (e1.isBdd() && e2.isBdd()) {
    bddNodeT *tmp = new bddNodeT;
    *tmp = (*e1.getBddNodeP() - *e2.getBddNodeP());
    newElem.setType(_scv_expr::BDD);
    newElem.setBddNodeP(tmp);
  } else if (e1.isBddVector() && e2.isBddVector()) {
    newElem = _exprMinus(e1, e2);
  } else if (e1.isBddVector() && e2.isConstant()) {
    _scv_expr constElem = getConstantExprRep(e2, e1.getVecSize());
    newElem = _exprMinus(e1, constElem);
    delete constElem.getBddVectorP();
  } else if (e1.isConstant() && e2.isBddVector()) {
    _scv_expr constElem = getConstantExprRep(e1, e2.getVecSize());
    newElem = _exprMinus(constElem, e2);
    delete constElem.getBddVectorP();
  } else if (e1.isConstant() && e2.isConstant()) {
    _scv_expr constElem1 = getConstantExprRep(e1);
    _scv_expr constElem2 = getConstantExprRep(e2);
    newElem = _exprMinus(constElem1, constElem2);
    delete constElem1.getBddVectorP();
    delete constElem2.getBddVectorP();
  } else if (e1.isDouble() || e2.isDouble()) {
    _scv_constraint_error::ignoreDoubleConstraints();
    newElem = getExprRepOne();
  } else if (e1.isString() || e2.isString()) {
    _scv_constraint_error::ignoreStringConstraints();
    newElem = getExprRepOne();
  } else if (e1.isEmpty() || e2.isEmpty()) {
  } else {
    _scv_constraint_error::internalError(
                            "Trying exprMinus for unknown types\n");
  }
  return newElem;
}

_scv_expr _scv_constraint_manager::_exprMinus(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem;

  _scv_expr twoscompliment = twosCompliment(e2);
  _scv_expr res = exprPlus(e1, twoscompliment);

  _scv_remove_data(&twoscompliment);
  if (e1.isSigned() || e1.isSigned()) {
    newElem.setType(_scv_expr::BDDVECTOR_SIGNED);
  } else {
    newElem.setType(_scv_expr::BDDVECTOR);
  }

  int e1size = e1.getVecSize();
  int e2size = e2.getVecSize();

  int resSize = (e1size >= e2size) ? e1size : e2size;

  newElem.setVecSize(resSize);
  newElem.setBddVectorP(res.getBddVectorP());
  return newElem;
}

_scv_expr _scv_constraint_manager::exprMultiply(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem;

  if (e1.isBdd() && e2.isBdd()) {
    bddNodeT *tmp = new bddNodeT;
    *tmp = (*(e1.getBddNodeP()) * (*(e2.getBddNodeP())));
    newElem.setType(_scv_expr::BDD);
    newElem.setBddNodeP(tmp);
  } else if (e1.isBddVector() && e2.isBddVector()) {
    newElem = _exprMultiply(e1, e2);
  } else if (e1.isBddVector() && e2.isConstant()) {
    _scv_expr constElem = getConstantExprRep(e2, e1.getVecSize());
    newElem = _exprMultiply(e1, constElem);
    delete constElem.getBddVectorP();
  } else if (e1.isConstant() && e2.isBddVector()) { 
    _scv_expr constElem = getConstantExprRep(e1, e2.getVecSize());
    newElem = _exprMultiply(constElem, e2);
    delete constElem.getBddVectorP();
  } else if (e1.isConstant() && e2.isConstant()) {
    _scv_expr constElem1 = getConstantExprRep(e1);
    _scv_expr constElem2 = getConstantExprRep(e2);
    newElem = _exprMultiply(constElem1, constElem2);
    delete constElem1.getBddVectorP();
    delete constElem2.getBddVectorP();
  } else if (e1.isDouble() || e2.isDouble()) {
    _scv_constraint_error::ignoreDoubleConstraints();
    newElem = getExprRepOne();
  } else if (e1.isString() || e2.isString()) {
    _scv_constraint_error::ignoreStringConstraints();
    newElem = getExprRepOne();
  } else if (e1.isEmpty() || e2.isEmpty()) {
  } else {
    _scv_constraint_error::internalError(
                            "Trying exprMultiply for unknown types\n");
  }
  return newElem;
}

_scv_expr _scv_constraint_manager::_exprMultiply(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem;

  int n = e1.getVecSize(); 
  int m = e2.getVecSize(); 

  bddVectorT *vptr = new bddVectorT(n+m);
  bddVectorT& vecR = *vptr; 
  bddVectorT *e1vec = e1.getBddVectorP();
  bddVectorT *e2vec = e2.getBddVectorP();
  bddVectorT& e1vecR = *e1vec; 
  bddVectorT& e2vecR = *e2vec; 

  if (e1.isSigned() || e2.isSigned()) {
    newElem.setType(_scv_expr::BDDVECTOR_SIGNED);
  } else {
    newElem.setType(_scv_expr::BDDVECTOR);
  }
  newElem.setVecSize(n+m);
  newElem.setBddVectorP(vptr);

  for(int i=0; i<n+m;i++) {
    vecR[i] = _mgr->bddZero();
  }

  bddVectorT *pp = new bddVectorT(n+m);
  bddVectorT& ppR = *pp;
  _scv_expr *pres = new _scv_expr;
  if (e1.isSigned() || e2.isSigned()) {
    pres->setType(_scv_expr::BDDVECTOR_SIGNED);
  } else {
    pres->setType(_scv_expr::BDDVECTOR);
  }
  pres->setVecSize(n+m);

  for (int i=0; i<m; ++i) {
    pres->setBddVectorP(pp);
    for (int k=0; k<n+m; ++k) { 
      if (k<i) {
        ppR[k] = _mgr->bddZero();
      } else if (k>=n+i) {
        ppR[k] = _mgr->bddZero();
      } else {
        ppR[k] = e1vecR[k-i] & e2vecR[i];
      }
    }
    _scv_expr tmp = newElem;
    newElem = (exprPlus(*pres, newElem));
    _scv_remove_data(&tmp);
  }

  newElem.setVecSize(n+m);
  _scv_remove(pres);
  return newElem;
}

_scv_expr _scv_constraint_manager::exprEqual(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem;

  if (e1.isBdd() && e2.isBdd()) { 
    bddNodeT *tmp = new bddNodeT();
    bddNodeT e1Bdd = *(e1.getBddNodeP());
    bddNodeT e2Bdd = *(e2.getBddNodeP());
    *tmp = (e1Bdd.Xnor(e2Bdd));
    newElem.setType(_scv_expr::BDD);
    newElem.setBddNodeP(tmp);
  } else if (e1.isBddVector() && e2.isBddVector()) {
    newElem = _exprEqual(e1, e2);
  } else if (e1.isBddVector() && e2.isConstant()) { 
    _scv_expr constElem = getConstantExprRep(e2, e1.getVecSize());
    newElem = _exprEqual(e1, constElem);
    delete constElem.getBddVectorP();
  } else if (e1.isConstant() && e2.isBddVector()) {
    _scv_expr constElem = getConstantExprRep(e1, e2.getVecSize());
    newElem = _exprEqual(constElem, e2);
    delete constElem.getBddVectorP();
  } else if (e1.isConstant() && e2.isConstant()) {
    _scv_expr constElem1 = getConstantExprRep(e1);
    _scv_expr constElem2 = getConstantExprRep(e2);
    newElem = _exprEqual(constElem1, constElem2);
    delete constElem1.getBddVectorP();
    delete constElem2.getBddVectorP();
  } else if (e1.isRecord() && e2.isRecord()) {
    scv_extensions_if* ext1 = e1.getExtensionP();
    scv_extensions_if* ext2 = e2.getExtensionP();

    int size1 = ext1->get_num_fields();
    int size2 = ext2->get_num_fields();

    _scv_expr fexpr;
    _scv_expr res;
 
    res = getExprRepOne();
 
    _scv_expr tmp;

    assert(size1 == size2);

    for (int j=0; j < size1; j++) {
      scv_extensions_if* ef1 = ext1->get_field(j);
      scv_extensions_if* ef2 = ext2->get_field(j);

      tmp = res;
      fexpr = exprEqual(createExprRep(ef1), createExprRep(ef2));
      res = exprAnd(res, fexpr);
      _scv_remove_data(&tmp);
    }
    newElem = res;
  } else if (e1.isArray() && e2.isArray()) {
    scv_extensions_if* ext1 = e1.getExtensionP();
    scv_extensions_if* ext2 = e2.getExtensionP();

    int size1 = ext1->get_array_size();
    int size2 = ext2->get_array_size();

    _scv_expr fexpr;
    _scv_expr res;
 
    res = getExprRepOne();
 
    _scv_expr tmp;

    assert(size1 == size2);
    for (int j=0; j < size1; j++) {
      scv_extensions_if* ef1 = ext1->get_array_elt(j);
      scv_extensions_if* ef2 = ext2->get_array_elt(j);

      tmp = res;
      fexpr = exprEqual(createExprRep(ef1), createExprRep(ef2));
      res = exprAnd(res, fexpr);
      _scv_remove_data(&tmp);
    }
    newElem = res;
  } else if (e1.isDouble() || e2.isDouble()) {
    _scv_constraint_error::ignoreDoubleConstraints();
    newElem = getExprRepOne();
  } else if (e1.isString() || e2.isString()) {
    _scv_constraint_error::ignoreStringConstraints();
    newElem = getExprRepOne();
  } else if (e1.isEmpty() || e2.isEmpty()) {
  } else {
    _scv_constraint_error::internalError(
                            "Trying exprEqual for unknown types\n");
  }
  return newElem;
}

_scv_expr _scv_constraint_manager::_exprEqual(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem;
  bddNodeT *tmp = new bddNodeT();
  int e1size = e1.getVecSize();
  int e2size = e2.getVecSize();
  int minBits = (e1size < e2size) ? e1size : e2size;
  bddVectorT* e1Vec = e1.getBddVectorP();
  bddVectorT* e2Vec = e2.getBddVectorP();
  bddVectorT& e1VecR = *e1Vec; 
  bddVectorT& e2VecR = *e2Vec;
  *tmp = _mgr->bddOne(); 
  for (int i=0; i<minBits; ++i) {
    *tmp = *tmp & (e1VecR[i].Xnor(e2VecR[i]));
  }
  for (int i=minBits; i<e1size; ++i) {
    *tmp = *tmp & (!e1VecR[i]);
  }
  for (int i=minBits; i<e2size; ++i) {
    *tmp = *tmp & (!e2VecR[i]);
  }
  newElem.setType(_scv_expr::BDD);
  newElem.setBddNodeP(tmp);
  return newElem;
}

_scv_expr _scv_constraint_manager::exprNEqual(const _scv_expr& e1, const _scv_expr& e2) 
{
  bddNodeT* tmp = new bddNodeT;
  _scv_expr newElem; 
  _scv_expr t1 = exprEqual(e1, e2);
  if (t1.isEmpty()) {
    return newElem;
  } else {
    *tmp = !(*(t1.getBddNodeP()));
  }
  newElem.setType(_scv_expr::BDD);
  newElem.setBddNodeP(tmp);
  _scv_remove_data(&t1);
  return newElem;
}

_scv_expr _scv_constraint_manager::exprGThan(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem;
  bddNodeT* result = new bddNodeT();
  _scv_expr lthan = exprLThan(e2,e1); 
  if (lthan.isEmpty()) {
    return newElem;
  } 
  bddNodeT& tmp = *(lthan.getBddNodeP());
  *result = tmp;
  newElem.setType(_scv_expr::BDD);
  newElem.setBddNodeP(result);
  _scv_remove_data(&lthan);
  return newElem;
}

_scv_expr _scv_constraint_manager::exprLEq(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem;

  if (e1.isBdd() && e2.isBdd()) {
    bddNodeT* tmp = new bddNodeT();
    bddNodeT* e1bdd = e1.getBddNodeP();
    bddNodeT* e2bdd = e2.getBddNodeP();
    *tmp = (!(*e1bdd) & *e2bdd) | (*e1bdd).Xnor(*e2bdd);
    newElem.setType(_scv_expr::BDD);
    newElem.setBddNodeP(tmp);
  } else if (e1.isBddVector() && e2.isBddVector()) {
    newElem = _exprLEq(e1, e2);
  } else if (e1.isBddVector() && e2.isConstant()) {
    _scv_expr constElem = getConstantExprRep(e2, e1.getVecSize());
    newElem = _exprLEq(e1, constElem);
    delete constElem.getBddVectorP();
  } else if (e1.isConstant() && e2.isBddVector()) {
    _scv_expr constElem = getConstantExprRep(e1, e2.getVecSize());
    newElem = _exprLEq(constElem, e2);
    delete constElem.getBddVectorP();
  } else if (e1.isConstant() && e2.isConstant()) {
    _scv_expr constElem1 = getConstantExprRep(e1);
    _scv_expr constElem2 = getConstantExprRep(e2);
    newElem = _exprLEq(constElem1, constElem2);
    delete constElem1.getBddVectorP();
    delete constElem2.getBddVectorP();
  } else if (e1.isDouble() || e2.isDouble()) {
    _scv_constraint_error::ignoreDoubleConstraints();
    newElem = getExprRepOne();
  } else if (e1.isString() || e2.isString()) {
    _scv_constraint_error::ignoreStringConstraints();
    newElem = getExprRepOne();
  } else if (e1.isEmpty() || e2.isEmpty()) {
  } else {
    _scv_constraint_error::internalError(
                            "unknown types\n");
  }
  return newElem;
}

_scv_expr _scv_constraint_manager::_exprLEq(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem;
  bddNodeT *tmp = new bddNodeT;

  *tmp = _mgr->bddOne();

  int e1size = e1.getVecSize();
  int e2size = e2.getVecSize();

  int numMagBits = 0;
  int maxMagBits = 0;

  int minBits = (e1size <= e2size) ? e1size : e2size;

  if (e1.isSigned() || e2.isSigned()) { 
    numMagBits = minBits - 1;
    maxMagBits = e1size-2;
  } else {
    numMagBits = minBits;
    maxMagBits = e1size-1;
  }

  bddVectorT *e1vec = e1.getBddVectorP();
  bddVectorT& e1vecR = *e1vec;
  bddVectorT *e2vec = e2.getBddVectorP();
  bddVectorT& e2vecR = *e2vec;

  for(int i=0; i<numMagBits; i++) {
    *tmp = (*tmp & (e1vecR[i].Xnor(e2vecR[i]))) | (!e1vecR[i] & e2vecR[i]);
  }

  if (e1.isSigned() || e2.isSigned()) {
    int e1Msb = e1size - 1;
    int e2Msb = e2size - 1;

    *tmp = ( ((e1vecR[e1Msb] & e2vecR[e2Msb]) & (*tmp) ) | (!e1vecR[e1Msb] & !e2vecR[e2Msb] & (*tmp)) | (e1vecR[e1Msb] & !e2vecR[e2Msb])  );
  } 

  for(int i=maxMagBits; i>=minBits; i--) {
    *tmp = (*tmp & (!e1vecR[i]));
  }

  newElem.setType(_scv_expr::BDD);
  newElem.setBddNodeP(tmp);
  return newElem;
}

_scv_expr _scv_constraint_manager::exprGEq(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem;
  bddNodeT* tmp = new bddNodeT();
  _scv_expr ex1 = exprGThan(e1,e2); 
  _scv_expr ex2 = exprEqual(e1,e2); 
  if (ex1.isEmpty() || ex2.isEmpty()) {
    return newElem;
  }
  bddNodeT* t1 = ex1.getBddNodeP();
  bddNodeT* t2 = ex2.getBddNodeP(); 
  *tmp = (*t1 | *t2);
  newElem.setType(_scv_expr::BDD);
  newElem.setBddNodeP(tmp);
  _scv_remove_data(&ex1);
  _scv_remove_data(&ex2);
  return newElem;
}

_scv_expr _scv_constraint_manager::exprLThan(const _scv_expr& e1, const _scv_expr& e2) 
{
  _scv_expr newElem;
  bddNodeT* tmp = new bddNodeT();
  _scv_expr ex1 = exprLEq(e1,e2);
  _scv_expr ex2 = exprNEqual(e1,e2);
  if (ex1.isEmpty() || ex2.isEmpty()) {
    return newElem;
  } 
  bddNodeT* t1 = ex1.getBddNodeP();
  bddNodeT* t2 = ex2.getBddNodeP();
  *tmp = (*t1 & *t2);
  newElem.setType(_scv_expr::BDD);
  newElem.setBddNodeP(tmp);

  _scv_remove_data(&ex1);
  _scv_remove_data(&ex2);
  return newElem;
}

_scv_expr _scv_constraint_manager::getBasicEnumConstraint(void)
{
  _scv_expr ret_expr;
  
  ret_expr = getExprRepOne();

  if (enumVarList->size() == 0)  {
    return ret_expr;
  } else {
    _scv_expr tmp_expr;
    _scv_expr var_expr;
    _scv_expr tmp;
    std::list<scv_extensions_if*>::iterator i;   
    for (i = enumVarList->begin(); i != enumVarList->end(); i++) {
      scv_extensions_if* enum_var = (*i);
      int bitwidth = enum_var->get_bitwidth();
     
      std::list<int> ilist;
      std::list<const char *> slist; 
      std::list<int>::iterator iter;

      enum_var->get_enum_details(slist, ilist);
      var_expr = getExprRepZero(); 
      for (iter = ilist.begin(); iter != ilist.end(); iter++) {
        long long val = (*iter);
        tmp = var_expr;
        tmp_expr = exprEqual(createExprRep(enum_var), createExprRep(val, bitwidth));
        var_expr = exprOr(var_expr, tmp_expr); 
        _scv_remove_data(&tmp);
        _scv_remove_data(&tmp_expr);
      }
      tmp = ret_expr;
      ret_expr = exprAnd(ret_expr, var_expr);
      _scv_remove_data(&var_expr);
      _scv_remove_data(&tmp);
    }
    return ret_expr;
  }
}

  
_scv_expr _scv_constraint_manager::createExprRep(long long v, int bit_width)
{
  _scv_expr newElem;
  newElem.setType(_scv_expr::INT);
  newElem.setIntValue(v);
  newElem.setBitWidth(bit_width);
  return newElem;
}

_scv_expr _scv_constraint_manager::createExprRep(unsigned long long v, int bit_width)
{
  _scv_expr newElem; 
  newElem.setType(_scv_expr::UNSIGNED);
  newElem.setUnsignedValue(v);
  newElem.setBitWidth(bit_width);
  return newElem;
}

_scv_expr _scv_constraint_manager::createExprRep(sc_bv_base& v, int bit_width)
{
  _scv_expr newElem; 
  newElem.setType(_scv_expr::SC_BV_BASE);
  newElem.setBigValue(v, bit_width);
  newElem.setBitWidth(bit_width);
  return newElem;
}

_scv_expr _scv_constraint_manager::createExprRep(bool v, int bit_width)
{
  _scv_expr newElem; 
  newElem.setType(_scv_expr::BOOLEAN);
  newElem.setBooleanValue(v);
  newElem.setBitWidth(bit_width);
  return newElem;
}

_scv_expr _scv_constraint_manager::createExprRep(double v)
{
  _scv_expr newElem;
  newElem.setType(_scv_expr::DOUBLE);
  newElem.setDoubleValue(v);
  return newElem;
}

_scv_expr _scv_constraint_manager::createExprRep(const char *v) 
{
  _scv_expr newElem;
  newElem.setType(_scv_expr::STRING);
  newElem.setStringValue(v);
  return newElem;
}

_scv_expr _scv_constraint_manager::createBddVec(int msize)
{

  _scv_expr newElem;

  bddVectorT *vptr = new bddVectorT(msize);
  bddVectorT& vptrR = *vptr;

  for ( int i= 0; i < msize; ++i) {
    vptrR[i] = _mgr->bddVar((maxvar*i)+nthvar);
    if (verboseLevel > 3) {
      scv_out << "i : " << vptrR[i].getNode() << endl;
    }
  }
  newElem.setType(_scv_expr::BDDVECTOR);
  newElem.setVecSize(msize);
  newElem.setBddVectorP(vptr);
  newElem.setIsVar();
  nthvar++;

  return newElem;
}

void _scv_constraint_manager::checkExprRep(scv_extensions_if* s)
{
  if (countExtHash->getValue(s)) {
    return;
  } else {
    maxvar++;
    if (maxNumBits < s->get_bitwidth()) {
      maxNumBits = s->get_bitwidth();
    }
    countExtHash->insert(s, 1);
    s->set_constraint(true);
  }
}

_scv_expr _scv_constraint_manager::createExprRep(scv_extensions_if* s)
{
  _scv_expr *assoc = NULL;

  int msize = getSizeOfBddVec(s);

  if ((assoc = getExprRepP(s))) {
    return *assoc;
  } else {
    _scv_expr *newElem = new _scv_expr; 
    newElem->setType(_scv_expr::BDDVECTOR);
    newElem->setIsVar();
    switch(s->get_type()) {
    case scv_extensions_if::INTEGER:
      {
        *newElem = createBddVec(msize);
        newElem->setType(_scv_expr::BDDVECTOR_SIGNED);
        setExprRep(s, *newElem);
        break;
      } 
    case scv_extensions_if::ENUMERATION:
      {
        *newElem = createBddVec(msize);
        newElem->setType(_scv_expr::BDDVECTOR_SIGNED);
        setExprRep(s, *newElem);
        enumVarList->push_back(s);
        break;
      } 
    case scv_extensions_if::UNSIGNED:
    case scv_extensions_if::BIT_VECTOR:
    case scv_extensions_if::LOGIC_VECTOR: 
    case scv_extensions_if::BOOLEAN: 
      {
        *newElem = createBddVec(msize);
        newElem->setType(_scv_expr::BDDVECTOR);
        setExprRep(s, *newElem);
        break;
      } 
    case scv_extensions_if::FLOATING_POINT_NUMBER:
      {
        newElem->setType(_scv_expr::DOUBLE_VAR);
        break;
      }
    case scv_extensions_if::RECORD:
      {
        int size = s->get_num_fields();

        for (int j=0; j < size; j++) {
          scv_extensions_if* ef = s->get_field(j);
          createExprRep(ef);
        }
        newElem->setType(_scv_expr::RECORD);
        newElem->setExtensionP(s);
        setExprRep(s, *newElem);
        break;
      }
    case scv_extensions_if::ARRAY:
      {
        int size = s->get_array_size();

        for (int j=0; j < size; j++) {
          scv_extensions_if* ef = s->get_array_elt(j);
          createExprRep(ef);
        }
        newElem->setType(_scv_expr::ARRAY);
        newElem->setExtensionP(s);
        setExprRep(s, *newElem);
        break;
      }
    default: 
     /*
      _scv_constraint_error::internalError(
         "unknown variable type in createExprRep\n");
    */
      break;
    } 
    return *newElem;
  }
}

void _scv_constraint_manager::setExprRep(scv_extensions_if* s, _scv_expr& e) 
{
  _smartDataRecordT * sDataP = NULL;

  if (verboseLevel > 3) {
    scv_out << "Setting Expresssion Rep:  " << s << " BitWidth: " << s->get_bitwidth() << endl;
  }
  if (!extHash->get(s, sDataP)) {
    extHash->update(s, _smartDataRecordT());
    extHash->get(s, sDataP);
  }
  sDataP->smartDataBddVector = &e;
  if (e.isBddVector()) {
    bddVectorT* vec = e.getBddVectorP();
    bddVectorT& vecR = *vec;
    sDataP->startIndex = vecR[0].getNode()->index;
    sDataP->numvar = maxvar;
  } else {
    sDataP->startIndex = 0;
  }
}

_scv_expr* _scv_constraint_manager::getExprRepP(scv_extensions_if* s) const
{
  _smartDataRecordT * sDataP = NULL;
  if (!extHash->get(s, sDataP)) {
    return NULL;
  }
  return sDataP->smartDataBddVector;
}

int _scv_constraint_manager::getVecSize(scv_extensions_if* s) const
{
  _scv_expr* expr = getExprRepP(s);
  int size = expr->getVecSize();
  return size;
}

unsigned _scv_constraint_manager::getStartIndex(scv_extensions_if* s) const
{
  _smartDataRecordT * sDataP = NULL;
  extHash->get(s, sDataP);
  return sDataP->startIndex;
}

unsigned _scv_constraint_manager::getEndIndex(scv_extensions_if* s) const
{
  _scv_expr* expr = getExprRepP(s);
  bddVectorT* vec = expr->getBddVectorP();
  int size = expr->getVecSize();
  bddVectorT& vecR = *vec;
  return (vecR[size-1].getNode()->index);
}

int _scv_constraint_manager::getSizeOfBddVec(_scv_expr& e) const
{
  return e.getBitWidth();
}

int _scv_constraint_manager::getSizeOfBddVec(scv_extensions_if* s) const
{
  if (s->is_bool()) {
    return 1;
  } else {
    return s->get_bitwidth();
  }
}

void _scv_constraint_manager::getVectorFromBdd(ddNodeT *N) 
{
  DdNode *node, *Nt, *Ne;

  node = N;
  if (!node) { 
    _scv_constraint_error::internalError(
      "File a bug report.\n");
    return;
  }
  while (node != oneNode && node != zeroNode) { 
    Nt = Cudd_T(node); Ne = Cudd_E(node);
    ValueIndexSizeType nodeIndex = Cudd_Regular(node)->index;
    if (nodeIndex >= valueIndex.size()) {
      valueIndex.resize(nodeIndex + 1);
    }
    if (Cudd_IsComplement(node)) { 
      Nt = Cudd_Not(Nt);
      Ne = Cudd_Not(Ne);
    }
    if (Ne == zeroNode) {
      valueIndex[nodeIndex] = 1;
      node = Nt;
    } else if (Nt == zeroNode) {
      valueIndex[nodeIndex] = 0;
      node = Ne;
    } else {
      int branchElse;
      unsigned pElseBranch;

      if (1) {
        if (nodeHashP->get(node, eProb)) {
          pElseBranch = (unsigned)*eProb;
        } else {
          pElseBranch = ROUNDTO_BASE;
        }
        update16BitValue();
        branchElse = (randomnext % ROUNDTO <= pElseBranch); 
      } else {
        updateRandomNext();
        branchElse = ((randomnext % 2) == 1); 
      }
      if (branchElse) {
        valueIndex[nodeIndex] = 0;
        node = Ne;
      } else {
        valueIndex[nodeIndex] = 1;
        node = Nt;
      }
    }
  }
  return;
}

void _scv_constraint_manager::getVector(_scv_expr e, unsigned* v) const
{
  int i;
  int msize = getSizeOfBddVec(e);

  switch(e.get_type()) {
    case _scv_expr::UNSIGNED:  {
      unsigned long long value = e.getUnsignedValue();
      unsigned long long tmp = 0x0001;
      unsigned long long res = 0;

      for (i=0; i < msize; i++) {
        res = tmp & value ;
        v[i] = (unsigned) res; 
        value = value >> 1;
      }
      break;
    }
    case _scv_expr::INT: {
      long long value = e.getIntValue();
      long long tmp = 0x0001;
      unsigned res = 0;
     
      for (i=0; i < (msize); i++) {
        res = (unsigned) (tmp & value);
        v[i] = (unsigned) res; 
        value = value >> 1;
      }
      break;
    }
    case _scv_expr::BOOLEAN: {
      bool value = e.getBoolValue();
      v[0] = value;
      for (i=1; i < (msize); i++) {
        v[i] = 0; 
      }
      break;
    }
    case _scv_expr::SC_BV_BASE: {  
      sc_bv_base val = e.getBigValue();
      for (i=0; i < msize; i++) {
        v[i] = val.get_bit(i) ;
      }
      break;
    }
    case _scv_expr::DOUBLE:
      for (i=0; i<msize; i++) {
      } 
      _scv_constraint_error::internalError(
        "getVector called for double.\n");
      break;
    case _scv_expr::STRING:
      _scv_constraint_error::internalError(
        "getVector called for string.\n");
      break;
    case _scv_expr::RECORD:
      _scv_constraint_error::internalError(
        "getVector called for record.\n");
      break;
    case _scv_expr::ARRAY:
      _scv_constraint_error::internalError(
        "getVector called for array.\n");
      break;
    default:
      _scv_constraint_error::internalError(
        "Trying to get a value for unknown data type.\n");
      break;
  }  
}

template <typename T>
void _scv_constraint_manager::_setValue(T value, scv_extensions_if* s, int starti, int msize, int numvar) {
  T uvalue = 0;
  T tmp = 1; 

  randomnext = getRandomGenP()->next();
  unsigned checkvalue = 0;

  ValueIndexSizeType valueIndexMaxIndex = (msize > 0) ? (msize-1) * numvar + starti : 0;
  if (valueIndexMaxIndex >= valueIndex.size()) {
    valueIndex.resize(valueIndexMaxIndex + 1);
  }
  for (int i=0; i < msize; i++) {
    tmp = 0x1;
    checkvalue = valueIndex[(i*numvar)+starti];
    switch(checkvalue) {
    case 0:
      break;
    case 1:
      tmp = tmp << i;
      uvalue = (uvalue | tmp);
      break;
    case 2:
      updateRandomNext();
      if (randomnext%2) { 
        tmp = tmp << i; 
        uvalue = (uvalue | tmp);
      }
      break;
    }
  }
  s->assign(uvalue);
}

void _scv_constraint_manager::_setBigValue(scv_extensions_if* s, int starti, int msize, int numvar) {
  unsigned checkvalue = 0;

  randomnext = getRandomGenP()->next();
  sc_bv_base base_value(msize);
  assert(msize>0);
  ValueIndexSizeType valueIndexMaxIndex = (((msize-1)/32)*32 + 31) * numvar + starti;
  if (valueIndexMaxIndex >= valueIndex.size()) {
    valueIndex.resize(valueIndexMaxIndex + 1);
  }
  for (int j=0; j <= (msize-1) / 32; j++) {
    unsigned uvalue = 0;
    unsigned tmp = 1; 

    for (int i=0; i < 32; i++) {
      checkvalue = valueIndex[(j*32+i)*numvar+starti];
      switch(checkvalue) {
      case 0:
        break;
      case 1:
        tmp = 0x1 << i;
        uvalue = (uvalue | tmp);
        break;
      case 2:
        updateRandomNext();
        if (randomnext%2) { 
          tmp = 0x1 << i;
          uvalue = (uvalue | tmp);
        }
        break;
      }
    }
    base_value.set_word(j, uvalue); 
  }
  s->assign(base_value);
}

void _scv_constraint_manager::setValue(scv_extensions_if* s, int starti, int msize, int numvar) 
{
  switch(s->get_type()) {
  case scv_extensions_if::INTEGER: 
  case scv_extensions_if::ENUMERATION: 
    {
      if (msize <= 64) {
        long long value=0;
        _setValue(value, s, starti, msize, numvar);
      } else {
        _setBigValue(s, starti, msize, numvar);
      }
      break;
    }
  case scv_extensions_if::UNSIGNED: 
  case scv_extensions_if::BIT_VECTOR: 
  case scv_extensions_if::LOGIC_VECTOR: 
  case scv_extensions_if::BOOLEAN: 
    {
      if (msize <= 64) {
        unsigned long long value=0;
        _setValue(value, s, starti, msize, numvar);
      } else {
        _setBigValue(s, starti, msize, numvar);
      }
      break;
    }
  case scv_extensions_if::FLOATING_POINT_NUMBER: 
  case scv_extensions_if::RECORD: 
  case scv_extensions_if::ARRAY: 
  case scv_extensions_if::POINTER: 
     break;
  default: 
      _scv_constraint_error::internalError(
        "Internal Error: Should not have reached here - setValue.");
     break;
  }
}

void _scv_constraint_manager::generateWeight(bddNodeT* b) 
{
  numBddVar = maxvar * maxNumBits;
  if (verboseLevel > 3 ) {
    scv_out << "========== GENERATE WEIGHT ========== " << endl;
    scv_out << oneNode << " : OneNode" << endl;
    scv_out << zeroNode << " : zeroNode" << endl << endl;
    scv_out << "Number of Bdd variable : "  << numBddVar << endl << endl;
    scv_out << "nthvar: "  << nthvar << endl << endl;
  } 
  getWeightNode(b->getNode());
  return;
}

static double pthenBranch;
static double pelseBranch;
static double nodeWeight;

double _scv_constraint_manager::getWeightNode(ddNodeT* node)
{
  double t, e;
  DdNode *Nt, *Ne;

  if (!node) {
    _scv_constraint_error::internalError(
      "File a bug report.\n");
    return 0;
  }

  Nt = Cudd_T(node); Ne = Cudd_E(node);
  if (Cudd_IsComplement(node)) {
    Nt = Cudd_Not(Nt);
    Ne = Cudd_Not(Ne);
  }
  if (node == oneNode) {
    return 1;
  } else if (node == zeroNode) {
    return 0;
  }

  if (wasVisited->getValue(node)) {
    return nodeWeightHash->getValue(node);
  } else {
    wasVisited->insert(node, 1);
  }

  t = getWeightNode(Nt);
  e = getWeightNode(Ne);

  if (Nt != oneNode && Nt != zeroNode)  {
    pthenBranch = pow(2.0, static_cast<int>(Cudd_Regular(Nt)->index - Cudd_Regular(node)->index - 1)) * t;
  } else if (Nt == oneNode) {
    pthenBranch = pow(2.0, static_cast<int>(numBddVar - Cudd_Regular(node)->index - 1)) * t;
  } else {
    pthenBranch = t ;
  }
  if (Ne != oneNode && Ne != zeroNode) {
    pelseBranch =  pow(2.0, static_cast<int>(Cudd_Regular(Ne)->index - Cudd_Regular(node)->index - 1)) * e  ;
  } else if (Ne == oneNode) {
    pelseBranch = pow(2.0, static_cast<int>(numBddVar - Cudd_Regular(node)->index - 1)) * e;
  } else {
    pelseBranch = e ;
  }

  nodeWeight = pthenBranch + pelseBranch;

  nodeWeightHash->insert(node, nodeWeight);
  nodeHashP->update(node, static_cast<int>(std::floor((pelseBranch/nodeWeight)*ROUNDTO)));

  if (verboseLevel > 3) {
    scv_out << "Branching : " << node << "(" << Cudd_Regular(node)->index << ")" ;
    scv_out << " Else : " << Ne << "( " << Cudd_Regular(Ne)->index << ")";
    scv_out << " Then : " << Nt << "( " << Cudd_Regular(Nt)->index << ")" << endl;
    scv_out << " t : " << t << " " << " e : " << e << " " << pthenBranch << " " << pelseBranch << endl;
    scv_out << Cudd_Regular(node) << " : " << pelseBranch  ;
    scv_out << " /  " << nodeWeight  ;
    scv_out << " : " << static_cast<int>(std::floor((pelseBranch/nodeWeight)*ROUNDTO)) << endl;
  }

  return nodeWeight;
}

void _scv_constraint_manager::setDoubleValue(scv_extensions_if* e, scv_constraint_base* c, 
  scv_shared_ptr<scv_random> g) 
{
  setRandomGen(g);

  double size = FLT_MAX;
  double i = getRandomGenP()->next(); 
  double max = 0xFFFFFFFF;
  double remain = (i / max) * size;

  double val = -(FLT_MAX/2) + remain;
  
  e->assign(val); 
}

bool _scv_constraint_manager::isOverConstrained(const _scv_expr& e)
{
  if (e.isBdd()) {
    if ((e.getBddNodeP()->getNode() == _mgr->bddZero().getNode()) ) {
      return true;
    } else {
      return false;
    }
  } else {
    _scv_constraint_error::internalError(
      "checkForZeroBdd can only be applied to _scv_expr of type BDD.");
  }
  return true;
}

/////////////////////////////////////////////////////////////////
// registry routines for constraint construction 
//   _scv_push_constraint
//   _scv_pop_constraint
//   _scv_print_registry
/////////////////////////////////////////////////////////////////

struct registry_record {
  scv_constraint_base * c;
  std::list<scv_smart_ptr_if*> m;
  registry_record(scv_constraint_base *c) : c(c) {}
  registry_record(const registry_record& rhs) : c(rhs.c), m(rhs.m) {}
};

static std::list<registry_record> in_progress;
static std::list<registry_record> registry;

void _scv_insert_smart_ptr(scv_smart_ptr_if * new_ptr) {
  if (!in_progress.empty()) { 
    in_progress.back().m.push_back(new_ptr);
  }
}

static void _scv_push_constraint(scv_constraint_base * new_container) {
  in_progress.push_back(registry_record(new_container));
}

void _scv_pop_constraint() {
  assert(!in_progress.empty());
  registry.push_back(in_progress.back());
  in_progress.pop_back();

  registry.back().c->set_up_members(registry.back().m);
}

void _scv_print_registry() {
  std::list<registry_record>::iterator i = registry.begin();
  scv_out << "The registry has "
       << registry.size() << " scv_constraint_base." << endl;
  for (; i != registry.end(); ++i) {
    scv_out << "\t- one with "
	 << i->m.size() << " scv_smart_ptr." << endl;
  }
}

////////////////////////////////////////////////////////////////////
// Static utility routines used in this file only
//   _scv_is_avoid_duplicate
//   _scv_copy_values
//   _scv_remove
//   _scv_remove_data
//   _scv_remove_if_bdd
////////////////////////////////////////////////////////////////////
static bool _scv_is_avoid_duplicate(scv_constraint_base * c) 
{
  return (c->get_mode() == scv_extensions_if::RANDOM_AVOID_DUPLICATE);
}

static bool _scv_is_scan(scv_constraint_base * c) 
{
  return (c->get_mode() == scv_extensions_if::SCAN);
}

static void _scv_copy_values(scv_extensions_if* to, scv_extensions_if* from)
{
  switch(from->get_type()) {
    case scv_extensions_if::INTEGER :
    case scv_extensions_if::ENUMERATION :
      to->assign(from->get_integer());
      break;
    case scv_extensions_if::UNSIGNED :
    case scv_extensions_if::BIT_VECTOR :
    case scv_extensions_if::LOGIC_VECTOR :
      to->assign(from->get_unsigned());
      break;
    case scv_extensions_if::BOOLEAN :
      to->assign(from->get_bool());
      break;
    case scv_extensions_if::RECORD : {
      int size = from->get_num_fields();
      for (int j=0; j<size; ++j) {
        scv_extensions_if *efrom = from->get_field(j);
        scv_extensions_if *eto = to->get_field(j);
        _scv_copy_values(eto, efrom);
      }
      break;
    }
    case scv_extensions_if::ARRAY : {
      int size = from->get_array_size();
      for (int j=0; j < size; ++j) {
        scv_extensions_if *efrom = from->get_array_elt(j);
        scv_extensions_if *eto = to->get_array_elt(j);
        _scv_copy_values(eto, efrom);
      }
      break;
    }
    case scv_extensions_if::FLOATING_POINT_NUMBER :
      to->assign(from->get_double());
      break;
    case scv_extensions_if::POINTER : 
      break;
    case scv_extensions_if::FIXED_POINT_INTEGER :
    case scv_extensions_if::UNSIGNED_FIXED_POINT_INTEGER : {
      const std::string msg = "type " + std::string(to->get_type_name());
      _scv_constraint_error::notImplementedYet(msg.c_str());
      break;
    }
    default: 
      _scv_constraint_error::internalError("illegal type");
      break;
  }
  return;
}

void _scv_remove(_scv_expr* e) 
{
  if (e && e->isBddVector()) {
    delete e->getBddVectorP();
  } else if (e && e->isBdd()) {
    delete e->getBddNodeP();
  }
  delete e;
}

static void _scv_remove_data(_scv_expr* e) 
{
  if (e && e->isBddVector()) {
    delete e->getBddVectorP();
  } else if (e && e->isBdd()) {
    delete e->getBddNodeP();
  }
}

static void _scv_remove_if_bdd(_scv_expr* e) 
{
  if (e && e->isBdd()) {
    delete e->getBddNodeP();
  } else if (e && e->isBddVector()) {
    if (!e->isVarExpr()) {
      delete e->getBddVectorP();
    }
  }
}

///////////////////////////////////////////////////////////////////
// Utility Internal routines called from other code within 
// SystemC Verification Standard.
//   _scv_set_value
//   _scv_bdd_and
//   _scv_find_extension
//   _scv_copy_values
//   scv_constraint_startup
//   _scv_constraint_wrapup
//   _scv_new_constraint
//   _scv_delete_constraint
///////////////////////////////////////////////////////////////////
void _scv_set_value(scv_extensions_if* e, _scv_constraint_data* cdata_) 
{
  scv_constraint_base * c = cdata_->get_constraint();
  scv_shared_ptr<scv_random> g = cdata_->get_random(e);

  _scv_constraint_manager* m = scv_constraint_manager::getConstraintManagerP();

  bool remove_ = false;
  if (e->is_floating_point_number()) {
    m->setDoubleValue(e, c, g); 
  } else if (cdata_->is_no_constraint() || cdata_->is_range_constraint()) {
    generate_value_range_constraint(e, cdata_);
  } else {
    bddNodeT* b;
    bddNodeT* sb;
 
    if (cdata_->is_avoid_duplicate_mode()) {
      sb = m->avoidDuplicateHash->getValue(c);
      b = m->simplifyConstraint(c, remove_, sb);
    } else {
      b = m->simplifyConstraint(c, remove_);
    }
    m->assignRandomValue(e,b,cdata_);
    if (remove_) {
      delete b;
    }
  }
}

void _scv_set_value(scv_extensions_if* e, scv_constraint_base* c, scv_shared_ptr<scv_random> g) 
{
  _scv_constraint_manager* m = scv_constraint_manager::getConstraintManagerP();

  assert(c);
  bool remove_ = false;
  if (e->is_floating_point_number()) {
    m->setDoubleValue(e, c, g); 
  } else {
    bddNodeT* b = m->simplifyConstraint(c, remove_);
    m->assignRandomValue(e,b,g);
    if (remove_) {
      delete b;
    }
  }
}

bddNodeT& _scv_bdd_and(bddNodeT& bh, bddNodeT& bs, const scv_constraint_base* c)
{
  bddNodeT * b = new bddNodeT;
  _scv_constraint_manager* m = scv_constraint_manager::getConstraintManagerP();
  bddManagerT* _mgr = m->getManagerP();

  *b = (bh & bs);
  if (b->getNode() == _mgr->bddZero().getNode()) {
    _scv_constraint_error::ignoredLevel(c->get_name());
    *b = bh;
  } 
  m->generateWeight(b);
  return *b;
}

scv_extensions_if * _scv_find_extension(scv_constraint_base * c, scv_extensions_if* e)
{
  std::list<scv_smart_ptr_if*>& frommembers = e->get_constraint_data()->
                                         get_constraint()->get_members();
  std::list<scv_smart_ptr_if*>& inmembers = c->get_members();

  std::list<scv_smart_ptr_if*>::iterator i;
  std::list<scv_smart_ptr_if*>::iterator j;
  for (i = frommembers.begin(), j = inmembers.begin();
       (i != frommembers.end()) && (j != inmembers.end());
       ++i, ++j) {
    if ((*i)->get_extensions_ptr() == e) {
      return (*j)->get_extensions_ptr();
    }
  }
  assert(0);
  return 0;
}

void _scv_copy_values(scv_constraint_base* to, scv_constraint_base* from)
{
  std::list<scv_smart_ptr_if*>& to_pointers_ = to->get_members();
  std::list<scv_smart_ptr_if*>& from_pointers_ = from->get_members();

  std::list<scv_smart_ptr_if*>::iterator i;
  std::list<scv_smart_ptr_if*>::iterator j;
  for (i = to_pointers_.begin(), j = from_pointers_.begin();
       (i != to_pointers_.end()) && (j != from_pointers_.end());
       ++i, ++j) {
    _scv_copy_values((*i)->get_extensions_ptr(), (*j)->get_extensions_ptr());
  }
}

void scv_constraint_startup() {
  scv_constraint_manager::globalConstraintManagerP = new _scv_constraint_manager;
  errorExprP = new _scv_expr;
}

void _scv_constraint_wrapup(scv_extensions_if* e) 
{
  scv_constraint_manager::wrapup(e, NULL);
}

scv_constraint_base * _scv_new_constraint(scv_constraint_base * from)
{
  scv_constraint_base * obj = from->get_copy(from);
  return obj;
}

void _scv_delete_constraint(scv_constraint_base * c)
{
  delete c;
}

void _scv_assign_enum_value(scv_extensions_if* e, _scv_constraint_data* cdata_, int e1_val, int e2_val)
{
  std::list<int> ilist;
  std::list<const char *> slist;
  std::list<int>::iterator iter;
  std::list<int>::iterator belem;
  std::list<int>::iterator eelem;
 
  unsigned count = 0; 
  unsigned nth = 0;
  unsigned mth = 0;

  e->get_enum_details(slist, ilist);

  for (iter=ilist.begin(); iter != ilist.end(); iter++) {
    count++;
    if (*iter == e1_val) {
      nth = count;
      belem = iter;
    } 
    if (*iter == e2_val) {
      mth  = count;
      eelem = iter;
      break;
    }
  }

  if (mth < nth) {
    _scv_message::message(_scv_message::CONSTRAINT_INVALID_RANGE, e->get_name());
    e->assign(*eelem);
    return;
  } 

  unsigned size = mth - nth + 1;
  unsigned elmt = (cdata_->get_random(e)->next() % size);

  count = 0;
  
  if ((size == 1) && (belem == eelem)) {
    e->assign(*belem); 
  } else {
    for (iter = belem; ; iter++) {
      if (elmt == count) {
        e->assign(*iter);
        break;
      }
      ++count;
    }
  }
}

const std::string& _scv_get_name(scv_constraint_base* c)
{
  return c->get_name_string();
}

bool _scv_has_complex_constraing(scv_extensions_if* e)
{
  _scv_constraint_manager* mgr = scv_constraint_manager::getConstraintManagerP();
  return mgr->has_complex_constraint(e); 
}

#define _SCV_OBTAIN_VALUE(_cdata, data, gen)           \
  switch(cdata_->get_ext_mode()) {                    \
  case scv_extensions_if::RANDOM:                      \
    data->assign(gen->randomNext());                  \
    break;                                            \
  case scv_extensions_if::SCAN:                        \
    data->assign(gen->scanNext());                    \
    break;                                            \
  case scv_extensions_if::RANDOM_AVOID_DUPLICATE:      \
    data->assign(gen->randomAvoidDuplicateNext());    \
    break;                                            \
  default:                                            \
    break;                                            \
  }

#define _SCV_OBTAIN_VALUE_SC_BV_BASE(_cdata, data, gen)\
  sc_bv_base value(data->get_bitwidth());             \
  switch(cdata_->get_ext_mode()) {                    \
  case scv_extensions_if::RANDOM:                      \
    value = gen->randomNext();                        \
    break;                                            \
  case scv_extensions_if::SCAN:                        \
    value = gen->scanNext();                          \
    break;                                            \
  case scv_extensions_if::RANDOM_AVOID_DUPLICATE:      \
    value = gen->randomAvoidDuplicateNext();          \
    break;                                            \
  default:                                            \
    break;                                            \
  }                                                   \
  data->assign(value);                    

void generate_value_range_constraint(scv_extensions_if* data,
  _scv_constraint_data* cdata_) {
  data->get_generator();
  switch(cdata_->get_gen_type()) {
  case _scv_constraint_data::IGEN: {
    _scv_constraint_range_generator_int * gen =
     cdata_->get_int_generator(data);
    _SCV_OBTAIN_VALUE(_cdata, data, gen);
    break;
  }
  case _scv_constraint_data::ILGEN: {
    _scv_constraint_range_generator_int_ll * gen =
      cdata_->get_int_ll_generator(data);
    _SCV_OBTAIN_VALUE(_cdata, data, gen);
    break;
  }
  case _scv_constraint_data::IBGEN: {
    _scv_constraint_range_generator_signed_big * gen =
     cdata_->get_signed_big_generator(data);
    _SCV_OBTAIN_VALUE_SC_BV_BASE(_cdata, data, gen);
    break;
  }
  case _scv_constraint_data::UGEN: {
    _scv_constraint_range_generator_unsigned * gen =
      cdata_->get_unsigned_generator(data);
    if (!data->is_enum()) {
      _SCV_OBTAIN_VALUE(_cdata, data, gen);
    } else {
      unsigned elmt = 0;
      switch(cdata_->get_ext_mode()) {
        case scv_extensions_if::RANDOM:
          elmt = gen->randomNext();
          break;
        case scv_extensions_if::SCAN:
          elmt = gen->scanNext();
          break;
        case scv_extensions_if::RANDOM_AVOID_DUPLICATE:
          elmt = gen->randomAvoidDuplicateNext();
          break;
        default: 
          break;
      }
      std::list<int> ilist;
      std::list<const char *> slist;
      std::list<int>::iterator iter;
      unsigned count = 0;

      data->get_enum_details(slist, ilist);
      for (iter = ilist.begin(); iter != ilist.end(); iter++) {
        if (elmt == count) {
          data->assign(*iter);
          break;
        }
        ++count;
      }
    }
    break;
  }
  case _scv_constraint_data::ULGEN: {
    _scv_constraint_range_generator_unsigned_ll * gen =
      cdata_->get_unsigned_ll_generator(data);
    _SCV_OBTAIN_VALUE(_cdata, data, gen);
    break;
  }
  case _scv_constraint_data::UBGEN: {
    _scv_constraint_range_generator_unsigned_big * gen =
      cdata_->get_unsigned_big_generator(data);
    _SCV_OBTAIN_VALUE_SC_BV_BASE(_cdata, data, gen);
    break;
  }
  case _scv_constraint_data::DGEN: {
    _scv_constraint_range_generator_double * gen =
      cdata_->get_double_generator(data);
    _SCV_OBTAIN_VALUE(_cdata, data, gen);
    break;
  }
  case _scv_constraint_data::EMPTY:
    break;
  default:
    break;
  }
}

void generate_value_no_constraint(scv_extensions_if* data,
  _scv_constraint_data* cdata_)
{
  if (data->is_floating_point_number()) {
    _scv_set_value(data, cdata_);
  } else {
    generate_value_range_constraint(data, cdata_);
  }  
}

void generate_value_extension(scv_extensions_if* data,
  _scv_constraint_data* cdata_)
{
  scv_extensions_if *e = cdata_->get_extension();
  _scv_set_value(e, e->get_constraint_data()->get_constraint(), cdata_->get_random(data));
  if (data->is_integer() ||
      data->is_enum() ||
      data->is_bool() )  {
    data->assign(e->get_integer());
  } else if (data->is_unsigned() ||
             data->is_bit_vector()  ||
             data->is_logic_vector()) {
    data->assign(e->get_unsigned());
  } else if (data->is_floating_point_number()) {
    data->assign(e->get_double());
  } else {
    const std::string msg = "use_constraint for type " + 
                       std::string(e->get_type_name()) +   
                       "This message will be printed only once." ;
    static bool flag = false;
    if (!flag) {
      _scv_message::message(_scv_message::INTERNAL_ERROR, msg.c_str()); 
      flag = true;
    }
  }
}

_scv_constraint_data::_scv_constraint_data(): constr_(NULL),
  extv_(NULL),
  mode_(NO_CONSTRAINT),
  ext_mode_(scv_extensions_if::RANDOM)
   { prev_val_.unsigned_ = 0; prev_val_.double_ = 0; prev_val_.int_ = 0;
     _range_gen.igen = NULL; _gen_type = EMPTY;
 }

_scv_constraint_data::_scv_constraint_data(const _scv_constraint_data& rhs) {
  constr_ = rhs.constr_;
  extv_ = rhs.extv_;
  gen_ = rhs.gen_;
  mode_ = rhs.mode_;
  ext_mode_ = rhs.ext_mode_;
  lb_scan_ = rhs.lb_scan_;
  ub_scan_ = rhs.ub_scan_;
  prev_val_ = rhs.prev_val_;
  _gen_type = rhs._gen_type;
}

_scv_constraint_data::~_scv_constraint_data()
{
  switch(_gen_type) {
    case IGEN:
      if (_range_gen.igen) delete _range_gen.igen;
      break;
    case ILGEN:
      if (_range_gen.igen) delete _range_gen.i_ll_gen;
      break;
    case IBGEN:
      if (_range_gen.igen) delete _range_gen.i_big_gen;
      break;
    case UGEN:
      if (_range_gen.igen) delete _range_gen.ugen;
      break;
    case ULGEN:
      if (_range_gen.igen) delete _range_gen.u_ll_gen;
      break;
    case UBGEN:
      if (_range_gen.igen) delete _range_gen.u_big_gen;
      break;
    case DGEN:
      if (_range_gen.igen) delete _range_gen.dgen;
      break;
    case EMPTY:
    default:
      break;
  }
}

scv_shared_ptr<scv_random> _scv_constraint_data::get_random(scv_extensions_if* e) {
  if (gen_.isNull()) {
    if (e->get_parent()) {
      e->get_parent()->get_random();
    } else {
      scv_random* gen = new scv_random(e->get_name());
      scv_shared_ptr<scv_random> g(gen);
      e->set_random(g);
    }
  }
  return gen_;
}

#define _SCV_REMOVE_GEN(gen) \
  if (gen) {                \
    delete (gen);           \
    gen = NULL;             \
  }                         \
  set_gen_type(EMPTY);

void _scv_constraint_data::set_generator_from(scv_extensions_if* to, scv_extensions_if* from) {
  switch(from->get_constraint_data()->get_gen_type()) {
  case IGEN:
    _SCV_REMOVE_GEN(_range_gen.igen) 
    _range_gen.igen = new _scv_constraint_range_generator_int(
            *(from->get_constraint_data()->get_int_generator(from)),
            to->get_name());
    set_gen_type(IGEN);
    break;
  case ILGEN:
    _SCV_REMOVE_GEN(_range_gen.i_ll_gen) 
    _range_gen.i_ll_gen = new _scv_constraint_range_generator_int_ll(
            *(from->get_constraint_data()->get_int_ll_generator(from)),
            to->get_name());
    set_gen_type(ILGEN);
    break;
  case IBGEN:
    _SCV_REMOVE_GEN(_range_gen.i_big_gen) 
    _range_gen.i_big_gen = new _scv_constraint_range_generator_signed_big(
            *(from->get_constraint_data()->get_signed_big_generator(from)),
            to->get_name());
    set_gen_type(IBGEN);
    break;
  case UGEN:
    _SCV_REMOVE_GEN(_range_gen.ugen) 
    _range_gen.ugen = new _scv_constraint_range_generator_unsigned(
            *(from->get_constraint_data()->get_unsigned_generator(from)),
            to->get_name());
    set_gen_type(UGEN);
    break;
  case ULGEN:
    _SCV_REMOVE_GEN(_range_gen.u_ll_gen) 
    _range_gen.u_ll_gen = new _scv_constraint_range_generator_unsigned_ll(
            *(from->get_constraint_data()->get_unsigned_ll_generator(from)),
            to->get_name());
    set_gen_type(ULGEN);
    break;
  case UBGEN:
    _SCV_REMOVE_GEN(_range_gen.u_big_gen) 
    _range_gen.u_big_gen = new _scv_constraint_range_generator_unsigned_big(
            *(from->get_constraint_data()->get_unsigned_big_generator(from)),
            to->get_name());
    set_gen_type(UBGEN);
    break;
  case DGEN:
    _SCV_REMOVE_GEN(_range_gen.dgen) 
    _range_gen.dgen = new _scv_constraint_range_generator_double(
          *(from->get_constraint_data()->get_double_generator(from)),
          to->get_name());
    set_gen_type(DGEN);
    break;
  case EMPTY:
    set_gen_type(EMPTY);
    break;
  default:
    break;
  }
  return;
}

 void _scv_constraint_data::set_ext_mode(scv_extensions_if::mode_t m, int lb, int ub ) {
  ext_mode_ = m;
  if (lb == 0 && ub == 0) {
    lb_scan_ = 1;
    ub_scan_ = 1;
  } else {
    lb_scan_ = lb;
    ub_scan_ = ub;
  }
}

void _scv_constraint_data::reset_distribution(scv_extensions_if* s)
{
  switch(get_gen_type()) {
  case IGEN:
    _SCV_REMOVE_GEN(_range_gen.igen) 
    _range_gen.igen = get_int_generator(s);
    break;
  case ILGEN:
    _SCV_REMOVE_GEN(_range_gen.i_ll_gen) 
    _range_gen.i_ll_gen = get_int_ll_generator(s);
    break;
  case IBGEN:
    _SCV_REMOVE_GEN(_range_gen.i_big_gen) 
    _range_gen.i_big_gen = get_signed_big_generator(s);
    break;
  case UGEN:
    _SCV_REMOVE_GEN(_range_gen.ugen) 
    _range_gen.ugen = get_unsigned_generator(s);
    break;
  case ULGEN:
    _SCV_REMOVE_GEN(_range_gen.u_ll_gen) 
    _range_gen.u_ll_gen = get_unsigned_ll_generator(s);
    break;
  case UBGEN:
    _SCV_REMOVE_GEN(_range_gen.u_big_gen) 
    _range_gen.u_big_gen = get_unsigned_big_generator(s);
    break;
  case DGEN:
    _SCV_REMOVE_GEN(_range_gen.dgen) 
    _range_gen.dgen = get_double_generator(s);
    break;
  case EMPTY:
    break;
  default:
    break;
  }
  if (is_distribution_constraint() || is_range_constraint()) {
    if (!get_extension() && get_constraint()) {
      set_mode(_scv_constraint_data::CONSTRAINT);
    } else {
      set_mode(_scv_constraint_data::NO_CONSTRAINT);
    }
    set_ext_mode(scv_extensions_if::RANDOM);
  }
}

_scv_constraint_range_generator_unsigned* _scv_constraint_data::get_unsigned_generator(scv_extensions_if* s)
{
  if (!_range_gen.ugen) {
    unsigned lb = 0;
    unsigned ub = 0;
    if (s->get_bitwidth() == 32) {
      ub = UINT_MAX;
    } else {
      ub = ((0x1 << s->get_bitwidth()) -1);
    }
    if (s->is_bool()) { lb = 0; ub = 1; }
    if (s->is_enum()) { lb = 0; ub = (s->get_enum_size()-1); }
    _range_gen.ugen = new _scv_constraint_range_generator_unsigned(lb, ub,
      get_random(s), s->get_name());
    set_gen_type(UGEN);
  }
  return _range_gen.ugen;
}

_scv_constraint_range_generator_unsigned_ll* _scv_constraint_data::get_unsigned_ll_generator(scv_extensions_if* s) {
  if (!_range_gen.u_ll_gen) {
    unsigned long long lb = 0;
    unsigned long long ub = 0;
    unsigned long long tmp = 0x01;
    if (s->get_bitwidth() == 64) {
      for (int i=0; i < 64; i++) {
        ub = ub | (tmp << i);
      }
    } else {
      ub = ((tmp << s->get_bitwidth()) -1);
    }
    _range_gen.u_ll_gen = new _scv_constraint_range_generator_unsigned_ll(lb, ub,
      get_random(s), s->get_name());
    set_gen_type(ULGEN);
  }
  return _range_gen.u_ll_gen;
}

_scv_constraint_range_generator_int* _scv_constraint_data::get_int_generator(scv_extensions_if* s) {
  if (!_range_gen.igen) {
    int ub = (0x1 << (s->get_bitwidth()-1) ) -1;
    int lb = (0-ub-1);
    if (s->is_bool()) { lb = 0; ub = 1; }
    _range_gen.igen = new _scv_constraint_range_generator_int(lb, ub,
      get_random(s), s->get_name());
    set_gen_type(IGEN);
  }
  return _range_gen.igen;
}

_scv_constraint_range_generator_int_ll* _scv_constraint_data::get_int_ll_generator(scv_extensions_if* s) {
  if (!_range_gen.i_ll_gen) {
    long long tmp = 0x01;
    long long ub = (tmp << (s->get_bitwidth()-1) ) -1;
    long long lb = (0-ub-1);
    _range_gen.i_ll_gen = new _scv_constraint_range_generator_int_ll(lb, ub,
      get_random(s), s->get_name());
    set_gen_type(ILGEN);
  }
  return _range_gen.i_ll_gen;
}

_scv_constraint_range_generator_unsigned_big* _scv_constraint_data::get_unsigned_big_generator(scv_extensions_if* s) {
  if (!_range_gen.u_big_gen) {
    sc_unsigned tmp(s->get_bitwidth()) ;
    sc_unsigned lb(s->get_bitwidth());
    sc_unsigned ub(s->get_bitwidth());
    tmp = 0x1;
    lb = 0;
    for (int i=0; i < s->get_bitwidth(); i++) {
      ub = ub | (tmp << i);
    }
    _range_gen.u_big_gen = new _scv_constraint_range_generator_unsigned_big(
     lb, ub, get_random(s), s->get_name());
    set_gen_type(UBGEN);
  }
  return _range_gen.u_big_gen;
}

_scv_constraint_range_generator_signed_big* _scv_constraint_data::get_signed_big_generator(scv_extensions_if*s) {
  if (!_range_gen.i_big_gen) {
    sc_signed tmp(s->get_bitwidth());
    sc_signed ub(s->get_bitwidth());
    sc_signed lb(s->get_bitwidth());
    tmp = 0x1;
    ub = ((tmp << s->get_bitwidth()-1) -1);
    lb = (0-ub-1);
    _range_gen.i_big_gen = new _scv_constraint_range_generator_signed_big(
     lb, ub, get_random(s), s->get_name());
    set_gen_type(IBGEN);
  }
  return _range_gen.i_big_gen;
}

_scv_constraint_range_generator_double* _scv_constraint_data::get_double_generator(scv_extensions_if* s) {
  if (!_range_gen.dgen) {
    double lb = -FLT_MAX/2;
    double ub = FLT_MAX/2;
    _range_gen.dgen = new _scv_constraint_range_generator_double(lb, ub,
      get_random(s), s->get_name());
    set_gen_type(DGEN);
  }
  return _range_gen.dgen;
}

static void _scv_update_dynamic_constraints(scv_extensions_if* to, scv_extensions_if* from);

void _scv_use_constraint(scv_extensions_if* to, scv_extensions_if* e)
{
  _scv_constraint_data* cdata_ = to->get_constraint_data();
  _scv_constraint_data* cd = e->get_constraint_data();

  if (sDebugLevel > 3) {
    scv_out << "From _scv_use_constraint: " << to << "  " << e << endl;
  }

  if (!cdata_->get_extension() && cdata_->get_constraint()) { 
    _scv_message::message(_scv_message::CONSTRAINT_INVALID_USE_CONSTRAINT, 
      to->get_name(), _scv_get_name(cdata_->get_constraint()).c_str());
    return;
  }
  if (strcmp(to->get_type_name(),  e->get_type_name())) { 
    _scv_message::message(_scv_message::CONSTRAINT_TYPE_MISMATCH, 
      to->get_name(), to->get_type_name(), 
      e->get_name(), e->get_type_name());
    return;
  }
  if (cd->is_no_constraint()) {
    _scv_message::message(_scv_message::CONSTRAINT_DEFAULT_CONSTRAINT,
      e->get_name());
  } else if (cd->is_range_constraint()) {
    cdata_->set_generator_from(to, e); 
    cdata_->set_mode(_scv_constraint_data::RANGE_CONSTRAINT);
  } else if (cd->is_distribution_constraint()) {
    to->set_distribution_from(e);
    cdata_->set_mode(cd->get_mode());
  } else if (cd->is_complex_constraint()) {
    assert(cd->get_constraint());
    scv_constraint_base* c = _scv_new_constraint(cd->get_constraint());
    scv_extensions_if * ne = _scv_find_extension(c, e);
    to->set_constraint(c);
    to->set_extension(ne);
    _scv_update_dynamic_constraints(ne, e);
  }
  return;
}

void _scv_update_dynamic_constraints(scv_extensions_if* to, scv_extensions_if* from)
{
  int size;
  _scv_constraint_data* cdata_ = to->get_constraint_data();
  _scv_constraint_data* cd = from->get_constraint_data();

  if (sDebugLevel > 3) {
    scv_out << "Called _scv_update_dynamic_constraints on: " << to->get_name() << endl;
  }

  if (to->is_record()) {
    size = to->get_num_fields();
    for (int j=0; j<size; ++j) {
      scv_extensions_if *tof = to->get_field(j);
      scv_extensions_if *fromf = from->get_field(j);
      _scv_update_dynamic_constraints(tof, fromf);
    }
  } else if (to->is_array()) {
    size = to->get_array_size();
    for (int j=0; j<size; ++j) {
      scv_extensions_if *tof = to->get_array_elt(j);
      scv_extensions_if *fromf = from->get_array_elt(j);
      _scv_update_dynamic_constraints(tof, fromf);
    }
  } else {
    if (cd->is_range_constraint()) {
      cdata_->set_generator_from(to, from); 
      cdata_->set_mode(_scv_constraint_data::RANGE_CONSTRAINT);
      if (sDebugLevel > 3) {
        scv_out << "Copying dynamic range constraint from: " << from->get_name() << endl;
      }
    } else if (cd->is_distribution_constraint()) {
      to->set_distribution_from(from);
      cdata_->set_mode(cd->get_mode());
      if (sDebugLevel > 3) {
        scv_out << "Copying dynamic range constraint from: " << from->get_name() << endl;
      }
    } 
  }
}

void _scv_set_constraint(scv_extensions_if* s, bool mode)
{
  if (s->get_parent()) {
    s->get_parent()->set_constraint(mode); 
  }
  if (mode == true) {
    _scv_constraint_data* cd = s->get_constraint_data();
    if (cd->is_range_constraint()) {
      //_scv_message::message(_scv_message::CONSTRAINT_INVALID_COMBINATION_RANGE, 
       // s->get_name(), _scv_get_name(cd->get_constraint()).c_str());
    } else if (cd->is_distribution_constraint()) {
      //_scv_message::message(_scv_message::CONSTRAINT_DISTRIBUTION_OVERWRITTEN,
        //s->get_name(), _scv_get_name(cd->get_constraint()).c_str());
    } else {
      cd->set_mode(_scv_constraint_data::CONSTRAINT); 
    }
  }
}

scv_extensions_if* _scv_get_extension(scv_smart_ptr_if& s)
{
  return s.get_extensions_ptr();
}


//////////////////////////////////////////////////////////////
// Class : _scv_constraint_error
//   - Define error messages for constraint solver
//
//////////////////////////////////////////////////////////////

void _scv_constraint_error::ignoreDoubleConstraints(void) {
  static bool flag = false;
  if (!flag) {
    notImplementedYet("constraints on floating point types (will be ignored). This message will be printed only once." );
    flag = true;
  }
}

void _scv_constraint_error::ignoreStringConstraints(void) {
  static bool flag = false;
  if (!flag) {
    notImplementedYet("constraints on string (will be ignored). This message will be printed only once." );
    flag = true;
  }
}
