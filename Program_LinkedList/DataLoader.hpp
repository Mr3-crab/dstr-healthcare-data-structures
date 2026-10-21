// DataLoader.hpp - Task 2: finds the data folder and loads the CSV files.
#pragma once

#include "Utils.hpp"
#include "PatientList.hpp"

enum { COL_ID, COL_AGE, COL_CARE_TYPE, COL_STAY, COL_RATE, COL_VISITS, NUM_CSV_COLUMNS };

const char* const CSV_COLUMNS[NUM_CSV_COLUMNS] = {
    "PatientID", "Age", "CareType", "LengthOfStay", "BaseCostPerHour", "DaysVisitsPerYear"
};

const int MAX_CSV_FIELDS = 16;

// Number of rows loaded and skipped for one file.
struct LoadInfo {
    int loadedRows = 0;
    int skippedRows = 0;
};

// True if the file exists and can be opened.
inline bool fileExists(const string& path) {
    ifstream file(path);
    return file.is_open();
}

// True if all three dataset files are in the folder.
inline bool folderHasAllFiles(const string& folder) {
    for (int dataset = 0; dataset < NUM_DATASETS; dataset++) {
        if (!fileExists(folder + FACILITIES[dataset].fileName)) return false;
    }
    return true;
}

// Finds the folder that holds the CSV files. Visual Studio runs the program from
// the project folder, so the current folder and a few parent folders are checked
// first; if the files are still not found, the user is asked for the folder.
inline bool findDataFolder(string& dataFolder) {
    const char* candidates[] = { "", "../", "../../", "../../../", "../../../../" };
    for (const char* candidate : candidates) {
        if (folderHasAllFiles(candidate)) {
            dataFolder = candidate;
            return true;
        }
    }

    cout << "\n  Could not find the dataset files:\n";
    for (int dataset = 0; dataset < NUM_DATASETS; dataset++) cout << "    " << FACILITIES[dataset].fileName << "\n";
    while (true) {
        string folder = readLine("\n  Folder containing the CSV files (Enter to quit): ");
        if (folder.empty()) return false;
        if (folder.size() >= 2 && folder.front() == '"' && folder.back() == '"') folder = folder.substr(1, folder.size() - 2);
        if (folder.empty()) continue;
        if (folder.back() != '/' && folder.back() != '\\') folder += '\\';
        if (folderHasAllFiles(folder)) {
            dataFolder = folder;
            return true;
        }
        cout << "  Files not found in that folder.\n";
    }
}

// Reads one CSV dataset into the linked list. Columns are matched by their header name,
// rows with missing or invalid values are skipped and counted, and each patient's
// total cost is calculated as stay x rate x visits.
inline bool loadCsvFile(const string& path, int facilityIndex, PatientList& patients, LoadInfo& info) {
    ifstream file(path);
    string line;
    if (!file.is_open() || !getline(file, line)) {
        cout << "  [ERROR] Cannot read " << path << "\n";
        return false;
    }
    if (line.size() >= 3 && static_cast<unsigned char>(line[0]) == 0xEF) line = line.substr(3);   // skip UTF-8 BOM

    string headerFields[MAX_CSV_FIELDS];
    int headerCount = splitLine(trimText(line), ',', headerFields, MAX_CSV_FIELDS);
    if (headerCount > MAX_CSV_FIELDS) headerCount = MAX_CSV_FIELDS;

    // Find the position of each expected column in this file's header.
    int columnPos[NUM_CSV_COLUMNS];
    for (int column = 0; column < NUM_CSV_COLUMNS; column++) {
        columnPos[column] = -1;
        for (int field = 0; field < headerCount; field++) {
            if (equalsIgnoreCase(headerFields[field], CSV_COLUMNS[column])) columnPos[column] = field;
        }
        if (columnPos[column] < 0) {
            cout << "  [ERROR] Column " << CSV_COLUMNS[column] << " missing in " << path << "\n";
            return false;
        }
    }

    string fields[MAX_CSV_FIELDS];
    int lineNumber = 1;
    while (getline(file, line)) {
        lineNumber++;
        line = trimText(line);
        if (line.empty()) continue;

        Patient patient;
        bool valid = splitLine(line, ',', fields, MAX_CSV_FIELDS) >= headerCount;
        if (valid) {
            patient.id = fields[columnPos[COL_ID]];
            patient.careType = fields[columnPos[COL_CARE_TYPE]];
            valid = !patient.id.empty() && !patient.careType.empty()
                && parseInt(fields[columnPos[COL_AGE]], patient.age)
                && parseDouble(fields[columnPos[COL_STAY]], patient.stayHours)
                && parseDouble(fields[columnPos[COL_RATE]], patient.ratePerHour)
                && parseInt(fields[columnPos[COL_VISITS]], patient.visitsPerYear)
                && patient.age >= 0 && patient.age <= 120 && patient.stayHours >= 0
                && patient.ratePerHour >= 0 && patient.visitsPerYear >= 0;
        }
        if (!valid) {
            if (++info.skippedRows <= 5) cout << "  [WARNING] " << path << " line " << lineNumber << " skipped: " << line << "\n";
            continue;
        }

        patient.facilityIndex = facilityIndex;
        patient.totalCost = patient.stayHours * patient.ratePerHour * patient.visitsPerYear;
        patients.append(patient);
        info.loadedRows++;
    }
    return true;
}

// Loads the three datasets and builds one combined list of all patients.
inline bool loadAllDatasets(const string& dataFolder, PatientList datasets[], PatientList& allPatients, LoadInfo info[]) {
    for (int dataset = 0; dataset < NUM_DATASETS; dataset++) {
        if (!loadCsvFile(dataFolder + FACILITIES[dataset].fileName, dataset, datasets[dataset], info[dataset])) return false;
        for (const PatientNode* node = datasets[dataset].getHead(); node; node = node->next) {
            allPatients.append(node->patient);
        }
    }
    return true;
}
