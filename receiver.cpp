#include <iostream>
#include <chrono>
#include <asio/asio.hpp>
#include <opencv2/opencv.hpp>
#include <turbojpeg.h>
#include <array>
#include <rapidjson/document.h>

constexpr auto width = 384ul;
constexpr auto height = 384ul;

constexpr auto u8vec4_to_uint(std::array<uint8_t, 4> b) noexcept {
    return static_cast<uint32_t>(b[0]) << 24u | static_cast<uint32_t>(b[1]) << 16u | static_cast<uint32_t>(b[2]) << 8u | static_cast<uint32_t>(b[3]);
}

constexpr std::array<uint8_t, 4> uint_to_u8vec4(uint32_t u) noexcept {
    return {static_cast<uint8_t>(u >> 24u), static_cast<uint8_t>(u >> 16u), static_cast<uint8_t>(u >> 8u), static_cast<uint8_t>(u)};
}

int main() {

    // connect
    asio::io_service io_service;
    auto address = asio::ip::address_v4::from_string("192.168.1.142");
    asio::ip::tcp::socket pose_socket{io_service};
    asio::ip::tcp::socket frame_socket{io_service};
    pose_socket.connect({address, 23333});
    frame_socket.connect({address, 23334});

    // video for test
    cv::VideoCapture camera{"/Users/mike/Downloads/郑少锟 2019-12-12 11.59.15.mp4"};
    cv::Mat raw_frame;
    cv::Mat eye_frame;
    cv::Mat center_frame;

    raw_frame.create(height, width, CV_8UC3);
    eye_frame.create(height, width, CV_8UC3);
    center_frame.create(height, width * 2ul, CV_8UC3);

    std::vector<uint8_t> frame_buffer;

    auto frame_count = 0ul;
    auto last_time_point = std::chrono::high_resolution_clock::now();

    try {
        for (;;) {

            auto[frame, frame_width, frame_height] = [&] {
                std::array<uint8_t, 4> size_buffer{};
                pose_socket.read_some(asio::buffer(size_buffer.data(), 4ul));
                auto size = u8vec4_to_uint(size_buffer);
                static thread_local char buffer[4096ul];
                for (auto offset = 0ul; offset < size; offset += pose_socket.receive(asio::buffer(buffer + offset, 4096ul - offset))) {}
                std::string eye{rapidjson::Document{}.Parse(buffer)["eye"].GetString()};
                if (eye == "left") { return std::make_tuple(&eye_frame, width, height); }
                if (eye == "right") { return std::make_tuple(&eye_frame, width, height); }
                return std::make_tuple(&center_frame, width * 2ul, height);
            }();

            // read frame
            camera >> raw_frame;
            cv::resize(raw_frame, *frame, cv::Size{static_cast<int32_t>(frame_width), static_cast<int32_t>(frame_height)});
            cv::cvtColor(*frame, *frame, cv::COLOR_BGR2RGB);

            using namespace std::chrono_literals;

            // send frame
            auto frame_size = frame_width * frame_height * 3ul;
            auto jpeg_size = 0ul;
            auto t0 = std::chrono::high_resolution_clock::now();
            auto jpeg_handle = tjInitCompress();
            frame_buffer.resize(frame_size);
            tjCompress(jpeg_handle, frame->data, frame_width, 0, frame_height, tjPixelSize[TJPF_RGB], frame_buffer.data(), &jpeg_size, TJSAMP_444, 75, TJFLAG_FASTDCT);
            frame_buffer.resize(jpeg_size);
            std::cout << tjGetErrorStr2(jpeg_handle) << std::endl;
            tjDestroy(jpeg_handle);
            auto t1 = std::chrono::high_resolution_clock::now();
            auto size_buffer = uint_to_u8vec4(jpeg_size);
            frame_socket.write_some(asio::buffer(size_buffer.data(), size_buffer.size()));
            for (auto offset = 0ul; offset < jpeg_size; offset += frame_socket.send(asio::buffer(frame_buffer.data() + offset, jpeg_size - offset))) {}

            std::cout << "Compress size = " << jpeg_size << std::endl;
            std::cout << "Compress ratio = " << static_cast<double>(jpeg_size) / static_cast<double>(frame_size) << std::endl;
            std::cout << "Compress time = " << (t1 - t0) / 1ns * 1e-6 << " ms" << std::endl;

            cv::imdecode(frame_buffer, cv::IMREAD_UNCHANGED, frame);
            cv::imshow("Frame", *frame);
            cv::waitKey(1);

            if (++frame_count == 10ul) {
                auto current_time_point = std::chrono::high_resolution_clock::now();
                auto seconds = (current_time_point - last_time_point) / 1ns * 1e-9;
                std::cout << "FPS: " << frame_count / seconds << std::endl;
                frame_count = 0ul;
                last_time_point = current_time_point;
            }
        }
    } catch (...) {}

}
