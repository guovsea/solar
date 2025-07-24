//
// Created by guo on 2025/7/24.
//

#include "Address.h"

#include <sstream>

namespace solar {
int Address::getFamily() const { return getAddr()->sa_family; }

std::string Address::toString() const {
    std::stringstream ss;
    insert(ss);
    return ss.str(ss);
}

bool Address::operator<(const Address &rhs) const {}

bool Address::operator==(const Address &rhs) const {}

bool Address::operator!=(const Address &rhs) const {}
} // namespace solar