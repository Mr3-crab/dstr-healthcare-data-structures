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
    long long relinks = 0;      // 'next' pointer changes (records are never copied)
    long long nodeHops = 0;     // pointer steps from one node to the next
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

// Insertion sort (linked list version).
// Takes the nodes off the list one by one and links each into its correct place in a
// new sorted chain, so no records are copied - only 'next' pointers change. A singly
// linked list can only be scanned from the front, so finding the place costs a walk
// from the head each time.
// Time: O(n^2) (even on sorted input). Extra space: O(1). Stable.
// Adapted from: Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2022).
//   Introduction to algorithms (4th ed.). MIT Press. (INSERTION-SORT, Section 2.1)
inline void insertionSort(PatientList& patients, SortKey key, SortStats& stats) {
    PatientNode* sortedHead = nullptr;
    PatientNode* current = patients.getHead();
    while (current) {
        PatientNode* nextNode = current->next;
        stats.nodeHops++;
        bool insertAtFront = !sortedHead;
        if (!insertAtFront) {
            stats.comparisons++;
            insertAtFront = comparePatients(current->patient, sortedHead->patient, key) < 0;
        }
        if (insertAtFront) {
            current->next = sortedHead;
            sortedHead = current;
        } else {
            // Walk to the last node whose key is <= current's key (keeps equal keys in order).
            PatientNode* position = sortedHead;
            while (position->next) {
                stats.comparisons++;
                if (comparePatients(position->next->patient, current->patient, key) > 0) break;
                position = position->next;
                stats.nodeHops++;
            }
            current->next = position->next;
            position->next = current;
        }
        stats.relinks++;
        current = nextNode;
    }
    patients.setHead(sortedHead);
    stats.extraBytes = 4 * sizeof(PatientNode*);   // sortedHead, current, nextNode, position
}

// Cuts a chain in half using a slow pointer (1 step) and a fast pointer (2 steps):
// when the fast pointer reaches the end, the slow one is in the middle.
// Returns the first node of the second half.
// Adapted from: GeeksforGeeks. (n.d.). Merge sort for linked lists.
//   https://www.geeksforgeeks.org/merge-sort-for-linked-list/
inline PatientNode* splitInHalf(PatientNode* head, SortStats& stats) {
    PatientNode* slow = head;
    PatientNode* fast = head->next;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        stats.nodeHops += 3;
    }
    PatientNode* secondHalf = slow->next;
    slow->next = nullptr;
    return secondHalf;
}

// Merges two sorted chains into one by relinking nodes (no new nodes, no copying):
// repeatedly takes the smaller front node of the two chains and links it to the result.
inline PatientNode* mergeSortedChains(PatientNode* leftChain, PatientNode* rightChain, SortKey key, SortStats& stats) {
    PatientNode* mergedHead = nullptr;
    PatientNode* mergedTail = nullptr;
    while (leftChain && rightChain) {
        stats.comparisons++;
        PatientNode* smaller;
        if (comparePatients(leftChain->patient, rightChain->patient, key) <= 0) {   // '<=' keeps it stable
            smaller = leftChain;
            leftChain = leftChain->next;
        } else {
            smaller = rightChain;
            rightChain = rightChain->next;
        }
        if (mergedTail) mergedTail->next = smaller;
        else mergedHead = smaller;
        mergedTail = smaller;
        stats.relinks++;
        stats.nodeHops++;
    }
    PatientNode* remaining = leftChain ? leftChain : rightChain;
    if (!mergedTail) return remaining;
    mergedTail->next = remaining;
    stats.relinks++;
    return mergedHead;
}

// Recursively sorts a chain: split it in half, sort both halves, then merge them.
// Returns the first node of the sorted chain.
inline PatientNode* mergeSortChain(PatientNode* head, SortKey key, SortStats& stats, int depth) {
    if (depth > stats.maxDepth) stats.maxDepth = depth;
    if (!head || !head->next) return head;
    PatientNode* secondHalf = splitInHalf(head, stats);
    PatientNode* sortedLeft = mergeSortChain(head, key, stats, depth + 1);
    PatientNode* sortedRight = mergeSortChain(secondHalf, key, stats, depth + 1);
    return mergeSortedChains(sortedLeft, sortedRight, key, stats);
}

// Merge sort (linked list version).
// Same divide-and-conquer idea as the array version, but halves are found with
// slow/fast pointers and merged by relinking nodes, so no buffer is needed.
// Time: O(n log n). Extra space: only the O(log n) recursion stack. Stable.
// Adapted from: GeeksforGeeks. (n.d.). Merge sort for linked lists.
//   https://www.geeksforgeeks.org/merge-sort-for-linked-list/
inline void mergeSort(PatientList& patients, SortKey key, SortStats& stats) {
    patients.setHead(mergeSortChain(patients.getHead(), key, stats, 1));
    stats.extraBytes = stats.maxDepth * STACK_FRAME_BYTES;
}

// Runs the chosen sorting algorithm.
inline void runSort(PatientList& patients, SortAlgorithm algorithm, SortKey key, SortStats& stats) {
    if (algorithm == SortAlgorithm::Insertion) insertionSort(patients, key, stats);
    else mergeSort(patients, key, stats);
}

// True if the list is in ascending order on the key and no nodes were lost while relinking.
inline bool isSorted(const PatientList& patients, SortKey key, int expectedCount) {
    int nodeCount = 0;
    for (const PatientNode* node = patients.getHead(); node; node = node->next) {
        nodeCount++;
        if (node->next && comparePatients(node->patient, node->next->patient, key) > 0) return false;
    }
    return nodeCount == expectedCount;
}

// Task 6: sorts every dataset by every key with both algorithms. Each sort runs
// SORT_RUNS times on a fresh unsorted copy and the average time is reported with
// the comparisons, relinks, node hops and memory used; the results are also saved
// for the Array vs Linked List comparison.
inline void runSortExperiment(const PatientList datasets[], const PatientList& allPatients, ResultLog& resultLog) {
    printTitle("TASK 6: SORTING EXPERIMENTS (SINGLY LINKED LIST)");
    cout << "  Algorithms : Insertion Sort (O(n^2)) and Merge Sort (O(n log n)), both stable, ascending,\n"
         << "               adapted to relink nodes instead of copying records\n"
         << "  Keys       : Age, Visit Duration, Total Medical Cost\n"
         << "  Timing     : average of " << SORT_RUNS << " runs, each on a fresh unsorted copy (copying not timed)\n";

    const int targetCount = NUM_DATASETS + 1;
    const PatientList* targets[targetCount] = { &datasets[0], &datasets[1], &datasets[2], &allPatients };
    const char* targetNames[targetCount] = { "Facility A", "Facility B", "Facility C", "All" };
    const SortKey keys[3] = { SortKey::Age, SortKey::StayHours, SortKey::TotalCost };
    const SortAlgorithm algorithms[2] = { SortAlgorithm::Insertion, SortAlgorithm::Merge };
    double avgTimes[targetCount][3][2];

    printHeading("Data structure memory (before sorting)");
    Table memoryTable;
    memoryTable.addColumn("Dataset", 10);
    memoryTable.addColumn("Nodes", 7, true);
    memoryTable.addColumn("Node Size (B)", 13, true);
    memoryTable.addColumn("Structure Memory (B)", 20, true);
    memoryTable.printHeader();
    for (int target = 0; target < targetCount; target++) {
        string cells[4] = {
            targetNames[target], to_string(targets[target]->size()), to_string(sizeof(PatientNode)),
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
    resultTable.addColumn("Relinks", 8, true);
    resultTable.addColumn("Node Hops", 9, true);
    resultTable.addColumn("Aux Mem (B)", 11, true);
    resultTable.addColumn("Proc WS (KB)", 12, true);
    resultTable.printHeader();
    int failedChecks = 0;
    for (int target = 0; target < targetCount; target++) {
        if (target > 0) resultTable.printBorder();
        for (int keyIndex = 0; keyIndex < 3; keyIndex++) {
            for (int algoIndex = 0; algoIndex < 2; algoIndex++) {
                SortStats stats;
                size_t workingSetKB = 0;
                double totalMicros = 0;
                for (int run = 0; run < SORT_RUNS; run++) {
                    PatientList copy(*targets[target]);   // fresh unsorted copy (not timed)
                    SortStats runStats;
                    Clock::time_point start = Clock::now();
                    runSort(copy, algorithms[algoIndex], keys[keyIndex], runStats);
                    totalMicros += elapsedMicros(start, Clock::now());
                    if (run == 0) {
                        stats = runStats;
                        if (!isSorted(copy, keys[keyIndex], targets[target]->size())) failedChecks++;
                        workingSetKB = getWorkingSetKB();
                    }
                }
                double avgMicros = totalMicros / SORT_RUNS;
                avgTimes[target][keyIndex][algoIndex] = avgMicros;

                string cells[9] = {
                    targetNames[target], sortKeyName(keys[keyIndex]), sortAlgorithmName(algorithms[algoIndex]),
                    formatNumber(avgMicros, 3), formatCount(stats.comparisons), formatCount(stats.relinks),
                    formatCount(stats.nodeHops), formatCount(stats.extraBytes), formatCount(workingSetKB)
                };
                resultTable.printRow(cells);
                resultLog.addResult("SORT", string(targetNames[target]) + "/" + sortKeyName(keys[keyIndex]) + "/" +
                                    sortAlgorithmName(algorithms[algoIndex]), avgMicros, stats.comparisons, stats.extraBytes);
            }
        }
    }
    resultTable.printBorder();
    cout << "  Output check: " << (failedChecks == 0 ? string("every sorted list verified in ascending order")
                                                     : to_string(failedChecks) + " sort(s) FAILED verification") << ".\n"
         << "  Relinks = next-pointer changes (records are never copied). Node Hops = pointer steps.\n"
         << "  Aux Mem = extra bytes (merge sort: ~" << STACK_FRAME_BYTES << " B per recursion level, no buffer).\n"
         << "  Proc WS = process working set after the sort.\n";

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
inline void sortAndShowRecords(const PatientList datasets[], const PatientList& allPatients) {
    printTitle("TASK 6: SORT & VIEW A DATASET (SINGLY LINKED LIST)");
    for (int dataset = 0; dataset < NUM_DATASETS; dataset++) {
        cout << "    [" << dataset + 1 << "] " << FACILITIES[dataset].name << " (" << datasets[dataset].size() << " records)\n";
    }
    cout << "    [4] All facilities (" << allPatients.size() << " records)\n";
    int datasetChoice = readInt("  Dataset (1-4): ", 1, 4);
    cout << "    [1] Age   [2] Visit Duration   [3] Total Medical Cost\n";
    int keyChoice = readInt("  Sort key (1-3): ", 1, 3);
    cout << "    [1] Insertion Sort   [2] Merge Sort\n";
    SortAlgorithm algorithm = readInt("  Algorithm (1-2): ", 1, 2) == 1 ? SortAlgorithm::Insertion : SortAlgorithm::Merge;

    const PatientList& source = datasetChoice == 4 ? allPatients : datasets[datasetChoice - 1];
    int rowsToShow = readInt("  Records to display (1-" + to_string(source.size()) + "): ", 1, source.size());
    SortKey key = keyChoice == 1 ? SortKey::Age : (keyChoice == 2 ? SortKey::StayHours : SortKey::TotalCost);

    PatientList sortedCopy(source);
    SortStats stats;
    Clock::time_point start = Clock::now();
    runSort(sortedCopy, algorithm, key, stats);
    double micros = elapsedMicros(start, Clock::now());

    cout << "\n  Sorted " << sortedCopy.size() << " records by " << sortKeyName(key) << " using "
         << sortAlgorithmName(algorithm) << " Sort in " << formatNumber(micros, 3) << " us (comparisons: "
         << formatCount(stats.comparisons) << ", relinks: " << formatCount(stats.relinks)
         << ", aux memory: " << formatCount(stats.extraBytes) << " B)\n";
    showRecords(sortedCopy, string("Records sorted by ") + sortKeyName(key) + " (ascending)", rowsToShow);
}
