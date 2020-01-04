#pragma once

#include <string>
#include <sstream>
#include <functional>
#include <filesystem>
#include <array>
#include <fstream>

namespace util {

template<typename ...Args>
std::string serialize(Args &&...args) noexcept {
    std::ostringstream ss;
    static_cast<void>((ss << ... << std::forward<Args>(args)));
    return ss.str();
}

namespace _impl {
struct RAIIHelperImpl;
}

template<typename T>
class RAII {

private:
    T _value;
    std::function<void(T &)> _delete;
    
    friend struct _impl::RAIIHelperImpl;
    constexpr RAII(T value, std::function<void(T &)> del) noexcept : _value{std::move(value)}, _delete{std::move(del)} {}

public:
    RAII(RAII &&) noexcept = default;
    RAII(const RAII &) = delete;
    RAII &operator=(RAII &&) noexcept = default;
    RAII &operator=(const RAII &) = delete;
    
    ~RAII() noexcept { _delete(_value); }
    
    constexpr T &operator*() noexcept { return _value; }
};

namespace _impl {

struct RAIIHelperImpl {
    template<typename T, typename Func>
    static constexpr auto guard(T &&value, Func &&del) noexcept {
        return RAII<T>{std::forward<T>(value), std::forward<Func>(del)};
    }
};

}

template<typename T, typename Func>
constexpr RAII<T> guard(T &&value, Func &&del) noexcept {
    return _impl::RAIIHelperImpl::guard(std::forward<T>(value), std::forward<Func>(del));
}

inline std::string read_text_file(const std::filesystem::path &path) {
    std::ifstream file{path};
    if (!file.is_open()) {
        return {};
    }
    return {std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
}

constexpr auto u8vec4_to_uint(std::array<uint8_t, 4> b) noexcept {
    return static_cast<uint32_t>(b[0]) << 24u |
           static_cast<uint32_t>(b[1]) << 16u |
           static_cast<uint32_t>(b[2]) << 8u |
           static_cast<uint32_t>(b[3]);
}

constexpr std::array<uint8_t, 4> uint_to_u8vec4(uint32_t u) noexcept {
    return {static_cast<uint8_t>(u >> 24u), static_cast<uint8_t>(u >> 16u), static_cast<uint8_t>(u >> 8u), static_cast<uint8_t>(u)};
}

struct Noncopyable {
    Noncopyable() = default;
    Noncopyable(const Noncopyable &) = delete;
    Noncopyable &operator=(const Noncopyable &) = delete;
};

}