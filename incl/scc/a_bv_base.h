#ifndef A_BV_BASE_H_
#define A_BV_BASE_H_

#include <cstdlib>
#include <iostream>
#include "gmp.h"
#include "gmpxx.h"

#define MP1 ((mp_limb_t)1)

class a_bv_base {
    public:
        a_bv_base();
        a_bv_base(const a_bv_base& other);

        a_bv_base(const int W, mp_limb_t* src);
        virtual ~a_bv_base();

    bool operator!=(const a_bv_base& other) const;

    void change(const a_bv_base& v);

    a_bv_base& operator = (const a_bv_base &v);

    inline operator bool() const {
    if(new_val)
        return *new_val;
    else
        return false;
    }

    int length() const;
    const std::string bin_trace() const;
    mp_limb_t *new_val;
    mp_limb_t *old_val_base;
    int limbs;
    int m_len;

};


#endif //A_BV_BASE_H_
