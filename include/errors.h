#ifndef __errors_h__
#define __errors_h__

#include <stdexcept>
#include <string>

class InvalidParameterValueException;

class InvalidParameterValueException : public std::runtime_error {
  public:
    const std::string file_name;
    const int line_number;
    const std::string function_name;
    const std::string parameter_name;
    const std::string parameter_literal_value;

    InvalidParameterValueException(const std::string &file_name,
                                   const int line_number,
                                   const std::string &function_name,
                                   const std::string &parameter_name,
                                   const std::string &parameter_literal_value)
        : std::runtime_error(
            "file '" + file_name + "', line " + std::to_string(line_number)
            + ", function '" + function_name + "', parameter '" + parameter_name
            + "', invalid value '" + parameter_literal_value + "'")
        , file_name(file_name)
        , line_number(line_number)
        , function_name(function_name)
        , parameter_name(parameter_name)
        , parameter_literal_value(parameter_literal_value) {}
};

#endif
