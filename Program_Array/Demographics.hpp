// Demographics.hpp - Task 4: demographic & billing categorisation by age group.
#pragma once

#include "Table.hpp"
#include "CareTally.hpp"

// Totals for one age group.
struct AgeGroupStats {
    CareTally careTypes;
    int patientCount = 0;
    double totalCost = 0;
};

// Prints one age group's care type breakdown (patients, total and average cost per care type),
// followed by the group's total billing, average cost and most preferred care type.
inline void printAgeGroupBlock(int groupIndex, AgeGroupStats& stats) {
    const int lineWidth = 92;
    const AgeGroup& group = AGE_GROUPS[groupIndex];
    cout << "\nAge Group: " << group.range << " (" << group.label << ")\n" << string(lineWidth, '-') << "\n";
    cout << left << setw(20) << "Care Type" << setw(16) << "Patient Count" << setw(22) << "Total Cost (RM)"
         << "Average Cost per Patient (RM)\n" << string(lineWidth, '-') << "\n";

    if (stats.patientCount == 0) cout << "(no patients in this age group)\n";
    stats.careTypes.sortByPatientCount();
    for (int i = 0; i < stats.careTypes.size(); i++) {
        const CareTypeStats& care = stats.careTypes[i];
        cout << setw(20) << care.name << setw(16) << care.patientCount << setw(22) << formatNumber(care.totalCost)
             << formatNumber(safeAverage(care.totalCost, care.patientCount)) << "\n";
    }
    cout << right << string(lineWidth, '-') << "\n";
    cout << "Total Billing for Age Group: RM " << formatNumber(stats.totalCost) << "\n";
    cout << "Patients in Age Group: " << stats.patientCount << "   |   Average Cost per Patient: RM "
         << formatNumber(safeAverage(stats.totalCost, stats.patientCount)) << "\n";
    cout << "Most Preferred Care Type: " << stats.careTypes.getMostPreferred();
    if (stats.patientCount > 0) {
        int topCount = stats.careTypes.getTopCount();
        cout << " (" << topCount << " patients, " << formatPercent(topCount, stats.patientCount) << " of group)";
    }
    cout << "\n";
}

// Task 4: puts every patient into one of the 5 age groups and shows, per group, the most
// preferred care type, the total medical cost and the average cost per patient.
inline void runDemographicAnalysis(const PatientArray& allPatients) {
    printTitle("TASK 4: PATIENT DEMOGRAPHIC & BILLING CATEGORISATION (ALL FACILITIES)");
    cout << "  Total Medical Cost = Length of Stay x Base Cost Per Hour x Visits Per Year\n"
         << "  Average Cost       = Total Cost in Age Group / Number of Patients in Age Group\n";

    AgeGroupStats groups[NUM_AGE_GROUPS];
    CareTally allCareTypes;
    int ungroupedCount = 0, groupedCount = 0;
    double groupedCost = 0;
    for (int i = 0; i < allPatients.size(); i++) {
        const Patient& patient = allPatients[i];
        int groupIndex = getAgeGroupIndex(patient.age);
        if (groupIndex < 0) {
            ungroupedCount++;
            continue;
        }
        groups[groupIndex].patientCount++;
        groups[groupIndex].totalCost += patient.totalCost;
        groups[groupIndex].careTypes.addPatient(patient);
        allCareTypes.addPatient(patient);
        groupedCount++;
        groupedCost += patient.totalCost;
    }

    for (int group = 0; group < NUM_AGE_GROUPS; group++) printAgeGroupBlock(group, groups[group]);

    printHeading("SUMMARY: DEMOGRAPHIC & BILLING BY AGE GROUP");
    Table table;
    table.addColumn("Group", 6);
    table.addColumn("Category", 34);
    table.addColumn("Patients", 8, true);
    table.addColumn("Most Preferred Care", 22);
    table.addColumn("Total Cost (RM)", 15, true);
    table.addColumn("Avg/Patient (RM)", 16, true);
    table.printHeader();
    for (int group = 0; group < NUM_AGE_GROUPS; group++) {
        string cells[6] = {
            AGE_GROUPS[group].range, AGE_GROUPS[group].label, to_string(groups[group].patientCount),
            groups[group].careTypes.getMostPreferred(), formatNumber(groups[group].totalCost),
            formatNumber(safeAverage(groups[group].totalCost, groups[group].patientCount))
        };
        table.printRow(cells);
    }
    table.printBorder();
    string totalRow[6] = {
        "ALL", "All age groups", to_string(groupedCount), allCareTypes.getMostPreferred(),
        formatNumber(groupedCost), formatNumber(safeAverage(groupedCost, groupedCount))
    };
    table.printRow(totalRow);
    table.printBorder();
    if (ungroupedCount > 0) cout << "  Note: " << ungroupedCount << " patient(s) are outside the 0-100 age range.\n";
}
