#include "Setting.h"

template <class T>
Setting::Setting(const char* key, T& value) {
    sKey = string(key);
    sValue = v;
}

string Setting::getKey() const {
    return sKey;
}

template <class T>
T Setting::getValue() const {
    return sValue;
}

template <class T>
void Setting::setValue(T& value) {
    sValue = value;
}

template <class T>
stream operator<<(stream& str, const Setting<T>& set) {
    str << set.sKey << endl;
    str << set.sValue << endl;
}
