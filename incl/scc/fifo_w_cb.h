/*******************************************************************************
 * Copyright 2019 MINRES Technologies GmbH
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

#pragma once

#include <sysc/communication/sc_prim_channel.h>
#include <deque>
#include <functional>

namespace scc {

template<typename T>
class fifo_w_cb: public sc_core::sc_prim_channel {
public:
	fifo_w_cb(): sc_core::sc_prim_channel(sc_core::sc_gen_unique_name( "fifo_w_cb" )){}

	fifo_w_cb(const char* name): sc_core::sc_prim_channel(name){}

	virtual ~fifo_w_cb(){};

	void push_back(T& t) {
		in_queue.push_back(t);
		request_update();
	}
	void push_back(const T& t) {
		in_queue.push_back(t);
		request_update();
	}

	T& back() {
		return in_queue.back();
	}
	const T& back() const {
		return in_queue.back();
	}

	void pop_front() {
		out_queue.pop_front();
		if (empty_cb)
			empty_cb();
	}

	T& front() {
		return out_queue.front();
	}
	const T& front() const {
		return out_queue.front();
	}

	size_t avail() const {
		return out_queue.size();
	}
	bool empty() const {
		return out_queue.empty();
	}

	void set_avail_cb(std::function<void(void)> f) {
		avail_cb = f;
	}
	void set_empty_cb(std::function<void(void)> f) {
		empty_cb = f;
	}
protected:
	// the update method (does nothing by default)
	virtual void update() {
		if (in_queue.empty())
			return;
		out_queue.insert(out_queue.end(), in_queue.begin(), in_queue.end());
		in_queue.clear();
		if (avail_cb)
			avail_cb();
	}

	std::deque<T> in_queue, out_queue;
	std::function<void(void)> avail_cb, empty_cb;
};

} /* namespace scc */

