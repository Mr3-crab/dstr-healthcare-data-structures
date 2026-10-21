// Searching.hpp - Task 7: linear search, binary search and the searching experiments.
#pragma once

#include "Sorting.hpp"

enum class SearchType { AgeRange, CareType, StayLongerThan };

// What to search for.
struct SearchQuery {
    SearchType type = SearchType::AgeRange;
    int minAge = 0;
    int maxAge = 0;
    string careType;
    double minStayHours = 0;
    string label;         // short name used in the results file
    string description;   // shown on screen
};

// Counters collected during one search.
struct SearchStats {
    long long comparisons = 0;
    long long steps = 0;        // array elements visited
    int matchCount = 0;
    size_t extraBytes = 0;
};

// Builds a query for patients aged minAge..maxAge.
inline SearchQuery makeAgeQuery(int minAge, int maxAge, const string& groupLabel) {
    SearchQuery query;
    query.type = SearchType::AgeRange;
    query.minAge = minAge;
    query.maxAge = maxAge;
    query.label = "Age " + to_string(minAge) + "-" + to_string(maxAge);
    query.description = "Age group " + to_string(minAge) + "-" + to_string(maxAge) +
                        (groupLabel.empty() ? "" : " (" + groupLabel + ")");
    return query;
}

// Builds a query for patients with one care type.
inline SearchQuery makeCareTypeQuery(const string& careType) {
    SearchQuery query;
    query.type = SearchType::CareType;
    query.careType = careType;
    query.label = careType;
    query.description = "Care Type = " + careType;
    return query;
}

// Builds a query for visits longer than the given number of hours.
inline SearchQuery makeStayQuery(double minStayHours) {
    SearchQuery query;
    query.type = SearchType::StayLongerThan;
    query.minStayHours = minStayHours;
    query.label = "Stay > " + formatCompact(minStayHours) + "h";
    query.description = "Visit duration > " + formatCompact(minStayHours) + " hours";
    return query;
}

// The key the data must be sorted by for binary search to work on this query.
inline SortKey requiredSortKey(const SearchQuery& query) {
    if (query.type == SearchType::AgeRange) return SortKey::Age;
    if (query.type == SearchType::CareType) return SortKey::CareType;
    return SortKey::StayHours;
}

// True if the patient matches the query; adds the comparisons made to 'comparisons'.
inline bool matchesQuery(const Patient& patient, const SearchQuery& query, long long& comparisons) {
    comparisons++;
    switch (query.type) {
    case SearchType::AgeRange:
        if (patient.age < query.minAge) return false;
        comparisons++;
        return patient.age <= query.maxAge;
    case SearchType::CareType:
        return patient.careType == query.careType;
    case SearchType::StayLongerThan:
        return patient.stayHours > query.minStayHours;
    }
    return false;
}

// Used by binary search on sorted data: true if the patient comes before the start
// of the matching range (findEnd = false) or before its end (findEnd = true).
inline bool isBeforeBoundary(const Patient& patient, const SearchQuery& query, bool findEnd) {
    switch (query.type) {
    case SearchType::AgeRange:       return findEnd ? patient.age <= query.maxAge : patient.age < query.minAge;
    case SearchType::CareType:       return findEnd ? patient.careType <= query.careType : patient.careType < query.careType;
    case SearchType::StayLongerThan: return findEnd ? true : patient.stayHours <= query.minStayHours;
    }
    return false;
}

// Linear search (array version).
// Checks every record from the first to the last and keeps the positions that match.
// Time: O(n) on sorted or unsorted data. Extra space: the result list.
inline void linearSearch(const PatientArray& patients, const SearchQuery& query, IndexList& results, SearchStats& stats) {
    for (int i = 0; i < patients.size(); i++) {
        stats.steps++;
        if (matchesQuery(patients[i], query, stats.comparisons)) results.add(i);
    }
    stats.matchCount = results.size();
    stats.extraBytes = results.memoryBytes() + sizeof(int);
}

// Finds the first position whose record is not before the boundary, by halving the
// search range each step (checks the middle record, then keeps the left or right half).
// Adapted from: Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2022).
//   Introduction to algorithms (4th ed.). MIT Press. (Binary search, Exercise 2.3-6)
inline int findBoundary(const PatientArray& patients, const SearchQuery& query, bool findEnd, SearchStats& stats) {
    int low = 0, high = patients.size();
    while (low < high) {
        int mid = low + (high - low) / 2;
        stats.steps++;
        stats.comparisons++;
        if (isBeforeBoundary(patients[mid], query, findEnd)) low = mid + 1;
        else high = mid;
    }
    return low;
}

// Binary search (array version).
// On data sorted by the search key all matches sit next to each other, so two binary
// searches find where the matching block starts and ends, then the block is collected.
// Time: O(log n) to find the block + O(k) to collect k matches.
// Only correct on sorted data - on unsorted data the result is wrong.
inline void binarySearch(const PatientArray& patients, const SearchQuery& query, IndexList& results, SearchStats& stats) {
    int first = findBoundary(patients, query, false, stats);
    int last = query.type == SearchType::StayLongerThan ? patients.size() : findBoundary(patients, query, true, stats);
    for (int i = first; i < last; i++) {
        stats.steps++;
        results.add(i);
    }
    stats.matchCount = results.size();
    stats.extraBytes = results.memoryBytes() + 3 * sizeof(int);   // plus low, high and mid
}

// True if the search found exactly the expected number of records and all of them match.
inline bool isResultCorrect(const PatientArray& patients, const IndexList& results, const SearchQuery& query, int expectedCount) {
    if (results.size() != expectedCount) return false;
    for (int i = 0; i < results.size(); i++) {
        long long unusedComparisons = 0;
        if (!matchesQuery(patients[results[i]], query, unusedComparisons)) return false;
    }
    return true;
}

// Average time and comparisons of one search run (used in the summary table).
struct SearchTiming {
    double avgMicros = 0;
    long long comparisons = 0;
};

// Runs linear and binary search on the unsorted data and on a copy sorted by the search
// key (4 runs, in that order), prints the results table and previews the matching records.
inline void compareSearchAlgorithms(const PatientArray& allPatients, const SearchQuery& query, ResultLog& resultLog,
                                    int previewRows, SearchTiming timings[4]) {
    printHeading("Search: " + query.description);

    PatientArray sortedPatients(allPatients);
    SortStats unusedSortStats;
    mergeSort(sortedPatients, requiredSortKey(query), unusedSortStats);

    // The correct answer, from a linear search on the original data.
    IndexList expectedResults;
    SearchStats expectedStats;
    linearSearch(allPatients, query, expectedResults, expectedStats);
    int expectedCount = expectedResults.size();

    Table table;
    table.addColumn("Data", 8);
    table.addColumn("Algorithm", 9);
    table.addColumn("Matches", 7, true);
    table.addColumn("Correct", 7);
    table.addColumn("Avg Time (us)", 13, true);
    table.addColumn("Comparisons", 11, true);
    table.addColumn("Steps", 9, true);
    table.addColumn("Aux Mem (B)", 11, true);
    table.addColumn("Proc WS (KB)", 12, true);
    table.printHeader();

    int runIndex = 0;
    for (int order = 0; order < 2; order++) {
        bool useSortedData = order == 1;
        const PatientArray& data = useSortedData ? sortedPatients : allPatients;
        for (int algoIndex = 0; algoIndex < 2; algoIndex++) {
            bool useBinary = algoIndex == 1;
            // [!] BINARY SEARCH ON UNSORTED DATA: undefined result - demonstrates why sorting is required.
            // Binary search assumes the data is ordered by the search key; on file-order data its halving
            // decisions are meaningless. The run is kept on purpose and flagged in the table.
            bool binaryOnUnsorted = useBinary && !useSortedData;

            SearchStats stats;
            bool correct = false;
            size_t workingSetKB = 0;
            double totalMicros = 0;
            for (int run = 0; run < SEARCH_RUNS; run++) {
                IndexList results;
                SearchStats runStats;
                Clock::time_point start = Clock::now();
                if (useBinary) binarySearch(data, query, results, runStats);
                else linearSearch(data, query, results, runStats);
                totalMicros += elapsedMicros(start, Clock::now());
                if (run == 0) {
                    stats = runStats;
                    correct = isResultCorrect(data, results, query, expectedCount);
                    workingSetKB = getWorkingSetKB();
                }
            }
            timings[runIndex].avgMicros = totalMicros / SEARCH_RUNS;
            timings[runIndex].comparisons = stats.comparisons;

            const char* orderName = useSortedData ? "Sorted" : "Unsorted";
            const char* algorithmName = useBinary ? "Binary" : "Linear";
            string cells[9] = {
                orderName, algorithmName, to_string(stats.matchCount), correct ? "Yes" : "NO",
                formatNumber(timings[runIndex].avgMicros, 3), formatCount(stats.comparisons), formatCount(stats.steps),
                formatCount(stats.extraBytes), formatCount(workingSetKB)
            };
            table.printRow(cells);
            if (binaryOnUnsorted) table.printNote("  ^ [!] undefined result - demonstrates why sorting is required");

            resultLog.addResult("SEARCH", query.label + "/" + orderName + "/" + algorithmName,
                                timings[runIndex].avgMicros, stats.steps, stats.extraBytes);
            runIndex++;
        }
    }
    table.printBorder();
    cout << "  Expected matches: " << expectedCount << ". Binary search needs data sorted by "
         << sortKeyName(requiredSortKey(query)) << ".\n  Steps = array elements visited. Aux Mem = result list + local variables.\n";

    if (previewRows > 0 && expectedCount > 0) {
        IndexList results;
        SearchStats stats;
        binarySearch(sortedPatients, query, results, stats);
        int shown = previewRows < results.size() ? previewRows : results.size();
        cout << "\n  First " << shown << " of " << results.size() << " matching records:\n";
        Table recordTable;
        setupPatientColumns(recordTable);
        recordTable.printHeader();
        for (int i = 0; i < shown; i++) printPatientRow(recordTable, i + 1, sortedPatients[results[i]]);
        recordTable.printBorder();
    }
}

// Task 7: runs the three preset searches (age 61-100, Emergency, stay > 24 hours)
// and prints a summary comparing linear and binary search.
inline void runSearchExperiment(const PatientArray& allPatients, ResultLog& resultLog) {
    printTitle("TASK 7: SEARCHING EXPERIMENTS (ARRAY)");
    cout << "  Algorithms : Linear Search (O(n)) and Binary Search (O(log n), needs sorted data)\n"
         << "  Data       : all " << allPatients.size() << " patients, unsorted (file order) and sorted by the search key\n"
         << "  Timing     : average of " << SEARCH_RUNS << " runs per search\n";

    const AgeGroup& seniors = AGE_GROUPS[4];
    SearchQuery queries[3] = {
        makeAgeQuery(seniors.minAge, seniors.maxAge, seniors.label),
        makeCareTypeQuery("Emergency"),
        makeStayQuery(24)
    };
    SearchTiming timings[3][4];
    for (int i = 0; i < 3; i++) compareSearchAlgorithms(allPatients, queries[i], resultLog, 5, timings[i]);

    printHeading("SEARCH SUMMARY (average microseconds / comparisons)");
    Table table;
    table.addColumn("Query", 18);
    table.addColumn("Lin Unsort", 10, true);
    table.addColumn("Bin Unsort", 10, true);
    table.addColumn("Lin Sorted", 10, true);
    table.addColumn("Bin Sorted", 10, true);
    table.addColumn("Lin Comp", 8, true);
    table.addColumn("Bin Comp", 8, true);
    table.addColumn("Bin Speed-up", 12, true);
    table.printHeader();
    for (int i = 0; i < 3; i++) {
        const SearchTiming* run = timings[i];   // [0] linear unsorted, [1] binary unsorted, [2] linear sorted, [3] binary sorted
        string cells[8] = {
            queries[i].label, formatNumber(run[0].avgMicros, 3), formatNumber(run[1].avgMicros, 3),
            formatNumber(run[2].avgMicros, 3), formatNumber(run[3].avgMicros, 3),
            formatCount(run[2].comparisons), formatCount(run[3].comparisons),
            run[3].avgMicros > 0 ? formatFixed(run[2].avgMicros / run[3].avgMicros, 2) + "x" : "-"
        };
        table.printRow(cells);
    }
    table.printBorder();
    cout << "  Bin Speed-up = Linear (sorted) time / Binary (sorted) time.\n";

    resultLog.printComparison("SEARCH");
}

// Task 7: lets the user choose what to search for, then runs the same comparison.
inline void runCustomSearch(const PatientArray& allPatients, ResultLog& resultLog) {
    printTitle("TASK 7: CUSTOM SEARCH (ARRAY)");
    cout << "    [1] By age group\n    [2] By care type\n    [3] By visit duration (> hours)\n";
    int searchChoice = readInt("  Search type (1-3): ", 1, 3);

    SearchQuery query;
    if (searchChoice == 1) {
        for (int group = 0; group < NUM_AGE_GROUPS; group++) {
            cout << "    [" << group + 1 << "] " << AGE_GROUPS[group].range << " " << AGE_GROUPS[group].label << "\n";
        }
        cout << "    [6] Custom age range\n";
        int groupChoice = readInt("  Age group (1-6): ", 1, 6);
        if (groupChoice <= NUM_AGE_GROUPS) {
            const AgeGroup& group = AGE_GROUPS[groupChoice - 1];
            query = makeAgeQuery(group.minAge, group.maxAge, group.label);
        } else {
            int minAge = readInt("  Minimum age (0-120): ", 0, 120);
            int maxAge = readInt("  Maximum age (" + to_string(minAge) + "-120): ", minAge, 120);
            query = makeAgeQuery(minAge, maxAge, "");
        }
    } else if (searchChoice == 2) {
        CareTally careTypes = buildCareTally(allPatients);
        careTypes.sortByName();
        for (int i = 0; i < careTypes.size(); i++) cout << "    [" << i + 1 << "] " << careTypes[i].name << "\n";
        int careChoice = readInt("  Care type (1-" + to_string(careTypes.size()) + "): ", 1, careTypes.size());
        query = makeCareTypeQuery(careTypes[careChoice - 1].name);
    } else {
        query = makeStayQuery(readDouble("  Show visits longer than (hours): ", 0, 100000));
    }

    int previewRows = readInt("  Matching records to display (0-" + to_string(allPatients.size()) + "): ", 0, allPatients.size());
    SearchTiming timings[4];
    compareSearchAlgorithms(allPatients, query, resultLog, previewRows, timings);
    resultLog.printComparison("SEARCH");
}
