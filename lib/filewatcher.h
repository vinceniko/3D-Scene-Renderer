#pragma once

#include <iostream>

#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>
#include <memory>

struct FileWatcherVal {
    // last change time
    std::filesystem::file_time_type f_time;
    // indicates the file was changed
    bool changed_;
    // execute func when a change is detected
    std::function<void()> func;
};

// starts a thread that checks the file system for changes to the files stored in the paths_ container
// used for hotreloading shaders
class FileWatcher {
    std::chrono::duration<int, std::milli> delay_;
    std::unordered_map<std::string, std::unique_ptr<FileWatcherVal>> paths_;
    std::thread thread_;

    // stops the thread
    bool destroy_ = false;

public:
    FileWatcher(int milli_delay);
    FileWatcher();
    FileWatcher(FileWatcher&&) = default;
    ~FileWatcher();

    void add_path(const std::string& path, std::function<void()> func);
    bool check_change(const std::string& path) const;
    void set_unchanged(const std::string& path);
};