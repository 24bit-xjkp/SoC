#pragma once
#include <cstddef>
#include <cstdint>

namespace SoC
{
    void _init() noexcept;
    void _fini() noexcept;

    extern "C" void SystemInit();

    template <typename type>
    using cursor_t = type[];
}  // namespace SoC
