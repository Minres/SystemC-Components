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
#ifndef _SCC_CCI_PARAM_RESTRICTED_H_
#define _SCC_CCI_PARAM_RESTRICTED_H_

#include <cci_configuration>
#include <unordered_set>

namespace scc {
/// @cond DEV
template <typename T> struct _min_max_restriction {
    _min_max_restriction(T min, T max)
    : min(min)
    , max(max) {}

    bool operator()(cci::cci_param_write_event<T> const& ev) const { return ev.new_value >= min && ev.new_value <= max; }
    T const min;
    T const max;
};

template <typename T> struct _min_max_excl_restriction {
    _min_max_excl_restriction(T min, T max)
    : min(min)
    , max(max) {}

    bool operator()(cci::cci_param_write_event<T> const& ev) const { return ev.new_value > min && ev.new_value < max; }
    T const min;
    T const max;
};

template <typename T> struct _min_restriction {
    _min_restriction(T min)
    : min(min) {}

    bool operator()(cci::cci_param_write_event<T> const& ev) const { return ev.new_value >= min; }
    T const min;
};

template <typename T> struct _min_excl_restriction {
    _min_excl_restriction(T min)
    : min(min) {}

    bool operator()(cci::cci_param_write_event<T> const& ev) const { return ev.new_value > min; }
    T const min;
};

template <typename T> struct _max_restriction {
    _max_restriction(T max)
    : max(max) {}

    bool operator()(cci::cci_param_write_event<T> const& ev) const { return ev.new_value <= max; }
    T const max;
};

template <typename T> struct _max_excl_restriction {
    _max_excl_restriction(T max)
    : max(max) {}

    bool operator()(cci::cci_param_write_event<T> const& ev) const { return ev.new_value < max; }
    T const max;
};

template <typename T> struct _discrete_restriction {
    template <typename COLLECTION_TYPE>
    _discrete_restriction(COLLECTION_TYPE values)
    : values(std::begin(values), std::end(values)) {}

    bool operator()(cci::cci_param_write_event<T> const& ev) const { return values.count(ev.new_value) > 0; }
    std::unordered_set<T> const values;
};
/// @endcond
/**
 * @brief creates a min/max restriction with including the limits
 *
 * @tparam T the type of the value the restricion applies
 * @param min the lower bound
 * @param max the upper bound
 * @return _min_max_restriction<T> instance of the restriction functor
 */
template <typename T> inline _min_max_restriction<T> min_max_restriction(T min, T max) { return _min_max_restriction<T>(min, max); }
//! @brief alias for min_max_restriction(T min, T max)
template <typename T> inline _min_max_restriction<T> gte_lte_restriction(T min, T max) { return _min_max_restriction<T>(min, max); }
/**
 * @brief creates a min/max restriction with excluding the limits
 *
 * @tparam T the type of the value the restricion applies
 * @param min the lower bound
 * @param max the upper bound
 * @return _min_max_excl_restriction<T> instance of the restriction functor
 */
template <typename T> inline _min_max_excl_restriction<T> min_max_excl_restriction(T min, T max) {
    return _min_max_excl_restriction<T>(min, max);
}
//! @brief alias for min_max_excl_restriction(T min, T max)
template <typename T> inline _min_max_excl_restriction<T> gt_lt_restriction(T min, T max) { return _min_max_excl_restriction<T>(min, max); }
/**
 * @brief creates a minimum restriction including the minimum value
 *
 * @tparam T the type of the value the restricion applies
 * @param min the lower bound
 * @return _min_restriction<T>
 */
template <typename T> inline _min_restriction<T> min_restriction(T min) { return _min_restriction<T>(min); }
//! @brief alias for min_restriction(T min)
template <typename T> inline _min_restriction<T> gte_restriction(T min) { return _min_restriction<T>(min); }
/**
 * @brief creates a minimum restriction excluding the minimum value
 *
 * @tparam T the type of the value the restricion applies
 * @param min the lower bound
 * @return _min_excl_restriction<T>
 */
template <typename T> inline _min_excl_restriction<T> min_excl_restriction(T min) { return _min_excl_restriction<T>(min); }
//! @brief alias for min_excl_restriction(T min)
template <typename T> inline _min_excl_restriction<T> gt_restriction(T min) { return _min_excl_restriction<T>(min); }
/**
 * @brief creates a maximum restriction including the maximum value
 *
 * @tparam T the type of the value the restricion applies
 * @param max the upper bound
 * @return _max_restriction<T>
 */
template <typename T> inline _max_restriction<T> max_restriction(T max) { return _max_restriction<T>(max); }
//! @brief alias for max_restriction(T max)
template <typename T> inline _max_restriction<T> lte_restriction(T max) { return _max_restriction<T>(max); }
/**
 * @brief creates a maximum restriction excluding the maximum value
 *
 * @tparam T the type of the value the restricion applies
 * @param max the upper bound
 * @return _max_excl_restriction<T>
 */
template <typename T> inline _max_excl_restriction<T> max_excl_restriction(T max) { return _max_excl_restriction<T>(max); }
//! @brief alias for max_excl_restriction(T max)
template <typename T> inline _max_excl_restriction<T> lt_excl_restriction(T max) { return _max_excl_restriction<T>(max); }
/**
 * @brief creates a restriction for a discrete values set
 *
 * @tparam T the type of the value the restricion applies
 * @param values the set of allowed values
 * @return _discrete_restriction<T>
 */
template <typename T> inline _discrete_restriction<T> discrete_restriction(std::initializer_list<T> values) {
    return _discrete_restriction<T>(values);
}
/**
 * @brief creates a restriction for a discrete values set
 *
 * @tparam T the type of the value the restricion applies
 * @tparam SZ size of the array
 * @param values  the set of allowed values
 * @return _discrete_restriction<T>
 */
template <typename T, size_t SZ> inline _discrete_restriction<T> discrete_restriction(std::array<T, SZ> values) {
    return _discrete_restriction<T>(values);
}
/**
 * @brief creates a restriction for a discrete values set
 *
 * @tparam T the type of the value the restricion applies
 * @param values the set of allowed values
 * @return _discrete_restriction<T>
 */
template <typename T> inline _discrete_restriction<T> discrete_restriction(std::vector<T> values) {
    return _discrete_restriction<T>(values);
}
/**
 * @brief extension of \ref cci_param<T, TM> which automatically registeres a callback to restrict the valid values given to the parameter.
 *
 * The parameter can be use with a restriction created by:
 * - min_max_restriction(T min, T max)
 * - min_max_excl_restriction(T min, T max)
 * - min_restriction(T min), gte_restriction(T min)
 * - min_excl_restriction(T min), gt_excl_restriction(T min)
 * - max_restriction(T max), lte_restriction(T max)
 * - max_excl_restriction(T max), lt_excl_restriction(T max)
 * - discrete_restriction(std::initializer_list<T> values), discrete_restriction(std::array<T, SZ> values),
 *   discrete_restriction(std::vector<T> values)
 *
 * @tparam T type of the parameter value
 * @tparam TM  specifies the parameter type lock behavior
 */
template <typename T, cci::cci_param_mutable_type TM = cci::CCI_MUTABLE_PARAM> struct cci_param_restricted : public cci::cci_param<T, TM> {

    /** @name Constructors */
    //@{
    /**
     * Constructor with (local/hierarchical) name, default value, restriction,
     * description and originator.
     *
     * @param name Name of the parameter
     * @param default_value Default value of the parameter (Typed value)
     * @param restr Restriction to apply, will be checked befor every write
     * @param desc Description of the parameter
     * @param name_type Either the name should be absolute or relative
     * @param originator Originator of the parameter
     */
    template <typename RESTR>
    cci_param_restricted(const std::string& name, const T& default_value, RESTR const& restr, const std::string& desc = "",
                         cci::cci_name_type name_type = cci::CCI_RELATIVE_NAME,
                         const cci::cci_originator& originator = sc_core::sc_get_current_object() ? cci::cci_originator()
                                                                                                  : cci::cci_originator("sc_main"))
    : cci::cci_param<T, TM>(name, default_value, desc, name_type, originator) {
        this->register_pre_write_callback(restr);
        this->reset();
    }

    /**
     * Constructor with (local/hierarchical) name, default value, restriction,
     * private broker, description, name type and originator.
     *
     * @param name Name of the parameter
     * @param default_value Default value of the parameter (Typed value)
     * @param private_broker Associated private broker
     * @param restr Restriction to apply, will be checked befor every write
     * @param desc Description of the parameter
     * @param name_type Either the name should be absolute or relative
     * @param originator Originator of the parameter
     */
    template <typename RESTR>
    cci_param_restricted(const std::string& name, const T& default_value, RESTR const& restr, cci::cci_broker_handle private_broker,
                         const std::string& desc = "", cci::cci_name_type name_type = cci::CCI_RELATIVE_NAME,
                         const cci::cci_originator& originator = sc_core::sc_get_current_object() ? cci::cci_originator()
                                                                                                  : cci::cci_originator("sc_main"))
    : cci::cci_param<T, TM>(name, default_value, desc, name_type, originator) {
        this->register_pre_write_callback(restr);
        this->reset();
    }
    //@}
};
} // namespace scc
#endif /* _SCC_CCI_PARAM_RESTRICTED_H_ */
