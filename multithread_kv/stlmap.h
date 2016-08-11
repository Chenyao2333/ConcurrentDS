#include <iostream>
#include <map>

template <class K, class V>
class HashTable
{
	private:
		std::map<K, V> h;
	public:
		using key_type = K;
		using value_type = V;
		using iterator = typename std::map<key_type, value_type>::iterator;
		HashTable(){}
		void Put(key_type key, value_type val)
		{
			h[key] = val;
			if (h[key]==0)
				h.erase(key);
		}
		V Get(K key)
		{
			iterator it = h.find(key);
			if (it != h.end())
				return it->second;
			return V();
		}
		void AtomicAdd(key_type key, value_type delta)
		{
			h[key] += delta;
			if (h[key]==0)
				h.erase(key);
		}
		iterator begin() {return h.begin(); }
		iterator end() {return h.end(); }
		size_t size() { return h.size(); }
};

