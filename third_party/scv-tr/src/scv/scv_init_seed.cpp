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

  scv_init_seed.cpp

  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Stephan Schulz, Fraunhofer IIS-EAS, 2013-02-21
  Description of Modification: Added check for _WIN32 macro to support mingw32

 *****************************************************************************/

#if ! ((defined _MSC_VER) || (defined _WIN32))
#include <sys/times.h>
#endif

#include <cstring>

#if defined(__linux__) || defined(__APPLE__)
# include <sys/time.h>
#else 
#include <time.h>
#endif

extern unsigned long long _scv_get_global_seed(void);

unsigned long long
_scv_default_global_init_seed(unsigned long job_number)
{
#if ((defined _MSC_VER) || (defined _WIN32))
  __time64_t ltime_sec;
  _time64( &ltime_sec );
  unsigned __int64 ltime_usec = 0;
  long i = 6000000L;
  unsigned int clk;
  clock_t start, finish;

  // measure the duration of an event
  start = clock();
  while( i-- ) ;
  finish = clock();
  clk = finish - start;

  // select a "random" seed
  unsigned __int64 seed
    = ((unsigned long)(ltime_sec^ltime_usec^clk^job_number) * 654435769) >> 1;
#else
  // get time of date
  struct timeval tp;
  struct timezone dummy1;
  gettimeofday(&tp, &dummy1);

  // get system time
  struct tms dummy2;
  clock_t clk;
  clk = times(&dummy2);

  // select a "random" seed
  unsigned long long seed
    = ((unsigned long)(tp.tv_sec^tp.tv_usec^clk^job_number) * 654435769) >> 1;
#endif

  if (seed==0) seed = 46831694;
  return seed;
}

unsigned long long 
_scv_get_seed_from_name(const char * name, unsigned inst_num)
{
  unsigned long long seed = 0;

  if (name) {
    for (unsigned i=0;i < std::strlen(name); i++) {
      if (name[i] != 0) {
        seed += name[i];
      }
    }
  } else {
    seed = 1;
  }

  seed = ((unsigned long) ((_scv_get_global_seed() * seed) + 
            inst_num * 1023 ) * 654435769) >> 1;
 
  if (seed==0) seed = 46831694;
  return seed;
}

