#include "ir/ir.h"

#include <error.h>

#include <string>

#include "ir/type.h"
#include "ir/value.h"

namespace ir {

void Inst::CheckType(const std::shared_ptr<Value> &value,
                     const Type::TypeKind kind,
                     const IntType::Width width) {
    std::string need;
    switch (kind) {
        case Type::kVoid:
            need = "void";
            if (value != nullptr && value->GetType().kind == kind) return;
        case Type::kFunc:
            need = "func";
            if (value != nullptr && value->GetType().kind == kind) return;
        case Type::kInt:
            need = width == IntType::kI1 ? "i1" : "i32";
            if (value != nullptr && value->GetType().kind == kind
                && value->GetType().Cast<IntType>().GetWidth() == width) {
                return;
            }
        case Type::kPtr:
            need = "ptr";
            if (value != nullptr && value->GetType().kind == kind) return;
        case Type::kLabel:
            need = "label";
            if (value != nullptr && value->GetType().kind == kind) return;
        case Type::kArray:
            need = "array";
            if (value != nullptr && value->GetType().kind == kind) return;
    }
    throw InvalidValueTypeException(value->GetType().Str(), need);
}

std::string RetInst::Str() const {
    if (HasRet()) { return "ret " + ret->TypeStr(); }
    return "ret void";
}

void RetInst::Check() const { CheckType(ret, Type::kInt, IntType::kI32); }

std::string BrInst::Str() const {
    if (HasDest()) { return "br " + if_true->TypeStr(); }
    return "br " + cond->TypeStr() + ", " + if_true->TypeStr() + ", "
           + if_false->TypeStr();
}

void BrInst::Check() const {
    CheckType(cond, Type::kInt, IntType::kI1);
    CheckType(if_true, Type::kLabel);
    CheckType(if_false, Type::kLabel);
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
    CheckType(result, Type::kInt, IntType::kI32);
    CheckType(lhs, Type::kInt, IntType::kI32);
    CheckType(rhs, Type::kInt, IntType::kI32);
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
    CheckType(result, Type::kInt, IntType::kI1);
    CheckType(lhs, Type::kInt, IntType::kI1);
    CheckType(rhs, Type::kInt, IntType::kI1);
}

std::string AllocaInst::Str() const {
    return result->Str() + " = alloca " + result->GetType().Str();
}

void AllocaInst::Check() const { CheckType(result, Type::kPtr); }

std::string LoadInst::Str() const {
    return result->Str() + " = load " + result->GetType().Str() + ", "
           + ptr->TypeStr();
}

void LoadInst::Check() const {
    CheckType(result, Type::kInt, IntType::kI32);
    CheckType(ptr, Type::kPtr);
}

std::string StoreInst::Str() const {
    return "store " + value->TypeStr() + ", " + ptr->TypeStr();
}

void StoreInst::Check() const {
    CheckType(value, Type::kInt, IntType::kI32);
    CheckType(ptr, Type::kPtr);
}

std::string GetelementptrInst::Str() const {
    std::string str
        = result->Str() + " = getelementptr " + result->GetType().Str();
    str += ", " + ptr->TypeStr();
    for (auto idx : idx_list) str += ", i32 " + std::to_string(idx);
    return str;
}

void GetelementptrInst::Check() const {
    CheckType(result, Type::kInt, IntType::kI32);
    CheckType(ptr, Type::kPtr);
}

std::string ZextInst::Str() const {
    return result->Str() + " = zext i1 " + value->Str() + " to i32";
}

void ZextInst::Check() const {
    CheckType(result, Type::kInt, IntType::kI32);
    CheckType(value, Type::kInt, IntType::kI1);
}

std::string IcmpInst::Str() const {
    std::string str = result->Str() + " = ";
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
    return str + " i1 " + lhs->Str() + ", " + rhs->Str();
}

void IcmpInst::Check() const {
    CheckType(result, Type::kInt, IntType::kI1);
    CheckType(lhs, Type::kInt, IntType::kI32);
    CheckType(rhs, Type::kInt, IntType::kI32);
}

std::string PhiInst::PhiValue::Str() const {
    return "[ " + value->Str() + ", " + label->Str() + " ]";
}

void PhiInst::PhiValue::Check() const {
    CheckType(value, Type::kInt, IntType::kI32);
    CheckType(label, Type::kLabel);
}

std::string PhiInst::Str() const {
    std::string str = result->Str() + " = phi " + result->GetType().Str() + ' ';
    for (auto iter = value_list.cbegin(); iter != value_list.cend(); ++iter) {
        if (iter != value_list.cbegin()) str += ", ";
        str += (*iter).Str();
    }
    return str;
}

void PhiInst::Check() const { CheckType(result, Type::kInt, IntType::kI32); }

std::string CallInst::Str() const {
    std::string str = result->Str() + " = call " + result->GetType().Str();
    str += ' ' + func->Str() + '(';
    for (auto iter = param_list.cbegin(); iter != param_list.cend(); ++iter) {
        if (iter != param_list.cbegin()) str += ", ";
        str += (*iter)->TypeStr();
    }
    return str;
}

void CallInst::Check() const {
    CheckType(result, Type::kInt, IntType::kI32);
    CheckType(result, Type::kFunc);
}

void BasicBlock::Dump(std::ostream &ostream, const std::string &indent) const {
    ostream << label->Str() << ':' << std::endl;
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
    ostream << ' ' << ident->GetType().Cast<FuncType>().ParamListStr();
    ostream << std::endl << std::endl;
}

void FuncDef::Dump(std::ostream &ostream) const {
    ostream << "define";
    ostream << ' ' << ident->GetType().Cast<FuncType>().RetTypeStr();
    ostream << ' ' << ident->Str();
    ostream << ' ' << ident->GetType().Cast<FuncType>().ParamListWithNameStr();
    ostream << '{' << std::endl;
    for (const auto &bb : block_list) bb->Dump(ostream, "    ");
    ostream << '}' << std::endl << std::endl;
}

void Module::Dump(std::ostream &ostream) const {
    for (const auto &var : var_list) var->Dump(ostream);
    ostream << std::endl;
    for (const auto &func : func_decl_list) func->Dump(ostream);
    for (const auto &func : func_def_list) func->Dump(ostream);
}

}  // namespace ir
