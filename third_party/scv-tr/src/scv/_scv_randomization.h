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

  _scv_randomization -- The implementation for randomization to supplement
  extension "rand".

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

#include "scv/scv_shared_ptr.h"
#include "scv/scv_constraint_range.h"

#include <cfloat>

class scv_extensions_if;
class scv_constraint_base;
template <class T> class scv_bag;
template <class T> class scv_ext_rand_base;
class _scv_extension_rand_enum;
extern scv_constraint_base * _scv_new_constraint(scv_constraint_base * c);
extern void _scv_delete_constraint(scv_constraint_base * c);
extern void _scv_assign_enum_value(scv_extensions_if* e, _scv_constraint_data* cdata_, int e1_val, int e2_val);

scv_extensions_if * _scv_find_extension(scv_constraint_base * c, scv_extensions_if* e);

///////////////////////////////////////////////////////////
// Create single scv_random object for composite type.
///////////////////////////////////////////////////////////
#define GET_RANDOM() \
  static bool dummy = create_random(this); \
  if (0) cout << dummy << endl;

inline bool create_random(scv_extensions_if* e) {
  e->get_random();
  return true;
}

class _scv_constraint_data {
public:
  enum gen_mode {
    NO_CONSTRAINT,
    RANGE_CONSTRAINT,
    CONSTRAINT,
    EXTENSION,
    DISTRIBUTION,
    DISTRIBUTION_RANGE
  };
private:
  scv_constraint_base* constr_;
  scv_extensions_if* extv_;
  gen_mode mode_;
  scv_extensions_if::mode_t ext_mode_;
  scv_shared_ptr<scv_random> gen_;
public: // scan interval information used by _scv_constraint_manager
  union {
    int int_;
    unsigned int unsigned_;
    double double_; 
  } prev_val_;
  int lb_scan_;
  int ub_scan_;
public:
  enum gen_type {IGEN, ILGEN, IBGEN, UGEN, ULGEN, UBGEN, DGEN, EMPTY};
private:
  union {
    _scv_constraint_range_generator_int * igen;
    _scv_constraint_range_generator_int_ll * i_ll_gen;
    _scv_constraint_range_generator_signed_big * i_big_gen;
    _scv_constraint_range_generator_unsigned * ugen;
    _scv_constraint_range_generator_unsigned_ll * u_ll_gen;
    _scv_constraint_range_generator_unsigned_big * u_big_gen;
    _scv_constraint_range_generator_double * dgen;
  } _range_gen;
  gen_type _gen_type;
public:
  _scv_constraint_data();
  _scv_constraint_data(const _scv_constraint_data& rhs); 
  ~_scv_constraint_data(); 
public:
  scv_extensions_if* get_extension() {return extv_;}
  scv_constraint_base* get_constraint() {return constr_;}
  void set_generator_from(scv_extensions_if* to, scv_extensions_if* from) ;
  scv_shared_ptr<scv_random> get_random(scv_extensions_if* e); 
  gen_mode get_mode() {return mode_;}
  scv_extensions_if::mode_t get_ext_mode() {return ext_mode_;}
  void set_extension(scv_extensions_if* e) {extv_ = e;}
  void set_constraint(scv_constraint_base* c) {constr_ = c;}
  void set_random(scv_shared_ptr<scv_random> g) {gen_ = g;}
  void set_mode(gen_mode m) {mode_ = m;}
  void set_gen_type(gen_type t) {_gen_type = t;} 
  void reset_distribution(scv_extensions_if* s);
  gen_type get_gen_type() { return _gen_type; } 
  void set_ext_mode(scv_extensions_if::mode_t m, int lb=0, int ub=0 ); 
  _scv_constraint_range_generator_unsigned* get_unsigned_generator(scv_extensions_if* s); 
  _scv_constraint_range_generator_unsigned_ll* get_unsigned_ll_generator(scv_extensions_if* s);
  _scv_constraint_range_generator_int* get_int_generator(scv_extensions_if* s); 
  _scv_constraint_range_generator_int_ll* get_int_ll_generator(scv_extensions_if* s); 
  _scv_constraint_range_generator_unsigned_big* get_unsigned_big_generator(scv_extensions_if* s); 
  _scv_constraint_range_generator_signed_big* get_signed_big_generator(scv_extensions_if* s) ;
  _scv_constraint_range_generator_double* get_double_generator(scv_extensions_if* s); 
  bool is_complex_constraint() { return (mode_ == CONSTRAINT); } 
  bool is_range_constraint() { return (mode_ == RANGE_CONSTRAINT); }
  bool is_no_constraint() { return (mode_ == NO_CONSTRAINT); }
  bool is_distribution_constraint() {
    return (mode_ == DISTRIBUTION || mode_ == DISTRIBUTION_RANGE);
  }
  bool is_random_mode() { return (ext_mode_ == scv_extensions_if::RANDOM); }
  bool is_scan_mode() { return (ext_mode_ == scv_extensions_if::SCAN); }
  bool is_avoid_duplicate_mode() { 
    return (ext_mode_ == scv_extensions_if::RANDOM_AVOID_DUPLICATE); 
  }
};

void _scv_set_value(scv_extensions_if* e, _scv_constraint_data * cdata_);
void _scv_set_value(scv_extensions_if* e, scv_constraint_base* c, scv_shared_ptr<scv_random> g);

inline _scv_constraint_data* _get_constraint_data_enum(_scv_extension_rand_enum* data);
inline void _set_mode_enum(_scv_extension_rand_enum* data, _scv_constraint_data::gen_mode m);
inline scv_shared_ptr<scv_random> _get_random_enum(_scv_extension_rand_enum* data);

template <typename T> 
class _scv_distribution_base {
public:
  scv_bag<T>* dist_;
  scv_bag<std::pair<T, T> >* dist_r_;
public:
  _scv_distribution_base() : dist_(NULL), dist_r_(NULL) {}
  virtual ~_scv_distribution_base() {
    if (dist_)  delete dist_;
    if (dist_r_) delete dist_r_;
  }
public:
  virtual scv_bag<T>* get_distribution() { return dist_; }
  virtual scv_bag<std::pair<T,T> >* get_distribution_range() { return dist_r_; }
  virtual void generate_value_(scv_extensions_if * data,
		       _scv_constraint_data * cdata_) = 0; 

  // For all generic types except enumeration
  virtual void set_mode(scv_bag< std::pair<T,T> >& d, 
			_scv_constraint_data * constraint_data,
			scv_extensions_if * data) { 
    if (dist_r_) *dist_r_ = d;
    else {
      dist_r_ = new scv_bag< std::pair<T,T> >(d);
      dist_r_->setRandom(*(constraint_data->get_random(data)));
    }
    constraint_data->set_mode(_scv_constraint_data::DISTRIBUTION_RANGE);
    constraint_data->set_ext_mode(scv_extensions_if::DISTRIBUTION);
    _reset_distribution();
  }
  virtual void set_mode(scv_bag<T>& d,
			_scv_constraint_data * constraint_data,
			scv_extensions_if * data) { 
    if (dist_) *dist_ = d;
    else {
      dist_ = new scv_bag<T>(d);
      dist_->setRandom(*(constraint_data->get_random(data)));
    }
    constraint_data->set_mode(_scv_constraint_data::DISTRIBUTION);
    constraint_data->set_ext_mode(scv_extensions_if::DISTRIBUTION);
    _reset_distribution_range();
  }

  // For enumeration type
  virtual void set_mode(scv_bag< std::pair<T,T> >& d, 
                _scv_extension_rand_enum * data) {
    if (dist_r_) *dist_r_ = d;
    else {
      dist_r_ = new scv_bag< std::pair<T,T> >(d);
      dist_r_->setRandom(*_get_random_enum(data));
    }
    _set_mode_enum(data, _scv_constraint_data::DISTRIBUTION_RANGE);
    _reset_distribution();
  }
  virtual void set_mode(scv_bag<T>& d,
                _scv_extension_rand_enum * data) {
    if (dist_) *dist_ = d;
    else {
      dist_ = new scv_bag<T>(d);
      dist_->setRandom(*_get_random_enum(data));
    }
    _set_mode_enum(data, _scv_constraint_data::DISTRIBUTION);
    _reset_distribution_range();
  }
  virtual void reset_distribution() {
    _reset_distribution();
    _reset_distribution_range();
  }
  void _reset_distribution() {
    if (dist_) {
      delete dist_;
      dist_ = NULL;
    }
  }
  void _reset_distribution_range() {
    if (dist_r_) {
      delete dist_r_;
      dist_r_ = NULL;
    }
  }
};

//////////////////////////////////////////////////////////////////
// Forward declaration of routines required for generate_value_
//////////////////////////////////////////////////////////////////

template <typename T> 
void generate_value_distribution(scv_extensions_if *,
  scv_bag<T>*);
template <typename T> 
void generate_value_distribution_range(scv_extensions_if*,
  scv_bag<std::pair<T, T> >*, _scv_constraint_data*);
void generate_value_no_constraint(scv_extensions_if*,
  _scv_constraint_data*);
void generate_value_extension(scv_extensions_if*, 
  _scv_constraint_data*);
void generate_value_range_constraint(scv_extensions_if*, 
  _scv_constraint_data*);

//////////////////////////////////////////////////////////////////
// For all generic types <= 64 bits
//////////////////////////////////////////////////////////////////
template<typename T>
class _scv_distribution : public _scv_distribution_base<T> {
public:
  void generate_value_(scv_extensions_if * data,
		       _scv_constraint_data * cdata_) {
    switch(cdata_->get_mode()) {
    case _scv_constraint_data::DISTRIBUTION : {
      generate_value_distribution(data, this->get_distribution());
      break;
    }
    case _scv_constraint_data::DISTRIBUTION_RANGE : {
      generate_value_distribution_range(data, 
        this->get_distribution_range(), cdata_);
      break;
    }
    case _scv_constraint_data::NO_CONSTRAINT: {
      generate_value_no_constraint(data, cdata_); 
      break;
    }
    case _scv_constraint_data::RANGE_CONSTRAINT: {   
      generate_value_range_constraint(data, cdata_); 
      break;
    }
    case _scv_constraint_data::EXTENSION: 
    {
      generate_value_extension(data, cdata_);
      break;
    }
    case _scv_constraint_data::CONSTRAINT:
      _scv_set_value(data, cdata_);
      break;
    default:
      _scv_message::message(_scv_message::INTERNAL_ERROR, 
        "illegal randomization type");
      break;
    }
  }
};

///////////////////////////////////////////////////////////////////
// Forward declaration for solving distribution for types with
// bitwidth >= 64 bits
///////////////////////////////////////////////////////////////////

template <typename T>
void generate_value_distribution_bigvalue(scv_extensions_if*, 
  scv_bag<T>*);
template <typename T>
void generate_value_distribution_range_bigvalue(scv_extensions_if*, 
  scv_bag<std::pair<T,T> >*, _scv_constraint_data*, sc_unsigned*);

#define _SCV_DISTRIBUTION(type_name)                        \
template<int N>                                             \
class _scv_distribution <type_name<N> > :                   \
  public _scv_distribution_base<type_name<N> > {            \
public:                                                     \
  void generate_value_(scv_extensions_if * data,            \
		       _scv_constraint_data * cdata_) {     \
    switch(cdata_->get_mode()) {                            \
    case _scv_constraint_data::DISTRIBUTION : {             \
      generate_value_distribution_bigvalue(data,            \
        this->get_distribution());                          \
      break;                                                \
    }                                                       \
    case _scv_constraint_data::DISTRIBUTION_RANGE : {       \
      sc_biguint<N> big_num;                                \
      generate_value_distribution_range_bigvalue(data,      \
        this->get_distribution_range(), cdata_, &big_num);  \
      break;                                                \
    }                                                       \
    case _scv_constraint_data::NO_CONSTRAINT:               \
    case _scv_constraint_data::CONSTRAINT: {                \
      _scv_set_value(data, cdata_);                         \
      break;                                                \
    }                                                       \
    case _scv_constraint_data::RANGE_CONSTRAINT: {          \
      generate_value_range_constraint(data, cdata_);        \
      break;                                                \
    }                                                       \
    case _scv_constraint_data::EXTENSION: {                 \
      scv_extensions_if *e = cdata_->get_extension();       \
      _scv_constraint_data *cd = e->get_constraint_data();  \
      _scv_set_value(e, cd->get_constraint(),               \
        cdata_->get_random(data));                          \
      data->assign(e->get_unsigned());                      \
      break;                                                \
    }                                                       \
    default:                                                \
      _scv_message::message(_scv_message::INTERNAL_ERROR,   \
        "illegal randomization type");                      \
      break;                                                \
    }                                                       \
  }                                                         \
}

_SCV_DISTRIBUTION(sc_biguint);
_SCV_DISTRIBUTION(sc_bigint);

_SCV_DISTRIBUTION(sc_lv);
_SCV_DISTRIBUTION(sc_bv);

//////////////////////////////////////////////////////////////////////////
// Template methods to generate values  (bitwidth <= 64 bits)
//   - generate_value_distribution
//   - generate_value_distribution_range
//
// Non Template methods to obtain values (bitwidth <= 64 bits)
//   - generate_vlaue_no_constraint
//   - generate_value_extension
//////////////////////////////////////////////////////////////////////////
template <typename T>
void generate_value_distribution(scv_extensions_if * data,
  scv_bag<T>* dist)
{
  switch (data->get_type()) {
  case scv_extensions_if::ENUMERATION: 
  case scv_extensions_if::INTEGER : 
  case scv_extensions_if::BOOLEAN: {
    const scv_extensions<T> e = scv_get_const_extensions(
                                 dist->peekRandom());
    if (data->get_bitwidth() <= 64) {
      data->assign(e.get_integer());
    } else {
      sc_bv_base val(data->get_bitwidth());
      e.get_value(val);
      data->assign(val);
    }
    break;
  }
  case scv_extensions_if::UNSIGNED :
  case scv_extensions_if::BIT_VECTOR :
  case scv_extensions_if::LOGIC_VECTOR : {
    const scv_extensions<T> e = scv_get_const_extensions(
                                 dist->peekRandom());
    if (data->get_bitwidth() <= 64) {
      data->assign(e.get_unsigned());
    } else {
      sc_bv_base val(data->get_bitwidth());
      e.get_value(val);
      data->assign(val);
    }
    break;
  }
  case scv_extensions_if::FLOATING_POINT_NUMBER: {
    const scv_extensions<T> e = scv_get_const_extensions(
                                 dist->peekRandom());
    data->assign(e.get_double());
    break;
  }
  default: 
    break;
  }
}

template <typename T>
void generate_value_distribution_range(scv_extensions_if* data,
  scv_bag<std::pair<T, T> >* dist_range, _scv_constraint_data* cdata_)
{
  std::pair<T, T> p = dist_range->peekRandom();
  switch(data->get_type()) {
    case scv_extensions_if::INTEGER : 
    case scv_extensions_if::BOOLEAN : {
      const scv_extensions<T> e1 = scv_get_const_extensions(p.first);
      const scv_extensions<T> e2 = scv_get_const_extensions(p.second);
      if (data->get_bitwidth() <= 32) {
        int e1_val = (int) e1.get_integer();
        int e2_val = (int) e2.get_integer();

        if (e2_val < e1_val) {
          _scv_message::message(_scv_message::CONSTRAINT_INVALID_RANGE, data->get_name());
          data->assign(e2_val);
        } else {
          int size = e2_val - e1_val + 1;
          int val = e1_val + (cdata_->get_random(data)->next() % size);
          data->assign(val);
        }
      } else if (data->get_bitwidth() <=64) {
        long long e1_val = e1.get_unsigned();
        long long e2_val = e2.get_unsigned();
 
        if (e2_val < e1_val) {
          _scv_message::message(_scv_message::CONSTRAINT_INVALID_RANGE, data->get_name());
          data->assign(e2_val);
        } else {
          unsigned long long size = e2_val - e1_val + 1;
          unsigned long long big_num = cdata_->get_random(data)->next();
          big_num = big_num << 31;
          big_num |= (cdata_->get_random(data)->next());

          long long val = e1_val + (big_num % size);
          data->assign(val);          
        }
      } else {
        static bool flag = false;
        if (!flag) {
          _scv_message::message(_scv_message::INTERNAL_ERROR, 
            "distribution range with bitwidth > 64 bits. This message will be printed only once.");
          flag = true;
        }
      }
      break;
    }
    case scv_extensions_if::UNSIGNED :
    case scv_extensions_if::BIT_VECTOR :
    case scv_extensions_if::LOGIC_VECTOR : {
      const scv_extensions<T> e1 = scv_get_const_extensions(p.first);
      const scv_extensions<T> e2 = scv_get_const_extensions(p.second);
      if (data->get_bitwidth() <= 32) {
        unsigned e1_val = (unsigned) e1.get_unsigned();
        unsigned e2_val = (unsigned) e2.get_unsigned();

        if (e2_val < e1_val) {
          _scv_message::message(_scv_message::CONSTRAINT_INVALID_RANGE, data->get_name());
          data->assign(e2_val);
        } else {
          unsigned size = e2_val - e1_val + 1;
          unsigned val = e1_val + (cdata_->get_random(data)->next() % size);
          data->assign(val);
        }
      } else if (data->get_bitwidth() <= 64) {
        unsigned long long e1_val = e1.get_unsigned();
        unsigned long long e2_val = e2.get_unsigned();

        if (e2_val < e1_val) { 
          _scv_message::message(_scv_message::CONSTRAINT_INVALID_RANGE, data->get_name());
          data->assign(e2_val);
        } else {
          unsigned long long size = e2_val - e1_val + 1;
          unsigned long long big_num = cdata_->get_random(data)->next();
          big_num = big_num << 31;
          big_num |= (cdata_->get_random(data)->next());

          unsigned long long val = e1_val + (big_num % size);
          data->assign(val);            
        }
      } else {
        static bool flag = false;
        if (!flag) {
          _scv_message::message(_scv_message::INTERNAL_ERROR, 
            "distribution range with bitwidth > 64 bits. This message will be printed only once");
          flag = true;
        }
      }
      break;
    }
    case scv_extensions_if::ENUMERATION: {
      const scv_extensions<T> e1 = scv_get_const_extensions(p.first);
      const scv_extensions<T> e2 = scv_get_const_extensions(p.second);
      int e1_val = (int) e1.get_integer();
      int e2_val = (int) e2.get_integer();

      _scv_assign_enum_value(data, cdata_, e1_val, e2_val);
      break;
    }
    default: 
      break;
  }
}

template <typename T>
void generate_value_distribution_bigvalue(scv_extensions_if* data,
  scv_bag<T>* dist)
{
  const T e = dist->peekRandom(); 
  int bitwidth = data->get_bitwidth();

  if (bitwidth <= 64) {
    if (data->is_integer()) {
      sc_signed val(bitwidth);
      val = e;
      data->assign(val.to_int64());
    } else if (data->is_unsigned() ||
               data->is_bit_vector() ||
               data->is_logic_vector()) {
      sc_unsigned val(bitwidth);
      val = e;
      data->assign(val.to_uint64());
    } else  {
      _scv_message::message(_scv_message::INTERNAL_ERROR, "unknown type");
    }
  } else {
    sc_bv_base val(bitwidth);
    val = e;
    data->assign(val);
  }
}

template <typename T>
void generate_value_distribution_range_bigvalue(scv_extensions_if* data,
  scv_bag<std::pair<T, T> >* dist_range, _scv_constraint_data* cdata_, 
  sc_unsigned* big_num)
{
  std::pair<T, T> p = dist_range->peekRandom();
  const T e1_v = p.first;
  const T e2_v = p.second;
  int bitwidth = data->get_bitwidth();

  if (bitwidth <= 32) {
    switch(data->get_type()) {
      case scv_extensions_if::INTEGER: {
        sc_signed e1(bitwidth);
        sc_signed e2(bitwidth);
        e1 = e1_v; e2 = e2_v;
        int e1_val = e1.to_int();
        int e2_val = e2.to_int();
        if (e2_val < e1_val) {
          _scv_message::message(_scv_message::CONSTRAINT_INVALID_RANGE, data->get_name());
          data->assign(e2_val);
        } else {
          unsigned size = e2_val - e1_val + 1;
          int val = e1_val + (cdata_->get_random(data)->next() % size);
          data->assign(val);
        }
        break;
      }
      case scv_extensions_if::BIT_VECTOR: 
      case scv_extensions_if::LOGIC_VECTOR: 
      case scv_extensions_if::UNSIGNED: {
        sc_unsigned e1(bitwidth);
        sc_unsigned e2(bitwidth);
        e1 = e1_v; e2 = e2_v;
        unsigned e1_val = e1.to_uint();
        unsigned e2_val = e2.to_uint();
        if (e2_val < e1_val) {
          _scv_message::message(_scv_message::CONSTRAINT_INVALID_RANGE, data->get_name());
          data->assign(e2_val);
        } else {
          unsigned size = e2_val - e1_val + 1;
          unsigned val = e1_val + (cdata_->get_random(data)->next() % size);
          data->assign(val);
        }
        break;
      }
      default:
        break;  
    }
  } else if (bitwidth <= 64) {
    switch(data->get_type()) {
    case scv_extensions_if::INTEGER: {
      sc_signed e1(bitwidth);
      sc_signed e2(bitwidth);
      e1 = e1_v; e2 = e2_v;
      long long e1_val = e1.to_int64();
      long long e2_val = e2.to_int64();
      if (e2_val < e1_val) {
        _scv_message::message(_scv_message::CONSTRAINT_INVALID_RANGE, data->get_name());
        data->assign(e2_val);
      } else {
        unsigned long long size = e2_val - e1_val + 1;
        unsigned long long bnum = cdata_->get_random(data)->next();
        bnum = bnum << 31;
        bnum |= (cdata_->get_random(data)->next());
        long long val = e1_val + (bnum % size);
        data->assign(val);    
      }
      break;
    }
    case scv_extensions_if::BIT_VECTOR: 
    case scv_extensions_if::LOGIC_VECTOR: 
    case scv_extensions_if::UNSIGNED: {
      sc_unsigned e1(bitwidth);
      sc_unsigned e2(bitwidth);
      e1 = e1_v; e2 = e2_v;
      unsigned long long e1_val = e1.to_uint64();
      unsigned long long e2_val = e2.to_uint64();
      if (e2_val < e1_val) {
        _scv_message::message(_scv_message::CONSTRAINT_INVALID_RANGE, data->get_name());
        data->assign(e2_val);
      } else {
        unsigned long long size = e2_val - e1_val + 1;
        unsigned long long bnum = cdata_->get_random(data)->next();
        bnum = bnum << 31;
        bnum |= (cdata_->get_random(data)->next());
        unsigned long long val = e1_val + (bnum % size);
        data->assign(val);    
      }
      break;
    } 
    default:
      break;
    }        
  } else {
    unsigned remain = bitwidth % 32;
  
    *big_num = cdata_->get_random(data)->next();
    for (int i=0; i < ((bitwidth/32)-1);i++) {
      *big_num = *big_num << 32;
      *big_num |= cdata_->get_random(data)->next();
    }

    *big_num = *big_num << remain;
    unsigned value_left = ((0x1 << remain) - 1);
    *big_num |= (cdata_->get_random(data)->next() & value_left);
  
    sc_bv_base bval(bitwidth);

    if (data->is_unsigned() ||
        data->is_bit_vector() ||
        data->is_logic_vector()) {
      sc_unsigned e1_tmp(bitwidth);
      sc_unsigned e2_tmp(bitwidth);
      sc_unsigned size(bitwidth);
      e1_tmp = e1_v; e2_tmp = e2_v;
      size = (e2_tmp - e1_tmp + 1);
      sc_unsigned val(bitwidth);
      val = e1_tmp + ((*big_num) % size);
      bval = val;
      data->assign(bval);
    } else if (data->is_integer()) {
      sc_signed e1_tmp(bitwidth);
      sc_signed e2_tmp(bitwidth);
      sc_signed size(bitwidth);
      e1_tmp = e1_v; e2_tmp = e2_v;
      size = (e2_tmp - e1_tmp + 1);
      sc_signed val(bitwidth);
      val = e1_tmp + ((*big_num) % size);
      bval = val;
      data->assign(bval);
    }
  }
}

template <typename T>
bool check_mode(scv_extensions_if::mode_t t, 
  scv_extensions_if* e, const std::string& name,
   _scv_distribution<T> * dist)
{
  _scv_constraint_data* cdata_ = e->get_constraint_data();
  if (t == scv_extensions_if::DISTRIBUTION) {
    if (dist->get_distribution()) {
      cdata_->set_mode(_scv_constraint_data::DISTRIBUTION);
    } else if (dist->get_distribution_range()) {
      cdata_->set_mode(_scv_constraint_data::DISTRIBUTION_RANGE);
    } else {
      _scv_message::message(_scv_message::INTROSPECTION_EMPTY_DISTRIBUTION, 
        name.c_str());
      return false;
    }
  } else {
    if (cdata_->get_extension()) { 
      cdata_->set_mode(_scv_constraint_data::EXTENSION);
    } else if (cdata_->get_gen_type() != _scv_constraint_data::EMPTY) {
      cdata_->set_mode(_scv_constraint_data::RANGE_CONSTRAINT);
    } else if (cdata_->get_constraint()) {
      cdata_->set_mode(_scv_constraint_data::CONSTRAINT);
    } else {
      cdata_->set_mode(_scv_constraint_data::NO_CONSTRAINT);
    }
  }
  return true;
}

inline void _scv_rand_util_get_list(scv_extensions_if* e, int lb, int ub, unsigned& mth, unsigned& nth) {
  std::list<int> ilist;
  std::list<const char *> slist; 
  std::list<int>::iterator iter;
  unsigned count = 0;
  e->get_enum_details(slist, ilist);
  for (iter=ilist.begin(); iter != ilist.end(); iter++) {
    if (*iter == lb) {
      nth = count;
    }
    if (*iter == ub) {
      mth  = count;
      break;
    }
    count++;
  }
  if (mth < nth) {
    _scv_message::message(_scv_message::CONSTRAINT_INVALID_RANGE, e->get_name());
    mth = nth;
    return;
  }
}

bool _scv_has_complex_constraint(scv_extensions_if*);

#define _SCV_CHECK_DATA()                               \
  _scv_constraint_data* cd = e->get_constraint_data()

#define _SCV_ONLY_OR_OUT(dummy_values, exclude, gen, lb, ub)            \
  if (!dummy_values) {                                                  \
    if (!exclude)                                                       \
      gen->keepOnly(lb, ub);                                            \
    else                                                                \
      gen->keepOut(lb, ub);                                             \
  }          

#define _SCV_KEEP_RANGE_TYPE(type_name)                                 \
  template <int W>                                                      \
  inline void _scv_keep_range(scv_extensions_if* e, const type_name<W>& lb, \
                              const type_name<W>& ub, bool exclude,     \
                              bool dummy_values = false)

#define _SCV_KEEP_RANGE_TYPE_NO_W(type_name)                            \
  inline void _scv_keep_range(scv_extensions_if* e, const type_name& lb, \
                              const type_name& ub, bool exclude,        \
                              bool dummy_values = false)

#define _SCV_KEEP_RANGE_INT(type_name)                                  \
  _SCV_KEEP_RANGE_TYPE(type_name) {                                     \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_unsigned_big *ugen = NULL;          \
    _scv_constraint_range_generator_signed_big *sgen = NULL;            \
    if (e->is_integer()) {                                              \
      sgen = cd->get_signed_big_generator(e);                           \
      _SCV_ONLY_OR_OUT(dummy_values, exclude, sgen, lb, ub);            \
    } else if (e->is_unsigned()) {                                      \
      ugen = cd->get_unsigned_big_generator(e);                         \
      _SCV_ONLY_OR_OUT(dummy_values, exclude, ugen, lb, ub);            \
    }                                                                   \
    cd->set_mode(_scv_constraint_data::RANGE_CONSTRAINT);               \
  }

#define _SCV_SC_UNSIGNED_VALUE(dummy_values, W, lb, ub)                 \
  sc_unsigned lbv(W), ubv(W);                                           \
  if (dummy_values) { lbv = 0; ubv = 0; }                               \
  else { lbv = lb; ubv = ub; }

#define _SCV_KEEP_RANGE_BIT(type_name)                                  \
  _SCV_KEEP_RANGE_TYPE(type_name) {                                     \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_unsigned_big *ugen = NULL;          \
    ugen = cd->get_unsigned_big_generator(e);                           \
    _SCV_SC_UNSIGNED_VALUE(dummy_values, W, lb, ub);                    \
    _SCV_ONLY_OR_OUT(dummy_values, exclude, ugen, lbv, ubv);            \
    cd->set_mode(_scv_constraint_data::RANGE_CONSTRAINT);               \
  }

#define _SCV_UNSIGNED_VALUE(lb, ub)                                     \
  unsigned lbv, ubv;                                                    \
  lbv = (unsigned)lb.to_bool(); ubv = (unsigned)ub.to_bool()

#define _SCV_KEEP_RANGE_LOGIC(type_name)                                \
  _SCV_KEEP_RANGE_TYPE_NO_W(type_name) {                                \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_unsigned *ugen = NULL;              \
    ugen = cd->get_unsigned_generator(e);                               \
    _SCV_UNSIGNED_VALUE(lb, ub);                                        \
    _SCV_ONLY_OR_OUT(dummy_values, exclude, ugen, lbv, ubv);            \
    cd->set_mode(_scv_constraint_data::RANGE_CONSTRAINT);               \
  }

#define _SCV_KEEP_RANGE_FLOAT(type_name)                                \
  _SCV_KEEP_RANGE_TYPE_NO_W(type_name) {                                \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_double *dgen = NULL;                \
    dgen = cd->get_double_generator(e);                                 \
    _SCV_ONLY_OR_OUT(dummy_values, exclude, dgen, lb, ub);              \
    cd->set_mode(_scv_constraint_data::RANGE_CONSTRAINT);               \
  }

#define _SCV_KEEP_RANGE_BASE_TYPE(type_name)                            \
  _SCV_KEEP_RANGE_TYPE_NO_W(type_name) {                                \
    _SCV_CHECK_DATA();                                                  \
    if (e->is_integer()) {                                              \
      if (e->get_bitwidth() <= 64) {                                    \
        _scv_constraint_range_generator_int_ll * gen =                  \
          cd->get_int_ll_generator(e);                                  \
        _SCV_ONLY_OR_OUT(dummy_values, exclude, gen, lb.to_int64(),     \
                         ub.to_int64());                                \
      } else {                                                          \
        _scv_message::message(_scv_message::INTERNAL_ERROR,             \
                              "_scv_keep_range (sc_signed), unsupported bitwidth."); \
      }                                                                 \
    } else if (e->is_unsigned()) {                                      \
      if (e->get_bitwidth() <= 64) {                                    \
        _scv_constraint_range_generator_unsigned_ll * gen =             \
          cd->get_unsigned_ll_generator(e);                             \
        _SCV_ONLY_OR_OUT(dummy_values, exclude, gen, lb.to_uint64(),    \
                         ub.to_uint64());                               \
      } else {                                                          \
        _scv_message::message(_scv_message::INTERNAL_ERROR,             \
                              "_scv_keep_range (sc_unsigned), unsupported bitwidth."); \
      }                                                                 \
    }                                                                   \
    cd->set_mode(_scv_constraint_data::RANGE_CONSTRAINT);               \
  }

#define _SCV_KEEP_RANGE_BASE_BIG_TYPE(type_name)                        \
  _SCV_KEEP_RANGE_TYPE_NO_W(type_name) {                                \
    _SCV_CHECK_DATA();                                                  \
    if (e->is_integer()) {                                              \
      _scv_constraint_range_generator_signed_big *gen = NULL;           \
      gen = cd->get_signed_big_generator(e);                            \
      _SCV_ONLY_OR_OUT(dummy_values, exclude, gen, lb, ub);             \
    } else if (e->is_unsigned()) {                                      \
      _scv_constraint_range_generator_unsigned_big *ugen = NULL;        \
      ugen = cd->get_unsigned_big_generator(e);                         \
      _SCV_ONLY_OR_OUT(dummy_values, exclude, ugen, lb, ub);            \
    }                                                                   \
    cd->set_mode(_scv_constraint_data::RANGE_CONSTRAINT);               \
  }

#define _SCV_KEEP_RANGE_BASE_LOGIC_TYPE(type_name)                      \
  _SCV_KEEP_RANGE_TYPE_NO_W(type_name) {                                \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_unsigned_big *ugen = NULL;          \
    ugen = cd->get_unsigned_big_generator(e);                           \
    _SCV_SC_UNSIGNED_VALUE(dummy_values, e->get_bitwidth(), lb, ub);    \
    _SCV_ONLY_OR_OUT(dummy_values, exclude, ugen, lbv, ubv);            \
    cd->set_mode(_scv_constraint_data::RANGE_CONSTRAINT);               \
  }

#define _SCV_KEEP_RANGE_ERROR(type_name)                                \
  _SCV_KEEP_RANGE_TYPE_NO_W(type_name) {                                \
    _scv_message::message(_scv_message::RANDOM_TYPE_NOT_SUPPORTED, #type_name); \
  }

_SCV_KEEP_RANGE_INT(sc_biguint)
_SCV_KEEP_RANGE_INT(sc_bigint)

_SCV_KEEP_RANGE_BIT(sc_bv)
_SCV_KEEP_RANGE_BIT(sc_lv)

_SCV_KEEP_RANGE_LOGIC(sc_bit)
_SCV_KEEP_RANGE_LOGIC(sc_logic)

_SCV_KEEP_RANGE_FLOAT(double)
_SCV_KEEP_RANGE_FLOAT(float)

_SCV_KEEP_RANGE_ERROR(std::string)

_SCV_KEEP_RANGE_BASE_BIG_TYPE(sc_signed)
_SCV_KEEP_RANGE_BASE_BIG_TYPE(sc_unsigned)
_SCV_KEEP_RANGE_BASE_TYPE(sc_int_base)
_SCV_KEEP_RANGE_BASE_TYPE(sc_uint_base)
_SCV_KEEP_RANGE_BASE_LOGIC_TYPE(sc_lv_base)
_SCV_KEEP_RANGE_BASE_LOGIC_TYPE(sc_bv_base)

template <typename T>
inline void _scv_keep_range(scv_extensions_if* e, const T& lb, const T& ub, bool exclude, bool dummy_values=false) {
    _SCV_CHECK_DATA();
    switch(e->get_type()) {
      case scv_extensions_if::BOOLEAN:
      case scv_extensions_if::UNSIGNED: {
        if (e->get_bitwidth() <= 32) {
          _scv_constraint_range_generator_unsigned * gen =
          cd->get_unsigned_generator(e);
          _SCV_ONLY_OR_OUT(dummy_values, exclude, gen, (unsigned)lb, (unsigned)ub)
        } else if (e->get_bitwidth() <= 64) {
          _scv_constraint_range_generator_unsigned_ll * gen =
          cd->get_unsigned_ll_generator(e);
          _SCV_ONLY_OR_OUT(dummy_values, exclude, gen, (unsigned long long)lb, (unsigned long long)ub)
        } else {
          _scv_message::message(_scv_message::INTERNAL_ERROR, "_scv_keep_range (unsigned)");
        }
        cd->set_mode(_scv_constraint_data::RANGE_CONSTRAINT);
        break;
      }
      case scv_extensions_if::INTEGER: {
        if (e->get_bitwidth() <=32) {
          _scv_constraint_range_generator_int * gen =
          cd->get_int_generator(e);
          _SCV_ONLY_OR_OUT(dummy_values, exclude, gen, (int)lb, (int)ub);
        } else if (e->get_bitwidth() <= 64) {
          _scv_constraint_range_generator_int_ll * gen =
          cd->get_int_ll_generator(e);
          _SCV_ONLY_OR_OUT(dummy_values, exclude, gen, (long long)lb, (long long)ub)
        } else {
          _scv_message::message(_scv_message::INTERNAL_ERROR, "_scv_keep_range (signed)");
        }
        cd->set_mode(_scv_constraint_data::RANGE_CONSTRAINT);
        break;
      }
      case scv_extensions_if::ENUMERATION: {
         _scv_constraint_range_generator_unsigned * gen =              
           cd->get_unsigned_generator(e);               
         unsigned nth = 0;
         unsigned mth = 0;
         _scv_rand_util_get_list(e, (int)lb, (int)ub, mth, nth);       
         if (!exclude) {
           gen->keepOnly(nth, mth);                                      
         } else { 
           gen->keepOut(nth, mth);                                      
         }
         cd->set_mode(_scv_constraint_data::RANGE_CONSTRAINT); 
      }
      default:
        break;
    }
}

template <typename T>
inline void _scv_keep_range_list_enum(scv_extensions_if* e, const std::list<T>& vlist, bool exclude) {  
  std::list<int> ilist;
  std::list<const char *> slist;
  std::list<int>::iterator iter;
  unsigned count = 0;
  unsigned nth = 0;
  e->get_enum_details(slist, ilist);

  _scv_constraint_range_generator_unsigned * gen =         
     e->get_constraint_data()->get_unsigned_generator(e); 
  typename std::list<T>::const_iterator i;                
  std::list<unsigned> tlist;

  for (i = vlist.begin(); i != vlist.end(); i++) { 
    count = 0;
    for (iter=ilist.begin(); iter != ilist.end(); iter++) {
      if (*iter == *i) {
        nth = count;
        break;
      }
      count++;
    }
    tlist.push_back((unsigned)nth);               
  }                          
  if (!exclude) {
    gen->keepOnly(tlist);
  } else {
    gen->keepOut(tlist);
  }                    
  e->get_constraint_data()->set_mode(_scv_constraint_data::RANGE_CONSTRAINT);
}

template <typename T1, typename T2>
void _scv_get_list(const std::list<T2>& vlist, std::list<T1>& tlist) { 
  typename std::list<T2>::const_iterator i;                                        
  for (i = vlist.begin(); i != vlist.end(); i++)                             
    tlist.push_back(*i);                                                       
}

template <typename T1, typename T2>
void _scv_get_base_unsigned_list(const std::list<T2>& vlist, std::list<T1>& tlist) { 
  typename std::list<T2>::const_iterator i;                                        
  for (i = vlist.begin(); i != vlist.end(); i++)                             
    tlist.push_back((*i).to_uint64()); 
}

template <typename T1, typename T2>
void _scv_get_base_signed_list(const std::list<T2>& vlist, std::list<T1>& tlist) { 
  typename std::list<T2>::const_iterator i;                                        
  for (i = vlist.begin(); i != vlist.end(); i++)                             
    tlist.push_back((*i).to_int64()); 
}

template <typename T1, typename T2> 
void _scv_get_list_sc(int W, const std::list<T2>& vlist, std::list<T1>& tlist ) { 
  typename std::list<T2>::const_iterator i;                                   
  for (i = vlist.begin(); i != vlist.end(); i++)                        
    tlist.push_back(*i);                                               
}

template <typename T1, typename T2>
void _scv_get_sc_list(int W, const std::list<T2>& vlist, std::list<T1>& tlist) {
  typename std::list<T2>::const_iterator i;
  T1 val(W);                         
  for (i = vlist.begin(); i != vlist.end(); i++) { 
    val = *i;                                     
    tlist.push_back(val);                        
  }                                                                            
}

template <typename T1, typename T2> 
void _scv_get_logic_list(const std::list<T2> &vlist, std::list<T1> &tlist) {
  typename std::list<T2>::const_iterator i;                       
  for (i = vlist.begin(); i != vlist.end(); i++)            
    tlist.push_back((unsigned)((*i).to_bool()));           
}

template <typename T>
inline void _scv_keep_range(scv_extensions_if* e, const std::list<T>& vlist) {
  _scv_message::message(_scv_message::RANDOM_TYPE_NOT_SUPPORTED, e->get_type_name());
}

#define _SCV_KEEP_RANGE_LIST_HEADER(type_name)                          \
  template <>                                                           \
  inline void _scv_keep_range(scv_extensions_if* e, const std::list<type_name>& vlist)

#define _SCV_KEEP_RANGE_SC_HEADER(type_name)                            \
  template <int W>                                                      \
  inline void _scv_keep_range(scv_extensions_if* e, const std::list<type_name<W> >& vlist)

#define _SCV_KEEP_RANGE_SET_LIST(tlist)                                 \
  gen->keepOnly(tlist);                                                 \
  cd->set_mode(_scv_constraint_data::RANGE_CONSTRAINT)

#define _SCV_KEEP_RANGE_LIST_INT_TYPE(type_name)                        \
  _SCV_KEEP_RANGE_LIST_HEADER(type_name) {                              \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_int * gen =                         \
      cd->get_int_generator(e);                                         \
    std::list<int> tlist;                                               \
    _scv_get_list(vlist, tlist);                                        \
    _SCV_KEEP_RANGE_SET_LIST(tlist);                                    \
  }

#define _SCV_KEEP_RANGE_LIST_INT64_TYPE(type_name)                      \
  _SCV_KEEP_RANGE_LIST_HEADER(type_name) {                              \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_int_ll * gen =                      \
      cd->get_int_ll_generator(e);                                      \
    std::list<long long> tlist;                                         \
    _scv_get_list(vlist, tlist);                                        \
    _SCV_KEEP_RANGE_SET_LIST(tlist);                                    \
  }

#define _SCV_KEEP_RANGE_LIST_SC_INT_TYPE(type_name)                     \
  _SCV_KEEP_RANGE_SC_HEADER(type_name) {                                \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_int_ll * gen =                      \
      cd->get_int_ll_generator(e);                                      \
    std::list<long long> tlist;                                         \
    _scv_get_list_sc(W, vlist, tlist);                                  \
    _SCV_KEEP_RANGE_SET_LIST(tlist);                                    \
  }

#define _SCV_KEEP_RANGE_LIST_UNSIGNED_TYPE(type_name)                   \
  _SCV_KEEP_RANGE_LIST_HEADER(type_name) {                              \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_unsigned * gen =                    \
      cd->get_unsigned_generator(e);                                    \
    std::list<unsigned> tlist;                                          \
    _scv_get_list(vlist, tlist);                                        \
    _SCV_KEEP_RANGE_SET_LIST(tlist);                                    \
  }

#define _SCV_KEEP_RANGE_LIST_UNSIGNED64_TYPE(type_name)                 \
  _SCV_KEEP_RANGE_LIST_HEADER(type_name) {                              \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_unsigned_ll * gen =                 \
      cd->get_unsigned_ll_generator(e);                                 \
    std::list<unsigned long long> tlist;                                \
    _scv_get_list(vlist, tlist);                                        \
    _SCV_KEEP_RANGE_SET_LIST(tlist);                                    \
  }

#define _SCV_KEEP_RANGE_LIST_SC_UINT_TYPE(type_name)                    \
  _SCV_KEEP_RANGE_SC_HEADER(type_name) {                                \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_unsigned_ll * gen =                 \
      cd->get_unsigned_ll_generator(e);                                 \
    std::list<unsigned long long> tlist;                                \
    _scv_get_list_sc(W, vlist, tlist);                                  \
    _SCV_KEEP_RANGE_SET_LIST(tlist);                                    \
  }

#define _SCV_KEEP_RANGE_LIST_FLOAT_TYPE(type_name)                      \
  _SCV_KEEP_RANGE_LIST_HEADER(type_name) {                              \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_double * gen =                      \
      cd->get_double_generator(e);                                      \
    std::list<double> tlist;                                            \
    _scv_get_list(vlist, tlist);                                        \
    _SCV_KEEP_RANGE_SET_LIST(tlist);                                    \
  }

#define _SCV_KEEP_RANGE_LIST_SC_SIGNED_TYPE(type_name)                  \
  _SCV_KEEP_RANGE_SC_HEADER(type_name) {                                \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_signed_big * gen =                  \
      cd->get_signed_big_generator(e);                                  \
    std::list<sc_signed> tlist;                                         \
    _scv_get_sc_list(W, vlist, tlist);                                  \
    _SCV_KEEP_RANGE_SET_LIST(tlist);                                    \
  }

#define _SCV_KEEP_RANGE_LIST_SC_UNSIGNED_TYPE(type_name)                \
  _SCV_KEEP_RANGE_SC_HEADER(type_name) {                                \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_unsigned_big * gen =                \
      cd->get_unsigned_big_generator(e);                                \
    std::list<sc_unsigned> tlist;                                       \
    _scv_get_sc_list(e->get_bitwidth(), vlist, tlist);                  \
    _SCV_KEEP_RANGE_SET_LIST(tlist);                                    \
  }

#define _SCV_KEEP_RANGE_LIST_BASE_UNSIGNED_TYPE(type_name)              \
  _SCV_KEEP_RANGE_LIST_HEADER(type_name) {                              \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_unsigned_ll * gen =                 \
      cd->get_unsigned_ll_generator(e);                                 \
    std::list<unsigned long long> tlist;                                \
    _scv_get_base_unsigned_list(vlist, tlist);                          \
    _SCV_KEEP_RANGE_SET_LIST(tlist);                                    \
  }

#define _SCV_KEEP_RANGE_LIST_BASE_SIGNED_TYPE(type_name)                \
  _SCV_KEEP_RANGE_LIST_HEADER(type_name) {                              \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_int_ll * gen =                      \
      cd->get_int_ll_generator(e);                                      \
    std::list<long long> tlist;                                         \
    _scv_get_base_signed_list(vlist, tlist);                            \
    _SCV_KEEP_RANGE_SET_LIST(tlist);                                    \
  }

#define _SCV_KEEP_RANGE_LIST_BASE_SC_SIGNED_TYPE(type_name)             \
  _SCV_KEEP_RANGE_LIST_HEADER(type_name) {                              \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_signed_big * gen =                  \
      cd->get_signed_big_generator(e);                                  \
    std::list<sc_signed> tlist;                                         \
    _scv_get_sc_list(e->get_bitwidth(), vlist, tlist);                  \
    _SCV_KEEP_RANGE_SET_LIST(tlist);                                    \
  }

#define _SCV_KEEP_RANGE_LIST_BASE_SC_UNSIGNED_TYPE(type_name)           \
  _SCV_KEEP_RANGE_LIST_HEADER(type_name) {                              \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_unsigned_big * gen =                \
      cd->get_unsigned_big_generator(e);                                \
    std::list<sc_unsigned> tlist;                                       \
    _scv_get_sc_list(e->get_bitwidth(), vlist, tlist);                  \
    _SCV_KEEP_RANGE_SET_LIST(tlist);                                    \
  }

#define _SCV_KEEP_RANGE_LIST_LOGIC_TYPE(type_name)                      \
  _SCV_KEEP_RANGE_LIST_HEADER(type_name) {                              \
    _SCV_CHECK_DATA();                                                  \
    _scv_constraint_range_generator_unsigned * gen =                    \
      cd->get_unsigned_generator(e);                                    \
    std::list<unsigned> tlist;                                          \
    _scv_get_logic_list(vlist, tlist);                                  \
    _SCV_KEEP_RANGE_SET_LIST(tlist);                                    \
  }

_SCV_KEEP_RANGE_LIST_INT_TYPE(char)
_SCV_KEEP_RANGE_LIST_INT_TYPE(short)
_SCV_KEEP_RANGE_LIST_INT_TYPE(int)
_SCV_KEEP_RANGE_LIST_INT_TYPE(long)
_SCV_KEEP_RANGE_LIST_INT64_TYPE(long long)
_SCV_KEEP_RANGE_LIST_SC_INT_TYPE(sc_int)                 

_SCV_KEEP_RANGE_LIST_UNSIGNED_TYPE(bool)
_SCV_KEEP_RANGE_LIST_UNSIGNED_TYPE(unsigned char)
_SCV_KEEP_RANGE_LIST_UNSIGNED_TYPE(unsigned short)
_SCV_KEEP_RANGE_LIST_UNSIGNED_TYPE(unsigned int)
_SCV_KEEP_RANGE_LIST_UNSIGNED_TYPE(unsigned long)
_SCV_KEEP_RANGE_LIST_UNSIGNED64_TYPE(unsigned long long)
_SCV_KEEP_RANGE_LIST_SC_UINT_TYPE(sc_uint)                  

_SCV_KEEP_RANGE_LIST_SC_UNSIGNED_TYPE(sc_biguint)
_SCV_KEEP_RANGE_LIST_SC_UNSIGNED_TYPE(sc_lv)
_SCV_KEEP_RANGE_LIST_SC_UNSIGNED_TYPE(sc_bv)

_SCV_KEEP_RANGE_LIST_SC_SIGNED_TYPE(sc_bigint)

_SCV_KEEP_RANGE_LIST_FLOAT_TYPE(double)
_SCV_KEEP_RANGE_LIST_FLOAT_TYPE(float)

_SCV_KEEP_RANGE_LIST_LOGIC_TYPE(sc_logic)
_SCV_KEEP_RANGE_LIST_LOGIC_TYPE(sc_bit)

_SCV_KEEP_RANGE_LIST_BASE_SC_SIGNED_TYPE(sc_signed)
_SCV_KEEP_RANGE_LIST_BASE_SC_UNSIGNED_TYPE(sc_unsigned)
_SCV_KEEP_RANGE_LIST_BASE_SIGNED_TYPE(sc_int_base)
_SCV_KEEP_RANGE_LIST_BASE_UNSIGNED_TYPE(sc_uint_base)
_SCV_KEEP_RANGE_LIST_BASE_SC_UNSIGNED_TYPE(sc_lv_base)
_SCV_KEEP_RANGE_LIST_BASE_SC_UNSIGNED_TYPE(sc_bv_base)
