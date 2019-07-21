#include "nust/types/Exception.hpp"

nust::Exception::Exception(std::string filename, nust::UInt32 lineno, std::string message, std::string stacktrace)
    : std::runtime_error(""), filename_(std::move(filename)), lineno_(lineno),
      message_(std::move(message)), stackTrace_(std::move(stacktrace))
{
}

const char* nust::Exception::what() const noexcept
{
    try {
        return getMessage();
    } catch (...) {
        return "Exception caught in non-throwing Exception::what()";
    }
}

const char* nust::Exception::getFilename() const
{
    return filename_.c_str();
}

nust::UInt32 nust::Exception::getLineNumber() const
{
    return lineno_;
}

const char* nust::Exception::getMessage() const
{
    return message_.c_str();
}

const char* nust::Exception::getStackTrace() const
{
    return stackTrace_.c_str();
}
