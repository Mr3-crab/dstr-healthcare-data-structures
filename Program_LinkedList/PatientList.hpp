// PatientList.hpp - self-built singly linked lists that store the patients and search results.
#pragma once

#include "Common.hpp"

// One node of the patient list: the record and a pointer to the next node.
struct PatientNode {
    Patient patient;
    PatientNode* next;
};

// Singly linked list of patients. Each record lives in its own node on the heap.
// - append(): O(1) because the list keeps a pointer to its last node (tail).
// - No direct access by position: reaching the i-th record means following
//   'next' pointers from the head, which is O(n).
class PatientList {
public:
    PatientList() {}

    // Deep copy: creates a new node for every record of the other list.
    PatientList(const PatientList& other) {
        for (const PatientNode* node = other.head; node; node = node->next) append(node->patient);
    }

    PatientList& operator=(const PatientList&) = delete;

    // Deletes every node.
    ~PatientList() {
        while (head) {
            PatientNode* nextNode = head->next;
            delete head;
            head = nextNode;
        }
    }

    // Creates a node for the record and links it after the tail in O(1).
    void append(const Patient& patient) {
        PatientNode* newNode = new PatientNode{ patient, nullptr };
        if (tail) tail->next = newNode;
        else head = newNode;
        tail = newNode;
        count++;
    }

    // Used after a sort has relinked the same nodes into a new order:
    // stores the new first node and walks to the end to find the new tail.
    void setHead(PatientNode* newHead) {
        head = tail = newHead;
        while (tail && tail->next) tail = tail->next;
    }

    PatientNode* getHead() const { return head; }
    int size() const { return count; }

    // Memory used by the list object and all of its nodes, in bytes.
    size_t memoryBytes() const { return sizeof(*this) + count * sizeof(PatientNode); }

private:
    PatientNode* head = nullptr;
    PatientNode* tail = nullptr;
    int count = 0;
};

// One node of a search result list: a pointer to a matching patient (the record is not copied).
struct ResultNode {
    const Patient* patient;
    ResultNode* next;
};

// Singly linked list of pointers to the patients found by a search.
class ResultList {
public:
    ResultList() {}
    ResultList(const ResultList&) = delete;
    ResultList& operator=(const ResultList&) = delete;

    // Deletes every result node (the patients themselves are not deleted).
    ~ResultList() {
        while (head) {
            ResultNode* nextNode = head->next;
            delete head;
            head = nextNode;
        }
    }

    // Links a node pointing to a matching patient after the tail in O(1).
    void append(const Patient* patient) {
        ResultNode* newNode = new ResultNode{ patient, nullptr };
        if (tail) tail->next = newNode;
        else head = newNode;
        tail = newNode;
        count++;
    }

    const ResultNode* getHead() const { return head; }
    int size() const { return count; }

    // Memory used by the result list in bytes.
    size_t memoryBytes() const { return sizeof(*this) + count * sizeof(ResultNode); }

private:
    ResultNode* head = nullptr;
    ResultNode* tail = nullptr;
    int count = 0;
};
