// Expenditure.hpp - Task 5: healthcare expenditure & service analysis.
#pragma once

#include "Table.hpp"
#include "CareTally.hpp"

const int ALL_GROUPS_ROW = NUM_AGE_GROUPS;        // extra row that holds the totals of all age groups
const int ALL_FACILITIES_COLUMN = NUM_DATASETS;   // extra column that holds the totals of all facilities

// All totals needed for the Task 5 tables, collected in one pass over the patients.
struct SpendingSummary {
    int facilityPatients[NUM_DATASETS] = {};
    double facilityCost[NUM_DATASETS] = {};
    double facilityStayHours[NUM_DATASETS] = {};
    CareTally careTypes;
    int careFacilityPatients[MAX_CARE_TYPES][NUM_DATASETS] = {};
    double careFacilityCost[MAX_CARE_TYPES][NUM_DATASETS] = {};
    int groupPatients[NUM_AGE_GROUPS + 1][NUM_DATASETS + 1] = {};
    double groupCost[NUM_AGE_GROUPS + 1][NUM_DATASETS + 1] = {};
    double groupStayHours[NUM_AGE_GROUPS + 1][NUM_DATASETS + 1] = {};
    int totalPatients = 0;
    double totalCost = 0;
    double totalStayHours = 0;
};

// Traverses all patients once and adds each one to the facility, care type and
// age group totals (including the "all groups" row and "all facilities" column).
inline void collectSpending(const PatientList& allPatients, SpendingSummary& summary) {
    summary.careTypes = buildCareTally(allPatients);
    summary.careTypes.sortByName();
    for (const PatientNode* node = allPatients.getHead(); node; node = node->next) {
        const Patient& patient = node->patient;
        int facility = patient.facilityIndex;
        summary.facilityPatients[facility]++;
        summary.facilityCost[facility] += patient.totalCost;
        summary.facilityStayHours[facility] += patient.stayHours;
        summary.totalPatients++;
        summary.totalCost += patient.totalCost;
        summary.totalStayHours += patient.stayHours;

        int careIndex = summary.careTypes.findIndex(patient.careType);
        if (careIndex >= 0) {
            summary.careFacilityPatients[careIndex][facility]++;
            summary.careFacilityCost[careIndex][facility] += patient.totalCost;
        }

        int groupIndex = getAgeGroupIndex(patient.age);
        if (groupIndex < 0) continue;
        int rows[2] = { groupIndex, ALL_GROUPS_ROW };
        int columns[2] = { facility, ALL_FACILITIES_COLUMN };
        for (int row : rows) {
            for (int column : columns) {
                summary.groupPatients[row][column]++;
                summary.groupCost[row][column] += patient.totalCost;
                summary.groupStayHours[row][column] += patient.stayHours;
            }
        }
    }
}

// Table 5.1: total billing, average cost and average stay per dataset.
inline void showFacilityTotals(const SpendingSummary& summary) {
    printHeading("5.1 TOTAL MEDICAL BILLING PER DATASET");
    Table table;
    table.addColumn("Dataset", 10);
    table.addColumn("Facility Type", 25);
    table.addColumn("Patients", 8, true);
    table.addColumn("Total Cost (RM)", 15, true);
    table.addColumn("Avg/Patient (RM)", 16, true);
    table.addColumn("Avg Stay (h)", 12, true);
    table.addColumn("Share", 7, true);
    table.printHeader();
    for (int facility = 0; facility < NUM_DATASETS; facility++) {
        int patients = summary.facilityPatients[facility];
        string cells[7] = {
            FACILITIES[facility].name, FACILITIES[facility].type, to_string(patients),
            formatNumber(summary.facilityCost[facility]),
            formatNumber(safeAverage(summary.facilityCost[facility], patients)),
            formatNumber(safeAverage(summary.facilityStayHours[facility], patients)),
            formatPercent(summary.facilityCost[facility], summary.totalCost)
        };
        table.printRow(cells);
    }
    table.printBorder();
    string totalRow[7] = {
        "TOTAL", "All facilities", to_string(summary.totalPatients), formatNumber(summary.totalCost),
        formatNumber(safeAverage(summary.totalCost, summary.totalPatients)),
        formatNumber(safeAverage(summary.totalStayHours, summary.totalPatients)), "100.00%"
    };
    table.printRow(totalRow);
    table.printBorder();
    cout << "  Avg Stay = average hours per visit.\n";
}

// Table 5.2: total medical cost grouped by care type.
inline void showCareTypeTotals(const SpendingSummary& summary) {
    printHeading("5.2 TOTAL MEDICAL COST BY CARE TYPE (ALL FACILITIES)");
    Table table;
    table.addColumn("Care Type", 15);
    table.addColumn("Patients", 8, true);
    table.addColumn("Total Cost (RM)", 15, true);
    table.addColumn("Avg/Patient (RM)", 16, true);
    table.addColumn("Avg Stay (h)", 12, true);
    table.addColumn("Share", 7, true);
    table.printHeader();
    for (int i = 0; i < summary.careTypes.size(); i++) {
        const CareTypeStats& care = summary.careTypes[i];
        string cells[6] = {
            care.name, to_string(care.patientCount), formatNumber(care.totalCost),
            formatNumber(safeAverage(care.totalCost, care.patientCount)),
            formatNumber(safeAverage(care.totalStayHours, care.patientCount)),
            formatPercent(care.totalCost, summary.totalCost)
        };
        table.printRow(cells);
    }
    table.printBorder();
    string totalRow[6] = {
        "TOTAL", to_string(summary.totalPatients), formatNumber(summary.totalCost),
        formatNumber(safeAverage(summary.totalCost, summary.totalPatients)),
        formatNumber(safeAverage(summary.totalStayHours, summary.totalPatients)), "100.00%"
    };
    table.printRow(totalRow);
    table.printBorder();
}

// Table 5.3: medical cost of each care type in each dataset (patient count in brackets).
inline void showCareTypeByFacility(const SpendingSummary& summary) {
    printHeading("5.3 MEDICAL COST BY CARE TYPE AND DATASET (RM, patients in brackets)");
    Table table;
    table.addColumn("Care Type", 15);
    for (int facility = 0; facility < NUM_DATASETS; facility++) table.addColumn(FACILITIES[facility].name, 18, true);
    table.addColumn("Total (RM)", 15, true);
    table.printHeader();
    for (int careIndex = 0; careIndex < summary.careTypes.size(); careIndex++) {
        string cells[NUM_DATASETS + 2];
        cells[0] = summary.careTypes[careIndex].name;
        for (int facility = 0; facility < NUM_DATASETS; facility++) {
            int patients = summary.careFacilityPatients[careIndex][facility];
            cells[facility + 1] = patients == 0 ? "-"
                : formatNumber(summary.careFacilityCost[careIndex][facility]) + " (" + to_string(patients) + ")";
        }
        cells[NUM_DATASETS + 1] = formatNumber(summary.careTypes[careIndex].totalCost);
        table.printRow(cells);
    }
    table.printBorder();
    string totalRow[NUM_DATASETS + 2];
    totalRow[0] = "TOTAL";
    for (int facility = 0; facility < NUM_DATASETS; facility++) {
        totalRow[facility + 1] = formatNumber(summary.facilityCost[facility]) + " (" + to_string(summary.facilityPatients[facility]) + ")";
    }
    totalRow[NUM_DATASETS + 1] = formatNumber(summary.totalCost);
    table.printRow(totalRow);
    table.printBorder();
}

// Tables 5.4-5.6: age group x dataset comparison of total cost, average cost and average stay.
inline void showAgeGroupByFacility(const SpendingSummary& summary) {
    const char* titles[3] = {
        "5.4 TOTAL EXPENDITURE BY AGE GROUP AND DATASET (RM)",
        "5.5 AVERAGE COST PER PATIENT BY AGE GROUP AND DATASET (RM)",
        "5.6 AVERAGE VISIT DURATION BY AGE GROUP AND DATASET (hours per visit)"
    };
    for (int view = 0; view < 3; view++) {
        printHeading(titles[view]);
        Table table;
        table.addColumn("Age Group", 9);
        for (int facility = 0; facility < NUM_DATASETS; facility++) table.addColumn(FACILITIES[facility].name, 14, true);
        table.addColumn("All Facilities", 14, true);
        table.printHeader();
        for (int row = 0; row <= ALL_GROUPS_ROW; row++) {
            if (row == ALL_GROUPS_ROW) table.printBorder();
            string cells[NUM_DATASETS + 2];
            cells[0] = row == ALL_GROUPS_ROW ? "ALL" : AGE_GROUPS[row].range;
            for (int column = 0; column <= ALL_FACILITIES_COLUMN; column++) {
                int patients = summary.groupPatients[row][column];
                if (patients == 0) cells[column + 1] = "-";
                else if (view == 0) cells[column + 1] = formatNumber(summary.groupCost[row][column]);
                else if (view == 1) cells[column + 1] = formatNumber(summary.groupCost[row][column] / patients);
                else cells[column + 1] = formatNumber(summary.groupStayHours[row][column] / patients);
            }
            table.printRow(cells);
        }
        table.printBorder();
    }
}

// 5.7: picks out the highest spending dataset, care type and age group, and the longest stays.
inline void showKeyObservations(const SpendingSummary& summary) {
    printHeading("5.7 KEY OBSERVATIONS");
    int topFacility = 0, longestStayFacility = 0, topCareType = 0, topGroup = 0, topAverageGroup = 0;
    for (int facility = 1; facility < NUM_DATASETS; facility++) {
        if (summary.facilityCost[facility] > summary.facilityCost[topFacility]) topFacility = facility;
        if (safeAverage(summary.facilityStayHours[facility], summary.facilityPatients[facility]) >
            safeAverage(summary.facilityStayHours[longestStayFacility], summary.facilityPatients[longestStayFacility])) {
            longestStayFacility = facility;
        }
    }
    for (int careIndex = 1; careIndex < summary.careTypes.size(); careIndex++) {
        if (summary.careTypes[careIndex].totalCost > summary.careTypes[topCareType].totalCost) topCareType = careIndex;
    }
    const int allColumn = ALL_FACILITIES_COLUMN;
    for (int group = 1; group < NUM_AGE_GROUPS; group++) {
        if (summary.groupCost[group][allColumn] > summary.groupCost[topGroup][allColumn]) topGroup = group;
        if (safeAverage(summary.groupCost[group][allColumn], summary.groupPatients[group][allColumn]) >
            safeAverage(summary.groupCost[topAverageGroup][allColumn], summary.groupPatients[topAverageGroup][allColumn])) {
            topAverageGroup = group;
        }
    }

    cout << "  - Highest total expenditure dataset : " << FACILITIES[topFacility].name << " (" << FACILITIES[topFacility].type
         << "), RM " << formatNumber(summary.facilityCost[topFacility]) << " = "
         << formatPercent(summary.facilityCost[topFacility], summary.totalCost) << " of all billing\n";
    cout << "  - Longest average visit duration    : " << FACILITIES[longestStayFacility].name << ", "
         << formatNumber(safeAverage(summary.facilityStayHours[longestStayFacility], summary.facilityPatients[longestStayFacility]))
         << " hours per visit\n";
    if (summary.careTypes.size() > 0) {
        cout << "  - Most expensive care type overall  : " << summary.careTypes[topCareType].name << ", RM "
             << formatNumber(summary.careTypes[topCareType].totalCost) << "\n";
    }
    cout << "  - Highest spending age group        : " << AGE_GROUPS[topGroup].range << " (" << AGE_GROUPS[topGroup].label
         << "), RM " << formatNumber(summary.groupCost[topGroup][allColumn]) << "\n";
    cout << "  - Highest average cost per patient  : " << AGE_GROUPS[topAverageGroup].range << " ("
         << AGE_GROUPS[topAverageGroup].label << "), RM "
         << formatNumber(safeAverage(summary.groupCost[topAverageGroup][allColumn], summary.groupPatients[topAverageGroup][allColumn])) << "\n";
}

// Task 5: collects the spending totals and prints all expenditure and service analysis tables.
inline void runExpenditureAnalysis(const PatientList& allPatients) {
    printTitle("TASK 5: HEALTHCARE EXPENDITURE & SERVICE ANALYSIS");
    SpendingSummary* summary = new SpendingSummary();   // on the heap: the struct is large
    collectSpending(allPatients, *summary);
    showFacilityTotals(*summary);
    showCareTypeTotals(*summary);
    showCareTypeByFacility(*summary);
    showAgeGroupByFacility(*summary);
    showKeyObservations(*summary);
    delete summary;
}
