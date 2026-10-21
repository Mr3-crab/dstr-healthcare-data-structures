// ResultLog.hpp - saves timings to a file in the data folder and prints the
// Array vs Linked List table when the other program's file exists.
#pragma once

#include "Table.hpp"

// One saved experiment result.
struct ExperimentResult {
    string category;          // "SORT" or "SEARCH"
    string label;             // e.g. "All/Age/Merge"
    double avgMicros = 0;
    long long steps = 0;      // comparisons (sorting) or element visits / node hops (searching)
    long long memoryBytes = 0;
};

// Keeps this program's experiment results, saves them to a text file and compares
// them with the results file written by the other program (array vs linked list).
class ResultLog {
public:
    // Remembers the data folder and reloads any results saved by an earlier run,
    // so re-running one experiment does not lose the others.
    void init(const string& folder) {
        dataFolder = folder;
        int loaded = readResultsFile(MY_RESULTS_FILE, results, MAX_SAVED_RESULTS);
        resultCount = loaded < 0 ? 0 : loaded;
    }

    // Adds a result, replacing an older result with the same label.
    void addResult(const string& category, const string& label, double avgMicros, long long steps, long long memoryBytes) {
        int index = 0;
        while (index < resultCount && !(results[index].category == category && results[index].label == label)) index++;
        if (index == resultCount) {
            if (resultCount == MAX_SAVED_RESULTS) return;
            resultCount++;
        }
        results[index].category = category;
        results[index].label = label;
        results[index].avgMicros = avgMicros;
        results[index].steps = steps;
        results[index].memoryBytes = memoryBytes;
    }

    // Saves the results, then prints the Array vs Linked List table for one category
    // by matching each result with the same label in the other program's file.
    void printComparison(const string& category) const {
        printHeading(string("ARRAY vs LINKED LIST COMPARISON - ") + (category == "SORT" ? "SORTING" : "SEARCHING"));
        ofstream outFile(dataFolder + MY_RESULTS_FILE);
        for (int i = 0; i < resultCount; i++) {
            outFile << results[i].category << ";" << results[i].label << ";" << formatFixed(results[i].avgMicros, 4) << ";"
                    << results[i].steps << ";" << results[i].memoryBytes << "\n";
        }
        cout << "  Results saved to: " << dataFolder << MY_RESULTS_FILE << "\n";

        ExperimentResult* otherResults = new ExperimentResult[MAX_SAVED_RESULTS];
        int otherCount = readResultsFile(OTHER_RESULTS_FILE, otherResults, MAX_SAVED_RESULTS);
        if (otherCount < 0) {
            cout << "  [Note] " << OTHER_RESULTS_FILE << " not found. Run the same experiment in " << OTHER_PROGRAM_NAME
                 << ",\n         then run it here again to see the side-by-side comparison.\n";
            delete[] otherResults;
            return;
        }

        Table table;
        table.addColumn("Operation", 31);
        table.addColumn("Arr (us)", 11, true);
        table.addColumn("LL (us)", 11, true);
        table.addColumn("Arr Mem", 9, true);
        table.addColumn("LL Mem", 9, true);
        table.addColumn("Arr Step", 9, true);
        table.addColumn("LL Step", 9, true);
        table.addColumn("Faster", 6);
        table.printHeader();
        for (int i = 0; i < resultCount; i++) {
            if (results[i].category != category) continue;
            const ExperimentResult* match = nullptr;
            for (int j = 0; j < otherCount && !match; j++) {
                if (otherResults[j].category == category && otherResults[j].label == results[i].label) match = &otherResults[j];
            }
            const ExperimentResult* arrayResult = IS_ARRAY_PROGRAM ? &results[i] : match;
            const ExperimentResult* listResult = IS_ARRAY_PROGRAM ? match : &results[i];

            string faster = "-";
            if (arrayResult && listResult) {
                if (arrayResult->avgMicros < listResult->avgMicros) faster = "Array";
                else if (listResult->avgMicros < arrayResult->avgMicros) faster = "LL";
                else faster = "Tie";
            }
            string cells[8] = {
                results[i].label,
                arrayResult ? formatNumber(arrayResult->avgMicros, 3) : "-",
                listResult ? formatNumber(listResult->avgMicros, 3) : "-",
                arrayResult ? formatCount(arrayResult->memoryBytes) : "-",
                listResult ? formatCount(listResult->memoryBytes) : "-",
                arrayResult ? formatCount(arrayResult->steps) : "-",
                listResult ? formatCount(listResult->steps) : "-",
                faster
            };
            table.printRow(cells);
        }
        table.printBorder();
        cout << "  us = average microseconds; Mem = extra bytes used by the algorithm;\n"
             << "  Step = key comparisons (sorting) or element visits / node hops (searching).\n";
        delete[] otherResults;
    }

private:
    // Reads a results file (one "category;label;time;steps;bytes" line per result).
    // Returns the number of results read, or -1 if the file does not exist.
    int readResultsFile(const char* fileName, ExperimentResult output[], int maxResults) const {
        ifstream inFile(dataFolder + fileName);
        if (!inFile.is_open()) return -1;
        string line, fields[5];
        int readCount = 0;
        while (readCount < maxResults && getline(inFile, line)) {
            if (splitLine(trimText(line), ';', fields, 5) < 5) continue;
            double steps = 0, memoryBytes = 0;
            if (!parseDouble(fields[2], output[readCount].avgMicros) || !parseDouble(fields[3], steps) ||
                !parseDouble(fields[4], memoryBytes)) {
                continue;
            }
            output[readCount].category = fields[0];
            output[readCount].label = fields[1];
            output[readCount].steps = static_cast<long long>(steps);
            output[readCount].memoryBytes = static_cast<long long>(memoryBytes);
            readCount++;
        }
        return readCount;
    }

    ExperimentResult results[MAX_SAVED_RESULTS];
    int resultCount = 0;
    string dataFolder;
};
