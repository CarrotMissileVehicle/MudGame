//
// Created by z2996 on 2026/9/1.
//

#ifndef MUDGAME_TIME_H
#define MUDGAME_TIME_H

struct Time {
    int day;
    int hour;
    int minute;

    Time operator+(Time &t) const {
        Time res = *this;
        res.day += t.day;
        res.hour += t.hour;
        res.minute += t.minute;
        while (res.minute > 59) {
            res.minute -= 60;
            ++res.hour;
        }
        while (res.hour > 23) {
            res.hour -= 24;
            ++res.day;
        }
        return res;
    }
};

#endif //MUDGAME_TIME_H
