// CareTally.hpp - per care type totals (care types are read from the data).
#pragma once

#include "PatientArray.hpp"

// Totals for one care type.
struct CareTypeStats {
    string name;
    int patientCount = 0;
    double totalCost = 0;
    double totalStayHours = 0;
};

// Small fixed-size table of care type totals. Care types are not hard-coded:
// a new entry is created the first time a care type appears in the data.
class CareTally {
public:
    // Returns the position of a care type, or -1 if it is not in the table yet.
    int findIndex(const string& careType) const {
        for (int i = 0; i < typeCount; i++) {
            if (entries[i].name == careType) return i;
        }
        return -1;
    }

    // Adds one patient's figures to the totals of its care type.
    void addPatient(const Patient& patient) {
        int index = findIndex(patient.careType);
        if (index < 0) {
            if (typeCount == MAX_CARE_TYPES) return;
            index = typeCount++;
            entries[index].name = patient.careType;
        }
        entries[index].patientCount++;
        entries[index].totalCost += patient.totalCost;
        entries[index].totalStayHours += patient.stayHours;
    }

    int size() const { return typeCount; }
    const CareTypeStats& operator[](int index) const { return entries[index]; }

    // Insertion sort: most patients first (ties: higher cost first).
    void sortByPatientCount() {
        for (int i = 1; i < typeCount; i++) {
            CareTypeStats current = entries[i];
            int j = i - 1;
            while (j >= 0 && (entries[j].patientCount < current.patientCount ||
                              (entries[j].patientCount == current.patientCount && entries[j].totalCost < current.totalCost))) {
                entries[j + 1] = entries[j];
                j--;
            }
            entries[j + 1] = current;
        }
    }

    // Insertion sort: care types in alphabetical order.
    void sortByName() {
        for (int i = 1; i < typeCount; i++) {
            CareTypeStats current = entries[i];
            int j = i - 1;
            while (j >= 0 && entries[j].name > current.name) {
                entries[j + 1] = entries[j];
                j--;
            }
            entries[j + 1] = current;
        }
    }

    // Highest patient count of any care type.
    int getTopCount() const {
        int highest = 0;
        for (int i = 0; i < typeCount; i++) {
            if (entries[i].patientCount > highest) highest = entries[i].patientCount;
        }
        return highest;
    }

    // Most requested care type; if several tie, their names are joined with " / ".
    string getMostPreferred() const {
        int highest = getTopCount();
        if (highest == 0) return "N/A";
        string names;
        for (int i = 0; i < typeCount; i++) {
            if (entries[i].patientCount != highest) continue;
            if (!names.empty()) names += " / ";
            names += entries[i].name;
        }
        return names;
    }

    // Summary text, e.g. "Emergency (67), Inpatient (67)".
    string getSummary() const {
        string summary;
        for (int i = 0; i < typeCount; i++) {
            if (i > 0) summary += ", ";
            summary += entries[i].name + " (" + to_string(entries[i].patientCount) + ")";
        }
        return summary;
    }

private:
    CareTypeStats entries[MAX_CARE_TYPES];
    int typeCount = 0;
};

// Builds the care type totals for every patient in the array.
inline CareTally buildCareTally(const PatientArray& patients) {
    CareTally tally;
    for (int i = 0; i < patients.size(); i++) tally.addPatient(patients[i]);
    return tally;
}
