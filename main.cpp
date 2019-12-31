#include <core/engine.h>

int main() {
    
    Engine{}.set_network(Network::create("192.168.1.107"))
            .set_game_logic(GameLogic::create())
            .set_frame_render(FrameRender::create())
            .set_gesture_capture(GestureCapture::create())
            .run();
    
}
