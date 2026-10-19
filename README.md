# Campus Lost & Found Management System

A menu-driven C11 DSA mini-project for recording campus lost and found items, finding rule-based possible matches, and tracking claims. It uses a self-written binary search tree, linked FIFO queue, and linked list. It has no external libraries, database, or file persistence; records live until the program exits.

## Features

- Report lost and found items with unique, user-entered positive IDs.
- Search by ID and list items in ID order.
- Suggest lost/found matches using category, location, description keywords, and date proximity.
- Register claims for compatible, open lost/found pairs; block missing IDs, wrong item types, claimed items, and duplicate pairs (including historical pairs).
- Keep pending claims in a linked FIFO queue; view in registration order and resolve a selected claim as approved or rejected.
- Append resolved claims to a linked history list. Approval marks both items claimed; rejection leaves them open.
- View item/claim statistics and release allocated memory on exit.
- Validate numeric choices, required text fields, input length, and real calendar dates.

## Requirements

- GCC or another C11 compiler and POSIX-compatible shell
- macOS, Linux, or another environment with a standard C toolchain

## Compile and run

From the project root:

```sh
make
./lost_found
```

The equivalent strict compile command is:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror main.c model.c input.c matching.c -o lost_found
```

To remove the compiled executable:

```sh
make clean
```

## Test

```sh
make test
```

The shell test script compiles with strict warnings and drives the real console program through successful and unsuccessful searches, empty lists/queues, invalid menu input, duplicate IDs, invalid dates, item-pair checks, matching, duplicate claims, claim approval/rejection, status updates, statistics, long input, and clean exit. See [TEST_CASES.md](TEST_CASES.md) for scenarios and sample I/O.

The application and test suite require no third-party packages.

## Menu

1. Report a Lost Item
2. Report a Found Item
3. Search Items by ID
4. Display All Items
5. Find Possible Matches
6. Register a Claim
7. View Pending Claims
8. Resolve a Claim
9. View Resolved Claims
10. Display Statistics
11. Exit

Dates use `YYYY-MM-DD` and must be valid calendar dates. Item IDs are entered by the user and must be unique positive integers.

## Data structures and algorithms

| Structure | Role | Key operations |
|---|---|---|
| Binary search tree (`ItemNode`) | Index item records by ID | Search and insert: average `O(log n)`, worst-case `O(n)`; in-order display `O(n)` |
| Linked FIFO queue (`ClaimQueue`) | Hold pending claims in registration order | Enqueue at rear `O(1)`; view `O(q)`; finding/removing a selected claim `O(q)` |
| Singly linked list (`ClaimHistory`) | Preserve resolved-claim history in resolution order | Append at tail `O(1)`; view/free `O(r)` |

The application does not use arrays as substitutes for these required structures. Fixed-size character arrays are used only for bounded text fields inside records.

### Match score

Each open lost report is compared with every open found report:

- Same category (case-insensitive): **40 points**
- At least one common non-trivial location word: **25 points**
- Common non-trivial description words: **10 points each, at most 30**
- Report dates no more than 7 calendar days apart: **5 points**

A pair is printed as a possible match at **20 points or above**. Common short words such as "the" and "and" are ignored; keyword tokens are compared without regard to letter case. Scores and reasons are printed. This is deterministic rule-based matching, not AI or machine learning. The score is an explanation aid, not proof of ownership.

For `n` items, the straightforward pair search is `O(n^2 * k)` where `k` is tokenization/comparison work for the text fields; the text lengths are bounded. A BST's performance depends on insertion order; it is not self-balancing.

## Project layout

```text
.
├── main.c                 # menu, item/claim workflows and statistics
├── model.c / model.h      # BST, queue, resolved history, display and cleanup
├── input.c / input.h      # bounded input and date/integer validation
├── matching.c / matching.h # rule-based match scoring and display
├── Makefile
├── .gitignore
├── tests/test_workflows.sh
├── README.md
├── TEST_CASES.md
├── VIVA.md
├── report/
│   ├── project_report.md
│   └── project_report.pdf
└── presentation/
    ├── slides.md
    └── project_presentation.pptx
```

The PDF and PPTX exports are included alongside their editable Markdown sources. To update an export after editing its source, use a Markdown-capable document or presentation editor to export the corresponding PDF or PPTX.

## Limitations and future scope

- Data is in memory and is lost when the application exits.
- The BST is not balanced, so sorted IDs can produce linear-time operations.
- Matching is based on exact category text and shared words; synonyms, spelling variants, and context are not understood.
- Claims are adjudicated by a human operator; the program does not authenticate claimants.
- Future extensions could add file persistence, a self-balancing tree, configurable categories, and a carefully evaluated similarity method.

## GitHub publishing

No remote repository was supplied or published as part of this work. To publish it yourself:

1. Create an empty repository on GitHub and copy its HTTPS or SSH URL.
2. From this folder run:

   ```sh
   git init
   git add .
   git commit -m "Add campus lost and found DSA project"
   git branch -M main
   git remote add origin <your-repository-url>
   git push -u origin main
   ```

3. Replace the pending-link note in [report/project_report.md](report/project_report.md) with the repository URL only after the push succeeds.

**Repository link:** Not yet published; add the verified URL after publishing.

## Deliverables

- [Project report (PDF)](report/project_report.pdf) and [editable report (Markdown)](report/project_report.md)
- [Presentation (PPTX)](presentation/project_presentation.pptx) and [editable slide outline (Markdown)](presentation/slides.md)
- [Viva questions and answers](VIVA.md)
- [Test cases and sample I/O](TEST_CASES.md)
