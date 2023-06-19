#pragma once
#include <deque>
#include <functional>
#include <thread>

#include "BlockingContainer.hpp"
#include "ui/Feature.h"


namespace features {

class MultiDownload : public ui::main::Feature {
    MultiDownload();
    std::thread t_SearchThread;

public:
    struct Task {
        std::string url;
        int beg, end;
        // code | response | this
        std::function<void(int, std::string &, Task &)> callback;
        bool canceled = false;
        // for logger
        int id;
    };

private:
    container::Blocking<std::deque<Task>> m_Queue;

    [[noreturn]] static void SearchThread();

public:
    static MultiDownload &GetInstance() {
        static MultiDownload instance;
        return instance;
    }

    void doRequest(const Task &task);

    ui::main::FeatureInfo &getInfo() override;
    void drawMain() override;

};

}
