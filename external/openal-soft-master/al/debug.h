#ifndef AL_DEBUG_H
#define AL_DEBUG_H

#include <stdint.h>
#include <string>
#include <vector>

using uint = unsigned int;


/* Somewhat arbitrary. Avoid letting it get out of control if the app enables
 * logging but never reads it.
 */
constexpr uint8_t MaxDebugLoggedMessages{64};
constexpr uint16_t MaxDebugMessageLength{1024};
constexpr uint8_t MaxDebugGroupDepth{64};


constexpr uint DebugSourceBase{0};
enum class DebugSource : uint8_t {
    API = 0,
    System,
    ThirdParty,
    Application,
    Other,
};
constexpr uint DebugSourceCount{5};

constexpr uint DebugTypeBase{DebugSourceBase + DebugSourceCount};
enum class DebugType : uint8_t {
    Error = 0,
    DeprecatedBehavior,
    UndefinedBehavior,
    Portability,
    Performance,
    Marker,
    PushGroup,
    PopGroup,
    Other,
};
constexpr uint DebugTypeCount{9};

constexpr uint DebugSeverityBase{DebugTypeBase + DebugTypeCount};
enum class DebugSeverity : uint8_t {
    High = 0,
    Medium,
    Low,
    Notification,
};
constexpr uint DebugSeverityCount{4};

struct DebugGroup {
    const uint mId;
    const DebugSource mSource;
    std::string mMessage;
    std::vector<uint> mFilters;
    std::vector<uint64_t> mIdFilters;

    template<typename T>
    DebugGroup(DebugSource source, uint id, T&& message)
        : mId{id}, mSource{source}, mMessage{std::forward<T>(message)}
    { }
    DebugGroup(const DebugGroup&) = default;
    DebugGroup(DebugGroup&&) = default;
};

#endif /* AL_DEBUG_H */
