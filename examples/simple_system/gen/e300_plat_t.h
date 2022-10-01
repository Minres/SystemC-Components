/*******************************************************************************
 * Copyright 2017 eyck@minres.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations under
 * the License.
 ******************************************************************************/
#ifndef _E300_PLAT_MAP_H_
#define _E300_PLAT_MAP_H_
// need double braces, see
// https://stackoverflow.com/questions/6893700/how-to-construct-stdarray-object-with-initializer-list#6894191
const std::array<scc::target_name_map_entry<32>, 4> e300_plat_map = {{
    {"plic", 0x0c000000, 0x200008},
    {"gpio", 0x10012000, 0x1000},
    {"uart", 0x10013000, 0x1000},
    {"spi",  0x10014000, 0x1000},
}};

#endif /* _E300_PLAT_MAP_H_ */
