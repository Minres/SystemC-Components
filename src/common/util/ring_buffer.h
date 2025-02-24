/*
The MIT License (MIT)
 
Copyright (c) 1999-2022 Mircea Neacsu

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
/*!
 \file ring_buffer.h Simple circular (ring) buffer class

 (c) Mircea Neacsu 2018-2021. Licensed under MIT License.
 This is part of MLIB project. See README file for full license terms.
 (https://github.com/neacsum/mlib#readme)
 */
#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include <cassert>
#include <stdexcept>

namespace util {

/// Circular buffer
template<class T>
class ring_buffer {
public:
    /// Iterator through the circular buffer
    template<bool C_>
    class iterator_type {
    public:
        // Following typedefs must exist to allow instantiation of std::iterator_traits
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using reference = typename std::conditional_t<C_, T const&, T&>;
        using pointer = typename std::conditional_t<C_, T const*, T*>;
        using iterator_category = std::bidirectional_iterator_tag;

        /// Default constructor
        iterator_type():ring(nullptr), pos(0) {}

        /// Dereference operator
        reference operator*() {
            if (pos == (size_t) (-1)) throw std::range_error("Ring buffer iterator at end!");
            return ring->buf[pos];
        }

        /// Dereference operator (const version)
        const reference operator*() const {
            if (pos == (size_t) (-1)) throw std::range_error("Ring buffer iterator at end!");
            return ring->buf[pos];
        }

        /// Object pointer
        pointer operator->() {
            if (pos == (size_t) (-1)) throw std::range_error("Ring buffer iterator at end!");
            return &ring->buf[pos];
        }

        /// Object pointer (const version)
        const pointer operator->() const {
            if (pos == (size_t) (-1)) throw std::range_error("Ring buffer iterator at end!");
            return &ring->buf[pos];
        }

        /// Increment operator (postfix)
        iterator_type<C_> operator++(int) {
            iterator_type<C_> tmp = *this;
            pos = ring->increment(pos);
            return tmp;
        }

        /// Increment operator (prefix)
        iterator_type<C_>& operator++() {
            pos = ring->increment(pos);
            return *this;
        }

        /// Decrement operator (postfix)
        iterator_type<C_> operator--(int) {
            iterator_type<C_> tmp = *this;
            pos = ring->decrement(pos);
            return tmp;
        }

        /// Decrement operator (prefix)
        iterator_type<C_>& operator--() {
            pos = ring->decrement(pos);
            return *this;
        }

        /// Equality comparison
        bool operator==(iterator_type<C_> const& it) const {
            return (ring == it.ring) && (pos == it.pos);
        }

        /// Inequality comparison
        bool operator!=(iterator_type<C_> const& it) const {
            return !operator==(it);
        }

        /// Assignment operator
        iterator_type<C_>& operator=(iterator_type<C_> const& rhs) {
            if (this != &rhs) {
                ring = rhs.ring;
                pos = rhs.pos;
            }
            return *this;
        }

        /// Addition operator
        iterator_type<C_> operator+(size_t inc) const {
            iterator_type<C_> tmp = *this;
            tmp.pos = ring->add(pos, inc);
            return tmp;
        }

        /// Addition assignment operator
        iterator_type<C_>& operator+=(size_t inc) {
            pos = ring->add(pos, inc);
            return *this;
        }

        /// Subtraction operator
        iterator_type<C_> operator-(size_t dec) const {
            iterator_type<C_> tmp = *this;
            tmp.pos = ring->subtract(pos, dec);
            return tmp;
        }

        /// Subtraction assignment operator
        iterator_type<C_>& operator-=(size_t dec) {
            pos = ring->subtract(pos, dec);
            return *this;
        }

        /// Difference operator
        ptrdiff_t operator-(iterator_type<C_> const& other) const {
            assert(ring == other.ring);
            size_t p1 = (pos != -1) ? pos : (ring->back_idx == ring->front_idx) ? ring->sz : ring->back_idx;
            size_t p2 = (other.pos != -1) ? other.pos : (ring->back_idx == ring->front_idx) ? ring->sz : ring->back_idx;
            if (p1 >= p2)
                return p1 - p2;
            else
                return ring->cap - p2 + p1;
        }

    private:
        iterator_type(ring_buffer const* ring_, size_t pos_):ring(ring_), pos(pos_) {}

        ring_buffer const* ring;
        size_t pos;
        friend class ring_buffer;
    };

    typedef iterator_type<false> iterator;
    typedef iterator_type<true> const_iterator;

    /// Constructor
    ring_buffer(size_t size):buf(std::unique_ptr<T[]>(new T[size])), cap(size), front_idx(0), back_idx(0), sz(0) {}

    /// Default constructor
    ring_buffer():cap(0), front_idx(0), back_idx(0), sz(0) {}

    /// Copy constructor
    ring_buffer(ring_buffer const& other) = delete;

    /// Copy constructor
    ring_buffer(ring_buffer&& other) = delete;

    /// Initializer list constructor
    ring_buffer(std::initializer_list<T> il)
    : buf(std::unique_ptr<T[]>(new T[il.size()]))
    , cap(il.size())
    , front_idx(0)
    , back_idx(0)
    , sz(il.size()) {
        size_t i = 0;
        for (const auto v : il)
            buf[i++] = std::move(v);
    }

    /// Assignment operator
    ring_buffer& operator=( ring_buffer const& rhs) = delete;

    ring_buffer& operator=(ring_buffer&& rhs) = delete;

    /// Equality operator
    bool operator==(ring_buffer<T> const& other) const noexcept {
        if (cap == other.cap && sz == other.sz) {
            for (size_t i = 0; i < sz; i++) {
                size_t idx1 = (front_idx + i) % cap;
                size_t idx2 = (other.front_idx + i) % other.cap;
                if (buf[idx1] != other.buf[idx2]) return false;
            }
            return true;
        }
        return false;
    }

    /// Inequality operator
    bool operator!=(ring_buffer<T> const& other) const noexcept {
        return !operator==(other);
    }

    /// Vector conversion operator
    operator std::vector<T>() const {
        std::vector<T> v;
        v.reserve(sz);
        std::copy(std::begin(*this), std::end(*this), std::back_inserter(v));
        return v;
    }

    /// Inserts new element in buffer
    void push_back(T const& item) {
        if (!cap) return; // container not allocated
            throw std::length_error("ring_buffer: trying to add an element to a buffer of size 0");
        if (sz && back_idx == front_idx)
            throw std::overflow_error("ring_buffer: overflow error");
        sz++;
        buf[back_idx] = item;
        back_idx = (back_idx + 1) % cap;
    }

    void push_back(T && item) {
        if (!cap) return; // container not allocated
            throw std::length_error("ring_buffer: trying to add an element to a buffer of size 0");
        if (sz && back_idx == front_idx)
            throw std::overflow_error("ring_buffer: overflow error");
        sz++;
        buf[back_idx] = std::move(item);
        back_idx = (back_idx + 1) % cap;
    }

    /// Return an iterator pointing to first (oldest) element in buffer
    iterator begin() noexcept {
        return iterator(this, front_idx);
    }

    /// Return a const iterator pointing to first (oldest) element in buffer
    const_iterator begin() const noexcept {
        return const_iterator(this, front_idx);
    }

    /// Return a const iterator pointing to first (oldest) element in buffer
    const_iterator cbegin() const noexcept {
        return const_iterator(this, front_idx);
    }

    /// Return an iterator pointing past the last (newest) element in buffer
    iterator end() noexcept {
        return iterator(this, (size_t) (-1));
    }

    /// Return a const iterator pointing past the last (newest) element in buffer
    const_iterator end() const noexcept {
        return const_iterator(this, (size_t) (-1));
    }

    /// Return an iterator pointing past the last (newest) element in buffer
    const_iterator cend() const noexcept {
        return const_iterator(this, (size_t) (-1));
    }

    /// Remove oldest element from buffer
    void pop_front() {
        if (!sz)
            throw std::underflow_error("ring_buffer: buffer is empty");
        front_idx = (front_idx + 1) % cap;
        sz--;
    }

    /// Return a reference to first (oldest) element in buffer.
    T& front() {
        if (!sz)
            throw std::underflow_error("ring_buffer: buffer is empty");
        return buf[front_idx];
    }

    /// Return a reference to first (oldest) element in buffer.
    T const&  front() const {
        if (!sz)
            throw std::underflow_error("ring_buffer: buffer is empty");
        return buf[front_idx];
    }

    /// Return reference to last (newest) element in buffer.
    T& back() {
        if (!sz)
            throw std::underflow_error("ring_buffer: buffer is empty");
        return buf[(back_idx + cap - 1) % cap];
    }

    /// Return reference to last (newest) element in buffer.
    T const& back() const {
        if (!sz)
            throw std::underflow_error("ring_buffer: buffer is empty");
        return buf[(back_idx + cap - 1) % cap];
    }

    /// Remove all elements from buffer
    void clear(void) {
        for (size_t i = 0; i < sz; i++) {
            buf[(front_idx + i) % cap].~T();
        }

        front_idx = back_idx;
        sz = 0;
    }

    /// Return true if buffer is empty
    bool empty(void) const noexcept {
        return (sz == 0);
    }

    /// Return true if buffer is full
    bool full(void) const noexcept {
        return (sz == cap);
    }

    /// Return maximum buffer size
    size_t capacity(void) const noexcept {
        return cap;
    }
    ;

    ///(Re)allocate buffer with a different capacity
    void resize(size_t new_cap) {
        if(new_cap<sz)
            throw std::bad_array_new_length();
        std::unique_ptr<T[]> newbuf(new_cap ? new T[new_cap] : nullptr);
        if (new_cap) {
            for (size_t i = 0; i < sz; i++) {
                newbuf[i] = std::move(buf[(front_idx + i) % cap]);
            }
        } else
            sz = 0;
        cap = new_cap;
        buf.swap(newbuf);
        front_idx = 0;
        back_idx = cap ? (front_idx + sz) % cap : 0;
    }

    /// Return number of elements in buffer
    size_t size() const noexcept {
        return sz;
    }

private:
    /// increment an iterator index
    size_t increment(size_t pos) const noexcept {
        if (cap && pos != (size_t) -1) pos = (pos + 1) % cap;
        if (pos == back_idx) pos = (size_t) -1;
        return pos;
    }

    /// decrement an iterator index
    size_t decrement(size_t pos) const noexcept {
        if (cap) {
            if (pos == (size_t) -1)
                pos = (back_idx + cap - 1) % cap;
            else
                if (pos != front_idx) pos = (pos + cap - 1) % cap;
        }
        return pos;
    }

    /// add a value an iterator index
    size_t add(size_t oldpos, size_t delta) const noexcept {
        if (cap && oldpos != -1) {
            size_t np = oldpos + (delta % cap);
            if (np >= cap && np >= back_idx + cap)
                np = (size_t) (-1);
            else
                np %= cap;
            return np;
        }
        return oldpos;
    }

    /// subtract a value from an iterator index
    size_t subtract(size_t oldpos, size_t delta) const noexcept {
        if (cap) {
            size_t np = (oldpos == (size_t) -1 ? back_idx : oldpos) - (delta % cap) + cap;
            if (np < front_idx)
                np = front_idx;
            else
                np %= cap;
            return np;
        }
        return oldpos;
    }

    std::unique_ptr<T[]> buf;
    size_t front_idx, back_idx, cap, sz;
};

} // namespace util

