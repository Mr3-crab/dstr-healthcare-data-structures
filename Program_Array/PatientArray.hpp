// PatientArray.hpp - self-built dynamic arrays that store the patients and search results.
#pragma once

#include "Common.hpp"

// Dynamic array of patients stored in one contiguous block of memory.
// - add(): O(1) on average; when the array is full, a block twice as large is
//   allocated and the records are copied over (an occasional O(n) step).
// - operator[]: O(1) direct access to any position by index.
class PatientArray {
public:
    PatientArray() : items(new Patient[8]), count(0), allocated(8) {}

    // Deep copy: allocates a new block and copies every record.
    PatientArray(const PatientArray& other)
        : items(new Patient[other.allocated]), count(other.count), allocated(other.allocated) {
        for (int i = 0; i < count; i++) items[i] = other.items[i];
    }

    PatientArray& operator=(const PatientArray&) = delete;

    ~PatientArray() { delete[] items; }

    // Appends a record at the end, doubling the capacity when the array is full.
    void add(const Patient& patient) {
        if (count == allocated) {
            Patient* biggerBlock = new Patient[allocated * 2];
            for (int i = 0; i < count; i++) biggerBlock[i] = items[i];
            delete[] items;
            items = biggerBlock;
            allocated *= 2;
        }
        items[count++] = patient;
    }

    Patient& operator[](int index) { return items[index]; }
    const Patient& operator[](int index) const { return items[index]; }
    int size() const { return count; }
    int capacity() const { return allocated; }

    // Memory used by the array object plus its allocated block, in bytes.
    size_t memoryBytes() const { return sizeof(*this) + allocated * sizeof(Patient); }

private:
    Patient* items;
    int count;
    int allocated;
};

// Dynamic array of positions (indexes into a PatientArray), used to hold search results.
class IndexList {
public:
    IndexList() : items(new int[8]), count(0), allocated(8) {}
    IndexList(const IndexList&) = delete;
    IndexList& operator=(const IndexList&) = delete;
    ~IndexList() { delete[] items; }

    // Appends a position, doubling the capacity when the list is full.
    void add(int position) {
        if (count == allocated) {
            int* biggerBlock = new int[allocated * 2];
            for (int i = 0; i < count; i++) biggerBlock[i] = items[i];
            delete[] items;
            items = biggerBlock;
            allocated *= 2;
        }
        items[count++] = position;
    }

    int operator[](int index) const { return items[index]; }
    int size() const { return count; }

    // Memory used by the list in bytes.
    size_t memoryBytes() const { return sizeof(*this) + allocated * sizeof(int); }

private:
    int* items;
    int count;
    int allocated;
};
