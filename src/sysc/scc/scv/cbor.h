/*
 * cbor.h
 *
 *  Created on: Feb 7, 2023
 *      Author: eyckj
 */

#ifndef OSCI_LIB_SCC_SRC_SYSC_SCC_CBOR_H_
#define OSCI_LIB_SCC_SRC_SYSC_SCC_CBOR_H_

#include <fstream>
#include <vector>
#include <memory>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <lz4.h>
#include <ctime>

namespace cbor {
enum {
	MAX_TXBUFFER_SIZE=1<<16,
	MAX_REL_SIZE=1<<16,
	INFO_CHUNK_ID=0,
	DICT_CHUNK_ID=1,
	DIR_CHUNK_ID=2,
	TX_CHUNK_ID=3,
	REL_CHUNK_ID=4
};

struct memory_writer {
	std::vector<uint8_t> buffer;
	void push(uint8_t value) { buffer.push_back(value);	}
	void push(char value) { push(static_cast<uint8_t>(value)); }
	void push(uint8_t const* values, size_t size) { buffer.insert(std::end(buffer), values, values+size); }
	void push(char const* values, size_t size) { push(reinterpret_cast<uint8_t const*>(values), size); }
	bool is_empty(){return buffer.empty();}
	void clear(){ buffer.clear(); }
	void append(memory_writer& o) {
		buffer.insert(buffer.end(), o.buffer.begin(), o.buffer.end());
	}
};

struct file_writer {
	std::ofstream ofs;
	std::vector<uint8_t> buffer;
	void push(uint8_t value) { push(static_cast<char>(value));	}
	void push(char value) { ofs.write(&value, 1); }
	void push(uint8_t const* values, size_t size) { push(reinterpret_cast<char const*>(values), size); }
	void push(char const* values, size_t size) { ofs.write(values, size); }
	bool is_empty(){return false;}
	void clear(){ }
};

template<typename OUTPUT>
struct encoder: public OUTPUT {
	void write(bool value){ this->push(static_cast<uint8_t>(value?0xf5:0xf4)); }// 7::21, 7::20
	template<typename T>
	typename std::enable_if<std::is_signed<T>::value, void>::type
	write(T value) {
		if(value < 0)
			write_type_value(1, std::abs(value)-1);
		else
			write_type_value(0, value);
	}
	template<typename T>
	typename std::enable_if<std::is_unsigned<T>::value, void>::type
	write(T value){ write_type_value(0, value); }
	void write(float value){
		auto* punny = reinterpret_cast<uint8_t*>(&value);
		this->push(static_cast<uint8_t>((7<<5) | 26));
		this->push(*(punny+3));
		this->push(*(punny+2));
		this->push(*(punny+1));
		this->push(*(punny+0));
	}
	void write(double value){
		auto* punny = reinterpret_cast<uint8_t*>(&value);
		this->push(static_cast<uint8_t>((7<<5) | 27));
		this->push(*(punny+7));
		this->push(*(punny+6));
		this->push(*(punny+5));
		this->push(*(punny+4));
		this->push(*(punny+3));
		this->push(*(punny+2));
		this->push(*(punny+1));
		this->push(*(punny+0));
	}
	template<typename T>
	typename std::enable_if<std::is_unsigned<T>::value, void>::type
	write(T const* data, size_t size) {
		write_type_value(2, size);
		this->push(data, size);
	}
	template<typename T>
	typename std::enable_if<std::is_signed<T>::value, void>::type
	write(T const* data, size_t size){
		write_type_value(3, size);
		this->push(data, size);
	}
	void write(const std::string str){
		write_type_value(3, str.size());
		this->push(str.c_str(), str.size());
	}
	void start_array(size_t size = std::numeric_limits<size_t>::max()) {
		if(size < std::numeric_limits<size_t>::max())
			write_type_value(4, size);
		else
			this->push(static_cast<uint8_t>((4<<5) | 31));
	}
	void start_map(size_t size = std::numeric_limits<size_t>::max()) {
		if(size < std::numeric_limits<size_t>::max())
			write_type_value(5, size);
		else
			this->push(static_cast<uint8_t>((5<<5) | 31));
	}
	void write_break() { this->push(static_cast<uint8_t>(0xff)); } // 7::31
	void write_tag(const uint64_t tag) { write_type_value(6, tag); }
	void write_special(uint64_t special) { write_type_value(7, special); }
	void write_null() { this->push(0xf6); } // 7::22
	void write_undefined() { this->push(0xf7); } // 7::23
private:
	void write_type_value(int major_type, uint64_t value){
		major_type <<= 5;
		if(value < 24) {
			this->push(static_cast<uint8_t>(major_type | value));
		} else if(value < std::numeric_limits<uint8_t>::max()) {
			this->push(static_cast<uint8_t>(major_type | 24));
			this->push(static_cast<uint8_t>(value));
		} else if(value < std::numeric_limits<uint16_t>::max()) {
			this->push(static_cast<uint8_t>(major_type | 25));
			this->push(static_cast<uint8_t>(value >> 8));
			this->push(static_cast<uint8_t>(value));
		} else  if(value < std::numeric_limits<uint32_t>::max()) {
			this->push(static_cast<uint8_t>(major_type | 26));
			this->push(static_cast<uint8_t>(value >> 24));
			this->push(static_cast<uint8_t>(value >> 16));
			this->push(static_cast<uint8_t>(value >> 8));
			this->push(static_cast<uint8_t>(value));
		} else {
			this->push(static_cast<uint8_t>(major_type | 27));
			this->push(static_cast<uint8_t>(value >> 56));
			this->push(static_cast<uint8_t>(value >> 48));
			this->push(static_cast<uint8_t>(value >> 40));
			this->push(static_cast<uint8_t>(value >> 32));
			this->push(static_cast<uint8_t>(value >> 24));
			this->push(static_cast<uint8_t>(value >> 16));
			this->push(static_cast<uint8_t>(value >> 8));
			this->push(static_cast<uint8_t>(value));
		}
	}
};

template<bool COMPRESSED=false>
struct chunk_writer {
	encoder<file_writer> enc;
	chunk_writer(std::string const& filename) {
		enc.ofs.open(filename, std::ios::binary|std::ios::out);
		if(enc.ofs.is_open()) {
			enc.write_tag(55799); // Self-Described CBOR
			enc.start_array();
		}
	}

	~chunk_writer() {
		if(enc.ofs.is_open()) {
			enc.write_break();
			enc.ofs.close();
		}
	}

	void write_chunk(uint64_t type, std::vector<uint8_t> const& data, uint64_t subtype = std::numeric_limits<uint64_t>::max()){
		if(COMPRESSED && type>INFO_CHUNK_ID) {
	        enc.write_tag(6+type*2+1); // unassigned tags
	        if(subtype < std::numeric_limits<uint64_t>::max()) {
	            enc.start_array(3);
	            enc.write(subtype);
	        } else
	            enc.start_array(2);
            enc.write(data.size());
            const int max_dst_size = LZ4_compressBound(data.size());
            uint8_t* compressed_data = (uint8_t*)malloc(max_dst_size);
            const int compressed_data_size = LZ4_compress_default(
                    reinterpret_cast<char const*>(data.data()),
                    reinterpret_cast<char*>(compressed_data),
                    data.size(), max_dst_size);
            enc.write(compressed_data, compressed_data_size);
            free(compressed_data);
		} else {
	        enc.write_tag(6+type*2); // unassigned tags
	        if(subtype < std::numeric_limits<uint64_t>::max()) {
	            enc.start_array(2);
	            enc.write(subtype);
	        }
            enc.write(data.data(),  data.size());
		}
	}
};

struct info {
    encoder<memory_writer> enc;

    inline void add_time_scale(int8_t timescale) {
        enc.start_array(2);
        enc.write(timescale);
        enc.write_tag(1);
        enc.write(time(nullptr));
    }

    template<bool COMPRESSED>
    void flush(chunk_writer<COMPRESSED>& cw) {
        if(enc.is_empty()) return;
        cw.write_chunk(INFO_CHUNK_ID, enc.buffer);
        enc.buffer.clear();
    }

    size_t size() {
        return enc.buffer.size();
    }
};

struct dictionary {
	std::vector<std::string> out_dict{""};
	std::unordered_map<std::string, size_t> lut;
	size_t flushed_idx{0}, unflushed_size{1};

	size_t get_key(std::string const& str) {
		if(!str.length()) return 0;
		auto it = lut.find(str);
		if(it!=std::end(lut)) return it->second;
		out_dict.push_back(str);
		lut[str] = out_dict.size()-1;
		unflushed_size += str.size();
		return out_dict.size()-1;
	}

	template<bool COMPRESSED>
	void flush(chunk_writer<COMPRESSED>& cw) {
		if(!unflushed_size) return;
		encoder<memory_writer> enc;
		enc.start_map(out_dict.size()-flushed_idx);
		for(auto i=flushed_idx; i<out_dict.size(); ++i) {
			enc.write(i);
			enc.write(out_dict[i]);
		}
		cw.write_chunk(DICT_CHUNK_ID, enc.buffer);
		flushed_idx = out_dict.size();
		unflushed_size = 0;
	}
};

struct directory {
	encoder<memory_writer> enc;
	dictionary& dict;
	directory(dictionary& dict):dict(dict){}

	inline void add_stream(uint64_t id, std::string const& name, std::string const& kind) {
		if(enc.is_empty()) enc.start_array();
		enc.write_tag(16);
		enc.start_array(3);
		enc.write(id);
		enc.write(dict.get_key(name));
		enc.write(dict.get_key(kind));
	}

	inline void add_generator(uint64_t id, std::string const& name, uint64_t stream) {
		if(enc.is_empty()) enc.start_array();
		enc.write_tag(17);
		enc.start_array(3);
		enc.write(id);
		enc.write(dict.get_key(name));
		enc.write(stream);
	}

	template<bool COMPRESSED>
	void flush(chunk_writer<COMPRESSED>& cw) {
		if(enc.is_empty()) return;
		dict.flush(cw);
		enc.write_break();
		cw.write_chunk(DIR_CHUNK_ID, enc.buffer);
		enc.buffer.clear();
	}

	size_t size() {
		return enc.buffer.size();
	}
};

struct relations {
	encoder<memory_writer> enc;
	dictionary& dict;
	relations(dictionary& dict):dict(dict){}

	inline void add_relation(std::string const& name, uint64_t from, uint64_t to) {
		if(enc.is_empty()) enc.start_array();
		enc.start_array(3);
		enc.write(dict.get_key(name));
		enc.write(from);
		enc.write(to);
	}

	template<bool COMPRESSED>
	void flush(chunk_writer<COMPRESSED>& cw) {
		if(enc.is_empty()) return;
		dict.flush(cw);
		enc.write_break();
		cw.write_chunk(REL_CHUNK_ID, enc.buffer);
		enc.buffer.clear();
	}

	size_t size() {
		return enc.buffer.size();
	}
};

struct tx_entry {
	encoder<memory_writer> enc;
	size_t elem_count{0};
	uint64_t id{0};
	uint64_t generator{0};
	uint64_t stream{0};
	uint64_t start_time{0}, end_time{0};

	void reset(){
		enc.clear();
		elem_count=0;
		start_time=0;
		end_time=0;
	}

	template<typename T>
	void add_attribute(uint64_t type, uint64_t name_id, uint64_t type_id, T value) {
		enc.write_tag(7+type);
		enc.start_array(3);
		enc.write(name_id);
		enc.write(type_id);
		enc.write(value);
		elem_count++;
	}

	void append_to(encoder<memory_writer>& out) {
		out.start_array(elem_count+1);
		out.write_tag(6);
		out.start_array(4);
		out.write(id);
		out.write(generator);
		out.write(start_time);
		out.write(end_time);
		out.append(enc);
	}
};

struct tx_block {
	encoder<memory_writer> enc;
	dictionary& dict;
	const uint64_t stream_id;
	tx_block(dictionary& dict, uint64_t stream_id):dict(dict), stream_id(stream_id){}

	void append(tx_entry& e) {
		if(enc.is_empty()) enc.start_array();
		e.append_to(enc);
	}

	template<bool COMPRESSED>
	void flush(chunk_writer<COMPRESSED>& cw) {
		if(enc.is_empty()) return;
		dict.flush(cw);
		enc.write_break();
		cw.write_chunk(TX_CHUNK_ID, enc.buffer, stream_id);
		enc.buffer.clear();
	}

	size_t size() {
		return enc.buffer.size();
	}
};

/**
 * format according to RFC 8949: Concise Binary Object Representation (CBOR)
 * file format:
 *   cbot tag(55799)
 *   cbor array(*) of tagged chunks, chunks can be
 *     chunk type 0 (info)
 *       cbor tag(6)
 *       array(2);
 *         int - timescale (exponent of minimum timestep in seconds)
 *         epoch time - creation time 
 *           cbor tag(1)
 *           unsigned - timestamp
 *     chunk type 1 (dictionary)
 *       cbor tag(8) uncompressed
 *       bytes() - content
 *       cbor tag(9) compressed
 *       array(2)
 *         unsigned - uncompressed size
 *         bytes() - content
 *     chunk type 2 (directory)
 *       cbor tag(10) uncompressed
 *       bytes() - content
 *       cbor tag(11) compressed
 *       array(2)
 *         unsigned - uncompressed size
 *         bytes() - content
 *     chunk type 3 (tx block)
 *       cbor tag(12) (compressed: 13)
 *       array(2)
 *         unsigned - stream id
 *         bytes() - content
 *     chunk type 4 (tx relationships)
 *       cbor tag(14) (compressed: 15)
 *       bytes() - content
 * -----------------------------------------
 * chunk content formats:
 * 	- chunk type 1
 *    map of fixed size of (unsigned, string)
 *  - chunk of type 2
 *    array(*) of either
 *      cbor tag(16)
 *      array(3)
 *        unsinged - id
 *        unsinged - name (id of string)
 *        unsinged - kind (id of string)
 *      cbor tag(17)
 *      array(3)
 *        unsinged - id
 *        unsinged - name (id of string)
 *        unsinged - kind (id of string)
 *  - chunk of type 3
 *    array(*) - list of transactions
 *      array() - transaction with pro12perties:
 *        cbor tag(6) - time stamps
 *        array(4)
 *          unsigned - id
 *          unsigned - generator id
 *          unsigned - start time (in ps)
 *          unsigned - end time (in ps)
 *       cbor tag(7)
 *       array(3) - attribute at begin of tx
 *         unsigned - name (id of string)
 *         unsigned - data_type
 *         [signed, unsigned, double] - value (depending of type)
 *       cbor tag(8)
 *       array(3) - attribute at tx
 *         unsigned - name (id of string)
 *         unsigned - data_type
 *         [signed, unsigned, double] - value (depending of type)
 *       cbor tag(9)
 *       array(3) - attribute at end of tx
 *         unsigned - name (id of string)
 *         unsigned - data_type
 *         [signed, unsigned, double] - value (depending of type)
 */
enum class event_type { BEGIN, RECORD, END };
enum class data_type {
    BOOLEAN,                      // bool
    ENUMERATION,                  // enum
    INTEGER,                      // char, short, int, long, long long, sc_int, sc_bigint
    UNSIGNED,                     // unsigned { char, short, int, long, long long }, sc_uint, sc_biguint
    FLOATING_POINT_NUMBER,        // float, double
    BIT_VECTOR,                   // sc_bit, sc_bv
    LOGIC_VECTOR,                 // sc_logic, sc_lv
    FIXED_POINT_INTEGER,          // sc_fixed
    UNSIGNED_FIXED_POINT_INTEGER, // sc_ufixed
    POINTER,                      // T*
    STRING,                       // string, std::string
	TIME,                         // sc_time
	NONE
};

template<bool COMPRESSED=false>
struct chunked_writer  {

	chunk_writer<COMPRESSED> cw;
	info inf;
	dictionary dict;
	directory dir{dict};
	relations rel{dict};
	std::vector<std::unique_ptr<tx_block>> fiber_blocks;
	std::unordered_map<uint64_t, tx_entry*> txs;
	std::vector<tx_entry*> free_pool;
	std::vector<void*> free_pool_blocks;

	chunked_writer(const std::string& name): cw(name){}

	~chunked_writer() {
		dict.flush(cw);
		dir.flush(cw);
		for(auto&e: txs)
			endTransaction(e.first, 0);
		for(auto& block: fiber_blocks)
			if(block)
				block->flush(cw);
		rel.flush(cw);
		for(auto e: free_pool_blocks) free(e);
	}

    inline void writeInfo(int8_t timescale) {
        inf.add_time_scale(timescale);
        inf.flush(cw);
    }

    inline void writeStream(uint64_t id, std::string const& name, std::string const& kind) {
		dir.add_stream(id, name, kind);
		if(id>=fiber_blocks.size()) fiber_blocks.resize(id+1);
		fiber_blocks[id].reset(new tx_block(dict, id));
	}

	inline void writeGenerator(uint64_t id, std::string const& name, uint64_t stream) {
		dir.add_generator(id, name, stream);
	}

	inline void startTransaction(uint64_t id, uint64_t generator, uint64_t stream, uint64_t time) {
		if(dir.size())
			dir.flush(cw);
		if(!free_pool.size()) {
			const auto block_size = sizeof(tx_entry)*64;
			auto p = malloc(sizeof(uint8_t)*block_size);
			auto up = static_cast<uint8_t*>(p);
			for(auto pp=up; pp<(up+block_size); pp+=sizeof(tx_entry))
				free_pool.push_back(new(pp) tx_entry());
			free_pool_blocks.push_back(p);
		}
		auto* e = free_pool.back();
		free_pool.pop_back();
		txs[id] = e;
		e->id = id;
		e->generator = generator;
		e->stream = stream;
		e->start_time = time;
	}

	inline void endTransaction(uint64_t id, uint64_t time) {
		auto e = txs[id];
		e->end_time = time;
		auto* block = fiber_blocks[e->stream].get();
		block->append(*e);
		if(block->size()>MAX_TXBUFFER_SIZE){
			block->flush(cw);
		}
		txs.erase(id);
		e->reset();
		free_pool.push_back(e);
	}

	inline void writeAttribute(uint64_t id, event_type event, const std::string& name, data_type type, const std::string& value) {
		txs[id]->add_attribute(static_cast<uint64_t>(event), dict.get_key(name), static_cast<uint64_t>(type), dict.get_key(value));
	}

	inline void writeAttribute(uint64_t id, event_type event, const std::string& name, data_type type, const char* value) {
		txs[id]->add_attribute(static_cast<uint64_t>(event), dict.get_key(name), static_cast<uint64_t>(type), dict.get_key(value));
	}

	template<typename T>
	inline void writeAttribute(uint64_t id, event_type event, const std::string& name, data_type type, T value) {
		txs[id]->add_attribute(static_cast<uint64_t>(event), dict.get_key(name), static_cast<uint64_t>(type), value);
	}

	inline void writeRelation(const std::string& name, uint64_t sink_id, uint64_t src_id) {
		rel.add_relation(name, src_id, sink_id);
		if(rel.size()>MAX_REL_SIZE){
			rel.flush(cw);
		}
	}
};
}
#endif /* OSCI_LIB_SCC_SRC_SYSC_SCC_CBOR_H_ */
