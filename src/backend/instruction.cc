#include "backend/instruction.h"

#include <error.h>

namespace backend {

void InsMov::CheckImm() const {
    const auto &imm = Rm_imm->Cast<ImmOperand>();
    if (!(imm.IsImm16() || imm.IsImm8m())) {
        throw InvalidParameterException(imm.Str()
                                        + " is neither #<imm16> nor #<imm8m>");
    }
}

std::string InsMov::Str() const {
    return op_map[op] + cond_map[cond] + '\t' + Rd->Str() + ", "
           + Rm_imm->Str();
}

std::string InsLdr::Str() const {
    std::string str = op_map[op] + cond_map[cond] + '\t' + Rd->Str() + ", ";
    switch (Rn_imm_label->kind) {
        case Operand::kReg:
            if (offset != nullptr) {
                return str + '[' + Rn_imm_label->Str() + ", " + offset->Str()
                       + ']';
            }
            return str + '[' + Rn_imm_label->Str() + ']';
        default:
            return str + '=' + Rn_imm_label->Str();
    }
}

std::string InsStr::Str() const {
    std::string str = op_map[op] + cond_map[cond] + '\t' + Rd->Str() + ", ";
    if (offset != nullptr) {
        return str + '[' + Rn->Str() + ", " + offset->Str() + ']';
    }
    return str + '[' + Rn->Str() + ']';
}

std::string InsPush::Str() const {
    std::string str = op_map[op] + cond_map[cond] + "\t{";
    for (auto iter = reg_list.cbegin(); iter != reg_list.cend(); ++iter) {
        if (iter != reg_list.cbegin()) str += ", ";
        str += (*iter)->Str();
    }
    return str + '}';
}

std::string InsPop::Str() const {
    std::string str = op_map[op] + cond_map[cond] + "\t{";
    for (auto iter = reg_list.cbegin(); iter != reg_list.cend(); ++iter) {
        if (iter != reg_list.cbegin()) str += ", ";
        str += (*iter)->Str();
    }
    return str + '}';
}

std::string InsCmp::Str() const {
    return op_map[op] + cond_map[cond] + '\t' + Rn->Str() + ", "
           + Rm_imm->Str();
}

void InsCmp::CheckImm() const {
    const auto &imm = Rm_imm->Cast<ImmOperand>();
    if (!imm.IsImm8m()) {
        throw InvalidParameterException(imm.Str() + " is not #<imm8m>");
    }
}

std::string InsB::Str() const {
    return op_map[op] + cond_map[cond] + '\t' + label->Str();
}

std::string InsBl::Str() const {
    return op_map[op] + cond_map[cond] + '\t' + label->Str() + "(PLT)";
}

std::string InsBx::Str() const {
    return op_map[op] + cond_map[cond] + '\t' + Rm->Str();
}

std::string InsAdd::Str() const {
    return op_map[op] + cond_map[cond] + '\t' + Rd->Str() + ", " + Rn->Str()
           + ", " + Rm_imm->Str();
}

void InsAdd::CheckImm() const {
    const auto &imm = Rm_imm->Cast<ImmOperand>();
    if (!imm.IsImm8m()) {
        throw InvalidParameterException(imm.Str() + " is not #<imm8m>");
    }
}

std::string InsSub::Str() const {
    return op_map[op] + cond_map[cond] + '\t' + Rd->Str() + ", " + Rn->Str()
           + ", " + Rm_imm->Str();
}

void InsSub::CheckImm() const {
    const auto &imm = Rm_imm->Cast<ImmOperand>();
    if (!imm.IsImm8m()) {
        throw InvalidParameterException(imm.Str() + " is not #<imm8m>");
    }
}

std::string InsMul::Str() const {
    return op_map[op] + cond_map[cond] + '\t' + Rd->Str() + ", " + Rn->Str()
           + ", " + Rs->Str();
}

std::string InsSDiv::Str() const {
    return op_map[op] + cond_map[cond] + '\t' + Rd->Str() + ", " + Rn->Str()
           + ", " + Rs->Str();
}

std::string InsAnd::Str() const {
    return op_map[op] + cond_map[cond] + '\t' + Rd->Str() + ", " + Rn->Str()
           + ", " + Rm_imm->Str();
}

void InsAnd::CheckImm() const {
    const auto &imm = Rm_imm->Cast<ImmOperand>();
    if (!imm.IsImm8m()) {
        throw InvalidParameterException(imm.Str() + " is not #<imm8m>");
    }
}

std::string InsOrr::Str() const {
    return op_map[op] + cond_map[cond] + '\t' + Rd->Str() + ", " + Rn->Str()
           + ", " + Rm_imm->Str();
}

void InsOrr::CheckImm() const {
    const auto &imm = Rm_imm->Cast<ImmOperand>();
    if (!imm.IsImm8m()) {
        throw InvalidParameterException(imm.Str() + " is not #<imm8m>");
    }
}

void GlobalVar::Dump(std::ostream &os) const {
    os << '\n';
    os << "\t.global " << name << '\n';
    os << "\t.type " << name << ", %object\n";
    os << "\t.size " << name << ", " << init_value.size() * 4 << '\n';
    os << name << ":\n";
    int space = 0;
    for (auto value : init_value) {
        if (value == 0) {
            space += 4;
            continue;
        }
        if (space != 0) {
            os << "\t.space " << space << '\n';
            space = 0;
        }
        os << "\t.word " << value << '\n';
    }
    if (space != 0) os << "\t.space " << space << '\n';
}

void Function::Dump(std::ostream &os) const {
    os << '\n';
    os << "\t.global " << name << '\n';
    os << "\t.type " << name << ", %function\n";
    os << name << ":\n";
    for (const auto &ins : inst_list) { os << ins->Str() << '\n'; }
}

void Assembly::Dump(std::ostream &os) const {
    os << "\t.arch armv7-a\n";
    os << "\n\t.data\n";
    for (const auto &var : var_list) { var->Dump(os); }
    os << "\n\t.text\n";
    for (const auto &func : func_list) { func->Dump(os); }
}

}  // namespace backend
