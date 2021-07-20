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

  scv_constraint_range.h

  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey, Samir Agrawal
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/

#ifndef SCV_CONSTRAINT_RANGE_H
#define SCV_CONSTRAINT_RANGE_H

// *************************************************************************
// Implementation for Value Generation using _scv_constraint_range
// *************************************************************************

#include <string>
#include <iostream>

// *************************************************************************
// Simple Constraint Facility Implementation
// *************************************************************************

#include <list>
#include <cmath> // floor



//*****************************
// _scv_interval
//
// * a private class for implementation of _scv_constraint_range
// * it provides manipulation of intervals, like checking
//   whether two intervals overlap one another.
// * it can be used with "int" intervals, "unsigned" intervals,
//   and "double" intervals.
//
// * assumption:
//     - int/unsigned is Discrete and the upperbound is inclusive.
//     - double is not Discrete and the upperbound is exclusive.
// * size() returns 0 when empty or overflow (i.e. full)
// * the implementation needs to check overflow/underflow whenever
//   a bound is incremented or decremented for discrete types. 
//*****************************

#define _SCV_INTERVAL_FC_D(TypeId, EltT, SizeT, Discrete)  \
class _scv_interval_ ## TypeId {  \
public:  \
  bool _empty;  \
  EltT _lowerbound;  \
  EltT _upperbound;  \
  mutable SizeT _tmp;  \
public:  \
  _scv_interval_ ## TypeId()  \
    : _empty(true),  \
      _lowerbound(0),  \
      _upperbound(0),  \
      _tmp(0)  \
  {}  \
  _scv_interval_ ## TypeId(const EltT& lb, const EltT& ub)  \
    : _empty(false),  \
      _lowerbound(lb),  \
      _upperbound(ub),  \
      _tmp(lb)  \
  {  \
    if (ub<lb || (ub==lb && !Discrete))  \
      _empty = true;  \
  }  \
  _scv_interval_ ## TypeId(const _scv_interval_ ## TypeId& rhs)  \
    : _empty(rhs._empty),  \
      _lowerbound(rhs._lowerbound),  \
      _upperbound(rhs._upperbound),  \
      _tmp(rhs._tmp)  \
  {}  \
  ~_scv_interval_ ## TypeId() {}  \
  \
public:  \
  _scv_interval_ ## TypeId& operator=(const _scv_interval_ ## TypeId& rhs);  \
  friend bool operator==(const _scv_interval_ ## TypeId& a,  \
			 const _scv_interval_ ## TypeId& b);  \
  friend bool operator<(const _scv_interval_ ## TypeId& a,  \
			 const _scv_interval_ ## TypeId& b);  \
  void print(ostream& os) const;  \
  friend ostream& operator<<(ostream& os, const _scv_interval_ ## TypeId& a);  \
  \
public: /* basic access */  \
  bool empty() const { return _empty; }  \
  bool overflow() const { return !empty() && size() == 0; }  \
  SizeT size() const;  \
  EltT lowerbound() const { return _lowerbound; };  \
  EltT upperbound() const { return _upperbound; };  \
  \
public:  \
  int position(const EltT& v) const;  \
  bool contain(const _scv_interval_ ## TypeId& i) const;  \
  friend bool overlap(const _scv_interval_ ## TypeId& a,  \
		      const _scv_interval_ ## TypeId& b);  \
  void intersect(const _scv_interval_ ## TypeId& rhs);  \
  bool subtractable(const _scv_interval_ ## TypeId& rhs) const;  \
  void subtract(const _scv_interval_ ## TypeId& rhs);  \
  friend bool mergeable(const _scv_interval_ ## TypeId& a,  \
			const _scv_interval_ ## TypeId& b);  \
  void merge(const _scv_interval_ ## TypeId& rhs);  \
  \
public: /* others */  \
  friend bool operator!=(const _scv_interval_ ## TypeId& a, const _scv_interval_ ## TypeId& b)  \
    { return !(a==b); }  \
  friend bool operator>(const _scv_interval_ ## TypeId& a, const _scv_interval_ ## TypeId& b)  \
    { return b<a; }  \
  friend bool operator>=(const _scv_interval_ ## TypeId& a, const _scv_interval_ ## TypeId& b)   \
    { return a>b || a==b; }  \
  friend bool operator<=(const _scv_interval_ ## TypeId& a, const _scv_interval_ ## TypeId& b)   \
    { return a<b || a==b; }  \
};  \


_SCV_INTERVAL_FC_D(int,int,unsigned,true);
_SCV_INTERVAL_FC_D(unsigned,unsigned,unsigned,true);
_SCV_INTERVAL_FC_D(double,double,double,false);

_SCV_INTERVAL_FC_D(long_long,long long,unsigned long long,true);
_SCV_INTERVAL_FC_D(unsigned_long_long,unsigned long long,unsigned long long,true);

_SCV_INTERVAL_FC_D(sc_unsigned,sc_unsigned ,sc_unsigned,true);
_SCV_INTERVAL_FC_D(sc_signed ,sc_signed ,sc_unsigned,true);



// ****************************************
// _scv_constraint_range class
//
// * _explicits is only used when 'Discrete' is false.
// * _explicits and _intervals are sorted in assending order.
// * get*() returns 0 with appropriate bits when empty. 
// ****************************************

#define _SCV_CONSTRAINT_RANGE_FC_D(TypeId, EltT, SizeT, Discrete, FlexRandomT)  \
class _scv_constraint_range_ ## TypeId {  \
public: /* main routines */  \
    /* return 0 of appropriate type when empty */  \
  EltT getLowerBound() const { return _intervals.front().lowerbound(); }  \
  EltT getUpperBound() const { return _intervals.back().upperbound(); }  \
  EltT getRandomValue(scv_shared_ptr<scv_random> random) const;  \
  EltT getScanValue(const EltT& base, const SizeT& increment) const;  \
  \
public: /* constructor */  \
  _scv_constraint_range_ ## TypeId(); /* empty constraint */  \
  _scv_constraint_range_ ## TypeId(const EltT& sampleElt); /* empty constraint */  \
  _scv_constraint_range_ ## TypeId(const EltT& lb, const EltT& ub); /* simple interval */  \
  _scv_constraint_range_ ## TypeId(const _scv_constraint_range_ ## TypeId& rhs); /* copy */  \
  ~_scv_constraint_range_ ## TypeId() {}  \
  \
public: /* constructions */  \
  _scv_constraint_range_ ## TypeId& operator=(const _scv_constraint_range_ ## TypeId& rhs);  \
  static _scv_constraint_range_ ## TypeId merge(const _scv_constraint_range_ ## TypeId& a,  \
				     const _scv_constraint_range_ ## TypeId& b);  \
  void keepOnly(const EltT& lb, const EltT& ub);  \
  void keepOnly(const EltT& v);  \
  void keepOnly(const std::list<EltT>& l);  \
  void keepOut(const EltT& lb, const EltT& ub);  \
  void keepOut(const EltT& v); /* illegal if 'Discrete' is false. */  \
  void keepOut(const std::list<EltT>& l);  \
  \
public:  \
  bool isEmpty() const { return _mode == EMPTY; }  \
  bool isUnconstrainted() const {   \
    return !isEmpty() && _explicits.empty() && getSize() == 0;  \
  }  \
  bool satisfy(const EltT& v) const;  \
  friend bool operator==(const _scv_constraint_range_ ## TypeId& a,  \
			 const _scv_constraint_range_ ## TypeId& b) {  \
    return a._mode == b._mode && a._intervals == b._intervals  \
      && a._explicits == b._explicits;  \
  }  \
  friend bool operator!=(const _scv_constraint_range_ ## TypeId& a,  \
			 const _scv_constraint_range_ ## TypeId& b) {  \
    return ! (a==b);  \
  }  \
  friend ostream& operator<<(ostream& os, const _scv_constraint_range_ ## TypeId& a);  \
  \
public:  \
  void setNameP(const std::string & s) { _nameP = s; }  \
  const std::string & getNameP() const { return _nameP; }  \
  \
private:  \
  enum {  \
    EMPTY,  \
    INTERVAL_LIST  \
  } _mode;  \
  std::string _nameP;  \
  \
  std::list<_scv_interval_ ## TypeId > _intervals;  \
  std::list<EltT> _explicits; /* only for 'Discrete' == false */  \
  FlexRandomT _flexRandom;  \
  EltT _tmpUb, _tmpLb; /* variable to make sure lsb/msb are save as signal */  \
  \
private:  \
  mutable bool _sizeValid;  \
  mutable SizeT _size;  \
  SizeT getSize() const;  \
  void setSize() const;  \
  \
private:  \
  void checkExplicits();  \
  void checkIntervals();  \
  \
  void emptyMessage();  \
};  \




// ****************************************
// non-template class definitions for simple constraints
// ****************************************
class _scv_random_unsigned {
public:
  _scv_random_unsigned() {}
  _scv_random_unsigned(int) {}
  _scv_random_unsigned(const _scv_random_unsigned&) {}
  ~_scv_random_unsigned() {}
  unsigned int mod(unsigned int data, unsigned int n) const {
    return data % n;
  }
  unsigned int floor(unsigned int data) const {
    assert(0); return 0;
  }
  unsigned int next(scv_shared_ptr<scv_random> random) const {
    return random->next();
  } 
  unsigned int next(scv_shared_ptr<scv_random> random, unsigned int size) const {
    return random->next() % size;
  } 
};

class _scv_random_double {
public:
  _scv_random_double() {}
  _scv_random_double(int) {}
  _scv_random_double(const _scv_random_double&) {}
  _scv_random_double(double) {}
  ~_scv_random_double() {}
  double mod(double data, double n) const {
    double count1 = data / n;
    if (count1 - std::floor(count1) < 0.1) return 0;
    else return 1;
  }
  double floor(double data) const {
    return std::floor(data);
  }
  double next(scv_shared_ptr<scv_random> random) const {
    double i = random->next();
    double max = 0xFFFFFFFF;
    return (i / max);
  }
  double next(scv_shared_ptr<scv_random> random, double size) const {
    double i = random->next();
    double max = 0xFFFFFFFF;
    return (i / max) * size;
  }
};

class _scv_random_unsigned_ll {
public:
  _scv_random_unsigned_ll() {}
  _scv_random_unsigned_ll(int) {}
  _scv_random_unsigned_ll(const _scv_random_unsigned_ll&) {}
  ~_scv_random_unsigned_ll() {}
  unsigned long long mod(unsigned long long data, unsigned long long n) const {
    return data % n;
  }
  unsigned long long floor(unsigned long long data) const {
    assert(0); return 0;
  }
  unsigned long long next(scv_shared_ptr<scv_random> random) const {
    unsigned long long temp = random->next();
    temp = temp << 32;
    temp = temp | random->next();
    return temp;
  } 
  unsigned long long next(scv_shared_ptr<scv_random> random, unsigned long long size) const {
    unsigned long long temp = random->next();
    temp = temp << 32;
    temp = temp | random->next();
    return temp % size;
  } 
};

class _scv_random_unsigned_big {
  int _numBits;
  int _numBlocks;
  int _remainderSize;
  mutable sc_signed* _temp;
public:
  _scv_random_unsigned_big() 
    : _numBits(1), _numBlocks(0), _remainderSize(2), _temp(NULL) {}
  _scv_random_unsigned_big(const sc_unsigned& sample) 
    : _numBits(sample.length()),
      _numBlocks(sample.length()/32),
      _remainderSize(sample.length()%32) {
     _temp = new sc_signed(sample.length());
   }
  _scv_random_unsigned_big(const _scv_random_unsigned_big& rhs) 
    : _numBits(rhs._numBits),
      _numBlocks(rhs._numBlocks),
      _remainderSize(rhs._remainderSize) {
      _temp = new sc_signed(_numBits);
      *_temp = *rhs._temp;
  }
  ~_scv_random_unsigned_big() { if (_temp) delete _temp; }
  sc_unsigned  mod(const sc_unsigned& data, const sc_unsigned & n) const {
    return data % n;
  }
  unsigned int floor(const sc_unsigned& data) const {
    assert(0); return 0;
  }
  sc_unsigned next(scv_shared_ptr<scv_random> random) const {
    int i;
    for (i=0; i<_numBlocks; ++i) {
      (*_temp)(32*(i+1)-1,32*i) = (unsigned int ) random->next();
    }
    if (_remainderSize) {
      (*_temp)(_numBits-1, 32*_numBlocks) = ((unsigned int) random->next());
    }
    return *_temp;
  } 
  sc_unsigned  next(scv_shared_ptr<scv_random> random, const sc_unsigned& size) const {
    return next(random) % size;
  } 
};



_SCV_CONSTRAINT_RANGE_FC_D(int,int,unsigned,true,_scv_random_unsigned);
_SCV_CONSTRAINT_RANGE_FC_D(unsigned,unsigned,unsigned,true,_scv_random_unsigned);
_SCV_CONSTRAINT_RANGE_FC_D(double,double,double,false,_scv_random_double);

_SCV_CONSTRAINT_RANGE_FC_D(long_long,long long,unsigned long long,true,_scv_random_unsigned_ll);
_SCV_CONSTRAINT_RANGE_FC_D(unsigned_long_long,unsigned long long,unsigned long long,true,_scv_random_unsigned_ll);

_SCV_CONSTRAINT_RANGE_FC_D(sc_unsigned,sc_unsigned ,sc_unsigned,true,_scv_random_unsigned_big);
_SCV_CONSTRAINT_RANGE_FC_D(sc_signed ,sc_signed ,sc_unsigned,true,_scv_random_unsigned_big);



class _scv_constraint_range_error {
public:
  static void invalidScanInterval(const std::string& nameP) {
    setName(nameP);
    _scv_message::message(_scv_message::CONSTRAINT_INVALID_SCAN,nameP.c_str());
  }
  static void invalidDistance(const std::string& nameP) {
    setName(nameP);
    _scv_message::message(_scv_message::CONSTRAINT_INVALID_DISTANCE, nameP.c_str());
  }
  static void invalidDistribution(const std::string& nameP, const char * locationP) {
    setName(nameP);
    _scv_message::message(_scv_message::CONSTRAINT_BAD_BAG, nameP.c_str(),locationP);
  }
  static void overConstraint(const std::string&  nameP, const char * locationP) {
    setName(nameP);
    _scv_message::message(_scv_message::CONSTRAINT_ERROR_OVER_CONSTRAINED, nameP.c_str(),locationP);
  }
  static void emptyGenerator(const std::string& nameP) {
    setName(nameP);
    _scv_message::message(_scv_message::CONSTRAINT_ERROR_OVER_CONSTRAINED, nameP.c_str(),"value generation");
  }
private:
  static void setName(const std::string& nameP) {
    std::string s = nameP; 
  }
};

// ****************************************
// value generation algorithms with _scv_constraint_range 
// ****************************************
#define _SCV_CONSTRAINT_RANGE_GENERATOR_FC_D(TypeId, EltT, SizeT, Discrete, FlexRandomT)  \
class _scv_constraint_range_generator_base_ ## TypeId {  \
public: /* main interface */  \
  EltT randomNext() const;  \
  EltT distributionNext() const;  \
  EltT randomAvoidDuplicateNext() const;  \
  EltT scanNext() const;  \
  bool isEmpty() const { return _simpleConstraint.isEmpty(); }  \
  void reset() { _onGoingConstraintValid = false; _currentScanValueValid = false; }  \
  \
public: /* constructors */  \
  _scv_constraint_range_generator_base_ ## TypeId(const EltT& lb, const EltT& ub,  \
			    scv_shared_ptr<scv_random> random, const char * nameP);  \
  _scv_constraint_range_generator_base_ ## TypeId(const _scv_constraint_range_generator_base_ ## TypeId& rhs,  \
			    const char * nameP);  \
  ~_scv_constraint_range_generator_base_ ## TypeId() {  \
    if (_distributionP) delete _distributionP;  \
    if (_scanIntervalGenP) delete _scanIntervalGenP;  \
  }  \
  \
public: /* constraints */  \
  void setRandom(scv_shared_ptr<scv_random> random) {  \
    _randomP = random;  \
    if (_distributionP) _distributionP->setRandom(*random);  \
  }  \
  const _scv_constraint_range_ ## TypeId&  \
    getConstraint() const {  \
    return _simpleConstraint;  \
  }  \
  void setConstraint  \
    (const _scv_constraint_range_ ## TypeId& c) {  \
    _simpleConstraint = c;  \
    checkConstraint("setConstraint");  \
  }  \
  void keepOnly(const EltT& lb, const EltT& ub) {  \
    _simpleConstraint.keepOnly(lb,ub);  \
    checkConstraint("keepOnly");  \
  }  \
  void keepOnly(const EltT& v) {  \
    _simpleConstraint.keepOnly(v);  \
    checkConstraint("keepOnly");  \
  }  \
  void keepOnly(const std::list<EltT>& l) {  \
    _simpleConstraint.keepOnly(l);  \
    checkConstraint("keepOnly");  \
  }  \
  void keepOut(const EltT& lb, const EltT& ub) {  \
    _simpleConstraint.keepOut(lb,ub);  \
    checkConstraint("keepOut");  \
  }  \
  void keepOut(const EltT& v) {  \
    if (Discrete)  \
      _simpleConstraint.keepOut(v);  \
    else  \
      _simpleConstraint.keepOut(v-_duplicateDistance, v+_duplicateDistance);  \
    checkConstraint("keepOut");  \
  }  \
  void keepOut(const std::list<EltT>& l) {  \
    if (Discrete)  \
      _simpleConstraint.keepOut(l);  \
    else {  \
      for (std::list<EltT>::const_iterator i = l.begin(); i != l.end(); ++i)  \
	_simpleConstraint.keepOut(*i-_duplicateDistance, *i+_duplicateDistance);  \
    }  \
    checkConstraint("keepOut");  \
  }  \
  bool satisfyConstraints(const EltT& v) {  \
    return _simpleConstraint.satisfy(v);  \
  }  \
  \
public:  \
  void print(ostream& os, const char * prefixP) const;  \
  friend ostream& operator<<(ostream& os, const _scv_constraint_range_generator_base_ ## TypeId& a) {  \
    a.print(os,NULL);  \
    return os;  \
  }  \
  \
public:  \
  void setWeightDistribution(const scv_bag<EltT>& bag, bool useMarking);  \
  void _setWeightDistribution(const scv_bag<EltT>& bag, bool useMarking);  \
  void setDuplicateDistance(const SizeT& d) {   \
    if (d > 0)  \
      _duplicateDistance = d;  \
    else  \
      _scv_constraint_range_error::invalidDistance(_nameP);  \
  }  \
  void setScanInterval(const SizeT& lb, const SizeT& ub);  \
  \
protected:  \
  const char * _nameP;  \
  _scv_constraint_range_ ## TypeId _simpleConstraint;  \
  mutable bool _onGoingConstraintValid;  \
  mutable _scv_constraint_range_ ## TypeId _onGoingConstraint;  \
  mutable bool _currentScanValueValid;  \
  mutable EltT _currentScanValue;  \
  scv_shared_ptr<scv_random> _randomP;  \
  \
protected: /* mode specific */  \
  bool _distUseMarking;  \
  scv_bag<EltT> * _distributionP;  \
  SizeT _duplicateDistance;  \
  _scv_constraint_range_ ## TypeId * _scanIntervalGenP;  \
  bool _scanFixedIncrement;  \
  mutable SizeT _scanIncrement;  \
  \
protected:  \
  void checkConstraint(const char * locationP);  \
};  \


_SCV_CONSTRAINT_RANGE_GENERATOR_FC_D(int,int,unsigned,true,_scv_random_unsigned);
_SCV_CONSTRAINT_RANGE_GENERATOR_FC_D(unsigned,unsigned,unsigned,true,_scv_random_unsigned);
_SCV_CONSTRAINT_RANGE_GENERATOR_FC_D(double,double,double,false,_scv_random_double);

_SCV_CONSTRAINT_RANGE_GENERATOR_FC_D(long_long,long long,unsigned long long,true,_scv_random_unsigned_ll);
_SCV_CONSTRAINT_RANGE_GENERATOR_FC_D(unsigned_long_long,unsigned long long,unsigned long long,true,_scv_random_unsigned_ll);

_SCV_CONSTRAINT_RANGE_GENERATOR_FC_D(sc_unsigned,sc_unsigned ,sc_unsigned,true,_scv_random_unsigned_big);
_SCV_CONSTRAINT_RANGE_GENERATOR_FC_D(sc_signed ,sc_signed ,sc_unsigned,true,_scv_random_unsigned_big);





#define _SCV_CONSTRAINT_RANGE_GENERATOR_SIMPLE_FC_D(TypeId, EltT, SizeT, Discrete, FlexRandomT)  \
class _scv_constraint_range_generator_simple_ ## TypeId : public  \
       _scv_constraint_range_generator_base_ ## TypeId {  \
public:  \
  _scv_constraint_range_generator_simple_ ## TypeId(const EltT& lb, const EltT& ub,  \
                            scv_shared_ptr<scv_random> random, const char * nameP) : _scv_constraint_range_generator_base_ ## TypeId  \
  (lb, ub, random, nameP) {}  \
  _scv_constraint_range_generator_simple_ ## TypeId(const _scv_constraint_range_generator_simple_ ## TypeId& rhs,  \
                            const char * nameP) : _scv_constraint_range_generator_base_ ## TypeId(rhs) {}  \
  ~_scv_constraint_range_generator_simple_ ## TypeId() {}  \
public:  \
  void keepOnly(const EltT& v) {  \
    this->_simpleConstraint.keepOnly(v);  \
    this->checkConstraint("keepOnly");  \
  }  \
  void keepOnly(const std::list<EltT>& l) {  \
    this->_simpleConstraint.keepOnly(l);  \
    this->checkConstraint("keepOnly");  \
  }  \
  void keepOnly(const EltT& lb, const EltT& ub) {  \
    this->_simpleConstraint.keepOnly(lb,ub);  \
    this->checkConstraint("keepOnly");  \
  }  \
  void keepOut(const EltT& lb, const EltT& ub){  \
    this->_simpleConstraint.keepOut(lb,ub);  \
    this->checkConstraint("keepOut");  \
  }  \
  void keepOut(const EltT& v) {  \
    if (Discrete)  \
      this->_simpleConstraint.keepOut(v);  \
    else  \
      this->_simpleConstraint.keepOut(v-this->_duplicateDistance, v+this->_duplicateDistance);  \
    this->checkConstraint("keepOut");  \
  }  \
  void keepOut(const std::list<EltT>& l) {  \
    if (Discrete)  \
      this->_simpleConstraint.keepOut(l);  \
    else {  \
      for (std::list<EltT>::const_iterator i = l.begin(); i != l.end(); ++i)  \
        this->_simpleConstraint.keepOut(*i-this->_duplicateDistance, *i+this->_duplicateDistance);  \
    }  \
    this->checkConstraint("keepOut");  \
  }  \
};  \


_SCV_CONSTRAINT_RANGE_GENERATOR_SIMPLE_FC_D(int,int,unsigned,true,_scv_random_unsigned);
_SCV_CONSTRAINT_RANGE_GENERATOR_SIMPLE_FC_D(unsigned,unsigned,unsigned,true,_scv_random_unsigned);
_SCV_CONSTRAINT_RANGE_GENERATOR_SIMPLE_FC_D(double,double,double,false,_scv_random_double);

_SCV_CONSTRAINT_RANGE_GENERATOR_SIMPLE_FC_D(long_long,long long,unsigned long long,true,_scv_random_unsigned_ll);
_SCV_CONSTRAINT_RANGE_GENERATOR_SIMPLE_FC_D(unsigned_long_long,unsigned long long,unsigned long long,true,_scv_random_unsigned_ll);



// ****************************************
// non-template class definitions for simple value generation with simple constraints
// ****************************************

typedef _scv_constraint_range_generator_simple_int _scv_constraint_range_generator_int;
typedef _scv_constraint_range_generator_simple_unsigned _scv_constraint_range_generator_unsigned;
typedef _scv_constraint_range_generator_simple_double _scv_constraint_range_generator_double;
typedef _scv_constraint_range_generator_simple_long_long _scv_constraint_range_generator_int_ll;
typedef _scv_constraint_range_generator_simple_unsigned_long_long _scv_constraint_range_generator_unsigned_ll;

typedef _scv_constraint_range_generator_base_sc_unsigned _scv_constraint_range_generator_unsigned_big;
typedef _scv_constraint_range_generator_base_sc_signed _scv_constraint_range_generator_signed_big;



#endif
