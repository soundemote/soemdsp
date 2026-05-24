#pragma once
// clang-format off
//This class allows for pointing values to other values and replaces basic types
//such as double and int. The reason for having seperate write (w) and read (r) is
//so that you can point to another value and if you never do, you can write
//directly to the var class variable. If you do point to another variable, you
//cannot then write to that other variable, it would be read only.
namespace soemdsp {
namespace op {
    static void callback_empty(void*) {}
}
struct Wire {
    Wire() noexcept = default;
    constexpr Wire(double initial) noexcept : w(initial) {}

    constexpr void pointTo(double *master) noexcept { r = master; }
    constexpr void pointTo(Wire &master) noexcept { r = master.r; }
    constexpr void disconnect() noexcept { r = &w; }

    constexpr operator double() const noexcept { return *r; }
    constexpr Wire &operator=(double rhs) noexcept { w = rhs; return *this; }
    constexpr Wire &operator+=(double v) noexcept { w += v; return *this; }
    constexpr Wire &operator-=(double v) noexcept { w -= v; return *this; }
    constexpr Wire &operator*=(double v) noexcept { w *= v; return *this; }
    constexpr Wire &operator/=(double v) noexcept { w /= v;return *this; }

    const double* wire_ptr{};
    void (*callback)(void*) = op::callback_empty;
    void* context{this}; // The "this" pointer

    double w{}; // local storage
    double *r{ &w }; // read pointer
};
} // namespace soemdsp
