#pragma once

namespace mud::time
{

    enum class TimeEvent
    {
        MinuteChanged,
        HourChanged,
        DayChanged,
        MonthChanged,
        YearChanged
    };

}