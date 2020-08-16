#pragma once
#include <stdexcept>

struct NiceException : public std::runtime_error
{
public:
    NiceException(const std::stringstream& message)
        :runtime_error(message.str())
    {
    };

    NiceException(const std::string& message)
        :runtime_error(message)
    {
    };

    NiceException(const char* message)
        :runtime_error(message)
    {
    };
};
