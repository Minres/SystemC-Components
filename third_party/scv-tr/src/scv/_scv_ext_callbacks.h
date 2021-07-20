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

  _scv_ext_callbacks.h -- The implementation for the extension "callbacks".

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

inline static int s_add_callback(_scv_dynamic_data * data,
				 _scv_dynamic_data::callback_base * c) {
  if (!data) { 
    _scv_message::message(_scv_message::INTROSPECTION_INVALID_DYNAMIC_EXTENSIONS,"callbacks");
    return -1;
  }
  c->set_id(data->_next_id++);
  data->_callbacks.push_back(c);
  return c->get_id();
}

inline static void s_remove_callback(_scv_dynamic_data * data,
				 scv_extensions_if::callback_h id) {
  if(id !=-1) {
    if (!data) { 
      _scv_message::message(_scv_message::INTROSPECTION_INVALID_DYNAMIC_EXTENSIONS,"callbacks");
      return;
    }

    std::list<scv_extensions_if::callback_base*>::iterator i = data->_callbacks.begin();
    std::list<scv_extensions_if::callback_base*>::iterator e = data->_callbacks.end();
    while (i != e) {
      if ((*i)->get_id() == id) {
	data->_callbacks.erase(i);
	return;
      } else ++i;
    }
  }
  _scv_message::message(_scv_message::INTROSPECTION_BAD_CALLBACK_REMOVAL);
}

inline static   std::list<scv_extensions_if::callback_base*>::iterator 
s_select_callback(_scv_dynamic_data * data,
				 scv_extensions_if::callback_h id) {
  if (!data) { 
    _scv_message::message(_scv_message::INTROSPECTION_INVALID_DYNAMIC_EXTENSIONS,"callbacks");
    return std::list<scv_extensions_if::callback_base*>::iterator();
  }
 
  std::list<scv_extensions_if::callback_base*>::iterator i = data->_callbacks.begin();
  std::list<scv_extensions_if::callback_base*>::iterator e = data->_callbacks.end();
  while (i != e) {
    if ((*i)->get_id() == id) {
      return i;
    } else ++i;
  }
  return e;
}

// ----------------------------------------
// specialization for records
// ----------------------------------------
template<typename T>
class _scv_extension_callbacks_base
 : public _SCV_INTROSPECTION_BASE {
  // this class also for basic types
public:
  _scv_extension_callbacks_base() {}
  virtual ~_scv_extension_callbacks_base() {
    if (this->_has_dynamic_data() && !this->get_parent())
	this->_get_dynamic_data()->execute_callbacks(this, scv_extensions_if::DELETE);
  }

  virtual scv_extensions_if::callback_h _register_cb(scv_extensions_if::callback_base * c) {
    return s_add_callback(this->_get_dynamic_data(),c); 
  }
  virtual void remove_cb(scv_extensions_if::callback_h id) { 
    s_remove_callback(this->_get_dynamic_data(),id); 
  }

  // void trigger_value_change_cb();
  // this is implemented as non-virtual in extension "util" for efficiency.
};

template<typename T>
class scv_extension_callbacks
  : public _scv_extension_callbacks_base<T>
{
public:
  virtual scv_extensions_if::callback_h _register_cb(scv_extensions_if::callback_base * c) {
    int next_id = s_add_callback(this->_get_dynamic_data(),c); 
    if (next_id != -1) {
      int size = this->get_num_fields();
      int * children = new int[size];
      for (int i=0; i<size; ++i) {
	children[i] = this->get_field(i)->_register_cb(c->duplicate());
	c->set_children(children);
      }
    }
    return next_id;
  }
  virtual void remove_cb(scv_extensions_if::callback_h id) {
    if (id != -1) {
      std::list<scv_extensions_if::callback_base*>::iterator iter
	= s_select_callback(this->_get_dynamic_data(),id);
      int size = this->get_num_fields();
      int * children = (*iter)->get_children();
      for (int i=0; i<size; ++i) {
	this->get_field(i)->remove_cb(children[i]);
      }
      delete (*iter);
      this->_get_dynamic_data()->_callbacks.erase(iter);
    }
  }
};

// ----------------------------------------
// specialization for arrays
// (added cast of N to "int" since some compilers automatically
// regard it as unsigned even though I have declard it as int)
// ----------------------------------------
template<typename T, int N>
class scv_extension_callbacks<T[N]>
  : public _SCV_INTROSPECTION_BASE2 {
public:
  scv_extension_callbacks() {}
  virtual ~scv_extension_callbacks() {
    if (this->_has_dynamic_data() && !this->get_parent())
	this->_get_dynamic_data()->execute_callbacks(this, scv_extensions_if::DELETE);
  }

  virtual scv_extensions_if::callback_h _register_cb(scv_extensions_if::callback_base * c) {
    int next_id = s_add_callback(this->_get_dynamic_data(),c);
    int * children = new int[N];
    for (int i=0; i<(int)N; ++i) {
      children[i] = this->_get_array_elt(i)->_register_cb(c->duplicate());
    }
    c->set_children(children);
    return next_id;
  }
  virtual void remove_cb(scv_extensions_if::callback_h id) {
    if (id != -1) {
      std::list<scv_extensions_if::callback_base*>::iterator iter
	= s_select_callback(this->_get_dynamic_data(),id);
      int * children = (*iter)->get_children();
      for (int i=0; i<(int)N; ++i) {
	this->_get_array_elt(i)->remove_cb(children[i]);
      }
      delete (*iter);
      this->_get_dynamic_data()->_callbacks.erase(iter);
    }
  }
};

// ----------------------------------------
// specialization for pointers
// ----------------------------------------
template<typename T>
class scv_extension_callbacks<T*>
 : public _SCV_INTROSPECTION_BASE1 {
public:
  scv_extension_callbacks() {}
  virtual ~scv_extension_callbacks() {
    if (this->_has_dynamic_data() && !this->get_parent())
	this->_get_dynamic_data()->execute_callbacks(this, scv_extensions_if::DELETE);
  }

  virtual scv_extensions_if::callback_h _register_cb(scv_extensions_if::callback_base * c) {
    return s_add_callback(this->_get_dynamic_data(),c); 
  }
  virtual void remove_cb(scv_extensions_if::callback_h id) { 
    s_remove_callback(this->_get_dynamic_data(),id); 
  }
};

// ----------------------------------------
// specialization for enums
// ----------------------------------------
class _scv_extension_callbacks_enum
  : public _SCV_INTROSPECTION_BASE_ENUM {
public:
  _scv_extension_callbacks_enum() {}
  virtual ~_scv_extension_callbacks_enum() {
    if (this->_has_dynamic_data() && !this->get_parent())
	this->_get_dynamic_data()->execute_callbacks(this, scv_extensions_if::DELETE);
  }

public:
  virtual scv_extensions_if::callback_h _register_cb(scv_extensions_if::callback_base * c);
  virtual void remove_cb(scv_extensions_if::callback_h id);
};



// ----------------------------------------
// specialization for basis types
// ----------------------------------------
#define _SCV_EXT_CALLBACKS_FC_D(basic_type,type_id)                           \
class _scv_extension_callbacks_ ## type_id  \
: public scv_extension_rand<basic_type> {  \
public:  \
  _scv_extension_callbacks_ ## type_id();  \
  virtual ~_scv_extension_callbacks_ ## type_id();  \
  \
  virtual scv_extensions_if::callback_h _register_cb(scv_extensions_if::callback_base * c);  \
  virtual void remove_cb(scv_extensions_if::callback_h id);  \
  \
  /* void trigger_value_change_cb(); */  \
  /* this is implemented as non-virtual in extension "util" for efficiency. */  \
};  \
  \
template<>                                                   \
class scv_extension_callbacks<basic_type>                    \
  : public _scv_extension_callbacks_ ## type_id {};     \


#define _SCV_EXT_CALLBACKS_FC_1_D(basic_type,type_id)                         \
  _SCV_EXT_CALLBACKS_FC_D(basic_type,type_id);


#define _SCV_EXT_CALLBACKS_FC_N_D(basic_type)                         \
template<int N>                                              \
class scv_extension_callbacks<basic_type<N> >                \
  : public scv_extension_rand<basic_type<N> > {  \
public:  \
  scv_extension_callbacks() {}  \
  virtual ~scv_extension_callbacks() {  \
    if (this->_has_dynamic_data() && !this->get_parent())  \
	this->_get_dynamic_data()->execute_callbacks(this, scv_extensions_if::DELETE);  \
  }  \
  \
  virtual scv_extensions_if::callback_h _register_cb(scv_extensions_if::callback_base * c) {  \
    return s_add_callback(this->_get_dynamic_data(),c);   \
  }  \
  virtual void remove_cb(scv_extensions_if::callback_h id) {   \
    s_remove_callback(this->_get_dynamic_data(),id);   \
  }  \
  \
  /* void trigger_value_change_cb(); */  \
  /* this is implemented as non-virtual in extension "util" for efficiency. */  \
}; \


_SCV_EXT_CALLBACKS_FC_D(bool,bool);                   
_SCV_EXT_CALLBACKS_FC_D(char,char);                   
_SCV_EXT_CALLBACKS_FC_D(unsigned char,unsigned_char);		
_SCV_EXT_CALLBACKS_FC_D(short,short);			
_SCV_EXT_CALLBACKS_FC_D(unsigned short,unsigned_short);		
_SCV_EXT_CALLBACKS_FC_D(int,int);			
_SCV_EXT_CALLBACKS_FC_D(unsigned int,unsigned_int);		
_SCV_EXT_CALLBACKS_FC_D(long,long);			
_SCV_EXT_CALLBACKS_FC_D(unsigned long,unsigned_long);		
_SCV_EXT_CALLBACKS_FC_D(long long,long_long);		
_SCV_EXT_CALLBACKS_FC_D(unsigned long long,unsigned_long_long);	
_SCV_EXT_CALLBACKS_FC_D(float,float);			
_SCV_EXT_CALLBACKS_FC_D(double,double);			
_SCV_EXT_CALLBACKS_FC_D(std::string,string);			
_SCV_EXT_CALLBACKS_FC_N_D(test_uint);		


#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
_SCV_EXT_CALLBACKS_FC_1_D(sc_bit,sc_bit);		
_SCV_EXT_CALLBACKS_FC_1_D(sc_logic,sc_logic);		
_SCV_EXT_CALLBACKS_FC_N_D(sc_int);		
_SCV_EXT_CALLBACKS_FC_N_D(sc_uint);		
_SCV_EXT_CALLBACKS_FC_N_D(sc_bigint);		
_SCV_EXT_CALLBACKS_FC_N_D(sc_biguint);		
_SCV_EXT_CALLBACKS_FC_N_D(sc_bv);		
_SCV_EXT_CALLBACKS_FC_N_D(sc_lv);		
_SCV_EXT_CALLBACKS_FC_D(sc_signed,sc_signed);		
_SCV_EXT_CALLBACKS_FC_D(sc_unsigned,sc_unsigned);		
_SCV_EXT_CALLBACKS_FC_D(sc_int_base,sc_int_base);		
_SCV_EXT_CALLBACKS_FC_D(sc_uint_base,sc_uint_base);		
_SCV_EXT_CALLBACKS_FC_D(sc_lv_base,sc_lv_base);		
_SCV_EXT_CALLBACKS_FC_D(sc_bv_base,sc_bv_base);		

// SCV_EXT_CALLBACKS_FC_N_D(tag,sc_fixed);		       
// SCV_EXT_CALLBACKS_FC_N_D(tag,sc_ufixed);		       
#endif



#undef _SCV_EXT_CALLBACKS_FC_D
#undef _SCV_EXT_CALLBACKS_FC_1_D
#undef _SCV_EXT_CALLBACKS_FC_N_D



// ----------------------------------------
// wrap up this component
// ----------------------------------------
#undef _SCV_INTROSPECTION_BASE
#define _SCV_INTROSPECTION_BASE scv_extension_callbacks<T>
#undef _SCV_INTROSPECTION_BASE1
#define _SCV_INTROSPECTION_BASE1 scv_extension_callbacks<T*>
#undef _SCV_INTROSPECTION_BASE2
#define _SCV_INTROSPECTION_BASE2 scv_extension_callbacks<T[N]>

#undef _SCV_INTROSPECTION_BASE_ENUM
#define _SCV_INTROSPECTION_BASE_ENUM _scv_extension_callbacks_enum
