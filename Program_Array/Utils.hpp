// Utils.hpp - text, number formatting, console input, timing and memory helpers.
#pragma once

#include "Common.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

// Removes leading and trailing whitespace (also the '\r' of Windows line endings).
inline string trimText(const string& text) {
    size_t start = 0, end = text.size();
    while (start < end && isspace(static_cast<unsigned char>(text[start]))) start++;
    while (end > start && isspace(static_cast<unsigned char>(text[end - 1]))) end--;
    return text.substr(start, end - start);
}

// Splits a line on the delimiter into 'fields' (only the first maxFields are stored).
// Returns how many fields the line actually has.
inline int splitLine(const string& line, char delimiter, string fields[], int maxFields) {
    int fieldCount = 0;
    string current;
    for (char ch : line) {
        if (ch == delimiter) {
            if (fieldCount < maxFields) fields[fieldCount] = trimText(current);
            fieldCount++;
            current.clear();
        } else {
            current += ch;
        }
    }
    if (fieldCount < maxFields) fields[fieldCount] = trimText(current);
    return fieldCount + 1;
}

// Case-insensitive string comparison.
inline bool equalsIgnoreCase(const string& first, const string& second) {
    if (first.size() != second.size()) return false;
    for (size_t i = 0; i < first.size(); i++) {
        if (tolower(static_cast<unsigned char>(first[i])) != tolower(static_cast<unsigned char>(second[i]))) return false;
    }
    return true;
}

// Converts text to an int; returns false if it is not a whole number.
inline bool parseInt(const string& text, int& value) {
    try {
        size_t used = 0;
        value = stoi(text, &used);
        return used == text.size();
    } catch (...) {
        return false;
    }
}

// Converts text to a double; returns false if it is not a number.
inline bool parseDouble(const string& text, double& value) {
    try {
        size_t used = 0;
        value = stod(text, &used);
        return used == text.size();
    } catch (...) {
        return false;
    }
}

// Formats a number with a fixed number of decimal places.
inline string formatFixed(double value, int decimals) {
    ostringstream stream;
    stream << fixed << setprecision(decimals) << value;
    return stream.str();
}

// Formats a number with thousands separators, e.g. 12000 -> "12,000.00".
inline string formatNumber(double value, int decimals = 2) {
    string digits = formatFixed(value < 0 ? -value : value, decimals);
    size_t dotPos = 0;
    while (dotPos < digits.size() && digits[dotPos] != '.') dotPos++;
    string result;
    for (size_t i = 0; i < dotPos; i++) {
        result += digits[i];
        size_t digitsLeft = dotPos - i - 1;
        if (digitsLeft > 0 && digitsLeft % 3 == 0) result += ',';
    }
    return (value < 0 ? "-" : "") + result + digits.substr(dotPos);
}

// Formats a whole number with thousands separators.
inline string formatCount(long long value) {
    return formatNumber(static_cast<double>(value), 0);
}

// Shows whole numbers without decimals and other numbers with two decimals.
inline string formatCompact(double value) {
    long long wholePart = static_cast<long long>(value);
    return static_cast<double>(wholePart) == value ? formatCount(wholePart) : formatNumber(value);
}

// Returns part / whole as a percentage string, e.g. "12.50%".
inline string formatPercent(double part, double whole) {
    return whole > 0 ? formatFixed(part * 100 / whole, 2) + "%" : "0.00%";
}

// Average that returns 0 instead of dividing by zero.
inline double safeAverage(double total, double count) {
    return count > 0 ? total / count : 0;
}

// Prints a prompt and reads one line; exits cleanly if input is closed.
inline string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        cout << "\nInput closed. Exiting.\n";
        exit(0);
    }
    return trimText(line);
}

// Keeps asking until a whole number between minValue and maxValue is entered.
inline int readInt(const string& prompt, int minValue, int maxValue) {
    while (true) {
        int value = 0;
        if (parseInt(readLine(prompt), value) && value >= minValue && value <= maxValue) return value;
        cout << "  Please enter a whole number from " << minValue << " to " << maxValue << ".\n";
    }
}

// Keeps asking until a number between minValue and maxValue is entered.
inline double readDouble(const string& prompt, double minValue, double maxValue) {
    while (true) {
        double value = 0;
        if (parseDouble(readLine(prompt), value) && value >= minValue && value <= maxValue) return value;
        cout << "  Please enter a number from " << formatCompact(minValue) << " to " << formatCompact(maxValue) << ".\n";
    }
}

// Keeps asking until some text is entered.
inline string readText(const string& prompt) {
    while (true) {
        string text = readLine(prompt);
        if (!text.empty()) return text;
        cout << "  Cannot be empty.\n";
    }
}

// Pauses until Enter is pressed.
inline void waitForEnter() {
    readLine("\nPress Enter to return to the menu...");
}

using Clock = chrono::steady_clock;

// Time between two clock readings in microseconds.
inline double elapsedMicros(Clock::time_point start, Clock::time_point end) {
    return chrono::duration<double, micro>(end - start).count();
}

// Current memory used by the whole process (working set) in KB; 0 outside Windows.
inline size_t getWorkingSetKB() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS memoryInfo;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &memoryInfo, sizeof(memoryInfo))) return memoryInfo.WorkingSetSize / 1024;
#endif
    return 0;
}
