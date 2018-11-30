#include <Arduino.h>
#include <font8x8.h>
#include <pins_arduino.h>
#include <video_gen.h>

#include "buzzer.h"

Buzzer::Buzzer() {
    for (char i = 0; i <= 6; ++i)
        teams[i] = Team('A' + i);

    tv.begin(NTSC, 88, 70);
    tv.clear_screen();
    tv.select_font(font8x8);

    tv.printPGM(PSTR("A\nB\nC\nD\nE\nF"));
    refreshScores();
    refreshStates();
}

Buzzer::~Buzzer() {}

void Buzzer::refresh() {
    scanInput();
    checkTeam();
    refreshTimer();
}

void Buzzer::checkTeam() {
    if (current_team == nullptr) {
        queueGet();
        if (current_team != nullptr) {
            time = tv.millis();
            current_team->state = TeamState::ANSWERING;
            refreshStates();
        }
    }
}

void Buzzer::refreshTimer() {
    time_diff = tv.millis() - time;
    if (time_diff <= 15000) {
        tv.print(71, 0, 15 - time_diff / 1000);
        if (time_diff > 6000) {
            tv.print_char(79, 0, ' ');
        }
    } else {
        if (current_team->state == TeamState::ANSWERING) {
            tv.print(71, 0, "  ");
            current_team->state = TeamState::OUT_OF_TIME;
            refreshStates();
            tv.tone(2500, 500);
        }
    }
}

void Buzzer::refreshScores() {
    for (uint8_t i = 0; i < 6; ++i) {
        tv.print(12, 8 * i, "   ");
        tv.print(12, 8 * i, teams[i].score);
    }
}

void Buzzer::newQuestion() {
    current_team = nullptr;
    queueReset();
    for (auto& team : teams)
        team.state = TeamState::WAITING;
    time = 0;
    tv.print(71, 0, "  ");
    refreshScores();
    refreshStates();
}

void Buzzer::refreshStates() {
    constexpr char state_chars[] = " -<*X";
    for (uint8_t i = 0; i < 6; ++i) {
        tv.print_char(40, 8 * i, state_chars[static_cast<uint8_t>(teams[i].state)]);
    }

    if (mode == Mode::EDIT_SCORE) {
        tv.print(0, 61, "Editing");
    } else if (current_team == nullptr) {
        tv.print(0, 61, "Reading");
    } else if (current_team->state == TeamState::OUT_OF_TIME) {
        tv.print(0, 61, "Answer ");
    } else {
        tv.print(0, 61, "       ");
    }
}

void Buzzer::scanInput() {
    // Strips
    if (mode == Mode::QUESTION)
        for (uint8_t i = 0; i < 6; ++i) {
            Team& team = teams[i];
            if (digitalRead(strips[i]) == LOW && team.state == TeamState::WAITING) {
                queuePut(&team);
                team.state = TeamState::IN;
                refreshStates();
                tv.tone(2500, 100);
            }
        }

    // New Question
    if (digitalRead(2) == LOW && button_state[0] == false) {
        mode = Mode::QUESTION;
        newQuestion();
        button_state[0] = true;
    } else if (digitalRead(4) == HIGH) {
        button_state[0] = false;
    }

    // Edit Score
    if (digitalRead(3) == LOW && button_state[1] == false) {
        if (mode == Mode::QUESTION) {
            mode = Mode::EDIT_SCORE;
            newQuestion();
        } else {
            newQuestion();
            editing = (editing + 1) % 6;
            teams[editing].state = TeamState::ANSWERING;
            refreshStates();
        }
        button_state[1] = true;
    } else if (digitalRead(4) == HIGH) {
        button_state[1] = false;
    }

    // Yes
    if (digitalRead(4) == LOW && button_state[2] == false) {
        if (current_team != nullptr) {
            ++current_team->score;
            newQuestion();
        } else if (mode == Mode::EDIT_SCORE) {
            ++teams[editing].score;
            refreshScores();
        }
        button_state[2] = true;
    } else if (digitalRead(4) == HIGH) {
        button_state[2] = false;
    }

    // No
    if (digitalRead(5) == LOW && button_state[3] == false) {
        if (current_team != nullptr) {
            incorrect();
        } else if (mode == Mode::EDIT_SCORE && teams[editing].score != 0) {
            --teams[editing].score;
            refreshScores();
        }
        button_state[3] = true;
    } else if (digitalRead(5) == HIGH) {
        button_state[3] = false;
    }
}

void Buzzer::incorrect() {
    current_team->state = TeamState::INCORRECT;
    for (const auto& team : teams) {
        if (team.state != TeamState::INCORRECT)
            goto some_left;
    }
    newQuestion();
some_left:
    current_team = nullptr;
    time = 0;
    tv.print(71, 0, "  ");
    refreshStates();
}

inline void Buzzer::queuePut(Team* new_item) {
    team_queue[queue_in] = new_item;
    queue_in = (queue_in + 1) % 6;
}

inline void Buzzer::queueGet() {
    if (queue_in == queue_out) {
        current_team == nullptr;
        return;
    }

    auto& old = team_queue[queue_out];
    queue_out = (queue_out + 1) % 6;

    current_team = old;
}

void Buzzer::queueReset() {
    for (auto& item : team_queue)
        item = nullptr;
    queue_in = 0;
    queue_out = 0;
}

Buzzer::Team::Team(){};

Buzzer::Team::Team(uint8_t name) : name(name) {}

Buzzer::Team::~Team() {}

const uint8_t Buzzer::strips[] = {
    A0, A1, A2, A3, A4, A5,
};
