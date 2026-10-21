// Sorting.hpp - Task 6: insertion sort, merge sort and the sorting experiments.
#pragma once

#include "Records.hpp"
#include "ResultLog.hpp"

enum class SortKey { Age, StayHours, TotalCost, CareType };
enum class SortAlgorithm { Insertion, Merge };

// Display name of a sort key.
inline const char* sortKeyName(SortKey key) {
    switch (key) {
    case SortKey::Age:       return "Age";
    case SortKey::StayHours: return "Duration";
    case SortKey::TotalCost: return "Total Cost";
    case SortKey::CareType:  return "Care Type";
    }
    return "?";
}

// Display name of a sorting algorithm.
inline const char* sortAlgorithmName(SortAlgorithm algorithm) {
    return algorithm == SortAlgorithm::Insertion ? "Insertion" : "Merge";
}

// Counters collected during one sort.
struct SortStats {
    long long comparisons = 0;
    long long moves = 0;        // record copies
    size_t extraBytes = 0;      // extra memory used by the algorithm
    int maxDepth = 0;           // deepest recursion level (merge sort)
};

// Compares two patients on the chosen key; returns -1 (first is smaller), 0 (equal) or 1 (first is bigger).
inline int comparePatients(const Patient& first, const Patient& second, SortKey key) {
    switch (key) {
    case SortKey::Age:       return (first.age > second.age) - (first.age < second.age);
    case SortKey::StayHours: return (first.stayHours > second.stayHours) - (first.stayHours < second.stayHours);
    case SortKey::TotalCost: return (first.totalCost > second.totalCost) - (first.totalCost < second.totalCost);
    case SortKey::CareType:  return first.careType.compare(second.careType);
    }
    return 0;
}

// Insertion sort (array version).
// Takes each record in turn and shifts the larger records before it one place to the
// right until the gap is where the record belongs, then puts the record in the gap.
// Time: O(n^2) worst/average, O(n) if already sorted. Extra space: O(1). Stable.
// Adapted from: Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2022).
//   Introduction to algorithms (4th ed.). MIT Press. (INSERTION-SORT, Section 2.1)
inline void insertionSort(PatientArray& patients, SortKey key, SortStats& stats) {
    for (int i = 1; i < patients.size(); i++) {
        Patient current = patients[i];
        stats.moves++;
        int j = i - 1;
        while (j >= 0) {
            stats.comparisons++;
            if (comparePatients(patients[j], current, key) <= 0) break;
            patients[j + 1] = patients[j];   // shift the larger record one place right
            stats.moves++;
            j--;
        }
        patients[j + 1] = current;
        stats.moves++;
    }
    stats.extraBytes = sizeof(Patient) + 2 * sizeof(int);   // 'current' plus the two indexes
}

// Merges the two sorted halves patients[low..mid] and patients[mid+1..high]:
// both halves are copied into the buffer, then the smaller front record of the
// two halves is written back each time.
inline void mergeHalves(PatientArray& patients, Patient* buffer, int low, int mid, int high,
                        SortKey key, SortStats& stats) {
    for (int i = low; i <= high; i++) {
        buffer[i] = patients[i];
        stats.moves++;
    }
    int left = low, right = mid + 1, writePos = low;
    while (left <= mid && right <= high) {
        stats.comparisons++;
        // '<=' takes from the left half when equal, which keeps the sort stable
        patients[writePos++] = comparePatients(buffer[left], buffer[right], key) <= 0 ? buffer[left++] : buffer[right++];
        stats.moves++;
    }
    while (left <= mid) { patients[writePos++] = buffer[left++]; stats.moves++; }
    while (right <= high) { patients[writePos++] = buffer[right++]; stats.moves++; }
}

// Recursively sorts patients[low..high]: sort the left half, sort the right half, then merge them.
inline void mergeSortRange(PatientArray& patients, Patient* buffer, int low, int high,
                           SortKey key, SortStats& stats, int depth) {
    if (depth > stats.maxDepth) stats.maxDepth = depth;
    if (low >= high) return;
    int mid = low + (high - low) / 2;
    mergeSortRange(patients, buffer, low, mid, key, stats, depth + 1);
    mergeSortRange(patients, buffer, mid + 1, high, key, stats, depth + 1);
    mergeHalves(patients, buffer, low, mid, high, key, stats);
}

// Merge sort (array version).
// Divide and conquer: keeps splitting the array in half until each part has one
// record, then merges the sorted parts back together.
// Time: O(n log n) in every case. Extra space: O(n) buffer + O(log n) recursion stack. Stable.
// Adapted from: Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2022).
//   Introduction to algorithms (4th ed.). MIT Press. (MERGE-SORT, Section 2.3)
inline void mergeSort(PatientArray& patients, SortKey key, SortStats& stats) {
    if (patients.size() < 2) return;
    Patient* buffer = new Patient[patients.size()];
    mergeSortRange(patients, buffer, 0, patients.size() - 1, key, stats, 1);
    delete[] buffer;
    stats.extraBytes = patients.size() * sizeof(Patient) + stats.maxDepth * STACK_FRAME_BYTES;
}

// Runs the chosen sorting algorithm.
inline void runSort(PatientArray& patients, SortAlgorithm algorithm, SortKey key, SortStats& stats) {
    if (algorithm == SortAlgorithm::Insertion) insertionSort(patients, key, stats);
    else mergeSort(patients, key, stats);
}

// True if the records are in ascending order on the key (used to check every sort).
inline bool isSorted(const PatientArray& patients, SortKey key) {
    for (int i = 1; i < patients.size(); i++) {
        if (comparePatients(patients[i - 1], patients[i], key) > 0) return false;
    }
    return true;
}

// Task 6: sorts every dataset by every key with both algorithms. Each sort runs
// SORT_RUNS times on a fresh unsorted copy and the average time is reported with
// the comparisons, record moves and memory used; the results are also saved for
// the Array vs Linked List comparison.
inline void runSortExperiment(const PatientArray datasets[], const PatientArray& allPatients, ResultLog& resultLog) {
    printTitle("TASK 6: SORTING EXPERIMENTS (ARRAY)");
    cout << "  Algorithms : Insertion Sort (O(n^2)) and Merge Sort (O(n log n)), both stable, ascending\n"
         << "  Keys       : Age, Visit Duration, Total Medical Cost\n"
         << "  Timing     : average of " << SORT_RUNS << " runs, each on a fresh unsorted copy (copying not timed)\n";

    const int targetCount = NUM_DATASETS + 1;
    const PatientArray* targets[targetCount] = { &datasets[0], &datasets[1], &datasets[2], &allPatients };
    const char* targetNames[targetCount] = { "Facility A", "Facility B", "Facility C", "All" };
    const SortKey keys[3] = { SortKey::Age, SortKey::StayHours, SortKey::TotalCost };
    const SortAlgorithm algorithms[2] = { SortAlgorithm::Insertion, SortAlgorithm::Merge };
    double avgTimes[targetCount][3][2];

    printHeading("Data structure memory (before sorting)");
    Table memoryTable;
    memoryTable.addColumn("Dataset", 10);
    memoryTable.addColumn("Records", 7, true);
    memoryTable.addColumn("Capacity", 8, true);
    memoryTable.addColumn("Structure Memory (B)", 20, true);
    memoryTable.printHeader();
    for (int target = 0; target < targetCount; target++) {
        string cells[4] = {
            targetNames[target], to_string(targets[target]->size()), to_string(targets[target]->capacity()),
            formatCount(targets[target]->memoryBytes())
        };
        memoryTable.printRow(cells);
    }
    memoryTable.printBorder();

    printHeading("Sorting results");
    Table resultTable;
    resultTable.addColumn("Dataset", 10);
    resultTable.addColumn("Sort Key", 10);
    resultTable.addColumn("Algorithm", 9);
    resultTable.addColumn("Avg Time (us)", 13, true);
    resultTable.addColumn("Comparisons", 11, true);
    resultTable.addColumn("Data Moves", 10, true);
    resultTable.addColumn("Aux Mem (B)", 11, true);
    resultTable.addColumn("Proc WS (KB)", 12, true);
    resultTable.addColumn("OK", 3);
    resultTable.printHeader();
    for (int target = 0; target < targetCount; target++) {
        if (target > 0) resultTable.printBorder();
        for (int keyIndex = 0; keyIndex < 3; keyIndex++) {
            for (int algoIndex = 0; algoIndex < 2; algoIndex++) {
                SortStats stats;
                bool sortedCorrectly = false;
                size_t workingSetKB = 0;
                double totalMicros = 0;
                for (int run = 0; run < SORT_RUNS; run++) {
                    PatientArray copy(*targets[target]);   // fresh unsorted copy (not timed)
                    SortStats runStats;
                    Clock::time_point start = Clock::now();
                    runSort(copy, algorithms[algoIndex], keys[keyIndex], runStats);
                    totalMicros += elapsedMicros(start, Clock::now());
                    if (run == 0) {
                        stats = runStats;
                        sortedCorrectly = isSorted(copy, keys[keyIndex]);
                        workingSetKB = getWorkingSetKB();
                    }
                }
                double avgMicros = totalMicros / SORT_RUNS;
                avgTimes[target][keyIndex][algoIndex] = avgMicros;

                string cells[9] = {
                    targetNames[target], sortKeyName(keys[keyIndex]), sortAlgorithmName(algorithms[algoIndex]),
                    formatNumber(avgMicros, 3), formatCount(stats.comparisons), formatCount(stats.moves),
                    formatCount(stats.extraBytes), formatCount(workingSetKB), sortedCorrectly ? "Yes" : "NO"
                };
                resultTable.printRow(cells);
                resultLog.addResult("SORT", string(targetNames[target]) + "/" + sortKeyName(keys[keyIndex]) + "/" +
                                    sortAlgorithmName(algorithms[algoIndex]), avgMicros, stats.comparisons, stats.extraBytes);
            }
        }
    }
    resultTable.printBorder();
    cout << "  Data Moves = record copies. Aux Mem = extra bytes (merge sort: buffer + ~" << STACK_FRAME_BYTES
         << " B per recursion level).\n  Proc WS = process working set after the sort.\n";

    printHeading("Insertion Sort vs Merge Sort (speed-up)");
    Table speedTable;
    speedTable.addColumn("Dataset", 10);
    speedTable.addColumn("Sort Key", 10);
    speedTable.addColumn("Insertion (us)", 14, true);
    speedTable.addColumn("Merge (us)", 12, true);
    speedTable.addColumn("Faster", 9);
    speedTable.addColumn("Speed-up", 9, true);
    speedTable.printHeader();
    for (int target = 0; target < targetCount; target++) {
        for (int keyIndex = 0; keyIndex < 3; keyIndex++) {
            double insertionTime = avgTimes[target][keyIndex][0];
            double mergeTime = avgTimes[target][keyIndex][1];
            bool mergeIsFaster = mergeTime < insertionTime;
            double speedUp = mergeIsFaster ? insertionTime / mergeTime : mergeTime / insertionTime;
            string cells[6] = {
                targetNames[target], sortKeyName(keys[keyIndex]), formatNumber(insertionTime, 3),
                formatNumber(mergeTime, 3), mergeIsFaster ? "Merge" : "Insertion", formatFixed(speedUp, 2) + "x"
            };
            speedTable.printRow(cells);
        }
    }
    speedTable.printBorder();

    resultLog.printComparison("SORT");
}

// Task 6: lets the user pick a dataset, key and algorithm, sorts a copy and shows the sorted records.
inline void sortAndShowRecords(const PatientArray datasets[], const PatientArray& allPatients) {
    printTitle("TASK 6: SORT & VIEW A DATASET (ARRAY)");
    for (int dataset = 0; dataset < NUM_DATASETS; dataset++) {
        cout << "    [" << dataset + 1 << "] " << FACILITIES[dataset].name << " (" << datasets[dataset].size() << " records)\n";
    }
    cout << "    [4] All facilities (" << allPatients.size() << " records)\n";
    int datasetChoice = readInt("  Dataset (1-4): ", 1, 4);
    cout << "    [1] Age   [2] Visit Duration   [3] Total Medical Cost\n";
    int keyChoice = readInt("  Sort key (1-3): ", 1, 3);
    cout << "    [1] Insertion Sort   [2] Merge Sort\n";
    SortAlgorithm algorithm = readInt("  Algorithm (1-2): ", 1, 2) == 1 ? SortAlgorithm::Insertion : SortAlgorithm::Merge;

    const PatientArray& source = datasetChoice == 4 ? allPatients : datasets[datasetChoice - 1];
    int rowsToShow = readInt("  Records to display (1-" + to_string(source.size()) + "): ", 1, source.size());
    SortKey key = keyChoice == 1 ? SortKey::Age : (keyChoice == 2 ? SortKey::StayHours : SortKey::TotalCost);

    PatientArray sortedCopy(source);
    SortStats stats;
    Clock::time_point start = Clock::now();
    runSort(sortedCopy, algorithm, key, stats);
    double micros = elapsedMicros(start, Clock::now());

    cout << "\n  Sorted " << sortedCopy.size() << " records by " << sortKeyName(key) << " using "
         << sortAlgorithmName(algorithm) << " Sort in " << formatNumber(micros, 3) << " us (comparisons: "
         << formatCount(stats.comparisons) << ", moves: " << formatCount(stats.moves)
         << ", aux memory: " << formatCount(stats.extraBytes) << " B)\n";
    showRecords(sortedCopy, string("Records sorted by ") + sortKeyName(key) + " (ascending)", rowsToShow);
}
