#include <iostream>
#include <chrono>
#include <utility>

using namespace std::chrono;

template <typename Duration>
class Timer {
    std::chrono::steady_clock::time_point start_;

public:
    Timer() {}

    virtual void start() {
        start_ = high_resolution_clock::now();
    }

    Duration get_duration() {
        auto durr = duration_cast<Duration>(high_resolution_clock::now() - start_);

        return durr;
    }

    virtual Duration stop() {
        auto dur = get_duration();
        start();

        return dur;
    }
};


template <typename Interval>
class FrameTimer : public Timer<microseconds> {
    uint32_t num_frames = 0;
    
    Interval output_interval_;

public:
    FrameTimer(Interval interval) : output_interval_(interval) { start(); }

    int get_num_frames() {
        return num_frames;
    }
    void start() override {
        Timer::start();

        num_frames = 0;
    }
    bool interval_done() {
        return get_duration() > output_interval_;
     }

    std::pair<microseconds, bool> stop_ready() {
        microseconds frame_time;
        bool done = interval_done();

        if (done) {
            frame_time = get_duration() / num_frames;
            
            Timer::stop();

            num_frames = 0;
        } else {
            num_frames++;
        }
        
        return { frame_time, done };
    }

    void print() {
        int curr_num_frames = get_num_frames();
        auto [ frame_time, ready ] = stop_ready();
        if (ready) {
            std::cout << "frame_time: " << frame_time.count() / 1000.f << std::endl;
            std::cout << "fps: " << curr_num_frames << std::endl;
        }
    }
};