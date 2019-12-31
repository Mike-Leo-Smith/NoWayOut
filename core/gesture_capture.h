//
// Created by Mike Smith on 2019/12/31.
//

#pragma once

#include <memory>

struct GestureState {
    // todo
};

struct DisplayState;
struct GameState;

class GestureCapture {

private:
    GestureState _state;

public:
    void update(const GameState &game_state);
    [[nodiscard]] const GestureState &state() const noexcept { return _state; }
    
    template<typename ...Args>
    [[nodiscard]] static std::unique_ptr<GestureCapture> create(Args &&...args) noexcept {
        return std::make_unique<GestureCapture>(std::forward<Args>(args)...);
    }
};
