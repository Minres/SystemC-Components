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

  scv_tr.h -- 
  This is the public interface for transaction recording. This API is used
  to create transactions from your testbench, and also to record
  transactions into a recording database using callbacks.

  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Torsten Maehne,
                               Universite Pierre et Marie Curie, 2014-01-29
  Description of Modification: Undefine DELETE from winnt.h.

 *****************************************************************************/

/*
 * Author:  Bill Paulsen
 *
 * Description:
 *
 *  The scv_tr_db class allows you to open and close a transaction recording
 *  database object.  This object can be associated with separate facilities
 *  (which use callback registrations in scv_tr_db) to record transactions into
 *  a database.  scv_tr_db also alows you to suspend and resume transaction
 *  recording.
 * 
 *  The scv_tr_stream class allows you to create streams in a transaction
 *  object.  A stream has a hierarchical position in the elaborated design,
 *  and is the object on which transactions occur.
 *
 *  The scv_tr_generator template class allows you to describe the
 *  kinds of transactions that can occur on a stream.  The template parameters
 *  are classes (or structs) that specify the transaction attributes that are
 *  set at the beginning of a transaction, and the attributes that are set
 *  at the end of a transaction.  This class includes method to
 *  begin and to end transactions that are of this scv_tr_generator class.
 *  The begin_transaction() method accepts an argument that is an object
 *  that is of type of the first template parameter of this class.
 *  (The template parameter class must have a scv_extensions.)
 *  The values of the members of this object become the values of the
 *  attributes that are set at the beginning of this transaction.
 *  The end_transaction() method allows you to set the end-attributes 
 *  of this transaction.
 *
 *  The scv_tr_handle class is an object that represents a transaction.
 *  This object is returned from the begin_transaction() methods in the
 *  scv_tr_generator class.  (IE, you should not directly construct
 *  this class.)  This class has methods to get the begin and end times
 *  of the transaction, and also methods to get the scv_extensions of the
 *  attributes that are defined at the beginning and at the end of the
 *  transaction.
 *
 *
 *  Each class has methods that can register and remove callbacks on
 *  activity of that class.  For example, the scv_tr_generator class will
 *  give a callback when a transaction is begun, and another callback
 *  when it's ended.  In your registered callback function, you are passed
 *  a reference to the scv_tr_handle that was begun or ended.  For the
 *  scv_tr_stream class, you will get a callback when a scv_tr_stream is created
 *  and when it's deleted.
 *
 *  The callback methods can be used to implement applications that 
 *  record transaction activity into a recording database.  The scv_tr_text.cpp
 *  file contains an example that does transaction recording into a
 *  text file.
 *
 *
 * TBD:
 *
 *  Change the void* user_data_p in the callback registration functions
 *  to type-safe.
 *
 */

#ifndef SCV_TR_H
#define SCV_TR_H 

#if defined(_MSC_VER) || defined(_WIN32)
#  ifdef DELETE
#    undef DELETE // defined in winnt.h
#  endif
#endif

// ----------------------------------------------------------------------------
//
// The following is the default template parameter for the
// scv_tr_generator template class.  (You should not need to use
// this in your code.)
//
struct _scv_tr_generator_default_data {
  int _default_field_;
};
SCV_EXTENSIONS(_scv_tr_generator_default_data) {
public:
  scv_extensions<int> _default_field_;
  SCV_EXTENSIONS_CTOR(_scv_tr_generator_default_data) {
    SCV_FIELD(_default_field_);
  }
};

// ----------------------------------------------------------------------------

// Call this function from your sc_main() to use transaction text recording:
//
extern void scv_tr_text_init();

// ----------------------------------------------------------------------------

class _scv_tr_db_core;
class _scv_tr_stream_core;
class _scv_tr_generator_core;
class _scv_tr_handle_core;

class scv_tr_stream;
class scv_tr_generator_base;
template<class T1, class T2> class scv_tr_generator;
class scv_tr_handle;

typedef long scv_tr_relation_handle_t;

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class scv_tr_db : public scv_object_if {
 public:

  // The scv_tr_db class provides methods to:
  // 
  //   Open a transaction recording database.
  //
  //   Set the current time in the database.
  //
  //   Suspend and resume transaction recording in the database.
  //
  //   The standard print, show, kind, and dbg methods.
  //

  // The scv_tr_db constructor opens a transaction recording database.
  //
  // recording_file_name specifies the name of a file for transaction
  //   recording.
  //
  // time_scale specifies the time scale that is used for the recording
  //   database.  When you call set_time(), which is described below,
  //   the values of the argument to set_time() are presumed to be
  //   in units of time_scale.  The default is SC_FS.
  //
  scv_tr_db(
        const char* recording_file_name,
	const sc_time_unit& = SC_FS);


  // The dtor closes the recording database.
  //
  virtual ~scv_tr_db();


  // Set and get default scv_tr_db objects.
  //
  static void set_default_db(scv_tr_db*);
  static scv_tr_db* get_default_db();


  // Callbacks:
  //
  enum callback_reason { CREATE, DELETE, SUSPEND, RESUME };

  typedef int callback_h;

  // Your callback function:
  //
  typedef void callback_function(
		const scv_tr_db& obj,
                callback_reason reason,
		void* user_data_p);

  // Register a callback for any scv_tr_db object creation/deletion.
  //
  static callback_h register_class_cb(
		callback_function*,
                void* user_data_p = NULL);

  // Remove a callback.
  //
  static void remove_callback(callback_h);

  // set_recording allows you to temporarily suspend and resume recording.
  // set_recording(true) resumes recording (this is the default).
  // set_recording(false) suspends recording.
  // get_recording() returns the current state.
  //
  void set_recording(bool) const;
  bool get_recording() const;

  void print(ostream& o, int details=0, int indent=0) const;
  void show(int details=0, int indent=0) const;
  static void set_debug(int debug);
  static int get_debug();

  const char *get_name() const;
  const char *kind() const { return _kind; }


  // Create a new relation that can be defined between two transactions.
  // If a relation with relation_name had previously been created, then
  // return the handle to that relation.
  //
  scv_tr_relation_handle_t create_relation(
	const char* relation_name) const;

  // Get the name of a relation handle.  NULL is returned if the
  // relation_handle does not refer to a valid relation.
  //
  const char* get_relation_name(
	scv_tr_relation_handle_t relation_handle) const;

 private:
  friend class scv_tr_stream;
  _scv_tr_db_core* _scv_tr_db_core_p;
  static const char *_kind;
};

// --------------------------------------------------------------------------

class scv_tr_stream : public scv_object_if {
 public:

  // The scv_tr_stream ctor will create a stream on which transactions are
  // recorded at the specified scope, in the specified scv_tr_db.
  // 
  // full_stream_name is the full hierarchical path name of this stream.
  //   If full_stream_name already exists in this scv_tr_db, then that stream
  //   will be used.  If full_stream_scope_name is an invalid name, then this
  //   ctor will cause a fatal error.
  //
  // stream_kind is the kind of stream.  Common names are TRANSACTOR and TEST.
  //
  // scv_tr_db_p is the scv_tr_db object in which this stream is placed.  The
  //   default is the most recently opened database.  If this does not
  //   exist, then this ctor will cause a fatal error.
  //
  scv_tr_stream(
        const char* full_stream_name,
        const char* stream_kind,
	scv_tr_db* scv_tr_db_p = scv_tr_db::get_default_db());

  scv_tr_stream() { this->_scv_tr_stream_core_p = NULL; }

  virtual ~scv_tr_stream();


  // Callbacks:
  //
  enum callback_reason { CREATE, DELETE };

  typedef int callback_h;

  typedef void callback_function(
		const scv_tr_stream& obj,
		callback_reason reason,
                void* user_data_p);

  // Register a callback for stream creation/deletion.
  //
  static callback_h register_class_cb(
		callback_function*,
                void* user_data_p = NULL);

  // Remove a callback.
  //
  static void remove_callback(callback_h);

  void print(ostream& o, int details=0, int indent=0) const;
  void show(int details=0, int indent=0) const;
  static void set_debug(int debug);
  static int get_debug();

  const char *get_name() const;
  const char *kind() const { return _kind; }

  uint64 get_id() const;

  const char* get_stream_kind() const;

  // Return the most recently started scv_tr_handle on this scv_tr_stream,
  // on this process thread.  If none, then return an invalid
  // scv_tr_handle.  (Possibly remove this method)
  //
  scv_tr_handle get_current_transaction_handle();

  // Return the scv_tr_db in which this stream is defined:
  //
  const scv_tr_db* get_scv_tr_db() const;

 private:
  template<class T1, class T2> friend class scv_tr_generator;
  friend class scv_tr_generator_base;
  _scv_tr_stream_core* _scv_tr_stream_core_p;
  static const char *_kind;
};

// --------------------------------------------------------------------------

class scv_tr_handle : public scv_object_if {

 // This is a handle object, which includes an "is_valid()" method.
 //
 // You should use the scv_tr_generator::begin_transaction()
 // methods to create scv_tr_handle objects.
 // 
 // You should use the scv_tr_generator::end_transaction()
 // methods to end transactions.
 // 
 // After ending a transaction, you will still have a handle to the
 // transaction, which can be used to set relations, for example.
 // The transaction is deleted when all references to the transaction
 // go out of scope.

 public:

  // Note that the is_valid() method will return false if this ctor is
  // used directly.
  //
  scv_tr_handle();

  virtual ~scv_tr_handle();

  // Copy assignment:
  //
  scv_tr_handle &operator=(
    const scv_tr_handle& other);

  // Copy ctor:
  //
  scv_tr_handle(
    const scv_tr_handle& other);


  // End this transaction at the current simulation time.
  // The values of the attributes that are normally set at the end of
  // transaction, will be set to "undefined".  This method is also used
  // when the scv_tr_generator was defined to have no begin attributes.
  // (ie, the default was used for the T_begin template parameter.)
  //
  void end_transaction() {
    this->_end_transaction(NULL, sc_time_stamp());
  }

  // End this transaction at the current simulation time, and specify the
  // values of the end attributes.
  //
  template<class T_end> void end_transaction(
        const scv_tr_handle& t,
        const T_end& end_attribute_values)
  {
    scv_extensions<T_end> ext = scv_get_const_extensions(end_attribute_values);
    this->_end_transaction(&ext, sc_time_stamp());
  }

  // End this transaction at a simulation time that is in the past, and
  // specify the values of the end attributes.
  //
  template<class T_end> void end_transaction(
        const scv_tr_handle& t,
        const T_end& end_attribute_values,
        const sc_time& end_sc_time)
  {
    scv_extensions<T_end> ext = scv_get_const_extensions(end_attribute_values);
    this->_end_transaction(&ext, end_sc_time);
  }


  // Use these methods to record additional attributes on a transaction.
  // These methods can only be called after a transaction is started,
  // and before it's ended:
  //
  template <class T> void record_attribute(
	const char* attribute_name,
	const T& attribute_value) {
    scv_extensions<T> ext = scv_get_const_extensions(attribute_value);
    this->_record_attribute(attribute_name, &ext);
  }

  template <class T> void record_attribute(
        const T& attribute_value) {
    scv_extensions<T> ext = scv_get_const_extensions(attribute_value);
    this->_record_attribute(0, &ext);
  }


  // Callbacks:
  //
  enum callback_reason { BEGIN, END, DELETE };

  typedef int callback_h;

  // Your callback function for BEGIN/END/DELETE:
  //
  typedef void callback_function(
		const scv_tr_handle& obj,
		callback_reason reason,
                void* user_data_p);

  // Register a callback for transaction BEGIN/END/DELETE.
  //
  static callback_h register_class_cb(
		callback_function*,
		void* user_data_p = NULL);


  // Your callback function for when an attribute is set on this transaction,
  // other than at the begin or end of the transaction:
  //
  typedef void callback_record_attribute_function(
                const scv_tr_handle& obj,
        	const char* attribute_name,
		const scv_extensions_if* exts_p,
                void* user_data_p);

  // Register a callback for record attribute:
  //
  static callback_h register_record_attribute_cb(
                callback_record_attribute_function*,
                void* user_data_p = NULL);


  // Your callback function for related transactions:
  //
  typedef void callback_relation_function(
                const scv_tr_handle& transaction_1,
                const scv_tr_handle& transaction_2,
                void* user_data_p,
                scv_tr_relation_handle_t relation_handle);


  // Register a callback for when a transaction relation occurs.
  //
  static callback_h register_relation_cb(
		callback_relation_function*,
		void* user_data_p = NULL);

  // Remove a callback.
  //
  static void remove_callback(callback_h);


  // Return true if this is a valid transaction, which was created by a
  // successfull call to a scv_tr_generator::begin_transaction()
  // method.
  //
  bool is_valid() const;

  // Return true if the transaction is currently open (ie, the transaction
  // has begun, but is not yet ended.)  Returns false otherwise.
  //
  bool is_active() const;

  // Return the scv_extensions_if of the begin or end attributes.
  //
  const scv_extensions_if* get_begin_exts_p() const;
  const scv_extensions_if* get_end_exts_p() const;

  // Return the scv_extensions_if of the built-in attributes.
  //
  const scv_extensions_if* get_builtin_exts_p() const;

  // Get the unique ID for this transaction.
  //
  uint64 get_id() const;

  void print(ostream& o, int details=0, int indent=0) const;
  void show(int details=0, int indent=0) const;
  static void set_debug(int debug);
  static int get_debug();

  const char *get_name() const;
  const char *kind() const { return _kind; }


  // Set a relation from this transaction to another_transaction.
  // The relation is: this is a "relation_name" of other_transaction.
  //
  // If successful, return true, else return false (eg, either transaction
  // handle was invalid).
  //
  // relation_name is the name of the relation.
  //
  // other_transaction is the other transaction that will be related.
  //
  bool add_relation(
	scv_tr_relation_handle_t relation_handle,
        const scv_tr_handle& other_transaction_handle);

  bool add_relation(
	const char* relation_name,
	const scv_tr_handle& other_transaction_handle) {
    return add_relation(
	get_scv_tr_stream().get_scv_tr_db()->create_relation(relation_name),
	other_transaction_handle);
  };

  // If a related transaction is specified at the beginning of a new
  // transaction, then return the other related transaction and the
  // relation_handle.  Return NULL is there's no related transaction.
  //
  const scv_tr_handle* get_immediate_related_transaction(
	scv_tr_relation_handle_t* relation_handle_p) const;

  // Return the begin and end time of this transaction.
  //
  const sc_time& get_begin_sc_time() const;
  const sc_time& get_end_sc_time() const;

  // Return the stream on which this transaction occurs:
  //
  const scv_tr_stream& get_scv_tr_stream() const;

  // Return the transaction on which this transaction occurs:
  //
  // TBD get_scv_tr_generator_base => get_tr_generator_base
  const scv_tr_generator_base& get_scv_tr_generator_base() const;

 private:
  _scv_tr_handle_core* _scv_tr_handle_core_p;
  friend class scv_tr_generator_base;
  template<class T1, class T2> friend class scv_tr_generator;
  void _end_transaction(const scv_extensions_if*,
		const sc_time& end_sc_time) const;
  static const char *_kind;
  void _record_attribute(const char* attribute_name, const scv_extensions_if*);
};

// --------------------------------------------------------------------------

class scv_tr_generator_base : public scv_object_if {
  friend class scv_tr_stream;
  friend class scv_tr_handle;
 public:

  scv_tr_generator_base(
        const char* name,
        scv_tr_stream& s,
	const char* begin_attribute_name,
	const char* end_attribute_name);

  scv_tr_generator_base() { this->_scv_tr_generator_core_p = NULL; }

  virtual ~scv_tr_generator_base();

  // Get the names of hte begin and end attributes, as given in the
  // scv_tr_generator_base ctor:
  //
  const char* get_begin_attribute_name() const;
  const char* get_end_attribute_name() const;

  // Callbacks:
  //
  enum callback_reason { CREATE, DELETE };

  typedef int callback_h;

  typedef void callback_function(
		const scv_tr_generator_base& obj,
		callback_reason reason,
                void* user_data_p);

  // Register a callback for scv_tr_generator creation/deletion.
  //
  static callback_h register_class_cb(
                callback_function*,
                void* user_data_p = NULL);

  // Remove a callback.
  //
  static void remove_callback(callback_h);


  // TBD: Need callbacks on specific transaction objects


  void print(ostream& o, int details=0, int indent=0) const;
  void show(int details=0, int indent=0) const;
  static void set_debug(int debug);
  static int get_debug();

  const char *get_name() const;
  const char *kind() const { return "scv_tr_generator"; }

  uint64 get_id() const;

  // Return the stream on which this transaction is defined:
  //

// get_scv_tr_stream ==> get_tr_stream and check 

  const scv_tr_stream& get_scv_tr_stream() const;

  // Return the scv_extensions_if of the begin or end attributes.
  // These methods only return info about the attribute classes, and
  // do not contain valid values because they are not associated with
  // any particular transaction scv_tr_handle.
  //
  const scv_extensions_if* get_begin_exts_p() const;
  const scv_extensions_if* get_end_exts_p() const;

 private:
  _scv_tr_generator_core* _scv_tr_generator_core_p;

 public:
  scv_tr_handle _begin_transaction(
                const scv_extensions_if*,
                const sc_time&,
		scv_tr_relation_handle_t relation_handle,
                const scv_tr_handle* other_handle_p = NULL) const;

  void _end_transaction(
                const scv_tr_handle& t,
                const scv_extensions_if*,
                const sc_time& end_sc_time) const;
 public:
  // To be called only from scv_tr_generator:
  void _set_begin_exts_p(const scv_extensions_if*);
  void _set_end_exts_p(const scv_extensions_if*);
  void _process_callbacks();
};

// --------------------------------------------------------------------------

template < class T_begin = _scv_tr_generator_default_data,
           class T_end   = _scv_tr_generator_default_data >
class scv_tr_generator : public scv_tr_generator_base {
 public:

  // The scv_tr_generator class is used to define the kinds of transactions
  // that can be created on a stream.
  //
  // The scv_tr_generator ctor specifies the name of this scv_tr_generator,
  // the stream on which this scv_tr_generator is associated, and also
  // specifies the attributes of this scv_tr_generator that are defined at
  // the beginning and at the end of transactions of this scv_tr_generator.
  // For scalar begin and end attributes, you can explicitly give names 
  // to those attributes.
  //
  // If the T_begin or T_end template parameters are missing, then
  // this scv_tr_generatorwill record no attributes at the beginning
  // or at the end of transactions of this scv_tr_generator.
  // 
  // The template parameters must be scv_extensions.
  //
  // The begin_transaction() methods return scv_tr_handle objects
  // to transactions of this scv_tr_generator class.
  //
  // The end_transaction() methods end the transactions.
  // 

  // The scv_tr_generator ctor defines a scv_tr_generator on a stream.
  //
  // T_begin (template parameter) specifies the attributes that are
  //   defined at the beginning of the transaction.
  //
  // T_end (template parameter) specifies the attributes that are
  //   defined at the end of the transaction.
  //
  // name is the name of this scv_tr_generator.
  //
  // s is the scv_tr_stream in which this scv_tr_generator is defined.
  //
  scv_tr_generator(
	const char* name,
	scv_tr_stream& s,
	const char* begin_attribute_name = NULL,
	const char* end_attribute_name = NULL) :
		scv_tr_generator_base(
			name, s, begin_attribute_name, end_attribute_name),
		_begin_ext(scv_get_const_extensions(_tmp_begin_attributes)),
		_end_ext(scv_get_const_extensions(_tmp_end_attributes))
  {
    this->_set_begin_exts_p(&_begin_ext);
    this->_set_end_exts_p(&_end_ext);
    this->_process_callbacks();
  }

  scv_tr_generator() {}

  ~scv_tr_generator() {}

  // ------------------------------------------------------------------------

  // The begin_transaction methods start transactions of this
  // scv_tr_generator class.  The different versions of begin provide
  // various features.
  //
  // In all begin_transaction methods:
  //
  //   A scv_tr_handle object is returned.
  //
  //   The transaction is started on the scv_tr_stream of the scv_tr_generator
  //   object.
  //


  // This method starts a transaction with these characteristics:
  //
  //   The values of the attributes that are normally set at the beginning
  //   of the transaction will be set to "undefined".
  //
  //   This method is also used then this scv_tr_generator was defined to
  //   have no begin attributes.  (ie, the default was used for the T_begin
  //   template parameter.)
  // 
  //   The begin time of this transaction is the current simulation time.
  //
  scv_tr_handle begin_transaction()
  {
    return this->_begin_transaction(
	NULL,
	sc_time_stamp(),
	0);
  }


  // This method starts a transaction with these characteristics:
  //
  //   The values of the attributes that are normally set at the beginning
  //   of the transaction will be set to "undefined".
  //
  //   This method is also used then this scv_tr_generator was defined to
  //   have no begin attributes.  (ie, the default was used for the T_begin
  //   template parameter.)
  //
  //   The begin time of this transaction is the current simulation time.
  //
  //   The new transaction has the relation_handle or relation_name to
  //   other_transaction_handle.
  //
  scv_tr_handle begin_transaction(
	scv_tr_relation_handle_t relation_handle,
        const scv_tr_handle& other_transaction_handle)
  {
    return this->_begin_transaction(
	NULL,
	sc_time_stamp(),
	relation_handle,
	&other_transaction_handle);
  }

  scv_tr_handle begin_transaction(
	const char* relation_name,
        const scv_tr_handle& other_transaction_handle)
  {
    return this->_begin_transaction(
	NULL,
	sc_time_stamp(),
        get_scv_tr_stream().get_scv_tr_db()->create_relation(relation_name),
	&other_transaction_handle);
  }


  // This method starts a transaction with these characteristics:
  //
  //   The values of the begin attributes of this transaction are the
  //   values that are currently in the attribute_values argument.
  //
  //   The begin time of this transaction is the current simulation time.
  //
  scv_tr_handle begin_transaction(
	const T_begin& begin_attribute_values)
  {
    scv_extensions<T_begin> ext =
		scv_get_const_extensions(begin_attribute_values);
    return this->_begin_transaction(
	&ext,
	sc_time_stamp(),
	0);
  }


  // This method starts a transaction with these characteristics:
  //
  //   The values of the begin attributes of this transaction are the
  //   values that are currently in the attribute_values argument.
  //
  //   The begin time of this transaction is the current simulation time.
  //
  //   The new transaction has the "relation_name" to other_transaction_handle.
  //
  scv_tr_handle begin_transaction(
        const T_begin& begin_attribute_values,
        scv_tr_relation_handle_t relation_handle,
        const scv_tr_handle& other_transaction_handle)
  {
    scv_extensions<T_begin> ext =
                scv_get_const_extensions(begin_attribute_values);
    return this->_begin_transaction(
	&ext,
	sc_time_stamp(),
	relation_handle,
	&other_transaction_handle);
  }

  scv_tr_handle begin_transaction(
        const T_begin& begin_attribute_values,
        const char* relation_name,
        const scv_tr_handle& other_transaction_handle)
  {
    scv_extensions<T_begin> ext =
                scv_get_const_extensions(begin_attribute_values);
    return this->_begin_transaction(
	&ext,
	sc_time_stamp(),
        get_scv_tr_stream().get_scv_tr_db()->create_relation(relation_name),
	&other_transaction_handle);
  }


  // This method starts a transaction with these characteristics:
  //
  //   The values of the attributes that are normally set at the beginning
  //   of the transaction will be set to "undefined".
  //
  //   This method is also used then this scv_tr_generator was defined to
  //   have no begin attributes.  (ie, the default was used for the T_begin
  //   template parameter.)
  //
  //   The begin time of this transaction is specified by the begin_time
  //   argument.   This time must be less than or equal to the current
  //   simulation time.  This allows you to start a transaction in the past.
  //
  scv_tr_handle begin_transaction(
        const sc_time& begin_sc_time)
  {
    return this->_begin_transaction(
	NULL,
	begin_sc_time,
	0);
  }


  // This method starts a transaction with these characteristics:
  //
  //   The values of the attributes that are normally set at the beginning
  //   of the transaction will be set to "undefined".
  //
  //   This method is also used then this scv_tr_generator was defined to
  //   have no begin attributes.  (ie, the default was used for the T_begin
  //   template parameter.)
  //
  //   The begin time of this transaction is specified by the begin_time
  //   argument.   This time must be less than or equal to the current
  //   simulation time.  This allows you to start a transaction in the past.
  //
  //   The new transaction has the "relation_name" to other_transaction_handle.
  //
  scv_tr_handle begin_transaction(
        const sc_time& begin_sc_time,
        scv_tr_relation_handle_t relation_handle,
        const scv_tr_handle& other_transaction_handle)
  {
    return this->_begin_transaction(
	NULL,
	begin_sc_time,
	relation_handle,
	&other_transaction_handle);
  }

  scv_tr_handle begin_transaction(
        const sc_time& begin_sc_time,
        const char* relation_name,
        const scv_tr_handle& other_transaction_handle)
  {
    return this->_begin_transaction(
	NULL,
	begin_sc_time,
        get_scv_tr_stream().get_scv_tr_db()->create_relation(relation_name),
	&other_transaction_handle);
  }


  // This method starts a transaction with these characteristics:
  //
  //   The values of the begin attributes of this transaction are the
  //   values that are currently in the attribute_values argument.
  //
  //   The begin time of this transaction is specified by the begin_time
  //   argument.   This time must be less than or equal to the current
  //   simulation time.  This allows you to start a transaction in the past.
  //
  scv_tr_handle begin_transaction(
	const T_begin& begin_attribute_values,
	const sc_time& begin_sc_time)
  {
    scv_extensions<T_begin> ext =
                scv_get_const_extensions(begin_attribute_values);
    return this->_begin_transaction(
	&ext,
	begin_sc_time,
	0);
  }


  // This method starts a transaction with these characteristics:
  //
  //   The values of the begin attributes of this transaction are the
  //   values that are currently in the attribute_values argument.
  //
  //   The begin time of this transaction is specified by the begin_time
  //   argument.   This time must be less than or equal to the current
  //   simulation time.  This allows you to start a transaction in the past.
  //
  scv_tr_handle begin_transaction(
        const T_begin& begin_attribute_values,
        const sc_time& begin_sc_time,
        scv_tr_relation_handle_t relation_handle,
        const scv_tr_handle& other_transaction_handle)
  {
    scv_extensions<T_begin> ext =
                scv_get_const_extensions(begin_attribute_values);
    return this->_begin_transaction(
	&ext,
	begin_sc_time,
	relation_handle,
	&other_transaction_handle);
  }

  scv_tr_handle begin_transaction(
        const T_begin& begin_attribute_values,
        const sc_time& begin_sc_time,
        const char* relation_name,
        const scv_tr_handle& other_transaction_handle)
  {
    scv_extensions<T_begin> ext =
                scv_get_const_extensions(begin_attribute_values);
    return this->_begin_transaction(
	&ext,
	begin_sc_time,
        get_scv_tr_stream().get_scv_tr_db()->create_relation(relation_name),
	&other_transaction_handle);
  }

  // ------------------------------------------------------------------------

  // End a transaction at the current simulation time.
  // The values of the attributes that are normally set at the end of
  // transaction, will be set to "undefined".  This method is also used
  // when this scv_tr_generator was defined to have no begin attributes.
  // (ie, the default was used for the T_begin template parameter.)
  //
  void end_transaction(
        const scv_tr_handle& t)
  {
    this->_end_transaction(t, NULL, sc_time_stamp());
  }

  // End a transaction at the current simulation time, and specify the
  // values of the end attributes.
  //
  void end_transaction(
        const scv_tr_handle& t,
	const T_end& end_attribute_values)
  {
    scv_extensions<T_end> ext = scv_get_const_extensions(end_attribute_values);
    this->_end_transaction(t, &ext, sc_time_stamp());
  }

  // End a transaction at a simulation time that is in the past.
  // The values of the attributes that are normally set at the end of
  // transaction, will be set to "undefined".  This method is also used
  // when this scv_tr_generator was defined to have no begin attributes.
  // (ie, the default was used for the T_begin template parameter.)
  //
  void end_transaction(
        const scv_tr_handle& t,
	const sc_time& end_sc_time)
  {
    this->_end_transaction(t, NULL, end_sc_time);
  }

  // End a transaction at a simulation time that is in the past, and
  // specify the values of the end attributes.
  //
  void end_transaction(
        const scv_tr_handle& t,
        const T_end& end_attribute_values,
	const sc_time& end_sc_time)
  {
    scv_extensions<T_end> ext = scv_get_const_extensions(end_attribute_values);
    this->_end_transaction(t, &ext, end_sc_time);
  }

  // ------------------------------------------------------------------------

 private:
    friend class scv_tr_handle;
    T_begin _tmp_begin_attributes;
    T_end _tmp_end_attributes;
    scv_extensions<T_begin> _begin_ext;
    scv_extensions<T_end> _end_ext;
};

// ----------------------------------------------------------------------------

#endif  // SCV_TR_H

