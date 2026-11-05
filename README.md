# Unix Processes and IPC in C

Two command-line C programs for the same task: a registry of vineyard plots and the damage they have suffered. The first version is a file-based CRUD tool. The second extends it to coordinate child processes with `fork`, unnamed pipes and POSIX signals.

Built as a university project at ELTE Faculty of Informatics (Operating Systems, spring semester 2025/26). Both assignments were accepted.

> The program's menus and messages are in Hungarian, written without accents.

## The task

Each record describes one vineyard plot: an auto-incremented ID, growing region, plot name, grape variety, area (in square fathoms, *négyszögöl*) and the percentage of the crop that was damaged. Records are stored in `szoleszet.txt`, one per line, with `|` between fields:

```
4|Eger|Nagy-Eged|Kekfrankos|600|25
```

## Assignments

### 01: Vineyard registry (`01-vineyard-registry/`)

An interactive, menu-driven program that manages the records in the text file.

- Add a record. The next ID is the highest existing ID plus one.
- List all records in a formatted table.
- Filter records by region or grape variety (substring match).
- Modify or delete a record by ID.

**Unix and C mechanisms used**

- Buffered file I/O from the C standard library (`fopen`, `fgets`, `fprintf`, `fclose`), with the file opened in append mode for new records.
- Rewrite through a temporary file: modify and delete copy every record to `temp.txt`, then `remove()` the original and `rename()` the temporary file into its place. The temporary file is discarded if the ID is not found.
- Text parsing with `strtok`, `strcspn` and `atoi`, plus input-buffer handling around `scanf` and `fgets`.

### 02: Processes, signals and pipes (`02-vineyard-processes-signals-pipes/`)

Builds on assignment 01 and adds input validation (positive area, damage between 0 and 100). Menu option 6 runs a "pest control" workflow across three processes:

1. The parent reads all records and splits them by a damage threshold (`THRESHOLD`, 30%). Plots at or above the threshold go on the destruction list, and the rest on the chemical-treatment list.
2. It creates two pipes and forks two children: a destruction worker and a treatment worker.
3. Each child sends a "ready" signal to the parent (`SIGUSR1` from the destruction worker, `SIGUSR2` from the treatment worker).
4. When both children are ready, the parent writes the record count and then the `Record` structs into each child's pipe and closes the write end.
5. Each child reads its records from the pipe, prints its instructions, sends the same signal again to report that it is done, and exits.
6. The parent waits for both "done" signals, reaps the children with `waitpid`, deletes the destroyed plots from the data file and prints a summary.

**Unix mechanisms used**

- `fork()` to create two worker processes from one parent.
- `pipe()`, `read()`, `write()` and `close()`: two unnamed pipes carrying binary `Record` structs one way, from parent to child. Each process closes the pipe ends it does not use.
- Signals:
  - `kill()` sends `SIGUSR1` and `SIGUSR2` from the children to the parent.
  - `signal()` installs handlers that increment `volatile sig_atomic_t` counters.
- Race-free waiting with `sigprocmask()` and `sigsuspend()`. Both signals are blocked before the `fork`, so a child cannot signal before the parent is ready to wait. The parent then waits with `sigsuspend`, which unblocks the signals and sleeps in one atomic step. Afterwards the original signal mask is restored.
- `waitpid()` to reap both children so that no zombie processes remain.
- The same temporary-file-and-`rename()` technique as assignment 01 removes the destroyed records.

## Build and run

Requirements: a POSIX system (Linux or macOS) with `gcc` or `clang` and `make`.

```bash
git clone https://github.com/toroktomi/unix-process-ipc-c.git
cd unix-process-ipc-c
make          # builds both programs
make run1     # runs assignment 01 (from its own folder)
make run2     # runs assignment 02 (from its own folder)
make clean
```

You can also build one program by hand:

```bash
cd 02-vineyard-processes-signals-pipes
gcc -Wall -o hegybiro hegybiro.c
./hegybiro
```

The programs read and write `szoleszet.txt` in the current working directory. Menu option 6 in assignment 02 permanently removes plots at or above the threshold from that file. Restore the sample data with `git checkout -- '*/szoleszet.txt'`.

## Project structure

```
Makefile
01-vineyard-registry/
    hegybiro.c            File-based CRUD program
    szoleszet.txt         Sample data
02-vineyard-processes-signals-pipes/
    hegybiro.c            CRUD program plus fork/pipe/signal workflow
    szoleszet.txt         Sample data
```

## What I learned / key challenges

- Avoiding the lost-signal race between `fork` and waiting: the signals have to be blocked before the children start and then waited for with `sigsuspend`, not `pause`.
- Managing pipe file descriptors in several processes: each process closes the ends it does not use, and the receiver reads a count followed by fixed-size structs.
- Using a two-phase handshake (ready, then done) to synchronize a parent with two children using only `SIGUSR1` and `SIGUSR2`.
- Updating a text file safely by writing a temporary copy and renaming it over the original.

## Author

Török Tamás
