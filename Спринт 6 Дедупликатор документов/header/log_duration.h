#pragma once

#include <chrono>
#include <iostream>
#include <stdio.h>
#include <algorithm>

using namespace std;
using namespace chrono;
using namespace literals;

#define PROFILE_CONCAT_INTERNAL(X, Y) X ## Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)
#define LOG_DURATION_STREAM(x, out) LogDuration UNIQUE_VAR_NAME_PROFILE(x, out)

class LogDuration {
public:
    LogDuration(const std::string &text, std::ostream &out = std::cerr) : _text(text), _out(&out)
    {
    }

    ~LogDuration() {
        const auto end_time = steady_clock::now();
        const auto dur = end_time - start_time_;
        cerr << _text << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << endl;
    }

private:
    const std::string _text;
    const steady_clock::time_point start_time_ = steady_clock::now();
    std::ostream *_out;
};
