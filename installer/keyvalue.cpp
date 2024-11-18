#include "keyvalue.h"

std::string KeyValue::get() const override {
    std::ostringstream oss;
    oss << key << "\t" << value << "\n";
    return oss.str();
}
