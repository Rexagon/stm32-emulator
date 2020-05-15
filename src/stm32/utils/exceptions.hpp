#pragma once

#include <exception>
#include <string_view>

namespace stm32::utils
{
enum ExceptionType : uint16_t {
    Reset = 1,
    NMI = 2,
    HardFault = 3,
    MemManage = 4,
    BusFault = 5,
    UsageFault = 6,
    SVCall = 11,
    // DebugMonitor = 12,
    PendSV = 14,
    SysTick = 15,
};

class CpuException : virtual public std::exception {
public:
    explicit CpuException(uint16_t type)
        : m_type{type}
    {
    }

    virtual ~CpuException() throw() {}

    virtual const char* what() const noexcept override
    {
        switch (m_type) {
            case ExceptionType::Reset:
                return "Reset";
            case NMI:
                return "NMI";
            case HardFault:
                return "HardFault";
            case MemManage:
                return "MemManage";
            case BusFault:
                return "BusFault";
            case UsageFault:
                return "UsageFault";
            case SVCall:
                return "SVCall";
            case PendSV:
                return "PendSV";
            case SysTick:
                return "SysTick";
            default:
                return "Unknown";
        }
    }

protected:
    uint16_t m_type;
};

class UndefinedException : virtual public std::exception {
public:
    explicit UndefinedException(std::string_view message)
        : m_message{message}
    {
    }

    virtual ~UndefinedException() throw() {}

    virtual const char* what() const noexcept override { return m_message.data(); }

protected:
    std::string_view m_message;
};

class UnpredictableException : virtual public std::exception {
public:
    explicit UnpredictableException(std::string_view message)
        : m_message{message}
    {
    }

    virtual ~UnpredictableException() throw() {}

    virtual const char* what() const noexcept override { return m_message.data(); }

protected:
    std::string_view m_message;
};

}  // namespace stm32::utils
