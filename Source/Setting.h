#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <iostream>
#include <string>

using namespace std;

template<class T>
class Setting {
public:
    Setting(const char* key, T& value);
    
    string getKey() const;
    T getValue() const;
    
    void setValue(T& value);
    
    stream operator<<(stream& s, const Setting<T>& s);
private:
    string sKey;
    T sValue;    
};

#endif /* SETTINGS_H_ */