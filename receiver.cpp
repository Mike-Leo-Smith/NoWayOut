#include <iostream>
#include <chrono>
#include <asio/asio.hpp>
#include <opencv2/opencv.hpp>
#include <turbojpeg.h>
#include <array>
#include <zstd.h>
#include <rapidjson/document.h>

constexpr auto eye_frame_width = 512ul;
constexpr auto eye_frame_height = 512ul;

constexpr auto u8vec4_to_uint(std::array<uint8_t, 4> b) noexcept {
    return static_cast<uint32_t>(b[0]) << 24u | static_cast<uint32_t>(b[1]) << 16u | static_cast<uint32_t>(b[2]) << 8u | static_cast<uint32_t>(b[3]);
}

constexpr std::array<uint8_t, 4> uint_to_u8vec4(uint32_t u) noexcept {
    return {static_cast<uint8_t>(u >> 24u), static_cast<uint8_t>(u >> 16u), static_cast<uint8_t>(u >> 8u), static_cast<uint8_t>(u)};
}

int main() {

    // connect
    asio::io_service io_service;
    auto address = asio::ip::address_v4::from_string("169.254.89.26");
    asio::ip::tcp::socket pose_socket{io_service};
    asio::ip::tcp::socket frame_socket{io_service};
    pose_socket.connect({address, 7766});
    frame_socket.connect({address, 6677});

    // video for test
    cv::VideoCapture camera{"data/test.mp4"};
    cv::Mat raw_frame;
    cv::Mat eye_frame;
    cv::Mat frame;

    auto jpeg_handle = tjInitCompress();

    frame.create(eye_frame_height, eye_frame_width * 2ul, CV_8UC3);
    std::vector<uint8_t> frame_buffer(eye_frame_width * eye_frame_height * 2ul * 3ul + 4ul);

    auto frame_count = 0ul;
    auto last_time_point = std::chrono::high_resolution_clock::now();

    try {
        for (;;) {

            // receive head pose
            std::array<uint8_t, 4> json_size_buffer{};
            asio::read(pose_socket, asio::buffer(json_size_buffer.data(), 4ul));
            auto size = u8vec4_to_uint(json_size_buffer);
            static thread_local uint8_t buffer[4096ul];
            asio::read(pose_socket, asio::buffer(buffer, size));
            static thread_local char json_string[4096ul];
            ZSTD_decompress(json_string, 4096, buffer, size);

            std::string eye{rapidjson::Document{}.Parse(json_string)["eye"].GetString()};

            // read frame
            camera >> raw_frame;
            if (eye == "left") {
                cv::resize(raw_frame, eye_frame, cv::Size2i{eye_frame_width, eye_frame_height});
                eye_frame.copyTo(frame(cv::Rect{0, 0, eye_frame_width, eye_frame_height}));
            } else if (eye == "right") {
                cv::resize(raw_frame, eye_frame, cv::Size2i{eye_frame_width, eye_frame_height});
                eye_frame.copyTo(frame(cv::Rect{eye_frame_width, 0, eye_frame_width, eye_frame_height}));
            } else {
                cv::resize(raw_frame, frame, cv::Size2i{eye_frame_width * 2ul, eye_frame_height});
            }

            if (eye != "left") {

                // send frame
                auto jpeg_size = 0ul;
                tjCompress(jpeg_handle, frame.data, eye_frame_width * 2ul, 0, eye_frame_height, 3, frame_buffer.data() + 4ul, &jpeg_size, TJSAMP_420, 75, TJFLAG_FASTDCT);
                *reinterpret_cast<std::array<uint8_t, 4ul> *>(frame_buffer.data()) = uint_to_u8vec4(jpeg_size);
                asio::write(frame_socket, asio::buffer(frame_buffer.data(), jpeg_size + 4ul));

                if (++frame_count == 10ul) {
                    using namespace std::chrono_literals;
                    auto current_time_point = std::chrono::high_resolution_clock::now();
                    auto seconds = (current_time_point - last_time_point) / 1ns * 1e-9;
                    std::cout << "FPS: " << frame_count / seconds << std::endl;
                    frame_count = 0ul;
                    last_time_point = current_time_point;
                }
            }
        }
    } catch (...) {}

}
