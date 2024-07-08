/*******************************************************************************
 * Copyright 2024 MINRES Technologies GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#ifndef _SYSC_SCC_CCI_UTIL_H_
#define _SYSC_SCC_CCI_UTIL_H_

#include <boost/preprocessor.hpp>
#include <cci_configuration>

#define DEFINE_ENUM_DECL_VAL(r, name, val) val // BOOST_PP_CAT(name, BOOST_PP_CAT(_, val))
#define DEFINE_ENUM_VAL_STR(r, name, val) BOOST_PP_STRINGIZE(val)
#define ASSIGN_ENUM_MAP_ENTRY(r, name, val) lut[#val] = name::val;
#define CONCAT_OP(s, state, x) state x
#define CONCAT(SEQ) BOOST_PP_SEQ_FOLD_LEFT(CONCAT_OP, BOOST_PP_SEQ_HEAD(SEQ), BOOST_PP_SEQ_TAIL(SEQ)) // expands to boost

/**
 * \brief Generates the declaration of an enum class including cci pack/unpack converter functions.
 *
 * \note This cannot be used inside name spaces!
 *
 * \example DEFINE_ENUM4CCI(log_lvl, (NONE)(LOW)(MEDIUM)(HIGH)(FULL))
 *
 * \param name name of the enum class.
 * \param val_seq sequence of the enum class values
 * \returns code declaring the enum class.
 */
#if CCI_VERSION_MAJOR == 1 && CCI_VERSION_MINOR == 0 && CCI_VERSION_PATCH == 0
#define DEFINE_ENUM4CCI(name, val_seq)                                                                                                     \
    enum class name { BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(DEFINE_ENUM_DECL_VAL, name, val_seq)) };                                    \
    namespace cci {                                                                                                                        \
    namespace {                                                                                                                            \
    std::unordered_map<std::string, name> name##_lut_init() {                                                                              \
        std::unordered_map<std::string, name> lut;                                                                                         \
        CONCAT(BOOST_PP_SEQ_TRANSFORM(ASSIGN_ENUM_MAP_ENTRY, name, val_seq))                                                               \
        return lut;                                                                                                                        \
    }                                                                                                                                      \
    }                                                                                                                                      \
    template <> inline bool cci_value_converter<name>::pack(cci_value::reference dst, name const& src) {                                   \
        static const char* str_val[] = {BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(DEFINE_ENUM_VAL_STR, name, val_seq))};                    \
        dst.set_string(str_val[static_cast<unsigned>(src)]);                                                                               \
        return true;                                                                                                                       \
    }                                                                                                                                      \
    template <> inline bool cci_value_converter<name>::unpack(name& dst, cci::cci_value::const_reference src) {                            \
        static const std::unordered_map<std::string, name> lut = name##_lut_init();                                                        \
        if(!src.is_string())                                                                                                               \
            return false;                                                                                                                  \
        auto it = lut.find(src.get_string());                                                                                              \
        if(it != std::end(lut)) {                                                                                                          \
            dst = it->second;                                                                                                              \
            return true;                                                                                                                   \
        }                                                                                                                                  \
        return false;                                                                                                                      \
    }                                                                                                                                      \
    }

#define DEFINE_NS_ENUM4CCI(ns_name, name, val_seq)                                                                                         \
    namespace ns_name {                                                                                                                    \
    enum class name { BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(DEFINE_ENUM_DECL_VAL, name, val_seq)) };                                    \
    }                                                                                                                                      \
    namespace cci {                                                                                                                        \
    namespace {                                                                                                                            \
    std::unordered_map<std::string, ns_name::name> ns_name##_##name##_lut_init() {                                                         \
        std::unordered_map<std::string, ns_name::name> lut;                                                                                \
        CONCAT(BOOST_PP_SEQ_TRANSFORM(ASSIGN_ENUM_MAP_ENTRY, ns_name::name, val_seq))                                                      \
        return lut;                                                                                                                        \
    }                                                                                                                                      \
    }                                                                                                                                      \
    template <> inline bool cci_value_converter<ns_name::name>::pack(cci_value::reference dst, ns_name::name const& src) {                 \
        static const char* str_val[] = {BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(DEFINE_ENUM_VAL_STR, name, val_seq))};                    \
        dst.set_string(str_val[static_cast<unsigned>(src)]);                                                                               \
        return true;                                                                                                                       \
    }                                                                                                                                      \
    template <> inline bool cci_value_converter<ns_name::name>::unpack(ns_name::name& dst, cci::cci_value::const_reference src) {          \
        static const std::unordered_map<std::string, ns_name::name> lut = ns_name##_##name##_lut_init();                                   \
        if(!src.is_string())                                                                                                               \
            return false;                                                                                                                  \
        auto it = lut.find(src.get_string());                                                                                              \
        if(it != std::end(lut)) {                                                                                                          \
            dst = it->second;                                                                                                              \
            return true;                                                                                                                   \
        }                                                                                                                                  \
        return false;                                                                                                                      \
    }                                                                                                                                      \
    }
#else
#define DEFINE_ENUM4CCI(name, val_seq)                                                                                                     \
    enum class name { BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(DEFINE_ENUM_DECL_VAL, name, val_seq)) };                                    \
    namespace cci {                                                                                                                        \
    namespace {                                                                                                                            \
    std::unordered_map<std::string, name> name##_lut_init() {                                                                              \
        std::unordered_map<std::string, name> lut;                                                                                         \
        CONCAT(BOOST_PP_SEQ_TRANSFORM(ASSIGN_ENUM_MAP_ENTRY, name, val_seq))                                                               \
        return lut;                                                                                                                        \
    }                                                                                                                                      \
    }                                                                                                                                      \
    template <> struct cci_value_converter<name> {                                                                                         \
        static inline bool pack(cci_value::reference dst, name const& src) {                                                               \
            static const char* str_val[] = {BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(DEFINE_ENUM_VAL_STR, name, val_seq))};                \
            dst.set_string(str_val[static_cast<unsigned>(src)]);                                                                           \
            return true;                                                                                                                   \
        }                                                                                                                                  \
        static inline bool unpack(name& dst, cci::cci_value::const_reference src) {                                                        \
            static const std::unordered_map<std::string, name> lut = name##_lut_init();                                                    \
            if(!src.is_string())                                                                                                           \
                return false;                                                                                                              \
            auto it = lut.find(src.get_string());                                                                                          \
            if(it != std::end(lut)) {                                                                                                      \
                dst = it->second;                                                                                                          \
                return true;                                                                                                               \
            }                                                                                                                              \
            return false;                                                                                                                  \
        }                                                                                                                                  \
    };                                                                                                                                     \
    }

#define DEFINE_NS_ENUM4CCI(ns_name, name, val_seq)                                                                                         \
    namespace ns_name {                                                                                                                    \
    enum class name { BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(DEFINE_ENUM_DECL_VAL, name, val_seq)) };                                    \
    }                                                                                                                                      \
    namespace cci {                                                                                                                        \
    namespace {                                                                                                                            \
    std::unordered_map<std::string, ns_name::name> ns_name##_##name##_lut_init() {                                                         \
        std::unordered_map<std::string, ns_name::name> lut;                                                                                \
        CONCAT(BOOST_PP_SEQ_TRANSFORM(ASSIGN_ENUM_MAP_ENTRY, ns_name::name, val_seq))                                                      \
        return lut;                                                                                                                        \
    }                                                                                                                                      \
    }                                                                                                                                      \
    template <> struct cci_value_converter<ns_name ::name> {                                                                               \
        static inline bool pack(cci_value::reference dst, ns_name::name const& src) {                                                      \
            static const char* str_val[] = {BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(DEFINE_ENUM_VAL_STR, name, val_seq))};                \
            dst.set_string(str_val[static_cast<unsigned>(src)]);                                                                           \
            return true;                                                                                                                   \
        }                                                                                                                                  \
        static inline bool unpack(ns_name::name& dst, cci::cci_value::const_reference src) {                                               \
            static const std::unordered_map<std::string, ns_name::name> lut = ns_name##_##name##_lut_init();                               \
            if(!src.is_string())                                                                                                           \
                return false;                                                                                                              \
            auto it = lut.find(src.get_string());                                                                                          \
            if(it != std::end(lut)) {                                                                                                      \
                dst = it->second;                                                                                                          \
                return true;                                                                                                               \
            }                                                                                                                              \
            return false;                                                                                                                  \
        }                                                                                                                                  \
    };                                                                                                                                     \
    }
#endif
#endif /* _SYSC_SCC_CCI_UTIL_H_ */
