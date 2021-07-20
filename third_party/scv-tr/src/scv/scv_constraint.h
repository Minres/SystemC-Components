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

  scv_constraint.h -- 
  The public interface for the constraint facility.
  - Constraints are be specified by creating a constraint class
    derived from the scv_constraint_base class and specifying 
    constraint expressions using convinient macros provided
    by the constraints interface
  - Constraint expressions are created by scv_expression class.
    See scv_expression.h for all the operators and operations
    supported by this class. 
  - Public interface for the constraints base class provides
    methods for obtaining random values and selecting value
    generation modes.
  - This file also contains definitions for scv_constraint_manager
    and _scv_constraint_manager class. scv_constraint_manager 
    provides an interface to the constraint solver. 
    _scv_constraint_manager class encapsulates data and methods 
    for solving constraints.
  - Contains definition and implementation for _scv_hash class
    Provides a fast hash table essentially for storing 
    weight information for BDD nodes.

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
  Description of Modification: Fix Clang compiler warnings due to declaration
                               of inline functions isPrime() and nextPrime()
                               as static and not static inline.

 *****************************************************************************/

#ifndef SCV_CONSTRAINT_H
#define SCV_CONSTRAINT_H

#include "scv/scv_introspection.h"

#include "scv/scv_report.h"
#include <vector>

// This file defines classes used to solve complex constraints 
// using BDD's (Binary Decision Diagrams).

// Declaration of the classes used from the CUDD package.
// Make definition of datatypes independent of the CUDD package.

class BDD;
class Cudd;
class BDDvector;
struct DdNode;

typedef BDD bddNodeT;
typedef Cudd bddManagerT;
typedef BDDvector bddVectorT;
typedef DdNode ddNodeT; 

// Forward declarations
class _scv_constraint_manager;
class _scv_expr;

#define MAX_MESSAGE_SIZE 1024
#define SIZE_HINT 100000


//************************************************************* 
// hash function
//*************************************************************
class _scv_hash {
  unsigned long ** _factor;
public:  
  _scv_hash()
    : _factor(new unsigned long * [sizeof(void*)])
    {
      unsigned long b = 1;
      for (int i = sizeof(void*)-1; i>=0 ; i--) {
	_factor[i] = new unsigned long [256];
	for (int j = 0; j<256; j++) {
	  _factor[i][j] = b*j;
	  b *= 131;
	}
      }
    }
  ~_scv_hash() {
    for (unsigned i = 0; i<sizeof(void*); i++)
      delete[] _factor[i];
    delete[] _factor;
  }
  unsigned long operator()(const void * key) const {
    unsigned char * byteP = (unsigned char *) &key;
    unsigned long sum;
    sum = 0;
    for (int i = sizeof(void*)-1; i>=0; i--) { 
      sum += _factor[i][*byteP++]; 
    }
    return sum;
  }
};

//************************************************************* 
//  Utilities (local to this file) 
//*************************************************************
static inline bool isPrime(unsigned long n) {
  unsigned long i = 3;
  if ( n < 3 ) return true;
  if ( n % 2 == 0 ) return false;
  while( i * i <= n ) {
    if( n % i == 0 ) return false;
    i += 2;
  }
  return true;
}

static inline unsigned long nextPrime(unsigned long n) {
  while( !isPrime(n) ) n++;
  return n;
}

//************************************************************* 
// a hash table that uses a pointer as keys
//*************************************************************
template <class eltT>
class _scv_open_table {
  class bucketT {
  public:
    const void * _key;
    eltT _element;
    bucketT * _nextP;
    bucketT(const void * key, const eltT& element, bucketT * nextP) 
      : _key(key), _element(element), _nextP(nextP) {}
    bool update(const void * key, const eltT& element) {
      if (_key == key) { _element = element; return true; }
      if (_nextP) return _nextP->update(key,element);
      return false; 
    }
    bool get(const void * key, eltT *& elementP) {
      if (_key == key) { elementP = &_element; return true; }
      if (_nextP) return _nextP->get(key,elementP);
      return false; 
    }
    bool removeNextBucket(const void * key) {
      if (_nextP && _nextP->_key == key) {
	bucketT * matchP = _nextP;
	_nextP = _nextP->_nextP;
	delete matchP;
	return true;
      } 
      if (_nextP) return _nextP->removeNextBucket(key);
      return false;
    }
    static void remove(bucketT * chainP) {
      if (chainP) {
	remove(chainP->_nextP);
	delete chainP;
      }
    }
#if 0
    inline static tb_memory_manager& _mm() { static tb_memory_manager mm(sizeof(bucketT)); return mm; }
    inline static void * operator new(size_t size, void * p) { return p; } 
    inline static void * operator new(size_t size) { return _mm().alloc(size); }
    inline static void operator delete(void *p, size_t size) { _mm().free(p,size); }
#endif
  };
  _scv_hash _hash;
  unsigned long _numElts;
  unsigned long _numBuckets;
  bucketT ** _arrayP;
public:
  _scv_open_table(long sizeHint) 
    : _hash(),
    _numElts(0),
    _numBuckets(nextPrime(sizeHint)),
    _arrayP(new bucketT*[_numBuckets]) 
    {
      for (unsigned long i=0; i<_numBuckets; ++i) {
	_arrayP[i] = NULL;
      }
    }
  ~_scv_open_table() {
    reset();
    delete[] _arrayP;
  }
  inline long numElts() const { return _numElts; }
  inline long numBuckets() const { return _numBuckets; }
  
  inline void reset() {
      for (unsigned long i=0; i<_numBuckets; ++i) {
	bucketT::remove(_arrayP[i]);
	_arrayP[i] = NULL;
      }
      _numElts = 0;
  }
  inline bool remove(const void * key) {
    if (_remove(key)) { --_numElts; return true; }
    return false;
  }
  inline bool _remove(const void * key) {
    long location = _hash(key) % _numBuckets;
    if (_arrayP[location]) {
      if (_arrayP[location]->_key == key) {
	bucketT * matchP = _arrayP[location];
	_arrayP[location] = matchP->_nextP;
	delete matchP;
	return true;
      } else {
	return _arrayP[location]->removeNextBucket(key);
      }
    } else { 
      return false;
    }
  }
  
  inline bool update(const void * key, const eltT& element) {
    long location = _hash(key) % _numBuckets;
    if (_arrayP[location] && _arrayP[location]->update(key,element))
      return true;
    else {
      _arrayP[location] = new bucketT(key, element, _arrayP[location]);
      ++_numElts;
      return false;
    }
  }
  inline bool get(const void * key, eltT *& elementP) {
    long location = _hash(key) % _numBuckets;
    if (_arrayP[location]) {
      return _arrayP[location]->get(key,elementP);
    } else {
      return false;
    }
  }
};

class scv_constraint_base;

//*******************************************************************
// scv_constraint_manager
//
// * This class acts as external interface to the 
//   constraint solving mechanism using BDD's. 
//   
// * data introspection class interfaces with this class by registering
//   routines bddRandomize and wrapup as callbacks for random 
//   number generation.
// 
// * only contains static methods, should not be instantiated.
//*******************************************************************
class scv_constraint_manager {
  friend void scv_constraint_startup();
  static _scv_constraint_manager *globalConstraintManagerP;
public:
  static void set_value(scv_extensions_if* s, bddNodeT* b, _scv_constraint_data * cdata_);
  static void set_value(scv_constraint_base* c, bool simplify=false);
  static bddNodeT& get_bdd(scv_expression e, scv_constraint_base* c, bool hard_constraint);
  static void init_bdd(scv_expression e, scv_constraint_base* c, bool hard_constraint);
  static void init_maxvar(const scv_expression & he, const scv_expression &se);
  static void reset(void);
  static void wrapup(scv_extensions_if* s, void *argP);
  static _scv_constraint_manager * getConstraintManagerP(void);
  static void add_extension(scv_extensions_if* s);
};

//***********************************************************
// _scv_expr
// 
// * Internal BDD representation for a scv_expression
// 
// * _scv_expr can either of the types defined by valueType
//
//***********************************************************
class _scv_expr {
public:
  typedef enum valueType {
    EMPTY,
    INT, 
    UNSIGNED, 
    SC_BV_BASE,
    BOOLEAN,
    DOUBLE, 
    STRING, 
    UNSIGNED_64BIT,
    BDD, 
    BDDVECTOR,
    BDDVECTOR_SIGNED,
    DOUBLE_VAR,
    STRING_VAR,
    RECORD, 
    ARRAY
  } ExprValueType;
private:
  ExprValueType type;
  union {
    long long         ivalue;
    unsigned long long   uvalue;
    bool        bvalue;
    double      dvalue;
    char*       cvalue;
    bddNodeT*   bdd;
    bddVectorT* bddvec;
    scv_extensions_if* ext;
  } value;
  scv_shared_ptr<sc_bv_base> sc_data;
  int vecsize; // Only used if ExprValueType == BDDVECTOR
  int sigLsb; // Only used if ExprValueType == BDDVECTOR
  int sigMsb; // Only used if ExprValueType == BDDVECTOR
  int isVar;
  int bit_width;
public:
  _scv_expr();
  _scv_expr(const _scv_expr& other);
  ~_scv_expr();
public:
  void setType(ExprValueType type);
  void setBddNodeP(bddNodeT*);
  void setIntValue(long long);
  void setUnsignedValue(unsigned long long);
  void setBigValue(sc_bv_base& v, int bw);
  void setBooleanValue(bool);
  void setDoubleValue(double);
  void setStringValue(const char*);
  void setBddVectorP(bddVectorT*);
  void setExtensionP(scv_extensions_if*);
  void setVecSize(int s);
  void setBitWidth(int bit_width);
  void setSigWidth(int lsb, int msb);
  void setSigLsb(int lsb);
  void setSigMsb(int msb);
  void setIsVar(void) { 
    isVar = 1;
  };
  const ExprValueType get_type(void) const;
  bddNodeT* getBddNodeP(void) const;
  long long getIntValue(void) const;
  unsigned long long getUnsignedValue(void) const;
  bool getBoolValue(void) const;
  double getDoubleValue(void) const;
  sc_bv_base getBigValue(void) const;
  char* getStringValue(void) const;
  bddVectorT* getBddVectorP(void) const;
  scv_extensions_if* getExtensionP(void) const;
  int getVecSize(void) const;
  int getBitWidth(void) const;
  int getSigLsb(void) const;
  int getSigMsb(void) const;
  int isConstant(void) const {
    return (type == UNSIGNED || type == UNSIGNED_64BIT ||
            type == DOUBLE || type == INT || type == BOOLEAN ||
            type == SC_BV_BASE);
  };
  int isBddVector(void) const {
    return (type == BDDVECTOR || type == BDDVECTOR_SIGNED);
  };
  int isBdd(void) const {
    return (type == BDD);
  };
  int isRecord(void) const {
    return (type == RECORD);
  }
  int isArray(void) const {
    return (type == ARRAY);
  }
  int isSigned(void) const {
    return (type == BDDVECTOR_SIGNED);
  }
  int isVarExpr(void) const {
    return isVar;
  }
  int isDouble(void) const {
    return (type == DOUBLE_VAR);
  }
  int isString(void) const {
    return (type == STRING_VAR);
  }
  int isEmpty(void) const {
    return (type == EMPTY);
  }
public:
  _scv_expr& operator=(const _scv_expr& rhs);
}; 

void _scv_remove(_scv_expr* e);

//***********************************************************
// Class :  _smartDataRecordT
//   - Data association of a smart_ptr with a _scv_expr
//   - Also stores BDD indexing information 
//***********************************************************
class _smartDataRecordT {
public:
  _scv_expr * smartDataBddVector;

  int startIndex;
  int numvar;
  int bSize;
public:
  _smartDataRecordT() {
    bSize = 0;
    startIndex = 0;
    numvar = 0;
    smartDataBddVector = NULL;
  }
  ~_smartDataRecordT() {
    if (smartDataBddVector) {
      _scv_remove(smartDataBddVector);
    }
  }
};

//***********************************************************
// Class :  _nodeRecordT
//   - Data associated with a BDD node and required for
//   - probabilistic branching and randomization
//***********************************************************
class _nodeRecordT {
public:
  double pthenBranch;
  double pelseBranch;
  double nodeWeight;
  int wasVisited; 
public:
  _nodeRecordT() {
    pthenBranch = 0;
    pelseBranch = 0;
    nodeWeight = 0;
    wasVisited = 0;
  }
  ~_nodeRecordT() {}
};


//******************************************************************
// _scv_constraint_manager
// 
// * this class encapsulates data to interface with the Bdd
//   package and build BDD's from the expressions specified
//   by the users.
// 
// * Provides methods to generate random numbers based on 
//   complex constraints specified for the data objects
//   involved in expressions.
// 
// * Public interface to this class is provided via bddRandomize
//   and wrapup routines which generate random numbers and
//   do appropriate data structure cleanup based on the usage 
//   of a data object.
//******************************************************************

#define ROUNDTO 65536
#define ROUNDTO_BASE 32768

class _scv_constraint_manager
{
private:
  bddManagerT* _mgr; // Bdd Manager, maintains Bdd cache and data
  scv_shared_ptr<scv_random> _gen; // Random number generator used by bddRandomize 

  _scv_open_table<int> *nodeHashP;       
  _scv_associative_array<ddNodeT*, double> *nodeWeightHash;
  _scv_associative_array<ddNodeT*, int> *wasVisited;

  _scv_open_table<_smartDataRecordT> *extHash;       
  _scv_associative_array<scv_extensions_if*, int> *countExtHash;       
  _scv_associative_array<scv_constraint_base*, bddNodeT*> *avoidDuplicateHash;       
  _scv_associative_array<scv_extensions_if*, bddNodeT*> *avoidDuplicateExtHash;       
  std::list<scv_extensions_if*>* enumVarList;

  _scv_expr* exprRepZero; // internal representation for Zero 
  _scv_expr* exprRepOne;  // internal representation for One
  int verboseLevel; // internal variable used to define level of verbosity for
                    // debugging 
  int nthvar;       // used to obtain bdd variable index for interleaved variable ordering
  int maxvar;       // used fo obtain bdd variable index for interleaved variable ordering
  int numBddVar;
  int maxNumBits;
  int numBitsSigned;
  int *eProb;
  std::vector<unsigned> valueIndex;
  typedef std::vector<unsigned>::size_type ValueIndexSizeType;
  scv_extensions_if::mode_t mode;
  unsigned randomnext;
  DdNode* oneNode;
  DdNode* zeroNode;
public:
  _scv_constraint_manager();
  ~_scv_constraint_manager();
public:
  friend void _scv_set_value(scv_extensions_if* e, scv_constraint_base* c, scv_shared_ptr<scv_random> g);
  friend void _scv_set_value(scv_extensions_if* e, _scv_constraint_data* cdata_);
  friend bddNodeT& _scv_bdd_and(bddNodeT& bh, bddNodeT& bs, const scv_constraint_base* c);
  bddNodeT& get_bdd(scv_expression e, scv_constraint_base* c, bool hard_constraint);
  void init_bdd(scv_expression e, scv_constraint_base* c,  bool hard_constraint);
  void countMaxVar(const scv_expression& e);
  void add_sparse_var(scv_extensions_if* e, bddNodeT* b);
  void check_sparse_var(scv_constraint_base* c, bddNodeT* b);
  void reset(void);
  bddManagerT* getManagerP(void) {return _mgr;}
  void assignRandomValue(scv_extensions_if* s, bddNodeT* argP, _scv_constraint_data* cdata_);
  void assignRandomValue(scv_extensions_if* s, bddNodeT* argP, scv_shared_ptr<scv_random> g);
  void assignRandomValue(scv_constraint_base* c, bool simplify);
  void wrapup(scv_extensions_if* s);
  void add_extension(scv_extensions_if* s);
  bool has_complex_constraint(scv_extensions_if* s);
private:
  _scv_expr assignValueMember(scv_extensions_if* s, bool ad);
  _scv_expr simplifyMember(scv_extensions_if* s, _scv_expr t, scv_constraint_base* c, 
    bool& remove_, bool& over_constraint);
  void initMember(scv_extensions_if* s);
  bddNodeT* simplifyConstraint(scv_constraint_base* c, bool& remove_, bddNodeT* b=NULL);
  _scv_expr simplifyField(_scv_expr e, scv_extensions_if* s, bool& remove_);
  void setDoubleValue(scv_extensions_if* e, scv_constraint_base * c, scv_shared_ptr<scv_random> g);
  void generateWeight(bddNodeT* b);
  double getWeightNode(ddNodeT* node);
  void setRandomGen(scv_shared_ptr<scv_random> g) {_gen = g; randomnext = 0;}
  bool isOverConstrained(const _scv_expr& e);
  scv_shared_ptr<scv_random> getRandomGenP(void) const{return _gen;} 
  _scv_expr getConstantExprRep(_scv_expr e, int signExtend=-1) const;
  _scv_expr getExpressionRep(const scv_expression& e);
  void initExpression(const scv_expression& e);
  _scv_expr getExprRepZero(void);
  _scv_expr getExprRepOne(void);
  _scv_expr exprAnd(const _scv_expr& e1, const _scv_expr& e2);
  _scv_expr _exprAnd(const _scv_expr& e1, const _scv_expr& e2);
  _scv_expr exprOr(const _scv_expr& e1, const _scv_expr& e2);
  _scv_expr _exprOr(const _scv_expr& e1, const _scv_expr& e2);
  _scv_expr exprNot(const _scv_expr& e1);
  _scv_expr _exprNot(const _scv_expr& e1);
  _scv_expr exprPlus(const _scv_expr& e1, const _scv_expr& e2);
  _scv_expr _exprPlus(const _scv_expr& e1, const _scv_expr& e2);
  _scv_expr exprMinus(const _scv_expr& e1, const _scv_expr& e2);
  _scv_expr _exprMinus(const _scv_expr& e1, const _scv_expr& e2);
  _scv_expr exprMultiply(const _scv_expr& e1, const _scv_expr& e2);
  _scv_expr _exprMultiply(const _scv_expr& e1, const _scv_expr& e2);
  _scv_expr exprEqual(const _scv_expr& e1, const _scv_expr& e2); 
  _scv_expr _exprEqual(const _scv_expr& e1, const _scv_expr& e2); 
  _scv_expr exprNEqual(const _scv_expr& e1, const _scv_expr& e2); 
  _scv_expr exprGThan(const _scv_expr& e1, const _scv_expr& e2); 
  _scv_expr exprLThan(const _scv_expr& e1, const _scv_expr& e2); 
  _scv_expr exprGEq(const _scv_expr& e1, const _scv_expr& e2); 
  _scv_expr exprLEq(const _scv_expr& e1, const _scv_expr& e2); 
  _scv_expr _exprLEq(const _scv_expr& e1, const _scv_expr& e2); 
  _scv_expr twosCompliment(const _scv_expr& e);
  _scv_expr getBasicEnumConstraint(void);
  _scv_expr createExprRep(long long v, int bit_width); 
  _scv_expr createExprRep(unsigned long long v, int bit_width); 
  _scv_expr createExprRep(sc_bv_base& v, int bit_width); 
  _scv_expr createExprRep(bool v, int bit_width); 
  _scv_expr createExprRep(double v); 
  _scv_expr createExprRep(const char* v); 
  _scv_expr createExprRep(scv_extensions_if* s);
  void checkExprRep(scv_extensions_if* s);
  _scv_expr createBddVec(int msize);

  void setExprRep(scv_extensions_if* s, _scv_expr& e);
  _scv_expr* getExprRepP(scv_extensions_if* s) const;

  int getVecSize(scv_extensions_if* s) const;
  unsigned getStartIndex(scv_extensions_if* s) const;
  unsigned getEndIndex(scv_extensions_if* s) const;
  int getSizeOfBddVec(_scv_expr& e) const;
  int getSizeOfBddVec(scv_extensions_if* s) const;

  void getVectorFromBdd(ddNodeT*); 
  void getVector(_scv_expr e, unsigned* v) const;
  void setValue(scv_extensions_if* s, int starti, int msize, int numvar);
  template<typename T>
  void _setValue(T value, scv_extensions_if* s, int starti, int msize, int numvar);
  void _setBigValue(scv_extensions_if* s, int starti, int msize, int numvar);

  void updateRandomNext(void) { 
    if (mode == scv_extensions_if::SCAN) {
      randomnext = randomnext >> 1;
    } else {
      randomnext = randomnext >> 1;
      if (randomnext==0) {
        randomnext = getRandomGenP()->next();
      }
    }
  }
  void update16BitValue(void) { 
    if (randomnext <= ROUNDTO) {
      randomnext = getRandomGenP()->next();
    } else {
      randomnext = randomnext >> 16;
    }
  }
};


////////////////////////////////////////////////////////////////
// Class : scv_constraint_base 
//  - Base class for constraint specification
//  - Create constraint classes by inheriting from scv_constraint_base
//  - Use SCV_CONSTRAINT_CTOR,  SCV_CONSTRAINT, SCV_SOFT_CONSTRAINT, 
//    and SCV_BASE_CONSTRAINT macros for specifying constraint 
//    expressions and constructors etc.
////////////////////////////////////////////////////////////////

class scv_constraint_base;
extern bddNodeT& _scv_bdd_and(bddNodeT& bh, bddNodeT& bs, const scv_constraint_base* c);

// Declare internal functions _scv_pop_constraint and _scv_copy_values
// to make them accessible to SCV_CONSTRAINT_CTOR
void _scv_pop_constraint();
void _scv_copy_values(scv_constraint_base* to, scv_constraint_base* from);

class scv_constraint_base : public scv_object_if {
public: // default constructor for the constraint classes
  scv_constraint_base(); 
  virtual ~scv_constraint_base(); 
public: // randomization value generation and configuration  

  // generate and assign new random value to all members of the 
  // constraint class. You can disable or enable randomization
  // of specific fields by using disable_randomzation() and 
  // enable_randomization() methods on scv_extensions<T>

  virtual void next(); 

  // specify the mode in which legal values will be generated
  // for the constraint object
  //  RANDOM - randomly pick a value with uniform distribution
  //  RANDOM_AVOID_DUPLICATE - exhaust all sets of legal values
  //    before randomly picking new random values
  //  SCAN - start from the lower bound and pick new values in
  //    a specific order
  //  DISTRIBUTION - 
  // default mode is set to RANDOM
  
  void set_mode(scv_extensions_if::mode_t m); 

  // return the current mode 
  scv_extensions_if::mode_t get_mode(void) const;

  // attach a specific random stream for the constraint object
  void set_random(scv_shared_ptr<scv_random> g); 

  // return the random stream attached with the constraint object
  scv_shared_ptr<scv_random> get_random(void);

  // provide external list
  void get_members(std::list<scv_smart_ptr_if*>& vlist); 

public: // debugging interface
  const char *get_name() const;
  const std::string& get_name_string() const;
  virtual const char *kind() const;
  void print(ostream& o=scv_out, int details=0, int indent=0) const;
  virtual void show(int details=0, int indent=0) const;
  static int get_debug();
  static void set_debug(int i);

protected: // constraint expressions

  // return hard constraints for the class
  virtual scv_expression get_constraint() const; 

  // return soft constraints for the class
  virtual scv_expression get_soft_constraint() const; 

private: // implementation specific interface

  // put all members of the constraint object in undefined state
  // to generate new random values
  void uninitialize(); 

  // generate random value for all members in undefined (uninitialized)
  // state
  void initialize();

  // setup data structures when object is created
  void set_up_members(std::list<scv_smart_ptr_if*>& members); 

  // get a copy of the constraint object and initialize members to
  // same value as of the parent constraint object
  virtual scv_constraint_base* get_copy(scv_constraint_base * from); 

  // create a bdd for the constraint expressions
  virtual bddNodeT& get_bdd(); 

  // initialize bdd data for variables in expression
  virtual void init_bdd() ; 

  // return hard constraints for this class
  virtual scv_expression& eh() const; 
  
  // soft constraints for this class
  virtual scv_expression& es() const; 

  // hard constraint expression for base classes
  virtual scv_expression& ebh() const;

  // soft constraints for base classes
  virtual scv_expression& ebs() const;

  // ignore soft constraint 
  void ignore_soft_constraint(void); 

  // are soft constraints ignored
  bool is_ignored_soft_constraint(void); 

  // used for scan mode
  int get_scan_counter(void);

  // set expression strings
  void set_expression_string(const char * e, bool hard_constraint);

  // list of scv_smart_ptr members in the constraint object
  std::list<scv_smart_ptr_if*>& get_members(void); 


private: // friend methods and classes for solving constraints
  friend class _scv_constraint_manager;
  friend void _scv_pop_constraint();
  friend scv_extensions_if * _scv_find_extension(scv_constraint_base * c,
    scv_extensions_if* e);
  friend void _scv_copy_values(scv_constraint_base* to, 
    scv_constraint_base* from);
  friend scv_constraint_base * _scv_new_constraint(
    scv_constraint_base * from);

private:
  std::list<scv_smart_ptr_if*> pointers_;
  scv_shared_ptr<scv_random> gen_;
  scv_extensions_if::mode_t mode_;
  static int debug_;
  int scan_counter_;
protected:
  bool ignore_; 
  std::string name_;
  std::string _hard_constraints;
  std::string _soft_constraints;
};

// define macro to avoid warning on linux
#define GET_BDD() \
  static bddNodeT& d = get_bdd(); \
  if (0) cout << &d << endl;

/////////////////////////////////////////////////////////////////////
// SCV_CONSTRAINT_CTOR macro
//   - a name must be provided by the user to get around the 
//     inheritance problem 
//   - creates constructors and initialization methods for 
//     constraint classes 
/////////////////////////////////////////////////////////////////////

#define SCV_CONSTRAINT_CTOR(class_name) \
protected: \
  class_name() { init(); } \
public: \
  class_name(const char* name) { name ? name_ = name : name = "<anonymous>"; \
    _scv_pop_constraint(); init(); init_bdd(); GET_BDD(); next();} \
  virtual const char *kind() const { \
    static const char *name = #class_name; return name; } \
private: \
  virtual void init_bdd() { \
    scv_constraint_manager::init_maxvar(class_name::get_constraint(), \
                                       class_name::get_soft_constraint()); \
    scv_constraint_manager::init_bdd(class_name::get_constraint(), this, true); \
    scv_constraint_manager::init_bdd(class_name::get_soft_constraint(), this, false); \
  } \
  virtual bddNodeT& get_bdd() { \
    static bddNodeT& bh = scv_constraint_manager::get_bdd( \
      class_name::get_constraint(), this, true); \
    static bddNodeT& bs = scv_constraint_manager::get_bdd( \
      class_name::get_soft_constraint(), this, false); \
    static bddNodeT& b = _scv_bdd_and(bh, bs, this); \
    if (ignore_) return bh; else return b; } \
  virtual scv_expression& eh() const { static scv_expression e; return e; } \
  virtual scv_expression& es() const { static scv_expression e; return e; } \
  virtual scv_expression& ebh() const { static scv_expression e; return e; } \
  virtual scv_expression& ebs() const { static scv_expression e; return e; } \
  virtual scv_constraint_base* get_copy(scv_constraint_base * from) { \
    scv_constraint_base* obj = new class_name("class_name"); \
    _scv_copy_values(obj, from); \
    return obj; \
  } \
protected: \
  virtual scv_expression get_constraint() const { \
    return class_name::eh() && class_name::ebh(); } \
  virtual scv_expression get_soft_constraint() const { \
    return class_name::es() && class_name::ebs(); } \
private: \
  bool init() { \
    eh() = es() = ebh() = ebs() = true; \
    init_core(); return true; \
  } \
  void init_core() \

#define SCV_CONSTRAINT(expr) eh() &= expr;

#define SCV_SOFT_CONSTRAINT(expr) es() &= expr;

#define SCV_BASE_CONSTRAINT(base_class) \
  ebh() &= base_class::get_constraint(); \
  ebs() &= base_class::get_soft_constraint(); \


//////////////////////////////////////////////////////////////
// Class : _scv_constraint_error
//   - Define error messages for constraint solver
//   
//////////////////////////////////////////////////////////////
class _scv_constraint_error {
public:
  static void notImplementedYet(const char * messageP) {
    _scv_message::message(_scv_message::CONSTRAINT_ERROR_NOTIMPLEMENTED,
                              messageP);
  }
  static void ignoreDoubleConstraints(void);
  static void ignoreStringConstraints(void);
  static void internalError(const char * messageP) {
    _scv_message::message(_scv_message::CONSTRAINT_ERROR_INTERNAL,
                              messageP);
  }
  static void cannotMeetConstraint(const std::string name) {
    _scv_message::message(_scv_message::CONSTRAINT_ERROR_OVER_CONSTRAINED,
                            name.c_str());
  }
  static void ignoredLevel(const std::string name) {
    _scv_message::message(_scv_message::CONSTRAINT_WARNING_IGNORE_SOFT_CONSTRAINT,
                              name.c_str());
  }
  static void internalWarning(const char * messageP, const char * name) {
    _scv_message::message(_scv_message::CONSTRAINT_WARNING_EQUAL_4_STATE,
                              messageP, name);
  }
  static void typeCheckError(const char * messageP) {
    _scv_message::message(_scv_message::CONSTRAINT_EXPRESSION_TYPEMISMATCHED,
                              messageP);
  }
};

extern void scv_constraint_startup();

#endif
