#ifndef __errors_h__
#define __errors_h__

#include <stdexcept>
#include <string>

// The followings are all unhandleable errors, used to throw directly, without
// catch statement. But still save info fields for future expansion.

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

class IdentRefNotFindException : public std::runtime_error {
  public:
    const std::string file_name;
    const std::string position;
    const std::string ident;

    IdentRefNotFindException(const std::string &file_name,
                             const std::string &position,
                             const std::string &ident)
        : std::runtime_error("file '" + file_name + "' " + position
                             + ", unable to find identifier '" + ident + '\'')
        , file_name(file_name)
        , position(position)
        , ident(ident) {}
};

#endif
