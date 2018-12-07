#pragma once

#include <functional>

template<typename key_t, typename val_t>
class jmap {
public:
	class KeyValuePair {
	public:
		KeyValuePair(KeyValuePair &kp) : mKey(kp.mKey), mValue(kp.mValue), mNext(kp.mNext) {}
		KeyValuePair(key_t &key, val_t &val) : mKey(key), mValue(val), mNext(nullptr) {}
		KeyValuePair(key_t &key, val_t &val, KeyValuePair* next) : mKey(key), mValue(val), mNext(next) {}
		key_t& Key() { return mKey; }
		val_t& Value() { return mValue; }
		void Value(val_t &val) { mValue = val; }
		KeyValuePair* Next() const { return mNext; }
		void Next(KeyValuePair* kp) {
			mNext = kp;
		}

		KeyValuePair& operator=(KeyValuePair& rhs) {
			mKey = rhs.mKey;
			mValue = rhs.mValue;
			mNext = rhs.mNext;
			return *this;
		}

	private:
		key_t mKey;
		val_t mValue;
		KeyValuePair* mNext;
	};
	class jmap_iterator
	{
	private:
		size_t _i;
		KeyValuePair* _kp;
		jmap<key_t, val_t>* _map;
		bool done;

	public:
		jmap_iterator(jmap<key_t, val_t>* map, size_t i, KeyValuePair* kp) : _map(map), _i(i), _kp(kp), done(false) {}

		jmap_iterator& operator++() {
			if (_kp) _kp = _kp->Next();
			while (!_kp){
				_i++;
				if (_i < DATA_SIZE)
					_kp = _map->mData[_i];
				else {
					done = true;
					break;
				}
			}
			return *this;
		}
		jmap_iterator operator++(int) { jmap_iterator retval = *this; ++(*this); return retval; }
		bool operator==(jmap_iterator& other) const { return _kp->Key() == other._kp->Key(); }
		bool operator!=(jmap_iterator& other) const { return !operator==(other); }
		KeyValuePair& operator*() { return *_kp; }
		bool Valid() const { return !done; }
	};
	
	jmap() {}
	jmap(jmap &m) {
		for (int i = 0; i < DATA_SIZE; i++) {
			if (!mData[i]) continue;
			KeyValuePair* nkp = m.mData[i];
			KeyValuePair* p = new(mData[i])KeyValuePair(*nkp);
			nkp = nkp->Next();
			while (nkp) {
				KeyValuePair* n = new KeyValuePair(*nkp);
				p->Next(n);
				p = n;
				nkp = nkp->Next();
			}
		}
	}
	~jmap() {
		clear();
	}

	void clear() {
		for (int i = 0; i < DATA_SIZE; i++) {
			KeyValuePair* kp = mData[i];
			while (kp) {
				KeyValuePair* kp_c = kp;
				kp = kp->Next();
				delete kp_c;
			}
			mData[i] = nullptr;
		}
	}
	bool empty() const {
		for (size_t i = 0; i < DATA_SIZE; ++i)
			if (mData[i] != nullptr) return false;
		return true;
	}

	void emplace(key_t &key, val_t &val) {
		size_t i;
		KeyValuePair* kp = get(key, i);
		if (kp)
			kp->Value(val);
		else
			mData[i] = new KeyValuePair(key, val, mData[i]);
	}
	val_t& at(key_t &key) const {
		size_t i;
		KeyValuePair* kp = get(key, i);
		if (!kp) throw std::out_of_range("Key does not exist!");
		return kp->Value();
	}
	bool has(key_t &key) const {
		size_t i;
		return get(key, i) != nullptr;
	}

	jmap_iterator begin() {
		for (size_t i = 0; i < DATA_SIZE; ++i)
			if (mData[i])
				return jmap_iterator(this, i, mData[i]);
		return jmap_iterator(this, (size_t)-1, nullptr);
	}
	jmap_iterator end() {
		for (size_t i = DATA_SIZE - 1; i >= 0; --i)
			if (mData[i]) {
				KeyValuePair* kp = mData[i];
				while (kp->Next()) kp = kp->Next();
				return jmap_iterator(this, i, kp);
			}
		return jmap_iterator(this, (size_t)-1, nullptr);
	}

	jmap& operator=(jmap& m) {
		for (int i = 0; i < DATA_SIZE; i++) {
			delete mData[i];
			if (!mData[i]) continue;
			KeyValuePair* nkp = m.mData[i];
			KeyValuePair* p = new(mData[i])KeyValuePair(*nkp);
			nkp = nkp->Next();
			while (nkp) {
				KeyValuePair* n = new KeyValuePair(*nkp);
				p->Next(n);
				p = n;
				nkp = nkp->Next();
			}
		}
		return *this;
	}

private:
	static const size_t DATA_SIZE = 61; // prime
	KeyValuePair* mData[DATA_SIZE];

	KeyValuePair* get(key_t &key, size_t &i) const {
		i = std::hash<key_t>{}(key) % DATA_SIZE;
		KeyValuePair* kp = mData[i];
		while (kp) {
			if (kp->Key() == key) return kp;
			kp = kp->Next();
		}
		return nullptr;
	}
};

