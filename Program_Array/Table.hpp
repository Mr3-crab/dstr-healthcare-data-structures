// Table.hpp - ASCII console table printer.
#pragma once

#include "Utils.hpp"

// Prints fixed-width text tables using only + - | characters, e.g.
//   +------+-------+
//   | Name | Value |
//   +------+-------+
class Table {
public:
    // Adds a column with its title, width and alignment.
    void addColumn(const string& title, int width, bool alignRight = false) {
        if (columnCount == MAX_TABLE_COLUMNS) return;
        titles[columnCount] = title;
        widths[columnCount] = width;
        rightAligned[columnCount] = alignRight;
        columnCount++;
    }

    // Prints a +----+ border line.
    void printBorder() const {
        cout << "+";
        for (int i = 0; i < columnCount; i++) cout << string(widths[i] + 2, '-') << "+";
        cout << "\n";
    }

    // Prints the column titles between two border lines.
    void printHeader() const {
        printBorder();
        printRow(titles);
        printBorder();
    }

    // Prints one row; 'cells' holds one value per column.
    void printRow(const string cells[]) const {
        cout << "|";
        for (int i = 0; i < columnCount; i++) {
            string cell = fitToWidth(cells[i], widths[i]);
            string padding(widths[i] - cell.size(), ' ');
            cout << " " << (rightAligned[i] ? padding + cell : cell + padding) << " |";
        }
        cout << "\n";
    }

    // Prints a single row that spans the full table width (used for notes).
    void printNote(const string& text) const {
        int innerWidth = -3;
        for (int i = 0; i < columnCount; i++) innerWidth += widths[i] + 3;
        string cell = fitToWidth(text, innerWidth);
        cout << "| " << cell << string(innerWidth - cell.size(), ' ') << " |\n";
    }

private:
    // Cuts text that is wider than the column and marks it with "...".
    static string fitToWidth(const string& text, int width) {
        if (static_cast<int>(text.size()) <= width) return text;
        return width > 3 ? text.substr(0, width - 3) + "..." : text.substr(0, width);
    }

    string titles[MAX_TABLE_COLUMNS];
    int widths[MAX_TABLE_COLUMNS] = {};
    bool rightAligned[MAX_TABLE_COLUMNS] = {};
    int columnCount = 0;
};

// Prints a section banner.
inline void printTitle(const string& title) {
    cout << "\n" << string(BANNER_WIDTH, '=') << "\n  " << title << "\n" << string(BANNER_WIDTH, '=') << "\n";
}

// Prints an underlined sub-heading.
inline void printHeading(const string& title) {
    cout << "\n" << title << "\n" << string(title.size(), '-') << "\n";
}
