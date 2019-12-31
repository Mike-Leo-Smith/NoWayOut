//
// Created by Mike Smith on 2019/12/31.
//

#pragma once

#include "frame_render.h"
#include "gesture_capture.h"

struct GameState {
    // todo
};

class GameLogic {

private:
    GameState _state;

public:
    void update(const DisplayState &display_state, const GestureState &gesture_state);
    [[nodiscard]] const GameState &state() const noexcept { return _state; }
    
    [[nodiscard]] bool should_exit() const noexcept { return false; /* todo */ }
    
    template<typename ...Args>
    [[nodiscard]] static std::unique_ptr<GameLogic> create(Args &&...args) noexcept {
        return std::make_unique<GameLogic>(std::forward<Args>(args)...);
    }
};
