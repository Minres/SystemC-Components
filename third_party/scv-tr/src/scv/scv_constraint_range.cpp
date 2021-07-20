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

  scv_constraint_range.cpp

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

#include "scv/scv_util.h"
#include "scv/scv_introspection.h"
#include "scv/scv_report.h"


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

#define _SCV_INTERVAL_FC_I(TypeId, EltT, SizeT, Discrete)  \
  _scv_interval_ ## TypeId& _scv_interval_ ## TypeId::operator=(const _scv_interval_ ## TypeId& rhs) {   \
    _empty = rhs._empty;  \
    _lowerbound = rhs._lowerbound;  \
    _upperbound = rhs._upperbound;  \
    /* don't need to copy _tmp as it is used as a temporary variable */ \
    return *this;  \
  }  \
  bool operator==(const _scv_interval_ ## TypeId& a,  \
			 const _scv_interval_ ## TypeId& b) {    \
    if (a._empty && b._empty) return true;  \
    if (!a._empty && !b._empty  \
	&& a._lowerbound == b._lowerbound  \
	&& a._upperbound == b._upperbound) return true;  \
    return false;  \
  }  \
  bool operator<(const _scv_interval_ ## TypeId& a,  \
			 const _scv_interval_ ## TypeId& b) {    \
    if (b._empty) return false;  \
    if (a._empty) return true;  \
    if (a._lowerbound < b._lowerbound  \
	|| (a._lowerbound == b._lowerbound  \
	    && a._upperbound < b._upperbound)) return true;  \
    return false;  \
  }  \
  void _scv_interval_ ## TypeId::print(ostream& os) const {  \
    if (_empty) os << "()";  \
    else if (Discrete) {  \
      if (_lowerbound != _upperbound)  \
	os << "[" << _lowerbound << "," << _upperbound << "]";  \
      else  \
	os << "[" << _lowerbound << "]";  \
    } else   \
      os << "[" << _lowerbound << "," << _upperbound << ")";  \
  }  \
  ostream& operator<<(ostream& os, const _scv_interval_ ## TypeId& a) {  \
    if (a._empty) os << "()";  \
    else if (Discrete) {  \
      if (a._lowerbound != a._upperbound)  \
	os << "[" << a._lowerbound << "," << a._upperbound << "]";  \
      else  \
	os << "[" << a._lowerbound << "]";  \
    } else   \
      os << "[" << a._lowerbound << "," << a._upperbound << ")";  \
    return os;  \
  }  \
  SizeT _scv_interval_ ## TypeId::size() const {  \
    if (_empty) {  \
      _tmp = 0;  \
    } else {  \
      _tmp = _upperbound - _lowerbound;  \
      if (Discrete) _tmp += 1;  \
    }  \
    return _tmp;  \
  }  \
  int _scv_interval_ ## TypeId::position(const EltT& v) const {  \
    if (_empty) return 1;  \
    if (v<_lowerbound) return -1;  \
    if (v>_upperbound) return 1;  \
    if (!Discrete && v== _upperbound) return 1;  \
    return 0;  \
  }  \
  bool _scv_interval_ ## TypeId::contain(const _scv_interval_ ## TypeId& i) const {  \
    if (_empty) return false;  \
    if (i._empty) return true;  \
    return i._upperbound <= _upperbound && i._lowerbound >= _lowerbound;  \
  }  \
  bool overlap(const _scv_interval_ ## TypeId& a,  \
		      const _scv_interval_ ## TypeId& b) {  \
    if (a._empty || b._empty) return false;  \
    if (Discrete) {  \
      if (a._lowerbound > b._upperbound) return false;  \
      if (a._upperbound < b._lowerbound) return false;  \
    } else {  \
      if (a._lowerbound >= b._upperbound) return false;  \
      if (a._upperbound <= b._lowerbound) return false;  \
    }  \
    return true;  \
  }  \
  void _scv_interval_ ## TypeId::intersect(const _scv_interval_ ## TypeId& rhs) {  \
    if (!overlap(*this,rhs)) { _empty = true; return ; }  \
    if (_lowerbound < rhs._lowerbound) _lowerbound = rhs._lowerbound;  \
    if (_upperbound > rhs._upperbound) _upperbound = rhs._upperbound;  \
  }  \
  bool _scv_interval_ ## TypeId::subtractable(const _scv_interval_ ## TypeId& rhs) const {  \
    return _empty || rhs._empty  \
      || !( rhs._lowerbound > _lowerbound   \
	    &&  rhs._upperbound < _upperbound);  \
  }  \
  void _scv_interval_ ## TypeId::subtract(const _scv_interval_ ## TypeId& rhs) {  \
    if (_empty || rhs._empty) return;  \
    if (!subtractable(rhs)) {  \
      _scv_message::message(_scv_message::CONSTRAINT_ERROR_INTERNAL,"interval subtraction");  \
    }  \
    if (Discrete) {  \
      if (rhs._lowerbound <= _lowerbound && _lowerbound <= rhs._upperbound) {  \
	_lowerbound = rhs._upperbound+1;  \
	if (_lowerbound < rhs._upperbound) _empty = true; /* overflow */  \
      }  \
      if (rhs._lowerbound <= _upperbound && _upperbound <= rhs._upperbound) {  \
	_upperbound = rhs._lowerbound-1;  \
	if (rhs._lowerbound < _upperbound) _empty = true; /* underflow */  \
      }  \
    } else {  \
      if (rhs._lowerbound <= _lowerbound && _lowerbound <= rhs._upperbound)  \
	_lowerbound = rhs._upperbound;  \
      if (rhs._lowerbound <= _upperbound && _upperbound <= rhs._upperbound)  \
	_upperbound = rhs._lowerbound;  \
    }  \
    if (Discrete) {  \
      if (_upperbound < _lowerbound) _empty = true;  \
    } else {  \
      if (_upperbound <= _lowerbound) _empty = true;  \
    }  \
  }  \
  bool mergeable(const _scv_interval_ ## TypeId& a,  \
			const _scv_interval_ ## TypeId& b) {  \
    if (a._empty || b._empty) return true;  \
    if (Discrete) {  \
      if ((a._upperbound < b._lowerbound && a._upperbound+1 < b._lowerbound)  \
	  || (b._upperbound < a._lowerbound && b._upperbound+1 < a._lowerbound))  \
	return false;  \
    } else {  \
      if (a._upperbound < b._lowerbound  \
	  || b._upperbound < a._lowerbound) return false;  \
    }  \
    return true;  \
  }  \
  void _scv_interval_ ## TypeId::merge(const _scv_interval_ ## TypeId& rhs) {  \
    if (rhs._empty) return;  \
    if (_empty) { operator=(rhs); return; }  \
    if (!mergeable(*this,rhs)) {  \
      _scv_message::message(_scv_message::CONSTRAINT_ERROR_INTERNAL,"interval merging");  \
    }  \
    if (_lowerbound > rhs._lowerbound) _lowerbound = rhs._lowerbound;  \
    if (_upperbound < rhs._upperbound) _upperbound = rhs._upperbound;  \
  }  \



_SCV_INTERVAL_FC_I(int,int,unsigned,true);
_SCV_INTERVAL_FC_I(unsigned,unsigned,unsigned,true);
_SCV_INTERVAL_FC_I(double,double,double,false);

_SCV_INTERVAL_FC_I(long_long,long long,unsigned long long,true);
_SCV_INTERVAL_FC_I(unsigned_long_long,unsigned long long,unsigned long long,true);

_SCV_INTERVAL_FC_I(sc_unsigned,sc_unsigned ,sc_unsigned,true);
_SCV_INTERVAL_FC_I(sc_signed ,sc_signed ,sc_unsigned,true);



// ****************************************
// _scv_constraint_range class
//
// * _explicits is only used when 'Discrete' is false.
// * _explicits and _intervals are sorted in assending order.
// * get*() returns 0 with appropriate bits when empty. 
// ****************************************

// ****************************************
// _scv_constraint_range implementation
// ****************************************

#define _SCV_CONSTRAINT_RANGE_FC_I(TypeId, EltT, SizeT, Discrete, FlexRandomT)  \
  ostream& operator<<(ostream& os, const _scv_constraint_range_ ## TypeId& a) {  \
    if (a.isEmpty()) os << "()";  \
    else {  \
      if (a._explicits.empty()) {  \
	if (a.getSize() == 0)  \
	  os << "(size:<unconstrainted>) ";  \
	else  \
	  os << "(size:" << a.getSize() << ") ";  \
	std::list<_scv_interval_ ## TypeId >::const_iterator i;  \
	i = a._intervals.begin();  \
	/* using print() instead of operator<< because gcc 3.0 has problem with -O2. */  \
	i->print(os);  \
	while (++i != a._intervals.end()) {  \
	  os << ","; i->print(os);  \
	}  \
      } else {  \
	std::list<EltT>::const_iterator i;  \
	i = a._explicits.begin();  \
	os << "[" << *i << "]";  \
	while (++i != a._explicits.end())  \
	  os << ",[" << *i << "]";  \
      }  \
    }  \
    return os;  \
  }  \
  \
  SizeT _scv_constraint_range_ ## TypeId::getSize() const {   \
    if (!_explicits.empty()) {  \
      _scv_message::message(_scv_message::CONSTRAINT_ERROR_INTERNAL,"interval size");  \
    }  \
    if (!_sizeValid)   \
      setSize();  \
    return _size;  \
  }  \
  void _scv_constraint_range_ ## TypeId::setSize() const {  \
    _sizeValid = true; _size = 0;  \
    std::list<_scv_interval_ ## TypeId >::const_iterator i;  \
    for (i = _intervals.begin(); i != _intervals.end(); ++i) {  \
      _size += i->size();  \
    }  \
  }  \
  \
  void _scv_constraint_range_ ## TypeId::checkExplicits() {  \
    if (_explicits.empty()) {  \
      _intervals.clear();  \
      _mode = EMPTY;  \
      _sizeValid = false;  \
      _size = 0;  \
      _intervals.push_front(_scv_interval_ ## TypeId(_size,_size));  \
      emptyMessage();  \
    }  \
    else {  \
      _intervals.push_front(_scv_interval_ ## TypeId(_explicits.front(),  \
							       _explicits.back()));  \
    }  \
  }  \
  void _scv_constraint_range_ ## TypeId::checkIntervals() {  \
    if (_intervals.empty()) {  \
      _mode = EMPTY;  \
      _sizeValid = false;  \
      _size = 0;  \
      _intervals.push_front(_scv_interval_ ## TypeId(_size,_size));  \
      emptyMessage();  \
    }  \
  }  \
  \
  void _scv_constraint_range_ ## TypeId::emptyMessage() {  \
    if (_nameP !="" ) {  \
      _scv_message::message(_scv_message::CONSTRAINT_ERROR_OVER_CONSTRAINED,_nameP.c_str());  \
    }  \
  }  \
EltT  \
_scv_constraint_range_ ## TypeId::  \
getRandomValue(scv_shared_ptr<scv_random> random) const {  \
  /* if it is non-empty, generate a random value */  \
  if (!isEmpty()) {  \
    if (isUnconstrainted()) {    \
      return _flexRandom.next(random);   \
    }  \
    if (_explicits.empty()) {  \
      SizeT remain = _flexRandom.next(random,getSize());  \
      std::list<_scv_interval_ ## TypeId >::const_iterator i;  \
      for (i = _intervals.begin(); i != _intervals.end(); ++i) {  \
	if (remain < i->size()) {  \
	  return i->lowerbound() + remain;   \
	}  \
	remain -= i->size();  \
      }  \
    } else {  \
      unsigned int index = random->next() % _explicits.size();  \
      std::list<EltT>::const_iterator i = _explicits.begin();  \
      while (index != 0) {  \
	--index;  \
	++i;  \
      }  \
      return *i;  \
    }  \
  }  \
  \
  /* trying to generate a value from an empty set; trigger an error exception. */  \
  _sizeValid = false; _size = 0; return _size;  \
}  \
  \
EltT  \
_scv_constraint_range_ ## TypeId::  \
getScanValue(const EltT& base, const SizeT& increment) const {  \
    /* if it is non-empty, generate a random value */  \
  if (!isEmpty()) {  \
    EltT nextValue = base + increment;  \
    if (nextValue < base) return base; /* overflow */  \
    if (isUnconstrainted()) { return nextValue; }  \
    if (_explicits.empty()) {  \
      std::list<_scv_interval_ ## TypeId >::const_iterator i;  \
      for (i = _intervals.begin(); i != _intervals.end(); ++i) {  \
	int position = i->position(nextValue);  \
	if (position == 0)  \
	  return nextValue; /* common case: need to be fast */  \
	else if (position < 0) {  \
	  if (Discrete) {  \
	    SizeT d = i->lowerbound() - nextValue;  \
	    if (_flexRandom.mod(d,increment) == 0)  \
	      return i->lowerbound();  \
	    nextValue = nextValue + (d / increment) * increment + increment;  \
	    if (nextValue < base) return base; /* overflow */  \
	    if (nextValue <= i->upperbound())  \
	      return nextValue;  \
	  } else {  \
	    /*	    while (nextValue < i->lowerbound())  */  \
	    /*	      nextValue += increment; */  \
	    /*	    if (i->position(nextValue) == 0) return nextValue; */  \
	    SizeT d = (i->lowerbound() - nextValue)/increment;  \
	    if (d < 1) {  \
	      nextValue += increment;  \
	      if (i->position(nextValue)==0) return nextValue;  \
	    } else {  \
	      nextValue += _flexRandom.floor((i->lowerbound() - nextValue)/increment) * increment;  \
	      if (i->position(nextValue)==0) return nextValue;  \
	      nextValue += increment;  \
	      if (i->position(nextValue)==0) return nextValue;  \
	    }  \
	  }  \
	}  \
      }  \
      return base;  \
    } else {  \
      std::list<EltT>::const_iterator i;  \
      for (i = _explicits.begin(); i != _explicits.end(); ++i) {  \
	if (nextValue == *i) return *i;  \
	if (nextValue < *i && _flexRandom.mod(*i-nextValue,increment) == 0) return *i;  \
      }  \
      return base;  \
    }  \
  }  \
  \
  /* trying to generate a value from an empty set; trigger an error exception. */  \
  _sizeValid = false; _size = 0; return _size;  \
}  \
  \
_scv_constraint_range_ ## TypeId::_scv_constraint_range_ ## TypeId()   \
  : _mode(EMPTY), _nameP(""), _intervals(), _explicits(),  \
    _flexRandom(), _tmpUb(0), _tmpLb(0), _sizeValid(false), _size(0) {  \
  _intervals.push_front(_scv_interval_ ## TypeId((EltT) 0, (EltT) 0));  \
}   \
  \
_scv_constraint_range_ ## TypeId::  \
_scv_constraint_range_ ## TypeId(const EltT& sampleElt)  \
  : _mode(EMPTY), _nameP(""), _intervals(), _explicits(),  \
    _flexRandom(sampleElt), _tmpUb(sampleElt), _tmpLb(sampleElt), _sizeValid(false), _size(sampleElt) {  \
}  \
  \
_scv_constraint_range_ ## TypeId::  \
_scv_constraint_range_ ## TypeId(const EltT& lb, const EltT& ub)  \
  : _mode(INTERVAL_LIST), _nameP(""), _intervals(), _explicits(),  \
    _flexRandom(lb), _tmpUb(lb), _tmpLb(lb), _sizeValid(false), _size(lb) {  \
  _intervals.push_front(_scv_interval_ ## TypeId(lb,ub));  \
}  \
  \
_scv_constraint_range_ ## TypeId::  \
_scv_constraint_range_ ## TypeId(const _scv_constraint_range_ ## TypeId& rhs)  \
  : _mode(rhs._mode),  \
    _nameP(rhs._nameP),  \
    _intervals(rhs._intervals),  \
    _explicits(rhs._explicits),  \
    _flexRandom(rhs._flexRandom),  \
    _tmpUb(rhs._tmpUb),  \
    _tmpLb(rhs._tmpLb),  \
    _sizeValid(rhs._sizeValid),  \
    _size(rhs._size)  \
{}  \
  \
_scv_constraint_range_ ## TypeId&  \
_scv_constraint_range_ ## TypeId::  \
operator=(const _scv_constraint_range_ ## TypeId& rhs) {  \
  _mode = rhs._mode;   \
  _nameP = rhs._nameP;  \
  _intervals = rhs._intervals;  \
  _explicits = rhs._explicits;  \
  /* don't need to copy _flexRandom as it only contains configuration information */  \
  _sizeValid = rhs._sizeValid;  \
  _size = rhs._size;  \
  return *this;  \
}  \
  \
_scv_constraint_range_ ## TypeId  \
_scv_constraint_range_ ## TypeId::  \
merge(const _scv_constraint_range_ ## TypeId& a,  \
      const _scv_constraint_range_ ## TypeId& b) {  \
  /* simple cases */  \
  if (a.isUnconstrainted()) return b;  \
  if (b.isUnconstrainted()) return a;  \
  if (a.isEmpty()) return a;  \
  if (b.isEmpty()) return b;  \
  \
  /* special cases */  \
  if (!a._explicits.empty()) {  \
    std::list<EltT> common;  \
    std::list<EltT>::const_iterator i = a._explicits.begin();  \
    while (i != a._explicits.end()) {  \
      if (b.satisfy(*i)) common.push_back(*i);  \
      ++i;  \
    }  \
    _scv_constraint_range_ ## TypeId c(a.getLowerBound());  \
    if (!common.empty()) {  \
      c._mode = INTERVAL_LIST;  \
      c._explicits = common;  \
      c._intervals.push_front  \
	(_scv_interval_ ## TypeId(c._explicits.front(),  \
				c._explicits.back()));  \
    }  \
  \
    return c;  \
  }  \
  if (!b._explicits.empty()) return merge(b,a);  \
  \
  /* common cases */  \
  _scv_constraint_range_ ## TypeId m(a.getLowerBound());  \
  m._mode = INTERVAL_LIST;  \
  std::list<_scv_interval_ ## TypeId >::const_iterator i,j;  \
  i = a._intervals.begin();  \
  j = b._intervals.begin();  \
  while (i != a._intervals.end() && j != b._intervals.end()) {  \
    if (overlap(*i,*j)) {  \
      _scv_interval_ ## TypeId tmp = *i;  \
      tmp.intersect(*j);  \
      m._intervals.push_back(tmp);  \
    }  \
    if (i->upperbound() < j->upperbound()) ++i;  \
    else ++j;  \
  }  \
  m.checkIntervals();  \
  return m;  \
}  \
  \
void _scv_constraint_range_ ## TypeId::  \
keepOnly(const EltT& lb, const EltT& ub) {  \
  _tmpLb = lb;  \
  _tmpUb = ub;  \
  /* simple cases */  \
  if (isEmpty()) return;  \
  if (isUnconstrainted()) {  \
    _sizeValid = false;  \
    _intervals.clear();  \
    _intervals.push_front(_scv_interval_ ## TypeId(_tmpLb,_tmpUb));  \
    return;  \
  }  \
  \
  /* setup */  \
  _sizeValid = false;  \
  _scv_interval_ ## TypeId interval(_tmpLb,_tmpUb);  \
  \
  /* special cases */  \
  if (!_explicits.empty()) {  \
    std::list<EltT>::iterator i = _explicits.begin();  \
    while (i != _explicits.end()) {  \
      int position = interval.position(*i);  \
      if (position != 0)  \
	i = _explicits.erase(i);  \
      else   \
	++i;  \
    }  \
    checkExplicits();  \
    return;  \
  }  \
  \
  std::list<_scv_interval_ ## TypeId >::iterator i;  \
  i = _intervals.begin();  \
  while (i != _intervals.end()) {  \
    if (overlap(*i,interval)) {  \
      _scv_interval_ ## TypeId tmp = *i;  \
      tmp.intersect(interval);  \
      _intervals.insert(i,tmp);  \
      i = _intervals.erase(i);  \
    } else {  \
      i = _intervals.erase(i);  \
    }  \
  }  \
  checkIntervals();  \
  return;  \
}  \
  \
void _scv_constraint_range_ ## TypeId::keepOnly(const EltT& v) {  \
  if (Discrete)  \
    keepOnly(v,v);  \
  else {  \
    if (satisfy(v)) {  \
      _intervals.clear();  \
      _explicits.clear();  \
      _explicits.push_front(v);  \
      _intervals.push_front(_scv_interval_ ## TypeId(v,v));  \
    } else {  \
      _intervals.clear();  \
      _explicits.clear();  \
      _mode = EMPTY;  \
      emptyMessage();  \
    }  \
  }  \
}  \
  \
void _scv_constraint_range_ ## TypeId::keepOnly(const std::list<EltT>& l) {  \
  /* simple cases */  \
  if (isEmpty()) return;  \
  \
  /* convert to a list of legal values */  \
  std::list<EltT> l2;  \
  for (std::list<EltT>::const_iterator i = l.begin();  \
       i != l.end();  \
       ++i) {  \
    if (satisfy(*i))  \
      l2.push_back(*i);  \
  }  \
  if (l2.empty()) {  \
    _intervals.clear();  \
    checkIntervals();  \
    return;  \
  }  \
  /* common cases */  \
  _intervals.clear();  \
  _sizeValid = false;  \
  if (Discrete) {  \
    for (std::list<EltT>::iterator j = l2.begin();  \
	 j != l2.end();  \
	 ++j) {  \
      std::list< _scv_interval_ ## TypeId >::iterator k;  \
      for (k = _intervals.begin(); k != _intervals.end(); ++k) {  \
	if (k->lowerbound() > *j) {  \
	  _tmpLb = *j;  \
	  _intervals.insert(k,_scv_interval_ ## TypeId(_tmpLb,_tmpLb));  \
	  break;  \
	}  \
      }  \
      if (k == _intervals.end()) {  \
	_tmpLb = *j;  \
	_intervals.push_back(_scv_interval_ ## TypeId(_tmpLb,_tmpLb));  \
      }  \
    }  \
    checkIntervals();  \
  } else {  \
    _explicits.clear();  \
    for (std::list<EltT>::iterator j = l2.begin();  \
	 j != l2.end();  \
	 ++j) {  \
      std::list<EltT>::iterator k;  \
      for (k = _explicits.begin(); k != _explicits.end(); ++k) {  \
	if (*j < *k) {  \
	  _explicits.insert(k,*j);  \
	  break;  \
	}   \
	if (*j == *k) /* remove duplicate */  \
	  break;  \
      }  \
      if (k == _explicits.end())  \
	_explicits.push_back(*j);  \
    }  \
    checkExplicits();  \
  }  \
}  \
  \
void _scv_constraint_range_ ## TypeId::  \
keepOut(const EltT& lb, const EltT& ub) {  \
  _tmpLb = lb;  \
  _tmpUb = ub;  \
  \
  /* simple cases */  \
  if (isEmpty()) return;  \
  \
  /* setup */  \
  _sizeValid = false;  \
  _scv_interval_ ## TypeId interval(_tmpLb,_tmpUb);  \
  \
  /* special cases */  \
  if (!_explicits.empty()) {  \
    std::list<EltT>::iterator i = _explicits.begin();  \
    while (i != _explicits.end()) {  \
      int position = interval.position(*i);  \
      if (position == 0)  \
	i = _explicits.erase(i);  \
      else   \
	++i;  \
    }  \
    checkExplicits();  \
    return;  \
  }  \
  \
  /* common cases */  \
  std::list<_scv_interval_ ## TypeId >::iterator i;  \
  i = _intervals.begin();  \
  while (i != _intervals.end()) {  \
    if (i->contain(interval)) {  \
      if (Discrete) {  \
	if (i->lowerbound() < interval.lowerbound()) {  \
	  _tmpUb = interval.lowerbound()-1;  \
	  _intervals.insert  \
	    (i, _scv_interval_ ## TypeId(i->lowerbound(),_tmpUb));  \
	}  \
	if (i->upperbound() > interval.upperbound()) {  \
	  _tmpLb = interval.upperbound()+1;  \
	  _intervals.insert  \
	    (i,_scv_interval_ ## TypeId(_tmpLb,i->upperbound()));  \
	}  \
      } else {  \
	if (i->lowerbound() < interval.lowerbound())  \
	  _intervals.insert  \
	    (i,_scv_interval_ ## TypeId(i->lowerbound(),  \
						  interval.lowerbound()));  \
	if (i->upperbound() > interval.upperbound())  \
	  _intervals.insert  \
	    (i,_scv_interval_ ## TypeId(interval.upperbound(),  \
						  i->upperbound()));  \
      }  \
      i = _intervals.erase(i);  \
    } else {  \
      if (i->subtractable(interval))  \
	i->subtract(interval);  \
      if (i->empty())   \
	i = _intervals.erase(i);  \
      else  \
	++i;  \
    }  \
  }  \
  checkIntervals();  \
}  \
  \
void _scv_constraint_range_ ## TypeId::keepOut(const EltT& v) {  \
  if (!Discrete) {  \
    _scv_message::message(_scv_message::CONSTRAINT_ERROR_INTERNAL,"non-discrete keepOut");  \
  }  \
  keepOut(v,v);  \
}  \
  \
void _scv_constraint_range_ ## TypeId::  \
keepOut(const std::list<EltT>& l) {  \
  if (!Discrete) {  \
    _scv_message::message(_scv_message::CONSTRAINT_ERROR_INTERNAL,"non-discrete keepOut");  \
  }  \
  for (std::list<EltT>::const_iterator i = l.begin(); i != l.end(); ++i)  \
    keepOut(*i);  \
}  \
  \
bool _scv_constraint_range_ ## TypeId::  \
satisfy(const EltT& v) const {  \
  \
  if (isEmpty()) return false;  \
  if (_explicits.empty()) {  \
    std::list<_scv_interval_ ## TypeId >::const_iterator i;  \
    for (i = _intervals.begin(); i != _intervals.end(); ++i) {  \
      int j = i->position(v);  \
      if (j== 0) return true;  \
      if (j < 0) return false;  \
    }  \
  } else {  \
    std::list<EltT>::const_iterator i;  \
    for (i = _explicits.begin(); i != _explicits.end(); ++i) {  \
      if (*i == v) return true;  \
      /* assuming that the explicit list is typically short,  */  \
      /* we avoid doing potentially expensive < comparison here. */  \
    }  \
  }  \
  return false;  \
}  \




_SCV_CONSTRAINT_RANGE_FC_I(int,int,unsigned,true,_scv_random_unsigned);
_SCV_CONSTRAINT_RANGE_FC_I(unsigned,unsigned,unsigned,true,_scv_random_unsigned);
_SCV_CONSTRAINT_RANGE_FC_I(double,double,double,false,_scv_random_double);

_SCV_CONSTRAINT_RANGE_FC_I(long_long,long long,unsigned long long,true,_scv_random_unsigned_ll);
_SCV_CONSTRAINT_RANGE_FC_I(unsigned_long_long,unsigned long long,unsigned long long,true,_scv_random_unsigned_ll);

_SCV_CONSTRAINT_RANGE_FC_I(sc_unsigned,sc_unsigned ,sc_unsigned,true,_scv_random_unsigned_big);
_SCV_CONSTRAINT_RANGE_FC_I(sc_signed ,sc_signed ,sc_unsigned,true,_scv_random_unsigned_big);




// ****************************************
// value generation algorithms with _scv_constraint_range 
// ****************************************

#define _SCV_CONSTRAINT_RANGE_GENERATOR_FC_I(TypeId, EltT, SizeT, Discrete, FlexRandomT)  \
  void _scv_constraint_range_generator_base_ ## TypeId::print(ostream& os, const char * prefixP) const {  \
    std::string prefix = "";  \
    if (prefixP) prefix = prefixP;  \
    os << prefix << "legal values : " << _simpleConstraint << endl;  \
    if (_onGoingConstraintValid)  \
      os << prefix << "legal values to avoid duplicate : " << _onGoingConstraint << endl;  \
    if (_currentScanValueValid)  \
      os << prefix << "scan status : last value generated is " << _currentScanValue << endl;  \
    if (_scanFixedIncrement)  \
      os << prefix << "scan interval : " << _scanIncrement << endl;  \
    else   \
      os << prefix << "scan interval : "   \
	 << *_scanIntervalGenP << endl;  \
    os << prefix << "duplicate distance : " << _duplicateDistance << endl;  \
    if (_distributionP && !_distributionP->empty()) {  \
      os << prefix << "distribution : " << endl;  \
      os << prefix << *_distributionP << endl;  \
    }  \
  }  \
  void _scv_constraint_range_generator_base_ ## TypeId::checkConstraint(const char * locationP) {  \
    reset();  \
    if (isEmpty())  \
      _scv_constraint_range_error::overConstraint(_nameP, locationP);  \
    if (_distributionP && !_distributionP->empty()) {  \
      _setWeightDistribution(*_distributionP,_distUseMarking);  \
    }  \
  }  \
EltT _scv_constraint_range_generator_base_ ## TypeId::  \
randomNext() const {  \
  if (isEmpty())  \
    _scv_constraint_range_error::emptyGenerator(_nameP);  \
  return _simpleConstraint.getRandomValue(_randomP);  \
}  \
  \
EltT _scv_constraint_range_generator_base_ ## TypeId::  \
distributionNext() const {  \
  if (_distributionP && !_distributionP->empty()) {  \
    if (_distributionP->unMarkedSize()==0) _distributionP->unmarkAll();  \
    return _distributionP->peekRandom(_distUseMarking);  \
  }  \
  _scv_constraint_range_error::invalidDistribution(_nameP,"next");  \
  return randomNext();  \
}  \
  \
EltT _scv_constraint_range_generator_base_ ## TypeId::  \
randomAvoidDuplicateNext() const {  \
  if (isEmpty())  \
    _scv_constraint_range_error::emptyGenerator(_nameP);  \
  if (!_onGoingConstraintValid) {  \
    _onGoingConstraint = _simpleConstraint;  \
    _onGoingConstraintValid = true;  \
  }  \
  EltT newValue =   \
    _onGoingConstraint.getRandomValue(_randomP);  \
  if (Discrete)  \
    _onGoingConstraint.keepOut(newValue);  \
  else  \
    _onGoingConstraint.keepOut(newValue-_duplicateDistance,  \
			       newValue+_duplicateDistance);   \
  if (_onGoingConstraint.isEmpty())  \
    _onGoingConstraintValid = false;  \
  return newValue;  \
}  \
  \
EltT _scv_constraint_range_generator_base_ ## TypeId::  \
scanNext() const {  \
  if (isEmpty())  \
    _scv_constraint_range_error::emptyGenerator(_nameP);  \
  if (!_onGoingConstraintValid) {  \
    _onGoingConstraint = _simpleConstraint;  \
    _onGoingConstraintValid = true;  \
  }  \
  if (!_currentScanValueValid) {  \
    _currentScanValue = _onGoingConstraint.getLowerBound();  \
    _currentScanValueValid = true;;  \
    return _currentScanValue;  \
  }  \
  \
  if (!_scanFixedIncrement)  \
    _scanIncrement = _scanIntervalGenP->getRandomValue(_randomP);  \
  \
  EltT nextValue  \
    = _onGoingConstraint.getScanValue(_currentScanValue,  \
				      _scanIncrement);  \
  if (nextValue == _currentScanValue) {  \
    _currentScanValueValid = false;  \
    _onGoingConstraintValid = false;  \
    return scanNext();  \
  }   \
  _currentScanValue = nextValue;  \
  return _currentScanValue;  \
}  \
  \
_scv_constraint_range_generator_base_ ## TypeId::  \
_scv_constraint_range_generator_base_ ## TypeId(const EltT& lb, const EltT& ub,  \
			  scv_shared_ptr<scv_random> random, const char * nameP)  \
  : _nameP(nameP),  \
    _simpleConstraint(lb,ub),  \
    _onGoingConstraintValid(false),  \
    _onGoingConstraint(lb,ub),  \
    _currentScanValueValid(false),  \
    _currentScanValue(lb),  \
    _randomP(random),  \
    _distUseMarking(false),  \
    _distributionP(NULL),  \
    _duplicateDistance(lb),  \
    _scanIntervalGenP(NULL),  \
    _scanFixedIncrement(true),  \
    _scanIncrement(lb)  \
{  \
  _duplicateDistance = 1;  \
  _scanIncrement = 1;  \
}  \
  \
_scv_constraint_range_generator_base_ ## TypeId::  \
_scv_constraint_range_generator_base_ ## TypeId(const _scv_constraint_range_generator_base_ ## TypeId& rhs,  \
			  const char * nameP)  \
  : _nameP(nameP),  \
    _simpleConstraint(rhs._simpleConstraint),  \
    _onGoingConstraintValid(rhs._onGoingConstraintValid),  \
    _onGoingConstraint(rhs._onGoingConstraint),  \
    _currentScanValueValid(rhs._currentScanValueValid),  \
    _currentScanValue(rhs._currentScanValue),  \
    _randomP(rhs._randomP),  \
    _distUseMarking(rhs._distUseMarking),  \
    _distributionP(NULL),  \
    _duplicateDistance(rhs._duplicateDistance),  \
    _scanIntervalGenP(NULL),  \
    _scanFixedIncrement(rhs._scanFixedIncrement),  \
    _scanIncrement(rhs._scanIncrement)  \
{  \
  if (rhs._distributionP) {  \
    _distributionP = new scv_bag<EltT>(*rhs._distributionP);  \
    _distributionP->setRandom(*_randomP);  \
  }  \
  if (rhs._scanIntervalGenP)   \
    _scanIntervalGenP = new _scv_constraint_range_ ## TypeId(*rhs._scanIntervalGenP);  \
}  \
  \
void _scv_constraint_range_generator_base_ ## TypeId::  \
setScanInterval(const SizeT& lb, const SizeT& ub) {  \
  if (_scanIntervalGenP) {  \
    delete _scanIntervalGenP;  \
    _scanIntervalGenP = NULL;  \
  }  \
  if (lb==0)   \
    _scv_constraint_range_error::invalidScanInterval(_nameP);  \
  if (ub==0 || ub<=lb) {  \
    _scanFixedIncrement = true;  \
    _scanIncrement = lb;  \
  } else {  \
    _scanFixedIncrement = false;  \
    _scanIntervalGenP = new _scv_constraint_range_ ## TypeId(lb,ub);  \
  }  \
}  \
  \
void _scv_constraint_range_generator_base_ ## TypeId::  \
setWeightDistribution(const scv_bag<EltT>& bag, bool useMarking) {  \
  _setWeightDistribution(bag,useMarking);  \
  if (_distributionP->empty()) {  \
    _scv_constraint_range_error::invalidDistribution(_nameP,"setWeightDistribution");  \
  }  \
}  \
  \
void _scv_constraint_range_generator_base_ ## TypeId::  \
_setWeightDistribution(const scv_bag<EltT>& bag, bool useMarking) {  \
  _distUseMarking = useMarking;  \
  if (_distributionP)  \
    *_distributionP = bag;  \
  else {  \
    _distributionP = new scv_bag<EltT>(bag);  \
    _distributionP->setRandom(*_randomP);  \
  }  \
  unsigned int size = _distributionP->distinctSize();  \
  for (unsigned int i=0; i<size; ++i) {  \
    EltT j = _distributionP->peekNext('a',true);  \
    if (!satisfyConstraints(j))  \
      _distributionP->remove(true);  \
  }  \
}  \



_SCV_CONSTRAINT_RANGE_GENERATOR_FC_I(int,int,unsigned,true,_scv_random_unsigned);
_SCV_CONSTRAINT_RANGE_GENERATOR_FC_I(unsigned,unsigned,unsigned,true,_scv_random_unsigned);
_SCV_CONSTRAINT_RANGE_GENERATOR_FC_I(double,double,double,false,_scv_random_double);

_SCV_CONSTRAINT_RANGE_GENERATOR_FC_I(long_long,long long,unsigned long long,true,_scv_random_unsigned_ll);
_SCV_CONSTRAINT_RANGE_GENERATOR_FC_I(unsigned_long_long,unsigned long long,unsigned long long,true,_scv_random_unsigned_ll);

_SCV_CONSTRAINT_RANGE_GENERATOR_FC_I(sc_unsigned,sc_unsigned ,sc_unsigned,true,_scv_random_unsigned_big);
_SCV_CONSTRAINT_RANGE_GENERATOR_FC_I(sc_signed ,sc_signed ,sc_unsigned,true,_scv_random_unsigned_big);

