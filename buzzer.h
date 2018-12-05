#pragma once
#include <TVout.h>

#include "ring_queue.h"

class Buzzer {
public:
    Buzzer();
    ~Buzzer();

    enum class Mode { QUESTION, EDIT_SCORE };
    enum class TeamState { WAITING, IN, ANSWERING, OUT_OF_TIME, INCORRECT };

    void refresh();

    struct Team {
        uint8_t score = 0;
        TeamState state = TeamState::WAITING;
    };

private:
    void scanInput();
    void handleNewQuestion();
    void handleEdit();
    void handleYes();
    void handleNo();
    void checkTeam();
    void refreshTimer();
    void refreshScores();
    void refreshStates();
    void clear();

    using funcPtr = void (Buzzer::*)();
    funcPtr buttonHandlers[4] = {&handleNewQuestion, &handleEdit, &handleYes, &handleNo};

    Mode mode = Mode::QUESTION;
    uint8_t editing = 0;
    Team teams[6];
    bool button_state[4] = {};
    TVout tv;
    bool answering = false;
    RingQueue<Team*, 6> team_queue;
    Team* current_team = nullptr;
    uint8_t queue_in = 0, queue_out = 0;
    unsigned long time = 0xDEADBEEF, time_diff = 0xDEADBEEF;
};
