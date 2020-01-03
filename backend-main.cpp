#include <core/core.h>

int main() {
    
    Engine{}.set_network(Network::create("127.0.0.1"))
            .set_game_logic(GameLogic::create())
            .set_frame_render(FrameRender::create())
            .set_gesture_capture(GestureCapture::create())
            .run();
    
}
