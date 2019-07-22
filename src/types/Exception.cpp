#include <nust/types/Exception.hpp>

namespace nust
{

Exception::Exception(std::string message)
    : std::runtime_error(""), message_(std::move(message))
{
}

const char *Exception::what() const noexcept
{
    try
    {
        return getMessage();
    }
    catch (...)
    {
        return "Exception caught in non-throwing Exception::what()";
    }
}

const char *Exception::getMessage() const { return message_.c_str(); }

} // namespace nust
