#include "ir/type.h"

#include <string>

namespace ir {

FuncType::FuncType(Type *ret_type, const std::vector<Type *> &param_list)
    : Type(kFunc), ret_type(ret_type) {
    for (auto *param : param_list) this->param_list.emplace_back(param);
}

std::string FuncType::RetTypeStr() const { return ret_type->Str(); }

std::string FuncType::ParamListStr() const {
    std::string type_str = "(";
    for (auto iter = param_list.cbegin(); iter != param_list.cend(); ++iter) {
        if (iter != param_list.cbegin()) type_str += ", ";
        type_str += (*iter)->Str();
    }
    return type_str + ')';
}

std::string FuncType::ParamListWithNameStr() const {
    int num = 0;
    std::string type_str = "(";
    for (auto iter = param_list.cbegin(); iter != param_list.cend(); ++iter) {
        if (iter != param_list.cbegin()) type_str += ", ";
        type_str += (*iter)->Str() + ' ' + std::to_string(num++);
    }
    return type_str + ')';
}

std::string FuncType::Str() const {
    return RetTypeStr() + ' ' + ParamListStr();
}

std::string IntType::Str() const {
    switch (width) {
        case kI1:
            return "i1";
        case kI32:
            return "i32";
    }
}

std::string ArrayType::Str() const {
    std::string type_str;
    for (auto iter = arr_dim_list.cbegin(); iter != arr_dim_list.cend();
         ++iter) {
        type_str += '[' + std::to_string(*iter) + " x ";
        if (iter + 1 == arr_dim_list.cend()) type_str += type->Str();
    }
    return type_str + std::string(arr_dim_list.size(), ']');
}

}  // namespace ir
