// Program_Array.cpp
// MetroHealth patient management - ARRAY version (Tasks 2-7).
// Uses a self-written dynamic array plus hand-coded sorting and searching
// (no STL containers or algorithms). Build as a C++17 Console App in Visual Studio;
// the modules are the .hpp files in this folder.

#include "Records.hpp"
#include "Demographics.hpp"
#include "Expenditure.hpp"
#include "Sorting.hpp"
#include "Searching.hpp"

// Shows the load summary, then the records of the dataset the user picks.
void viewDatasets(const string& dataFolder, const PatientArray datasets[], const PatientArray& allPatients,
                  const LoadInfo info[]) {
    showLoadSummary(dataFolder, datasets, allPatients, info);
    cout << "\n    [1] " << FACILITIES[0].name << "   [2] " << FACILITIES[1].name << "   [3] " << FACILITIES[2].name
         << "   [4] All facilities   [0] Back\n";
    int choice = readInt("  View records of (0-4): ", 0, 4);
    if (choice == 0) return;
    if (choice == 4) {
        showRecords(allPatients, "ALL FACILITIES - PATIENT RECORDS", allPatients.size());
    } else {
        const Facility& facility = FACILITIES[choice - 1];
        showRecords(datasets[choice - 1], string(facility.name) + " (" + facility.type + ") - PATIENT RECORDS",
                    datasets[choice - 1].size());
    }
}

// Prints the main menu with the number of records currently in memory.
void showMainMenu(const PatientArray datasets[], const PatientArray& allPatients) {
    cout << "\n" << string(BANNER_WIDTH, '=') << "\n"
         << "  METROHEALTH PATIENT MANAGEMENT SYSTEM - " << PROGRAM_MODE << " Implementation\n"
         << "  Records in memory: " << allPatients.size() << "  (Facility A: " << datasets[0].size()
         << " | Facility B: " << datasets[1].size() << " | Facility C: " << datasets[2].size() << ")\n"
         << string(BANNER_WIDTH, '=') << "\n"
         << "  [1] View loaded datasets & records            (Task 2/3)\n"
         << "  [2] Insert a new patient record               (Task 3)\n"
         << "  [3] Demographic & billing categorisation      (Task 4)\n"
         << "  [4] Healthcare expenditure & service analysis (Task 5)\n"
         << "  [5] Sorting experiments                       (Task 6)\n"
         << "  [6] Sort & view a dataset                     (Task 6)\n"
         << "  [7] Searching experiments - preset queries    (Task 7)\n"
         << "  [8] Custom search                             (Task 7)\n"
         << "  [9] Run ALL tasks (2, 4, 5, 6, 7)\n"
         << "  [0] Exit\n";
}

// Finds and loads the three datasets once, then runs the menu until the user exits.
int main() {
    printTitle(string("MetroHealth System - Hospital Patient Management (") + PROGRAM_MODE + "-Based)");

    string dataFolder;
    if (!findDataFolder(dataFolder)) return 1;

    PatientArray datasets[NUM_DATASETS];
    PatientArray allPatients;
    LoadInfo info[NUM_DATASETS];
    if (!loadAllDatasets(dataFolder, datasets, allPatients, info)) return 1;

    static ResultLog resultLog;   // static: too large for the stack
    resultLog.init(dataFolder);

    showLoadSummary(dataFolder, datasets, allPatients, info);

    while (true) {
        showMainMenu(datasets, allPatients);
        switch (readInt("  Select an option (0-9): ", 0, 9)) {
        case 1: viewDatasets(dataFolder, datasets, allPatients, info); break;
        case 2: addNewPatient(datasets, allPatients); break;
        case 3: runDemographicAnalysis(allPatients); break;
        case 4: runExpenditureAnalysis(allPatients); break;
        case 5: runSortExperiment(datasets, allPatients, resultLog); break;
        case 6: sortAndShowRecords(datasets, allPatients); break;
        case 7: runSearchExperiment(allPatients, resultLog); break;
        case 8: runCustomSearch(allPatients, resultLog); break;
        case 9:
            showLoadSummary(dataFolder, datasets, allPatients, info);
            runDemographicAnalysis(allPatients);
            runExpenditureAnalysis(allPatients);
            runSortExperiment(datasets, allPatients, resultLog);
            runSearchExperiment(allPatients, resultLog);
            break;
        case 0:
            cout << "\n  Goodbye.\n";
            return 0;
        }
        waitForEnter();
    }
}
