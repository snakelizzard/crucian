#include "nust/types/Exception.hpp"

nupic::Exception::Exception(std::string filename, nupic::UInt32 lineno, std::string message, std::string stacktrace)
    : std::runtime_error(""), filename_(std::move(filename)), lineno_(lineno),
      message_(std::move(message)), stackTrace_(std::move(stacktrace))
{
}

const char* nupic::Exception::what() const noexcept
{
    try {
        return getMessage();
    } catch (...) {
        return "Exception caught in non-throwing Exception::what()";
    }
}

const char* nupic::Exception::getFilename() const
{
    return filename_.c_str();
}

nupic::UInt32 nupic::Exception::getLineNumber() const
{
    return lineno_;
}

const char* nupic::Exception::getMessage() const
{
    return message_.c_str();
}

const char* nupic::Exception::getStackTrace() const
{
    return stackTrace_.c_str();
}
