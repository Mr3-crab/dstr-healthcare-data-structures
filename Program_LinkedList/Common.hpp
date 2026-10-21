// Common.hpp - shared constants, facilities, age groups and the Patient record.
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <chrono>
#include <cctype>
#include <cstdlib>

using namespace std;

const int NUM_DATASETS = 3;
const int NUM_AGE_GROUPS = 5;
const int MAX_CARE_TYPES = 16;
const int MAX_TABLE_COLUMNS = 12;
const int MAX_SAVED_RESULTS = 256;
const int SORT_RUNS = 20;          // each sort is repeated and the time averaged
const int SEARCH_RUNS = 1000;      // each search is repeated and the time averaged
const int BANNER_WIDTH = 100;
const size_t STACK_FRAME_BYTES = 64;   // rough size of one recursive call's stack frame (x64)

const char* const PROGRAM_MODE = "Linked List";
const bool IS_ARRAY_PROGRAM = false;
const char* const MY_RESULTS_FILE = "results_linkedlist.txt";
const char* const OTHER_RESULTS_FILE = "results_array.txt";
const char* const OTHER_PROGRAM_NAME = "Program_Array";

// One dataset file and the facility it belongs to.
struct Facility {
    const char* fileName;
    const char* name;
    const char* type;
};

const Facility FACILITIES[NUM_DATASETS] = {
    { "dataset1 facility_a.csv", "Facility A", "General Hospital" },
    { "dataset2 facility_b.csv", "Facility B", "University Medical Center" },
    { "dataset3_facility_c.csv", "Facility C", "Community Health Clinic" }
};

// Age range and label of one age group.
struct AgeGroup {
    int minAge;
    int maxAge;
    const char* range;
    const char* label;
};

const AgeGroup AGE_GROUPS[NUM_AGE_GROUPS] = {
    {  0,  17, "0-17",   "Pediatrics & Adolescents" },
    { 18,  25, "18-25",  "Young Adults / University Students" },
    { 26,  45, "26-45",  "Working Adults (Early Career)" },
    { 46,  60, "46-60",  "Working Adults (Late Career)" },
    { 61, 100, "61-100", "Senior Citizens / Geriatric Care" }
};

// One patient record. totalCost is calculated after loading:
// Total Medical Cost = stayHours x ratePerHour x visitsPerYear.
struct Patient {
    string id;
    int age = 0;
    string careType;
    double stayHours = 0;       // length of stay / visit duration (hours)
    double ratePerHour = 0;     // base cost per hour (RM)
    int visitsPerYear = 0;
    int facilityIndex = 0;      // index into FACILITIES
    double totalCost = 0;
};

// Returns the age group index for an age, or -1 if it is outside 0-100.
inline int getAgeGroupIndex(int age) {
    for (int group = 0; group < NUM_AGE_GROUPS; group++) {
        if (age >= AGE_GROUPS[group].minAge && age <= AGE_GROUPS[group].maxAge) return group;
    }
    return -1;
}

// Returns the age group range text, e.g. "18-25".
inline string getAgeGroupRange(int age) {
    int group = getAgeGroupIndex(age);
    return group < 0 ? "N/A" : AGE_GROUPS[group].range;
}
