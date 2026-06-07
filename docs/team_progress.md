# Group-F Progress Tracker

## C-DAC MPPLAB Internship

### Sub-Project-06: Multi-Threaded Telecom Data Extraction Server

---

## Team Members

| Member  | Module                      | Status      |
| ------- | --------------------------- | ----------- |
| Bhargav | Shared Buffer + Integration | In Progress |
| Samay   | Producer Module             | Not Started |
| Ajay    | Consumer Module             | Not Started |
| Akhila  | Documentation               | In Progress |

---

# Progress Log

---

## 2026-06-07

### Bhargav

**Completed**

* Created GitHub repository
* Defined DataUnit structure
* Implemented SharedBuffer
* Implemented enqueue()
* Implemented dequeue()
* Added pthread RW lock synchronization
* Added unit tests
* Added Makefile
* Added project documentation

**Current Task**

* Buffer API review
* Integration planning

**Blocked By**

* None

---

### Samay

**Completed**

* None

**Current Task**

* Producer design

**Blocked By**

* Waiting for Shared Buffer API

---

### Ajay

**Completed**

* None

**Current Task**

* Consumer design

**Blocked By**

* Waiting for Shared Buffer API

---

### Akhila

**Completed**

* Repository documentation structure

**Current Task**

* Architecture diagrams

**Blocked By**

* None

---

# Pending Tasks

## Shared Buffer

* [x] API design
* [x] Implementation
* [x] Testing

## Producer

* [ ] Header
* [ ] Implementation
* [ ] Testing

## Consumer

* [ ] Header
* [ ] Implementation
* [ ] Testing

## Integration

* [ ] Media Player
* [ ] End-to-End Testing

---

# Notes

* All code must use shared_buffer.h.
* Direct buffer access is prohibited.
* Use enqueue() and dequeue() APIs only.
* Update this file after every major task completion.
