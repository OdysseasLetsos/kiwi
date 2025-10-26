# Multithreaded Data Storage Engine (KIWI)

## Description

This project involves converting an existing data storage engine (KIWI Engine) into a multithreaded application.
The goal is to improve performance through concurrent put (`write`) and get (`read`) operations using the pthreads library.
The implementation ensures mutual exclusion and proper synchronization among threads.

---

## Engine Architecture

The engine consists of two main directories:
bench/ → Terminal interface, thread creation, and management
engine/ → Internal database functionality

### Engine Structure
| File | Description |
| `db.cc / db.h` | Database management (open, close, add, get) |
| `memtable.cc / memtable.h` | Temporary in-memory data storage |
| `sst.cc / sst.h` | Disk storage management & compaction process |

---

## Operations

### Write (put)
Adds records to the database via db_add().
When the memory (`memtable`) is full, the data is written to disk (`sst`) through compaction in a separate thread.

### Read (get)
Searches for data first in memory (`memtable`) and, if not found, on disk (`sst`).

### ReadWrite
Performs mixed operations concurrently with different read/write ratios:

| Option | Read | Write |
| 1 | 50% | 50% |
| 2 | 10% | 90% |
| 3 | 60% | 40% |

---

## Multithreaded Implementation

- Uses pthreads to create separate threads for each operation (`put` / `get`).
- Shared database among all threads.
- Mutex locks are implemented to:
  - Control access to the database.
  - Prevent conflicts during simultaneous access.
  - Synchronize execution timing statistics.

---

### Reader–Writer Problem

The access policy allows:

- Multiple readers simultaneously.  
- Only one writer at a time.  
- Writers have exclusive access during write operations.

Implemented using locks in the functions:

db_add();
db_get();


---

##Code Modifications

- bench.cc

Added pthread.h and struct dataset for thread arguments.
printer() function for displaying statistics.
Threads created with pthread_create() and joined with pthread_join().
Shared use of db_open() and db_close() for all threads.

- kiwi.cc

Introduced mutexes to prevent conflicts in execution times:
pthread_mutex_lock(&mymutex);
...
pthread_mutex_unlock(&mymutex);

- db.cc

Added reader and writer locks:
pthread_mutex_t writer;
pthread_mutex_t reader;


Ensured exclusive execution for writers and simultaneous access for readers.


##How to Run

1.Compile

make all

2.Run

./kiwi-bench <operation> <number_of_records> <number_of_threads>

Examples

./kiwi-bench write 100000 4
./kiwi-bench read 50000 2
./kiwi-bench readwrite 100000 8


or readwrite, the user chooses the read/write ratio:
1 -> 50% read / 50% write
2 -> 10% read / 90% write
3 -> 60% read / 40% write


 
##Results & Observations

The multithreaded approach significantly improved performance.
Proper synchronization prevented errors such as segmentation faults and race conditions.
The Reader–Writer implementation ensures data stability and safety.

---

##Contributors
Implementation: Odysseas Letsos
Language: C / C++
Thread Library: pthreads
Environment: Linux / Unix
