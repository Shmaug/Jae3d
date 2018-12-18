#pragma once

#include <stdexcept>

template<typename T>
class jvector {
public:
	jvector() : mSize(0), mCapacity(0), mData(nullptr) {}
	jvector(size_t capacity) : mSize(0), mCapacity(capacity), mData(new char[capacity * sizeof(T)]) {
		ZeroMemory(mData, capacity * sizeof(T));
	}
	jvector(const jvector &vec) : mSize(vec.mSize), mCapacity(vec.mSize) {
		if (mSize > 0 && vec.mData) {
			mData = new char[vec.mSize * sizeof(T)];
			ZeroMemory(mData, vec.mSize * sizeof(T));
			T* ndt = reinterpret_cast<T*>(vec.mData);
			T* mdt = reinterpret_cast<T*>(mData);
			for (size_t i = 0; i < vec.mSize; i++)
				mdt[i] = ndt[i];
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
				ZeroMemory(mData, c * sizeof(T));
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
		} else {
			if (sz > mCapacity) reserve(next_pow2(sz));
			mSize = sz;
		}
	}

	// clear and set data
	void set(const T* data, const size_t sz) {
		free();

		mSize = sz;
		mCapacity = sz;
		mData = new char[sizeof(T) * sz];
		ZeroMemory(mData, sz * sizeof(T));

		T* mdt = reinterpret_cast<T*>(mData);
		for (size_t i = 0; i < sz; i++)
			new (&(mdt[i])) T(data[i]);
	}

	// add an item, expands if necessary
	// calls copy assignment =
	void push_back(const T &item) {
		if (mSize + 1 >= mCapacity) reserve(next_pow2(mCapacity * 2));
		new(&(reinterpret_cast<T*>(mData)[mSize])) T(item);
		mSize++;
	}

	// remove an item at index and shift the data to fill
	void remove(const size_t index) {
		if (index >= mSize) throw std::out_of_range("Index out of bounds!");
		T* data = reinterpret_cast<T*>(mData);
		data[index].~T();
		for (size_t i = index; i < mSize - 1; i++)
			new(&data[i]) T(data[i + 1]);
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
		}
		mSize = 0;
		mCapacity = 0;
	}

	T& operator [](size_t i) {
		if (i >= mSize) throw std::out_of_range("Index out of bounds");
		return reinterpret_cast<T*>(mData)[i];
	}
	T const& operator [](size_t i) const {
		if (i >= mSize) throw std::out_of_range("Index out of bounds");
		return reinterpret_cast<T*>(mData)[i];
	}

	bool empty() const { return mSize == 0; }
	size_t size() const { return mSize; }
	size_t capacity() const { return mCapacity; }
	T* data() const { return reinterpret_cast<T*>(mData); }

	jvector& operator=(const jvector &rhs) {
		if (this == &rhs) return *this;

		realloc(rhs.mSize, rhs.mSize);

		T* ndt = reinterpret_cast<T*>(rhs.mData);
		T* mdt = reinterpret_cast<T*>(mData);
		for (size_t i = 0; i < rhs.mSize; i++)
			mdt[i] = ndt[i];

		return *this;
	}

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
			ZeroMemory(ndata, cap * sizeof(T));

			if (mData) {
				T* ndt = reinterpret_cast<T*>(ndata);
				T* mdt = reinterpret_cast<T*>(mData);
				for (size_t i = 0; i < mSize; i++) {
					if (i < (cap < sz ? cap : sz))
						new(&(ndt[i])) T(mdt[i]);
					mdt[i].~T();
				}

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

