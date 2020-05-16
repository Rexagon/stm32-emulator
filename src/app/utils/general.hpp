#pragma once

#define RESTRICT_COPY(ClassName)                     \
    ClassName(const ClassName&) = delete;            \
    ClassName& operator=(const ClassName&) = delete; \
    ClassName(ClassName&&) noexcept = delete;        \
    ClassName& operator=(ClassName&&) noexcept = delete

namespace app::utils
{
template <class... Ts>
struct VariantVisitor : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
VariantVisitor(Ts...) -> VariantVisitor<Ts...>;

}  // namespace app::utils
