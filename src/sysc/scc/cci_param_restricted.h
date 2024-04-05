/*
 * restricted_cci_param.h
 *
 *  Created on: Apr 4, 2024
 *      Author: eyck
 */

#ifndef _SCC_CCI_PARAM_RESTRICTED_H_
#define _SCC_CCI_PARAM_RESTRICTED_H_

#include "cci_cfg/cci_param_callbacks.h"
#include <cci_configuration>
#include <initializer_list>

namespace scc {
template <typename T> struct _min_max_restriction {

    using value_type = T;

    _min_max_restriction(T min, T max)
    : min(min)
    , max(max) {}

    bool operator()(cci::cci_param_write_event<T> const& ev) const { return ev.new_value >= min && ev.new_value <= max; }
    T const min;
    T const max;
};

template <typename T> struct _min_max_excl_restriction {

    using value_type = T;

    _min_max_excl_restriction(T min, T max)
    : min(min)
    , max(max) {}

    bool operator()(cci::cci_param_write_event<T> const& ev) const { return ev.new_value > min && ev.new_value < max; }
    T const min;
    T const max;
};

template <typename T> struct _min_restriction {

    using value_type = T;

    _min_restriction(T min)
    : min(min) {}

    bool operator()(cci::cci_param_write_event<T> const& ev) const { return ev.new_value >= min; }
    T const min;
};

template <typename T> struct _min_excl_restriction {

    using value_type = T;

    _min_excl_restriction(T min)
    : min(min) {}

    bool operator()(cci::cci_param_write_event<T> const& ev) const { return ev.new_value > min; }
    T const min;
};
template <typename T> struct _discrete_restriction {

    using value_type = T;

    _discrete_restriction(std::initializer_list<T> val)
    : val(val) {}

    bool operator()(cci::cci_param_write_event<T> const& ev) const { return std::find(begin(val), end(val), ev.new_value) != end(val); }
    std::vector<T> const val;
};

template <typename T> inline _min_max_restriction<T> min_max_restriction(T min, T max) { return _min_max_restriction<T>(min, max); }
template <typename T> inline _min_max_excl_restriction<T> min_max_excl_restriction(T min, T max) {
    return _min_max_excl_restriction<T>(min, max);
}
template <typename T> inline _min_restriction<T> min_restriction(T min) { return _min_restriction<T>(min); }
template <typename T> inline _min_excl_restriction<T> min_excl_restriction(T min) { return _min_excl_restriction<T>(min); }
template <typename T> inline _discrete_restriction<T> discrete_restriction(std::initializer_list<T> val) {
    return _discrete_restriction<T>(val);
}

template <typename T, cci::cci_param_mutable_type TM = cci::CCI_MUTABLE_PARAM> struct cci_param_restricted : public cci::cci_param<T, TM> {

    using value_type = T;

    /** @name Constructors */
    //@{
    /**
     * Constructor with (local/hierarchical) name, default value,
     * description and originator.
     *
     * @param name Name of the parameter
     * @param default_value Default value of the parameter (Typed value)
     * @param desc Description of the parameter
     * @param name_type Either the name should be absolute or relative
     * @param originator Originator of the parameter
     */
    template <typename RESTR>
    cci_param_restricted(const std::string& name, const value_type& default_value, RESTR const& rest, const std::string& desc = "",
                         cci::cci_name_type name_type = cci::CCI_RELATIVE_NAME,
                         const cci::cci_originator& originator = cci::cci_originator())
    : cci::cci_param<T, TM>(name, default_value, desc, name_type, originator) {
        this->template register_pre_write_callback(rest);
        this->template reset();
    }

    /**
     * Constructor with (local/hierarchical) name, default value,
     * private broker, description, name type and originator.
     *
     * @param name Name of the parameter
     * @param default_value Default value of the parameter (Typed value)
     * @param private_broker Associated private broker
     * @param desc Description of the parameter
     * @param name_type Either the name should be absolute or relative
     * @param originator Originator of the parameter
     */
    template <typename RESTR>
    cci_param_restricted(const std::string& name, const value_type& default_value, RESTR const& rest, cci::cci_broker_handle private_broker,
                         const std::string& desc = "", cci::cci_name_type name_type = cci::CCI_RELATIVE_NAME,
                         const cci::cci_originator& originator = cci::cci_originator())
    : cci::cci_param<T, TM>(name, default_value, desc, name_type, originator) {
        this->template register_pre_write_callback(rest);
        this->template reset();
    }

    //@}
};
} // namespace scc
#endif /* _SCC_CCI_PARAM_RESTRICTED_H_ */
