/**
Copyright (c) 2015-2018, Pavlo M, https://github.com/olvap80
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of deferpp nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef UTIL_DEFER_H
#define UTIL_DEFER_H

/// Defer following code until enclosing scope is exited
/** Usage: DEFER{ some_code_to_be_deferred };
    Remember that some_code_to_be_deferred shall not allow
    exceptions to be propagated out of curly braces */
#define DEFER const auto DEFER_CAT_ID(callOnScopeExit, __LINE__) = (Util::tagClassForLambda)->*[&]()

//==============================================================================
// Implementation details follow

// Helper macro to expand and concatenate macro arguments into combined identifier
#define DEFER_CAT_ID(a, b) DEFER_CAT_ID_EXPANDED_HELPER(a, b)
// helper macro to concatenate expanded macro arguments
#define DEFER_CAT_ID_EXPANDED_HELPER(a, b) a##b

namespace Util {
/// Helper type to trigger operator ->*
struct TagClassForLambda {
    constexpr TagClassForLambda() = default;
};
/// Use this "instance" to trigger overloaded operator ->*
/** The trick with tagClassForLambda is needed
    to infer the type of the lambda */
constexpr TagClassForLambda tagClassForLambda;

/// RAII for implementing DEFER behavior
template <class Lambda> struct CallOnScopeExit {
    /// Create RAII wrapper around Lambda
    /** Using Lambda directly, optimizer takes case due to [&]() in front */
    constexpr CallOnScopeExit(Lambda initialLambda)
    : lambda(initialLambda)
    , isOwner(true) {}

    /// Usually optimized away due to RVO
    CallOnScopeExit(CallOnScopeExit&& other)
    : lambda(other.lambda)
    , isOwner(true) {
        other.isOwner = false;
    }

    // ensure copy changes go only through move constructor
    CallOnScopeExit(const CallOnScopeExit& other) = delete;
    CallOnScopeExit& operator=(const CallOnScopeExit& other) = delete;

    /// Actual lambda call once CallOnScopeExit goes out of scope
    ~CallOnScopeExit() {
        if(isOwner) { // condition is usually optimized away
            lambda();
        }
    }

private:
    const Lambda lambda; ///< Hold lambda, avoid slow std::function here
    bool isOwner;        ///< Ensure 100% lambda is called only one time
};

/// Helper operator to easy catch lambda for DEFER macro
/** Use template to avoid slow std::function
    (raw lambda is copied/stored here) */
template <class Lambda> constexpr CallOnScopeExit<Lambda> operator->*(const TagClassForLambda&, Lambda lambda) {
    return CallOnScopeExit<Lambda>(lambda);
}
} // namespace Util
#endif // UTIL_DEFER_H