#pragma once

#include <string>
#include <sstream>
#include <functional>
#include <filesystem>
#include <array>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

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

struct Onb {
    explicit Onb(glm::vec3 normal) : m_normal{normal} {
        
        if (std::abs(m_normal.x) > std::abs(m_normal.z)) {
            m_binormal.x = -m_normal.y;
            m_binormal.y = m_normal.x;
            m_binormal.z = 0;
        } else {
            m_binormal.x = 0;
            m_binormal.y = -m_normal.z;
            m_binormal.z = m_normal.y;
        }
        m_binormal = normalize(m_binormal);
        m_tangent = cross(m_binormal, m_normal);
    }
    
    [[nodiscard]] glm::vec3 inverse_transform(glm::vec3 p) const {
        return p.x * m_tangent + p.y * m_binormal + p.z * m_normal;
    }
    
    [[nodiscard]] glm::mat3 inverse_transform() const noexcept {
        return {
            m_tangent.x, m_binormal.x, m_normal.x,
            m_tangent.y, m_binormal.y, m_normal.y,
            m_tangent.z, m_binormal.z, m_normal.z,
        };
    }
    
    [[nodiscard]] glm::vec3 transform(glm::vec3 p) const {
        return {glm::dot(p, m_tangent), glm::dot(p, m_binormal), glm::dot(p, m_normal)};
    }
    
    glm::vec3 m_tangent{};
    glm::vec3 m_binormal{};
    glm::vec3 m_normal{};
};


}