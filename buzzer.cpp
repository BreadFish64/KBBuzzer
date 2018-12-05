#include <Arduino.h>
#include <font8x8.h>
#include <pins_arduino.h>
#include <video_gen.h>

#include "buzzer.h"

Buzzer::Buzzer() {
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
        current_team = team_queue.get();
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
        tv.print(12, 8 * i, static_cast<unsigned int>(teams[i].score));
    }
}

void Buzzer::handleNewQuestion() {
    editing = NOT_EDITING;
    clear();
}

void Buzzer::clear() {
    current_team = nullptr;
    team_queue.reset();
    for (auto& team : teams)
        team.state = TeamState::WAITING;
    time = 0;
    tv.print(71, 0, "  ");
    refreshStates();
    refreshScores();
}

void Buzzer::refreshStates() {
    constexpr char state_chars[] = " -<*X";
    for (uint8_t i = 0; i < 6; ++i)
        tv.print_char(40, 8 * i, state_chars[static_cast<uint8_t>(teams[i].state)]);

    if (editing != NOT_EDITING) {
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
    if (editing == NOT_EDITING) {
        for (uint8_t i = 0; i < 6; ++i) {
            Team& team = teams[i];
            if (digitalRead(A0 + i) == LOW && team.state == TeamState::WAITING) {
                team_queue.put(&team);
                team.state = TeamState::IN;
                refreshStates();
                tv.tone(2500, 100);
            }
        }
    }

    // Buttons
    for (uint8_t i = 0; i < 4; ++i) {
        if (digitalRead(i + 2) == LOW && button_state[i] == false) {
            (this->*buttonHandlers[i])();
            button_state[i] = true;
        } else if (digitalRead(i + 2) == HIGH) {
            button_state[i] = false;
        }
    }
}

void Buzzer::handleEdit() {
    if (editing == NOT_EDITING) {
        editing = 0;
        clear();
        teams[editing].state = TeamState::ANSWERING;
        refreshStates();
    } else {
        teams[editing].state = TeamState::WAITING;
        editing = (editing + 1) % 6;
        teams[editing].state = TeamState::ANSWERING;
        refreshStates();
    }
}

void Buzzer::handleYes() {
    if (current_team != nullptr) {
        ++current_team->score;
        clear();
    } else if (editing != NOT_EDITING) {
        ++teams[editing].score;
        refreshScores();
    }
}

void Buzzer::handleNo() {
    if (current_team != nullptr) {
        current_team->state = TeamState::INCORRECT;
        for (const auto& team : teams) {
            if (team.state != TeamState::INCORRECT)
                goto some_left;
        }
        clear();
    some_left:
        current_team = nullptr;
        time = 0;
        tv.print(71, 0, "  ");
        refreshStates();
    } else if (editing != NOT_EDITING && teams[editing].score != 0) {
        --teams[editing].score;
        refreshScores();
    }
}
