#include <core/core.h>

int main() {

  try {
    Engine{}
        .set_network(Network::create("127.0.0.1"))
        .set_game_logic(GameLogic::create())
        .set_frame_render(FrameRender::create())
        .set_gesture_capture(GestureCapture::create())
        .run();
  } catch (const std::exception &error) {
    std::cerr << "Error occurred: " << error.what() << std::endl;
  }
}
