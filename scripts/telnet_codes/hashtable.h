
#include <iostream>
#include <unordered_map>

class Hashtable {
private:
    std::unordered_map<const void *, const void *> htmap;

public:
	Hashtable();
	virtual ~Hashtable();

    void put(const void *key, const void *value);
    const void *get(const void *key);

protected:
	virtual void InitializeData();
	virtual void Reset();
	virtual void DumpContentToLogs();
	virtual unsigned int Count();
	virtual bool Find(const char *first);
};

