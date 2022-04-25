#include "ir/ir.h"

#include <error.h>

#include <string>

#include "ir/type.h"
#include "ir/value.h"

namespace ir {

void Inst::CheckType(const std::string &inst,
                     const std::shared_ptr<Value> &value,
                     const Type::TypeKind kind,
                     const IntType::Width width) {
    if (value == nullptr) return;
    std::string need;
    switch (kind) {
        case Type::kVoid:
            need = "void";
            if (value != nullptr && value->GetType().kind == kind) return;
            break;
        case Type::kFunc:
            need = "func";
            if (value != nullptr && value->GetType().kind == kind) return;
            break;
        case Type::kInt:
            need = width == IntType::kI1 ? "i1" : "i32";
            if (value != nullptr && value->GetType().kind == kind
                && value->GetType().Cast<IntType>().GetWidth() == width) {
                return;
            }
            break;
        case Type::kPtr:
            need = "ptr";
            if (value != nullptr && value->GetType().kind == kind) return;
            break;
        case Type::kLabel:
            need = "label";
            if (value != nullptr && value->GetType().kind == kind) return;
            break;
        case Type::kArray:
            need = "array";
            if (value != nullptr && value->GetType().kind == kind) return;
            break;
    }
    throw InvalidValueTypeException(inst, value->GetType().Str(), need);
}

std::string RetInst::Str() const {
    if (HasRet()) { return "ret " + ret->TypeStr(); }
    return "ret void";
}

void RetInst::Check() const {
    CheckType("RetInst", ret, Type::kInt, IntType::kI32);
}

std::string BrInst::Str() const {
    if (HasDest()) { return "br " + if_true->TypeStr(); }
    return "br " + cond->TypeStr() + ", " + if_true->TypeStr() + ", "
           + if_false->TypeStr();
}

void BrInst::Check() const {
    if (cond != nullptr) CheckType("BrInst", cond, Type::kInt, IntType::kI1);
    CheckType("BrInst", if_true, Type::kLabel);
    if (if_false != nullptr) CheckType("BrInst", if_false, Type::kLabel);
}

std::string BinaryOpInst::Str() const {
    std::string str = result->Str() + " = ";
    switch (op_code) {
        case kAdd:
            str += "add";
            break;
        case kSub:
            str += "sub";
            break;
        case kMul:
            str += "mul";
            break;
        case kSDiv:
            str += "sdiv";
            break;
        case kSRem:
            str += "srem";
            break;
    }
    return str + " i32 " + lhs->Str() + ", " + rhs->Str();
}

void BinaryOpInst::Check() const {
    CheckType("BinaryOpInst", result, Type::kInt, IntType::kI32);
    CheckType("BinaryOpInst", lhs, Type::kInt, IntType::kI32);
    CheckType("BinaryOpInst", rhs, Type::kInt, IntType::kI32);
}

std::string BitwiseOpInst::Str() const {
    std::string str = result->Str() + " = ";
    switch (op_code) {
        case kAnd:
            str += "and";
            break;
        case kOr:
            str += "or";
            break;
    }
    return str + " i1 " + lhs->Str() + ", " + rhs->Str();
}

void BitwiseOpInst::Check() const {
    CheckType("BitwiseOpInst", result, Type::kInt, IntType::kI1);
    CheckType("BitwiseOpInst", lhs, Type::kInt, IntType::kI1);
    CheckType("BitwiseOpInst", rhs, Type::kInt, IntType::kI1);
}

std::string AllocaInst::Str() const {
    return result->Str() + " = alloca "
           + result->GetType().Cast<PtrType>().GetPointee().Str();
}

void AllocaInst::Check() const { CheckType("AllocaInst", result, Type::kPtr); }

std::string LoadInst::Str() const {
    return result->Str() + " = load " + result->GetType().Str() + ", "
           + ptr->TypeStr();
}

void LoadInst::Check() const {
    // CheckType("LoadInst", result, Type::kInt, IntType::kI32);
    CheckType("LoadInst", ptr, Type::kPtr);
}

std::string StoreInst::Str() const {
    return "store " + value->TypeStr() + ", " + ptr->TypeStr();
}

void StoreInst::Check() const {
    // CheckType("StoreInst", value, Type::kInt, IntType::kI32);
    CheckType("StoreInst", ptr, Type::kPtr);
}

std::string GetelementptrInst::Str() const {
    std::string str = result->Str() + " = getelementptr ";
    str += ptr->GetType().Cast<PtrType>().GetPointee().Str();
    str += ", " + ptr->TypeStr();
    for (const auto &idx : idx_list) str += ", " + idx->TypeStr();
    return str;
}

void GetelementptrInst::Check() const {
    CheckType("GetelementptrInst", result, Type::kPtr);
    CheckType("GetelementptrInst", ptr, Type::kPtr);
}

std::string ZextInst::Str() const {
    return result->Str() + " = zext i1 " + value->Str() + " to i32";
}

void ZextInst::Check() const {
    CheckType("ZextInst", result, Type::kInt, IntType::kI32);
    CheckType("ZextInst", value, Type::kInt, IntType::kI1);
}

std::string BitcastInst::Str() const {
    return result->Str() + " = bitcast " + value->TypeStr() + " to "
           + result->GetType().Str();
}

std::string IcmpInst::Str() const {
    std::string str = result->Str() + " = icmp ";
    switch (op_code) {
        case kEQ:
            str += "eq";
            break;
        case kNE:
            str += "ne";
            break;
        case kSGT:
            str += "sgt";
            break;
        case kSGE:
            str += "sge";
            break;
        case kSLT:
            str += "slt";
            break;
        case kSLE:
            str += "sle";
            break;
    }
    return str + ' ' + lhs->GetType().Str() + ' ' + lhs->Str() + ", "
           + rhs->Str();
}

void IcmpInst::Check() const {
    CheckType("IcmpInst", result, Type::kInt, IntType::kI1);
    // CheckType(lhs, Type::kInt, IntType::kI32);
    // CheckType(rhs, Type::kInt, IntType::kI32);
}

std::string PhiInst::PhiValue::Str() const {
    return "[ " + value->Str() + ", " + label->Str() + " ]";
}

void PhiInst::PhiValue::Check() const {
    CheckType("PhiValue", value, Type::kInt, IntType::kI32);
    CheckType("PhiValue", label, Type::kLabel);
}

std::string PhiInst::Str() const {
    std::string str = result->Str() + " = phi " + result->GetType().Str() + ' ';
    for (auto iter = value_list.cbegin(); iter != value_list.cend(); ++iter) {
        if (iter != value_list.cbegin()) str += ", ";
        str += (*iter).Str();
    }
    return str;
}

void PhiInst::Check() const {
    CheckType("PhiInst", result, Type::kInt, IntType::kI32);
}

std::string CallInst::Str() const {
    std::string str;
    if (has_ret) str += result->Str() + " = ";
    str += "call " + func->GetType().Cast<ir::FuncType>().RetTypeStr();
    str += ' ' + func->Str() + '(';
    for (auto iter = param_list.cbegin(); iter != param_list.cend(); ++iter) {
        if (iter != param_list.cbegin()) str += ", ";
        str += (*iter)->TypeStr();
    }
    return str + ')';
}

void CallInst::Check() const {
    if (result != nullptr) {
        CheckType("CallInst", result, Type::kInt, IntType::kI32);
    }
    CheckType("CallInst", func, Type::kFunc);
}

void BasicBlock::Dump(std::ostream &ostream, const std::string &indent) const {
    if (inst_list.empty()) return;
    ostream << label->GetName() << ':' << std::endl;
    for (const auto &inst : inst_list) {
        ostream << indent << inst->Str() << std::endl;
    }
}

void GlobalVarDef::Dump(std::ostream &ostream) const {
    ostream << ident->Str() << " = " << (is_const ? "constant" : "global");
    ostream << ' ' << ident->GetType().Str() << ' ';
    if (ident->GetType().kind == Type::kInt) {  // int
        ostream << init_list.front()->Str();
    } else {  // array
        if (is_zero_init) {
            ostream << "zeroinitializer";
        } else {
            ostream << '[';
            for (auto iter = init_list.cbegin(); iter != init_list.cend();
                 ++iter) {
                if (iter != init_list.cbegin()) ostream << ", ";
                ostream << (*iter)->TypeStr();
            }
            ostream << ']';
        }
    }
    ostream << std::endl;
}

void FuncDecl::Dump(std::ostream &ostream) const {
    ostream << "declare";
    ostream << ' ' << ident->GetType().Cast<FuncType>().RetTypeStr();
    ostream << ' ' << ident->Str();
    ostream << ident->GetType().Cast<FuncType>().ParamListStr();
    ostream << std::endl;
}

void FuncDef::Dump(std::ostream &ostream) const {
    ostream << "define";
    ostream << ' ' << ident->GetType().Cast<FuncType>().RetTypeStr();
    ostream << ' ' << ident->Str();
    ostream << '(';
    for (auto iter = param_list.cbegin(); iter != param_list.cend(); ++iter) {
        if (iter != param_list.cbegin()) ostream << ", ";
        ostream << (*iter)->TypeStr();
    }
    ostream << ") {" << std::endl;
    for (const auto &bb : block_list) bb->Dump(ostream, "    ");
    ostream << '}' << std::endl << std::endl;
}

void Module::Dump(std::ostream &ostream) const {
    ostream << "target triple = \"x86_64-pc-linux-gnu\"" << std::endl
            << std::endl;

    for (const auto &var : var_list) var->Dump(ostream);
    if (!var_list.empty()) ostream << std::endl;

    for (const auto &func : func_decl_list) func->Dump(ostream);
    if (!func_decl_list.empty()) ostream << std::endl;

    for (const auto &func : func_def_list) func->Dump(ostream);
}

}  // namespace ir
