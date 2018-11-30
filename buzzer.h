#pragma once
#include <TVout.h>

class Buzzer {
public:
    Buzzer();
    ~Buzzer();

    enum class Mode { QUESTION, EDIT_SCORE };
    enum class TeamState { WAITING, IN, ANSWERING, OUT_OF_TIME, INCORRECT };

    void refresh();

    struct Team {
        Team();
        Team(uint8_t name);
        ~Team();

        char name;
        uint16_t score = 0;
        TeamState state = TeamState::WAITING;
    };

private:
    void scanInput();
    void incorrect();
    void queuePut(Team* new_item);
    void queueGet();
    void queueReset();
    void checkTeam();
    void refreshTimer();
    void refreshScores();
    void newQuestion();
    void refreshStates();

    static const uint8_t strips[6];

    Mode mode = Mode::QUESTION;
    uint8_t editing = 0;
    Team teams[6];
    bool button_state[4] = {};
    TVout tv;
    bool answering = false;
    Team* team_queue[6];
    Team* current_team = nullptr;
    uint8_t queue_in = 0, queue_out = 0;
    unsigned long time = 0xDEADBEEF, time_diff = 0xDEADBEEF;
};
