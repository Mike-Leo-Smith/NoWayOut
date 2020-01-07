#include <core/core.h>

int main() {
    
    try {
        Engine{}.set_frame_render(FrameRender::create())
                .set_game_logic(GameLogic::create())
                .set_gesture_capture(GestureCapture::create())
                .set_network(Network::create("192.168.1.107"))
                .run();
    } catch (const std::exception &error) {
        std::cerr << "Error occurred: " << error.what() << std::endl;
    }
}
