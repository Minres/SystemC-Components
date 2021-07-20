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

  scv_random.h -- 
  The public interface for unsigned random stream and seed management
  
  - Provides global configuration methods for seed control 
  - Allows specifying custom random number generation algorithms for 
    all or specific random streams
  - Multiple independent random streams
  - Manage seed file registry for explicit seed control  
  - To make a consistent interface for seed control all seeds are 
    managed by using 'unsigned long long type'

  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/

#ifndef SCV_RANDOM_H
#define SCV_RANDOM_H

#include <cstdlib>
#include <cstdio>
#include <list>

#include "scv/_scv_data_structure.h"
#include "scv/_scv_associative_array.h"

class _scv_random_impl;

class scv_random : public _scv_data_structure {
public: // global configuration on algorithm and seed
  // value_generation_algorithm enum type specifies which
  // algorithm to use for generating unsigned random values
  //  RAND   - specifies using C rand() function 
  //  RAND32   - specifies using C rand() function 
  //  RAND48 - use jrand48() for uniform unsigned random streams
  //  CUSTOM - use custom algorithm specified by using set_default_algorithm
  //           for global configuration or set_algorithm for 
  //           specific random streams
  enum value_generation_algorithm {
    RAND,   // C rand() will
    RAND32,   // C rand() will
    RAND48, // jrand48 ()        
    CUSTOM  // plugin a custom random number generation algorithm.       
  };

  typedef unsigned int (*alg_func)(unsigned long long& next);


  // set_global_seed sets the global seed for randomization control
  // All random streams created explicitly or implicitly will have
  // seed based on the thread it is being created in and global
  // seed. So by setting just one global seed you can ensure same
  // random values for same test bench  

  static void set_global_seed (unsigned long long=1); 
  static unsigned long long get_global_seed (void); 

  static unsigned long long pick_random_seed(unsigned long job_number=0);

  // set_default_algorithm specifies default algorithm to use for
  // all the scv_random streams. You can override this setting using
  // set_algorithm for specific streams

  static void set_default_algorithm(value_generation_algorithm alg,
                                  alg_func customAlg = NULL);
public: // debugging and seed files management

  // access the list of all current scv_random objects
  static void get_generators(std::list<scv_random *>& genList);

public: // constructors for independent random stream

  // constructor to explicitly specify a unique name for random
  // stream. Names are used in the seed file registry to save and
  // restore seeds. Using unique names for random streams you can
  // have same random values in different simulation results even
  // with a slight change in the test bench
  // default value is set to "<anonymous"> if no name is specified 

  scv_random(const char* name = NULL);

  // constructor to provide explicit seed value
  scv_random(unsigned long long seed);

  // constructor to provide both explicit name and seed
  scv_random(const char* name, unsigned long long seed);

  // copy constructor with means to provide explicit name and seed
  // If no name and seed is provided it will use the same algorithm
  // as other but with name as "anonymous" and create unique seed
  // based on thread and global seed

  scv_random(const scv_random& other,
            const char* name= NULL, unsigned long long seed=0);

  // virtual destructof because of inheritance
  virtual ~scv_random();

public: // random value generation and configuration of individual random stream

  // return new random value based on the algorithm specified
  unsigned int next(void);

  // provide algorithm to generate random value for a specfic random stream.
  // by default use jrand48()

  void set_algorithm(value_generation_algorithm m = RAND48,
                  alg_func algorithm = NULL);

  // return initial seed value for the random stream.
  // initial seed is the seed assigned to this object when it is created

  unsigned long long get_initial_seed() const;

  // return current seed value for this object
  // current seed value is the value that will be used to generate the next
  // unsigned random value 
  // one way to use this information is to restart simulation starting from
  // the point it left before. 
  unsigned long long get_current_seed() const;

  // set current seed value to seed. 
  void set_current_seed(unsigned long long seed);

public: // print the seeds of all current scv_random objects 

  // initial seed is the seed assigned to this object when it is created
  // current seed is the current seed value that will be used to generate
  //   the next unsigned integer value

  static void print_initial_seeds(const char* fileName);
  static void print_initial_seeds(std::ostream& = scv_out);
  static void print_current_seeds(const char* fileName);
  static void print_current_seeds(std::ostream& = scv_out);

public: // a monitor saves or restores seeds from a file

  // starts global seed monitoring on. based on retrieve seeds will
  // be saved or retrieved from 'fileName'
  // if retrieve is false save seed values to 'fileName'
  // if retrieve is true obtain values of seeds from 'fileName'

  static void seed_monitor_on(bool retrieve, const char* fileName);

  static void seed_monitor_on(bool retrieve, const char* monitorName, std::FILE* file);

  // set global seed monitoring off
  static void seed_monitor_off();

public: // debugging interface

  // look at scv_object_if.h for explanation of these methods
  const char *kind() const; 
  void print(std::ostream& o=scv_out, int details=0, int indent=0) const; 
  void show(int details=0, int indent=0) const; 
  static int get_debug();
  static void set_debug(int i);
private:
  _scv_random_impl* _coreP;

private:
  static unsigned long long global_seed;
  static value_generation_algorithm global_alg_type;
};


#endif
