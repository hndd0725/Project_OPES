// TimerService.h
#pragma once
#include <thread>
#include <atomic>
#include "protocol.h"   // TIMER_ADVENTURE (resetRoom 기본 인자)

class IOCompletionPort;

class TimerService {
public:
    explicit TimerService(IOCompletionPort& iocp);
    ~TimerService();

    void start();
    void stop();

    void resetRoom(unsigned roomID, int seconds = TIMER_ADVENTURE); // 스테이지 진입/전환
    void stopRoom(unsigned roomID);                    // 스테이지 완전 종료

private:
    void loop();

    IOCompletionPort& iocp;
    std::thread timerThread;
    std::atomic_bool running{ false };
};
