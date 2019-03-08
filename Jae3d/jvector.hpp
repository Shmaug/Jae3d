#pragma once

#include <stdexcept>

template<typename T>
class jvector {
public:
	jvector() : mSize(0), mCapacity(0), mData(nullptr) {}
	jvector(size_t capacity) : mSize(0), mCapacity(capacity), mData(new char[capacity * sizeof(T)]) {
		memset(mData, 0, capacity * sizeof(T));
	}
	jvector(const jvector &vec) : mSize(vec.mSize), mCapacity(vec.mSize) {
		if (mSize > 0 && vec.mData) {
			mData = new char[vec.mSize * sizeof(T)];
			memset(mData, 0, vec.mSize * sizeof(T));
			T* ndt = reinterpret_cast<T*>(vec.mData);
			for (size_t i = 0; i < vec.mSize; i++)
				new (mData + i * sizeof(T)) T(ndt[i]);
		} else {
			mData = nullptr;
			mSize = 0;
			mCapacity = 0;
		}
	}
	~jvector() {
		free();
	}

	// increase capacity to c
	// if c <= capacity, does nothing
	void reserve(const size_t c) {
		if (c > mCapacity) {
			if (mData)
				realloc(c, mSize);
			else {
				mData = new char[c * sizeof(T)];
				memset(mData, 0, c * sizeof(T));
				mSize = 0; // should already be zero
			}
			mCapacity = c;
		}
	}

	// change size to sz
	// if sz > size, will expand
	// if sz < size, will shrink of shrink == true
	void resize(const size_t sz, bool shrink = false) {
		if (sz < mSize) {
			if (shrink) 
				realloc(next_pow2(sz), sz);
			else {
				destruct(sz, mSize);
				mSize = sz;
			}
		} else if (sz > mSize) {
			if (sz > mCapacity) reserve(sz);
			for (size_t i = mSize; i < sz; i++)
				new(&(reinterpret_cast<T*>(mData)[i])) T();
			mSize = sz;
		}
	}

	// clear and set data
	void set(const T* data, const size_t sz) {
		free();

		mSize = sz;
		mCapacity = sz;
		mData = new char[sizeof(T) * sz];
		memset(mData, 0, sz * sizeof(T));

		T* mdt = reinterpret_cast<T*>(mData);
		for (size_t i = 0; i < sz; i++)
			new (&(mdt[i])) T(data[i]);
	}

	int find(const T &item) {
		T* data = reinterpret_cast<T*>(mData);
		for (int i = 0; i < mSize; i++)
			if (data[i] == item)
				return i;
		return -1;
	}

	// add an item, expands if necessary
	// calls copy constructor
	T& push_back(const T &item) {
		if (mSize + 1 >= mCapacity) reserve(next_pow2(mSize + 1));
		mSize++;
		return *new(&(reinterpret_cast<T*>(mData)[mSize - 1])) T(item);
	}

	// remove an item at index and shift the data to fill
	void remove(const size_t index) {
		if (index >= mSize) throw std::out_of_range("Index out of bounds!");
		T* data = reinterpret_cast<T*>(mData);
		data[index].~T();
		memcpy(data + index, data + index + 1, mSize - index - 1);
		mSize--;
	}

	// destruct data, does not free memory
	void clear() {
		if (mSize == 0) return;
		if (mData) destruct(0, mSize);
		mSize = 0;
	}

	// destruct data and free memory
	void free() {
		if (mCapacity == 0) return;
		if (mData) {
			destruct(0, mSize);
			delete[] mData;
			mData = nullptr;
		}
		mSize = 0;
		mCapacity = 0;
	}

	inline T& operator [](size_t i) {
		if (i >= mSize) throw std::out_of_range("Index out of bounds");
		return reinterpret_cast<T*>(mData)[i];
	}
	inline T const& operator [](size_t i) const {
		if (i >= mSize) throw std::out_of_range("Index out of bounds");
		return reinterpret_cast<T*>(mData)[i];
	}

	inline bool empty() const { return mSize == 0; }
	inline size_t size() const { return mSize; }
	inline size_t capacity() const { return mCapacity; }
	inline T* data() const { return reinterpret_cast<T*>(mData); }

	jvector<T>& operator=(const jvector<T> &rhs) {
		if (this == &rhs) return *this;

		free();
		reserve(rhs.mSize);

		T* ndt = reinterpret_cast<T*>(rhs.mData);
		for (size_t i = 0; i < rhs.mSize; i++)
			new (mData + i * sizeof(T)) T(ndt[i]);

		mSize = rhs.mSize;

		return *this;
	}

	class iterator : public std::iterator<std::bidirectional_iterator_tag, T> {
	private:
		T* data;

	public:
		iterator(const iterator& it) : data(it.data) {};
		iterator(T* pos) : data(pos) {};
		
		iterator operator++(int) {
			iterator tmp(data);
			data++;
			return tmp;
		}
		iterator& operator++() {
			data++;
			return *this;
		}
		iterator operator--(int) {
			iterator tmp(data);
			data--;
			return tmp;
		}
		iterator& operator--() {
			data--;
			return *this;
		}
		bool operator==(const iterator& rhs) {
			return data == rhs.data;
		}
		bool operator!=(const iterator& rhs) {
			return data != rhs.data;
		}
		T& operator*() const { return *data; }
		T& operator->() const { return *data; }
	};

	iterator begin() const { return iterator(data()); }
	iterator end() const { return iterator(data() + size()); }

private:
	size_t mSize;
	size_t mCapacity;
	char* mData;

	inline size_t next_pow2(size_t x) {
		size_t p = 1;
		if (x && !(x & (x - 1)))
			return x;

		while (p < x)
			p <<= 1;

		return p;
	}

	void realloc(size_t cap, size_t sz) {
		if (cap != mCapacity) {
			char* ndata = new char[cap * sizeof(T)];
			memset(ndata, 0, cap * sizeof(T));

			if (mData) {
				memcpy(ndata, mData, sizeof(T) * (mSize < sz ? mSize : sz));
				destruct(sz, mSize);
				delete[] mData;
			}

			mData = ndata;
			mCapacity = cap;
		} else
			destruct(sz, mSize);

		mSize = sz;
	}
	inline void destruct(size_t from, size_t to) {
		T* mdt = reinterpret_cast<T*>(mData);
		for (size_t i = from; i < to; i++)
			mdt[i].~T();
	}
};