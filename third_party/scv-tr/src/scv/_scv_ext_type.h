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

  _scv_ext_type.h -- The implementation for the extension component "type".

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

// ----------------------------------------
// specialization for records
// ----------------------------------------
template<typename T>
class scv_extension_type
 : public _SCV_INTROSPECTION_BASE {
public:
  scv_extension_type() {}
  virtual ~scv_extension_type() {}

  virtual scv_extension_type_if::data_type get_type() const { return scv_extension_type_if::RECORD; }

  virtual int get_enum_size() const { return 0; }
  virtual void get_enum_details(std::list<const char *>&, std::list<int>&) const {}
  virtual const char * get_enum_string(int) const { return "_error"; }

  virtual int get_num_fields() const { return this->_get_num_fields(); }
  virtual scv_extensions_if * get_field(unsigned i) { return this->_get_field(i); }
  virtual const scv_extensions_if * get_field(unsigned i) const { return this->_get_field(i); }

  virtual scv_extensions_if * get_pointer() { return 0; }
  virtual const scv_extensions_if * get_pointer() const { return 0; }

  virtual int get_array_size() const { return 0; }
  virtual scv_extensions_if * get_array_elt(int) { return 0; }
  virtual const scv_extensions_if * get_array_elt(int) const { return 0; }

  virtual int get_bitwidth() const {
    std::list<_scv_extension_util*>::const_iterator f;
    int size = 0;
    for (f = this->_fields.begin(); f!= this->_fields.end(); ++f)
      { size += (*f)->get_bitwidth(); }
    return size;
  }

  virtual scv_extensions_if * get_parent() { return this->_parent; }
  virtual const scv_extensions_if * get_parent() const { return this->_parent; }
};

// ----------------------------------------
// specialization for array
// ----------------------------------------
template<typename T, int N>
class scv_extension_type<T[N]>
 : public _SCV_INTROSPECTION_BASE2 {
public:
  scv_extension_type() {}
  virtual ~scv_extension_type() {}

  virtual const char * get_type_name() const {   
    static const char * s = _scv_ext_util_get_name("%s[%d]", 
     scv_extensions<T>().get_type_name(), N);
    return s;
  }

  virtual scv_extension_type_if::data_type get_type() const { return scv_extension_type_if::ARRAY; }

  virtual int get_enum_size() const { return 0; }
  virtual void get_enum_details(std::list<const char *>&, std::list<int>&) const {}
  virtual const char * get_enum_string(int) const { return "_error"; }

  virtual int get_num_fields() const { return 0; }
  virtual scv_extensions_if * get_field(unsigned i) { return 0; }
  virtual const scv_extensions_if * get_field(unsigned i) const { return 0; }

  virtual scv_extensions_if * get_pointer() { return 0; }
  virtual const scv_extensions_if * get_pointer() const { return 0; }

  virtual int get_array_size() const { return N; }
  virtual scv_extensions_if * get_array_elt(int i) { return this->_get_array_elt(i); }
  virtual const scv_extensions_if * get_array_elt(int i) const { return this->_get_array_elt(i); }

  virtual int get_bitwidth() const { return get_array_elt(0)->get_bitwidth() * get_array_size(); }

  virtual scv_extensions_if * get_parent() { return this->_parent; }
  virtual const scv_extensions_if * get_parent() const { return this->_parent; }

};

// ----------------------------------------
// specialization for pointers
// ----------------------------------------
template<typename T>
class scv_extension_type<T*>
 : public _SCV_INTROSPECTION_BASE1 {
public:
  scv_extension_type() {}
  virtual ~scv_extension_type() {}

  virtual const char*  get_type_name() const { 
    static const char* s = _scv_ext_util_get_name("%s*", 
      scv_extensions<T>().get_type_name());
    return s;
  }

  virtual scv_extension_type_if::data_type get_type() const { return scv_extension_type_if::POINTER; }

  virtual int get_enum_size() const { return 0; }
  virtual void get_enum_details(std::list<const char *>&, std::list<int>&) const {}
  virtual const char * get_enum_string(int) const { return "_error"; }

  virtual int get_num_fields() const { return 0; }
  virtual scv_extensions_if * get_field(unsigned i) { return 0; }
  virtual const scv_extensions_if * get_field(unsigned i) const { return 0; }

  virtual scv_extensions_if * get_pointer() { return this->_get_pointer(); }
  virtual const scv_extensions_if * get_pointer() const { return this->_get_pointer(); }

  virtual int get_array_size() const { return 0; }
  virtual scv_extensions_if * get_array_elt(int) { return 0; }
  virtual const scv_extensions_if * get_array_elt(int) const { return 0; }

  virtual int get_bitwidth() const { return sizeof(T*); }

  virtual scv_extensions_if * get_parent() { return this->_parent; }
  virtual const scv_extensions_if * get_parent() const { return this->_parent; }
};

// ----------------------------------------
// specialization for enums
// ----------------------------------------
class _scv_extension_type_enum
  : public _SCV_INTROSPECTION_BASE_ENUM {
public:
  _scv_extension_type_enum() {}
  virtual ~_scv_extension_type_enum() {}

  // implemented in leaf classes
  // virtual const char *get_type_name() const;

  virtual scv_extension_type_if::data_type get_type() const;

  virtual int get_enum_size() const;
  virtual void get_enum_details(std::list<const char *>&, std::list<int>&) const;
  virtual const char * get_enum_string(int) const;

  virtual int get_num_fields() const;
  virtual scv_extensions_if * get_field(unsigned);
  virtual const scv_extensions_if * get_field(unsigned) const;

  virtual scv_extensions_if * get_pointer();
  virtual const scv_extensions_if * get_pointer() const;

  virtual int get_array_size() const;
  virtual scv_extensions_if * get_array_elt(int);
  virtual const scv_extensions_if * get_array_elt(int) const;

  // implemented in leaf classes
  // virtual int get_bitwidth() const;

  virtual scv_extensions_if * get_parent() { return this->_parent; }
  virtual const scv_extensions_if * get_parent() const { return this->_parent; }

public:
  void _set_enum(int e, const char * name) {
    _get_names().push_back(name);
    _get_values().push_back(e);
  }
};



// ----------------------------------------
// specialization for basic types
// ----------------------------------------

#define _SCV_EXT_TYPE_FC_D(type_name,type_id) \
class _scv_extension_type_ ## type_id \
 : public scv_extension_util<type_name> { \
public: \
  _scv_extension_type_ ## type_id() { } \
  virtual ~_scv_extension_type_ ## type_id() { } \
  \
  virtual int get_enum_size() const; \
  virtual void get_enum_details(std::list<const char *>&, std::list<int>&) const; \
  virtual const char * get_enum_string(int) const; \
  virtual int get_num_fields() const; \
  virtual scv_extensions_if * get_field(unsigned); \
  virtual const scv_extensions_if * get_field(unsigned) const; \
  virtual scv_extensions_if * get_pointer(); \
  virtual const scv_extensions_if * get_pointer() const; \
  virtual int get_array_size() const; \
  virtual scv_extensions_if * get_array_elt(int); \
  virtual const scv_extensions_if * get_array_elt(int) const; \
  virtual scv_extensions_if * get_parent(); \
  virtual const scv_extensions_if * get_parent() const; \
  \
  virtual const char* get_type_name() const; \
  virtual scv_extension_type_if::data_type get_type() const; \
  virtual int get_bitwidth() const; \
}; \
\
template<> \
class scv_extension_type<type_name> \
 : public _scv_extension_type_ ## type_id { \
public:  \
  scv_extension_type() {} \
  virtual ~scv_extension_type() {} \
}; \


#define _SCV_EXT_TYPE_1_FC_D(type_name,type_id) \
  _SCV_EXT_TYPE_FC_D(type_name,type_id) \


#define _SCV_EXT_TYPE_N_FC_D(type_name,id) \
template<int N> \
class scv_extension_type<type_name > \
 : public scv_extension_util<type_name > { \
public:  \
  scv_extension_type() {} \
  virtual ~scv_extension_type() {} \
         \
  virtual int get_enum_size() const { return 0; } \
  virtual void get_enum_details(std::list<const char *>&, std::list<int>&) const {} \
  virtual const char * get_enum_string(int) const { return "_error"; } \
  virtual int get_num_fields() const { return 0; } \
  virtual scv_extensions_if * get_field(unsigned) { return 0; } \
  virtual const scv_extensions_if * get_field(unsigned) const { return 0; } \
  virtual scv_extensions_if * get_pointer() { return 0; } \
  virtual const scv_extensions_if * get_pointer() const { return 0; } \
  virtual int get_array_size() const { return 0; } \
  virtual scv_extensions_if * get_array_elt(int) { return 0; } \
  virtual const scv_extensions_if * get_array_elt(int) const { return 0; } \
  virtual scv_extensions_if * get_parent() { return this->_parent; } \
  virtual const scv_extensions_if * get_parent() const { return this->_parent; } \
        \
  virtual const char* get_type_name() const { \
    static const char* s = _scv_ext_util_get_name("%s<%d>",#type_name, N); \
    return s; \
  } \
  virtual scv_extension_type_if::data_type get_type() const { \
    return scv_extensions_if::id; \
  } \
  virtual int get_bitwidth() const { return N; } \
}; \


#define _SCV_EXT_TYPE_D_FC_D(type_name,type_id) \
class _scv_extension_type_ ## type_id \
 : public scv_extension_util<type_name> { \
public: \
  _scv_extension_type_ ## type_id(); \
  virtual ~_scv_extension_type_ ## type_id(); \
  \
  virtual int get_enum_size() const; \
  virtual void get_enum_details(std::list<const char *>&, std::list<int>&) const; \
  virtual const char * get_enum_string(int) const; \
  virtual int get_num_fields() const; \
  virtual scv_extensions_if * get_field(unsigned); \
  virtual const scv_extensions_if * get_field(unsigned) const; \
  virtual scv_extensions_if * get_pointer(); \
  virtual const scv_extensions_if * get_pointer() const; \
  virtual int get_array_size() const; \
  virtual scv_extensions_if * get_array_elt(int); \
  virtual const scv_extensions_if * get_array_elt(int) const; \
  virtual scv_extensions_if * get_parent(); \
  virtual const scv_extensions_if * get_parent() const; \
  \
  virtual const char* get_type_name() const; \
  virtual scv_extension_type_if::data_type get_type() const; \
  virtual int get_bitwidth() const; \
  int _bitwidth; \
}; \
\
template<> \
class scv_extension_type<type_name> \
 : public _scv_extension_type_ ## type_id { \
public:  \
  scv_extension_type() {} \
  virtual ~scv_extension_type() {} \
}; \

_SCV_EXT_TYPE_FC_D(bool,bool);
_SCV_EXT_TYPE_FC_D(char,char);
_SCV_EXT_TYPE_FC_D(short,short);
_SCV_EXT_TYPE_FC_D(int,int);
_SCV_EXT_TYPE_FC_D(long,long);
_SCV_EXT_TYPE_FC_D(long long,long_long);
_SCV_EXT_TYPE_FC_D(unsigned char,unsigned_char);
_SCV_EXT_TYPE_FC_D(unsigned short,unsigned_short);
_SCV_EXT_TYPE_FC_D(unsigned int,unsigned_int);
_SCV_EXT_TYPE_FC_D(unsigned long,unsigned_long);
_SCV_EXT_TYPE_FC_D(unsigned long long,unsigned_long_long);
_SCV_EXT_TYPE_FC_D(float,float);
_SCV_EXT_TYPE_FC_D(double,double);
_SCV_EXT_TYPE_FC_D(std::string,string);

#ifdef TEST_NEST_TEMPLATE
_SCV_EXT_TYPE_N_FC_D(test_uint<N>,UNSIGNED);
#endif

#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
_SCV_EXT_TYPE_N_FC_D(sc_int<N>,INTEGER);
_SCV_EXT_TYPE_N_FC_D(sc_bigint<N>,INTEGER);
_SCV_EXT_TYPE_N_FC_D(sc_uint<N>,UNSIGNED);
_SCV_EXT_TYPE_N_FC_D(sc_biguint<N>,UNSIGNED);
_SCV_EXT_TYPE_1_FC_D(sc_bit,sc_bit);
_SCV_EXT_TYPE_N_FC_D(sc_bv<N>,BIT_VECTOR);
_SCV_EXT_TYPE_1_FC_D(sc_logic,sc_logic);
_SCV_EXT_TYPE_N_FC_D(sc_lv<N>,LOGIC_VECTOR);
// SCV_EXT_TYPE_N_FC_D(sc_fixed,FIXED_POINTER_INTEGER);
// SCV_EXT_TYPE_N_FC_D(sc_ufixed,UNSIGNED_FIXED_POINTER_INTEGER);
_SCV_EXT_TYPE_D_FC_D(sc_signed,sc_signed);
_SCV_EXT_TYPE_D_FC_D(sc_unsigned,sc_unsigned);
_SCV_EXT_TYPE_D_FC_D(sc_int_base,sc_int_base);
_SCV_EXT_TYPE_D_FC_D(sc_uint_base,sc_uint_base);
_SCV_EXT_TYPE_D_FC_D(sc_lv_base,sc_lv_base);
_SCV_EXT_TYPE_D_FC_D(sc_bv_base,sc_bv_base);
#endif

#undef _SCV_EXT_TYPE_FC_D
#undef _SCV_EXT_TYPE_N_FC_D
#undef _SCV_EXT_TYPE_1_FC_D


// ----------------------------------------
// wrap up this component
// ----------------------------------------
#undef _SCV_INTROSPECTION_BASE
#define _SCV_INTROSPECTION_BASE scv_extension_type<T>
#undef _SCV_INTROSPECTION_BASE1
#define _SCV_INTROSPECTION_BASE1 scv_extension_type<T*>
#undef _SCV_INTROSPECTION_BASE2
#define _SCV_INTROSPECTION_BASE2 scv_extension_type<T[N]>

#undef _SCV_INTROSPECTION_BASE_ENUM
#define _SCV_INTROSPECTION_BASE_ENUM _scv_extension_type_enum
