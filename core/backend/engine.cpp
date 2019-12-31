//
// Created by Mike Smith on 2019/12/31.
//

#include <iostream>
#include "engine.h"

void Engine::run() noexcept {
    
    if (_network == nullptr || _game_logic == nullptr || _frame_render == nullptr || _gesture_capture == nullptr) {
        std::cerr << "Error: bad engine state, some components are not initialized" << std::endl;
        abort();
    }
    
    _frame_count = 0ul;
    _last_time_point = std::chrono::high_resolution_clock::now();
    while (!_game_logic->should_exit()) {
        auto display_state = _network->receive_display_state();
        _gesture_capture->update(_game_logic->state());
        _game_logic->update(display_state, _gesture_capture->state());
        _frame_render->update(_game_logic->state(), display_state);
        _network->send_frame(_frame_render->frame());
        
        if (++_frame_count == 10ul) {
            using namespace std::chrono_literals;
            auto current_time_point = std::chrono::high_resolution_clock::now();
            auto seconds = (current_time_point - _last_time_point) / 1ns * 1e-9;
            std::cout << "FPS: " << _frame_count / seconds << std::endl;
            _frame_count = 0ul;
            _last_time_point = current_time_point;
        }
    }
}

Engine &Engine::set_network(std::unique_ptr<Network> network) noexcept {
    _network = std::move(network);
    return *this;
}

Engine &Engine::set_game_logic(std::unique_ptr<GameLogic> game_logic) noexcept {
    _game_logic = std::move(game_logic);
    return *this;
}

Engine &Engine::set_frame_render(std::unique_ptr<FrameRender> frame_render) noexcept {
    _frame_render = std::move(frame_render);
    return *this;
}

Engine &Engine::set_gesture_capture(std::unique_ptr<GestureCapture> gesture_capture) noexcept {
    _gesture_capture = std::move(gesture_capture);
    return *this;
}
