#include "filewatcher.h"

FileWatcher::FileWatcher(int milli_delay) : delay_(milli_delay), thread_([&] {
    while (!destroy_) {
        std::this_thread::sleep_for(delay_);  // currently it will always sleep for delay_ time regardless of how long it takes to do i/o

        for (auto& [path, val] : paths_) {
            auto last_time = std::filesystem::last_write_time(std::filesystem::path(path));
            if (last_time > val->f_time) {
                std::cout << "Watched File | Modified: " << path << std::endl;
                if (val->func) {
                    val->func();
                }
                val->f_time = last_time;
                val->changed_ = true;
            }
        }
    }
    }) {}

FileWatcher::FileWatcher() : FileWatcher(5000) {}
FileWatcher::~FileWatcher() {
    destroy_ = true;
    thread_.join();
}

void FileWatcher::add_path(const std::string& path, std::function<void()> func) {
    if (paths_.find(path) == paths_.end()) {
        auto f_time = std::filesystem::last_write_time(path);
        paths_[path] = std::unique_ptr<FileWatcherVal>(new FileWatcherVal{f_time, false, func}); // save allocation
    }
}
bool FileWatcher::check_change(const std::string& path) const {
    auto& val = paths_.at(path);
    return val->changed_;
}
void FileWatcher::set_unchanged(const std::string& path) {
    auto& val = paths_.at(path);
    val->changed_ = false;
}