#include "backend/operand.h"

#include <string>

#include "error.h"

namespace backend {

std::string RegOperand::Str() const {
    switch (id) {
        case kFp:
            return "fp";
        case kIp:
            return "ip";
        case kSp:
            return "sp";
        case kLr:
            return "lr";
        case kPc:
            return "pc";
        case kCpsr:
            return "cpsr";
        default:
            return 'r' + std::to_string(id);
    }
}

void RegOperand::CheckId() const {
    if (id < 0) {
        throw InvalidParameterValueException(
            __FILE__, __LINE__, "RegOperand::RegOperand(const int id)", "id",
            std::to_string(id));
    }
}

bool ImmOperand::CheckImm8m() const {
    std::uint32_t n = value;
    std::uint32_t window = 0xff;
    for (uint i = 0; i < 0xf; ++i) {
        if ((n & ~window) == 0 || (n | window) == 0xffffffff) return true;
        window = (window >> 2) | (window << (32 - 2));
    }
    return false;
}

bool ImmOperand::CheckImm16m() const {
    return static_cast<std::int16_t>(0x8000) <= value
           && value <= static_cast<std::int16_t>(0x7fff);
}

}  // namespace backend
