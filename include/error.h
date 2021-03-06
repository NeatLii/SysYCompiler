#ifndef __sysycompiler_error_h__
#define __sysycompiler_error_h__

#include <stdexcept>
#include <string>

// The followings are all unhandleable errors, used to throw directly, without
// catch statement. But still save info fields for future expansion.

class InvalidParameterException : public std::runtime_error {
  public:
    const std::string msg;

    explicit InvalidParameterException(const std::string &msg)
        : std::runtime_error(msg), msg(msg) {}
};

class InvalidParameterValueException : public std::runtime_error {
  public:
    const std::string file_name;
    const int line_number;
    const std::string function_name;
    const std::string parameter_name;
    const std::string parameter_literal_value;

    InvalidParameterValueException(const std::string &file_name,
                                   const int line_no,
                                   const std::string &func_name,
                                   const std::string &param_name,
                                   const std::string &param_literal_value)
        : std::runtime_error(
            "file '" + file_name + "', line " + std::to_string(line_no)
            + ", function '" + func_name + "', parameter '" + param_name
            + "', invalid value '" + param_literal_value + '\'')
        , file_name(file_name)
        , line_number(line_no)
        , function_name(func_name)
        , parameter_name(param_name)
        , parameter_literal_value(param_literal_value) {}
};

class InvalidOperatorException : public std::runtime_error {
  public:
    const std::string expr;
    explicit InvalidOperatorException(const std::string &expr)
        : std::runtime_error('\n' + expr), expr(expr) {}
};

class IdentRefNotFindException : public std::runtime_error {
  public:
    const std::string position;
    const std::string ident;

    IdentRefNotFindException(const std::string &position,
                             const std::string &ident)
        : std::runtime_error(position + ", unable to find identifier '" + ident
                             + '\'')
        , position(position)
        , ident(ident) {}
};

class WrongInitListFormatException : public std::runtime_error {
  public:
    explicit WrongInitListFormatException(const std::string &dump)
        : std::runtime_error('\n' + dump), dump(dump) {}

  private:
    const std::string dump;
};

class InvalidValueTypeException : public std::runtime_error {
  public:
    const std::string &inst;
    const std::string actual;
    const std::string need;

    explicit InvalidValueTypeException(const std::string &inst,
                                       const std::string &actual,
                                       const std::string &need)
        : std::runtime_error("inst: '" + inst + "', actual: '" + actual
                             + "', need '" + need + '\'')
        , inst(inst)
        , actual(actual)
        , need(need) {}
};

#endif
