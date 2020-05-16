#pragma once

#define RESTRICT_COPY(ClassName)                     \
    ClassName(const ClassName&) = delete;            \
    ClassName& operator=(const ClassName&) = delete; \
    ClassName(ClassName&&) noexcept = delete;        \
    ClassName& operator=(ClassName&&) noexcept = delete
