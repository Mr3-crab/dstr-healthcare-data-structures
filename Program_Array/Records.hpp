// Records.hpp - Tasks 2-3: record tables, load summary and inserting a patient.
#pragma once

#include "Table.hpp"
#include "CareTally.hpp"
#include "DataLoader.hpp"

// Sets up the columns of the patient record table.
inline void setupPatientColumns(Table& table) {
    table.addColumn("No", 4, true);
    table.addColumn("Patient ID", 10);
    table.addColumn("Facility", 10);
    table.addColumn("Age", 3, true);
    table.addColumn("Group", 6);
    table.addColumn("Care Type", 15);
    table.addColumn("Stay (h)", 8, true);
    table.addColumn("RM/Hour", 8, true);
    table.addColumn("Visits", 6, true);
    table.addColumn("Total Cost (RM)", 15, true);
}

// Prints one patient as a row of the record table.
inline void printPatientRow(const Table& table, int rowNumber, const Patient& patient) {
    string cells[10] = {
        to_string(rowNumber), patient.id, FACILITIES[patient.facilityIndex].name, to_string(patient.age),
        getAgeGroupRange(patient.age), patient.careType, formatCompact(patient.stayHours),
        formatNumber(patient.ratePerHour), to_string(patient.visitsPerYear), formatNumber(patient.totalCost)
    };
    table.printRow(cells);
}

// Traverses the array by index and prints up to 'limit' records as a table.
inline void showRecords(const PatientArray& patients, const string& title, int limit) {
    printHeading(title);
    Table table;
    setupPatientColumns(table);
    table.printHeader();
    int shown = limit < patients.size() ? limit : patients.size();
    for (int i = 0; i < shown; i++) printPatientRow(table, i + 1, patients[i]);
    table.printBorder();
    cout << "  Showing " << shown << " of " << patients.size() << " records.\n";
}

// Task 2: shows how many records were loaded from each file and the memory used.
inline void showLoadSummary(const string& dataFolder, const PatientArray datasets[], const PatientArray& allPatients,
                            const LoadInfo info[]) {
    printTitle("TASK 2: DATASETS LOADED INTO A DYNAMIC ARRAY");
    cout << "  Data folder: " << (dataFolder.empty() ? string("(current folder)") : dataFolder) << "\n";

    Table table;
    table.addColumn("Dataset", 10);
    table.addColumn("Facility Type", 25);
    table.addColumn("Source File", 24);
    table.addColumn("Loaded", 6, true);
    table.addColumn("Skipped", 7, true);
    table.addColumn("Capacity", 8, true);
    table.addColumn("Memory (B)", 10, true);
    table.printHeader();
    for (int dataset = 0; dataset < NUM_DATASETS; dataset++) {
        string cells[7] = {
            FACILITIES[dataset].name, FACILITIES[dataset].type, FACILITIES[dataset].fileName,
            to_string(info[dataset].loadedRows), to_string(info[dataset].skippedRows),
            to_string(datasets[dataset].capacity()), formatCount(datasets[dataset].memoryBytes())
        };
        table.printRow(cells);
    }
    table.printBorder();
    string totalRow[7] = {
        "All", "Combined (all facilities)", "-", to_string(allPatients.size()), "-",
        to_string(allPatients.capacity()), formatCount(allPatients.memoryBytes())
    };
    table.printRow(totalRow);
    table.printBorder();

    cout << "\n  Care types found in each dataset:\n";
    for (int dataset = 0; dataset < NUM_DATASETS; dataset++) {
        cout << "    " << FACILITIES[dataset].name << ": " << buildCareTally(datasets[dataset]).getSummary() << "\n";
    }
    cout << "\n  Storage: dynamic array (contiguous, capacity doubles when full), "
         << sizeof(Patient) << " bytes per record\n";
}

// Task 3: reads a new patient from the user and appends it to its facility's
// array and to the combined array (amortised O(1) insertion at the end).
inline void addNewPatient(PatientArray datasets[], PatientArray& allPatients) {
    printTitle("TASK 3: INSERT A NEW PATIENT RECORD (ARRAY)");
    Patient patient;
    while (true) {
        patient.id = readText("  Patient ID (e.g. PT10042): ");
        bool idTaken = false;
        for (int i = 0; i < allPatients.size() && !idTaken; i++) idTaken = equalsIgnoreCase(allPatients[i].id, patient.id);
        if (!idTaken) break;
        cout << "  That ID already exists.\n";
    }

    for (int dataset = 0; dataset < NUM_DATASETS; dataset++) {
        cout << "    [" << dataset + 1 << "] " << FACILITIES[dataset].name << " - " << FACILITIES[dataset].type << "\n";
    }
    patient.facilityIndex = readInt("  Facility (1-3): ", 1, NUM_DATASETS) - 1;
    patient.age = readInt("  Age (0-100): ", 0, 100);

    // Show the existing care types and reuse their exact spelling if the user's input matches one.
    CareTally careTypes = buildCareTally(allPatients);
    careTypes.sortByName();
    cout << "  Existing care types:";
    for (int i = 0; i < careTypes.size(); i++) cout << (i == 0 ? " " : ", ") << careTypes[i].name;
    cout << "\n";
    patient.careType = readText("  Care type: ");
    for (int i = 0; i < careTypes.size(); i++) {
        if (equalsIgnoreCase(careTypes[i].name, patient.careType)) patient.careType = careTypes[i].name;
    }

    patient.stayHours = readDouble("  Length of stay / visit duration (hours): ", 0, 10000);
    patient.ratePerHour = readDouble("  Base cost per hour (RM): ", 0, 100000);
    patient.visitsPerYear = readInt("  Visits per year: ", 0, 366);
    patient.totalCost = patient.stayHours * patient.ratePerHour * patient.visitsPerYear;

    datasets[patient.facilityIndex].add(patient);
    allPatients.add(patient);

    Table table;
    setupPatientColumns(table);
    cout << "\n  Added to the end of the array (amortised O(1)):\n";
    table.printHeader();
    printPatientRow(table, allPatients.size(), patient);
    table.printBorder();
    cout << "  Note: kept in memory only; the CSV files are not changed.\n";
}
