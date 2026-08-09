# Mini DBMS in C

A lightweight SQL-like **Database Management System implemented from scratch in C** to understand the internal working of a DBMS.

The project was developed incrementally, beginning with CSV-based data storage and basic `SELECT` queries, and gradually evolving into a functional query processor supporting parsing, validation, filtering, sorting, aggregation, joins, grouping, duplicate elimination, and SQL-style result formatting.

---

## 🎯 Overview

The objective of this project is to **mimic the internal working of a DBMS using C**, without relying on an external database engine.

Instead of passing SQL queries to MySQL, PostgreSQL, SQLite, or another database system, this project implements the major stages of query processing internally.

```text
SQL Query
    │
    ▼
Tokenizer
    │
    ▼
Parser
    │
    ▼
Query Validation
    │
    ▼
Query Execution
    │
    ▼
Result Formatting
    │
    ▼
SQL-like Output
```

---

## 📋 Assignment Requirements

The project was designed to satisfy the following requirements:

* Read SQL queries from the user
* Store data in a minimum of two files
* Parse SQL queries
* Perform error and exception handling
* Support `SELECT` queries
* Support filtering
* Support aggregation
* Support `JOIN`
* Support grouping
* Display results in an SQL-like format

---

## 🛠️ Technologies

| Component    | Technology              |
| ------------ | ----------------------- |
| Language     | C                       |
| Data Storage | CSV                     |
| Compiler     | GCC / Clang             |
| Platform     | macOS / Linux / Windows |
| Libraries    | Standard C Library      |

CSV is used as the underlying storage layer because it provides a simple, human-readable representation of tabular data that can also be opened using spreadsheet applications such as Microsoft Excel.

---

## 🗄️ Database

The current database contains two related tables:

```text
student.csv
department.csv
```

### Student Dataset

The project currently contains **50 student records**.

```csv
ID,Name,Age,Marks,DeptID
1,Akshay,20,91,101
2,Ravi,21,80,102
3,Anu,19,95,101
...
50,Keerthi,20,83,110
```

The dataset intentionally contains repeated ages, marks, and department IDs so that operations such as `DISTINCT`, `GROUP BY`, filtering, and aggregation can be properly demonstrated.

### Department Dataset

The project currently contains **10 departments**:

```csv
DeptID,Department
101,CSE
102,ECE
103,MECH
104,EEE
105,CIVIL
106,IT
107,AIDS
108,AIML
109,CHEM
110,BIO
```

The relationship between the two tables is:

```text
student.DeptID = department.DeptID
```

This relationship is used by the `JOIN` implementation.

---

# 🚀 Development Journey

The project was developed incrementally, with each feature being implemented and tested before moving to the next stage.

```text
CSV Data
   ↓
Basic SELECT
   ↓
Column Projection
   ↓
WHERE Filtering
   ↓
ORDER BY
   ↓
Aggregate Functions
   ↓
JOIN
   ↓
GROUP BY
   ↓
Error Handling
   ↓
DISTINCT
   ↓
Complete Mini DBMS
```

---

## 1️⃣ CSV Data Loading

The first stage was reading the database files and storing their contents in C structures.

For example:

```c
typedef struct
{
    int id;
    char name[50];
    int age;
    int marks;
    int deptID;
} Student;
```

A separate structure is used for department records.

This established the basic data storage and loading layer.

---

## 2️⃣ SQL Command Interface

The project was then extended with an interactive SQL-style command prompt:

```text
Mini DBMS Started

SQL>
```

Users can enter queries directly into the terminal.

Example:

```sql
SELECT * FROM student;
```

The query is processed dynamically rather than being hardcoded.

---

## 3️⃣ Tokenization

The SQL input is first divided into individual tokens.

For example:

```sql
SELECT * FROM student;
```

becomes:

```text
SELECT
*
FROM
student
```

This represents the lexical analysis stage.

SQL input is normalized so that commands can be interpreted independently of their letter case.

Therefore:

```sql
SELECT * FROM student;
```

```sql
select * from student;
```

and:

```sql
SeLeCt * FrOm student;
```

can be processed consistently.

---

## 4️⃣ SQL Parsing

After tokenization, the tokens are converted into a structured query representation.

For example:

```sql
SELECT * FROM student;
```

is represented as:

```text
Command : SELECT

Columns :
*

Table : STUDENT
```

The parser was progressively extended to understand:

* `SELECT`
* Column lists
* `FROM`
* `WHERE`
* `ORDER BY`
* Aggregate functions
* `JOIN`
* `GROUP BY`
* `DISTINCT`

The parsed representation is then passed to the validation and execution stages.

---

# 5️⃣ SELECT and Projection

The first query execution feature was basic `SELECT`.

### Select all columns

```sql
SELECT * FROM student;
```

Result:

```text
+----+--------+-----+-------+--------+
| ID | Name   | Age | Marks | DeptID |
+----+--------+-----+-------+--------+
| 1  | Akshay | 20  | 91    | 101    |
| 2  | Ravi   | 21  | 80    | 102    |
| 3  | Anu    | 19  | 95    | 101    |
| 4  | Kiran  | 20  | 70    | 103    |
| 5  | Raj    | 21  | 88    | 102    |
+----+--------+-----+-------+--------+
```

The implementation was later tested with the larger 50-record dataset.

### Select specific columns

```sql
SELECT Name FROM student;
```

Result:

```text
+--------+
| NAME   |
+--------+
| Akshay |
| Ravi   |
| Anu    |
| Kiran  |
| Raj    |
+--------+
```

This introduced relational **projection**, where only requested columns are returned.

---

# 6️⃣ WHERE Filtering

The query engine was extended to support row filtering.

Example:

```sql
SELECT Name FROM student WHERE Marks > 80;
```

Result:

```text
+--------+
| NAME   |
+--------+
| Akshay |
| Anu    |
| Raj    |
+--------+
```

The execution engine evaluates the specified condition for each row and returns only matching records.

---

# 7️⃣ ORDER BY

Sorting was then implemented.

Supported forms include:

```sql
SELECT * FROM student ORDER BY Marks ASC;
```

and:

```sql
SELECT * FROM student ORDER BY Marks DESC;
```

Example:

```sql
SELECT * FROM student ORDER BY Marks DESC;
```

The result is sorted according to the requested column and ordering direction.

---

# 8️⃣ Aggregate Functions

The project was extended to support:

```text
COUNT()
SUM()
AVG()
MIN()
MAX()
```

### COUNT

```sql
SELECT COUNT(*) FROM student;
```

With the current dataset:

```text
+-------+
| COUNT |
+-------+
| 50    |
+-------+
```

### MAX

```sql
SELECT MAX(Marks) FROM student;
```

### MIN

```sql
SELECT MIN(Marks) FROM student;
```

### SUM

```sql
SELECT SUM(Marks) FROM student;
```

### AVG

```sql
SELECT AVG(Marks) FROM student;
```

Aggregate processing converts a set of rows into a calculated result and forms the basis for the later `GROUP BY` implementation.

---

# 9️⃣ JOIN

The next major milestone was implementing table joins.

Example:

```sql
SELECT Name,Department
FROM student
JOIN department
ON student.DeptID=department.DeptID;
```

The implementation uses a nested-loop join:

```text
For each student
      │
      ▼
Compare with each department
      │
      ▼
DeptID matches?
      │
      ├── No  → Continue
      │
      └── Yes
           │
           ▼
      Create joined row
```

Example result:

```text
+--------+-------------+
| NAME   | DEPARTMENT  |
+--------+-------------+
| Akshay | CSE         |
| Ravi   | ECE         |
| Anu    | CSE         |
| Kiran  | MECH        |
| Raj    | ECE         |
+--------+-------------+
```

With the expanded dataset, the same `JOIN` operation can be tested across all 50 students and 10 departments.

---

# 🔟 GROUP BY

After implementing aggregation, `GROUP BY` was added.

Example:

```sql
SELECT DeptID, COUNT(*)
FROM student
GROUP BY DeptID;
```

With the current dataset, each department contains multiple students, allowing meaningful grouping and aggregation tests.

The execution engine divides rows into groups based on the selected column and applies the aggregate function independently to each group.

---

# 1️⃣1️⃣ Error Handling and Validation

A DBMS must handle invalid queries without terminating the program unexpectedly.

The project therefore includes a validation stage before query execution.

Examples include:

### Invalid SQL keyword

```sql
SELCT * FROM student;
```

```text
ERROR: Unknown SQL keyword.
```

### Invalid table

```sql
SELECT * FROM employee;
```

```text
ERROR: Table 'EMPLOYEE' does not exist.
```

### Invalid column

```sql
SELECT Salary FROM student;
```

```text
ERROR: Column 'SALARY' does not exist.
```

### Missing keyword

```sql
SELECT Name student;
```

```text
ERROR: Expected keyword FROM.
```

### Invalid aggregate

```sql
SELECT AVG(Name) FROM student;
```

```text
ERROR: AVG() requires a numeric column.
```

### Invalid GROUP BY column

```sql
SELECT COUNT(*) FROM student GROUP BY Salary;
```

```text
ERROR: GROUP BY column 'SALARY' not found.
```

Validation ensures that malformed or unsupported queries are rejected before reaching the execution stage.

---

# 1️⃣2️⃣ DISTINCT

The final feature added was `DISTINCT`.

Example:

```sql
SELECT DISTINCT Age FROM student;
```

With the expanded dataset, duplicate ages are intentionally present.

The query returns each unique value only once.

Example:

```text
+-----+
| AGE |
+-----+
| 20  |
| 21  |
| 19  |
| 22  |
+-----+
```

The same concept can be applied to other selected columns.

---

# 🧩 Query Processing Architecture

The final system follows a structured query-processing pipeline:

```text
                         SQL QUERY
                             │
                             ▼
                      ┌────────────┐
                      │  Tokenizer │
                      └─────┬──────┘
                            │
                            ▼
                      ┌────────────┐
                      │   Parser   │
                      └─────┬──────┘
                            │
                            ▼
                   ┌──────────────────┐
                   │ Query Structure  │
                   └────────┬─────────┘
                            │
                            ▼
                   ┌──────────────────┐
                   │ Query Validation │
                   └────────┬─────────┘
                            │
                            ▼
                   ┌──────────────────┐
                   │ Execution Engine │
                   └────────┬─────────┘
                            │
          ┌─────────────────┼─────────────────┐
          │                 │                 │
          ▼                 ▼                 ▼
       WHERE            ORDER BY            JOIN
          │                 │                 │
          └─────────────────┼─────────────────┘
                            │
                            ▼
                    GROUP BY / Aggregate
                            │
                            ▼
                     DISTINCT / SELECT
                            │
                            ▼
                    Result Formatting
                            │
                            ▼
                       SQL Output
```

---

# 📊 Supported SQL Features

| Feature           | Status |
| ----------------- | :----: |
| SQL Input         |    ✅   |
| CSV Data Storage  |    ✅   |
| Tokenization      |    ✅   |
| SQL Parsing       |    ✅   |
| Query Validation  |    ✅   |
| `SELECT *`        |    ✅   |
| Column Projection |    ✅   |
| `WHERE`           |    ✅   |
| `ORDER BY`        |    ✅   |
| `ASC / DESC`      |    ✅   |
| `COUNT()`         |    ✅   |
| `SUM()`           |    ✅   |
| `AVG()`           |    ✅   |
| `MIN()`           |    ✅   |
| `MAX()`           |    ✅   |
| `JOIN`            |    ✅   |
| `GROUP BY`        |    ✅   |
| `DISTINCT`        |    ✅   |
| SQL-style Output  |    ✅   |
| Error Handling    |    ✅   |

---

# 🧪 Example Queries

### Basic SELECT

```sql
SELECT * FROM student;
```

### Projection

```sql
SELECT Name, Marks FROM student;
```

### Filtering

```sql
SELECT Name FROM student WHERE Marks > 80;
```

### Sorting

```sql
SELECT * FROM student ORDER BY Marks DESC;
```

### Aggregation

```sql
SELECT AVG(Marks) FROM student;
```

### GROUP BY

```sql
SELECT DeptID, COUNT(*)
FROM student
GROUP BY DeptID;
```

### JOIN

```sql
SELECT Name,Department
FROM student
JOIN department
ON student.DeptID=department.DeptID;
```

### DISTINCT

```sql
SELECT DISTINCT Age FROM student;
```

---

# 📁 Project Structure

```text
miniDBMS/
│
├── main.c
├── student.csv
├── department.csv
└── README.md
```

---

# ▶️ How to Run

Clone the repository:

```bash
git clone <repository-url>
cd miniDBMS
```

Compile:

```bash
gcc main.c -o main
```

Run:

```bash
./main
```

The program starts an interactive SQL prompt:

```text
Mini DBMS Started

SQL>
```

Enter any supported query to execute it.

---

# 📚 What This Project Demonstrates

This project provides a practical implementation of fundamental DBMS concepts:

* Lexical analysis and tokenization
* SQL parsing
* Query representation
* Query validation
* File-based data storage
* Projection
* Selection and filtering
* Sorting
* Aggregation
* Nested-loop joins
* Grouping
* Duplicate elimination
* Result formatting
* Error handling

The goal was not to reproduce the complexity or performance of production database systems, but to understand **how SQL queries are processed internally by implementing the major components from scratch in C**.

---

# 📈 Dataset Design

The dataset was expanded from the initial small test dataset to provide more meaningful test cases.

### Current size

```text
Students       : 50
Departments    : 10
```

The dataset contains:

* Multiple students per department
* Repeated ages
* Repeated marks
* Multiple department relationships
* Different mark ranges
* Students distributed across all departments

This allows the query engine to be tested with larger inputs for:

```text
WHERE
ORDER BY
COUNT
SUM
AVG
MIN
MAX
JOIN
GROUP BY
DISTINCT
```

---

# ⚠️ Limitations

This is an educational implementation and therefore has a deliberately limited scope.

Current limitations include:

* CSV-based storage
* In-memory query processing
* Fixed-size C data structures
* Limited SQL grammar
* Limited JOIN functionality
* No indexing
* No query optimizer
* No transaction management
* No concurrency control
* No buffer manager
* No disk-page management

These limitations leave room for future extensions.

---

# 🚀 Future Improvements

Possible extensions include:

* `AND` / `OR` conditions
* Additional comparison operators
* Multiple JOINs
* Multiple `GROUP BY` columns
* `HAVING`
* `LIMIT`
* `OFFSET`
* `INNER JOIN`
* `LEFT JOIN`
* Query optimization
* Hash-based JOIN
* Indexing
* B-Tree indexes
* Disk-based storage
* Transaction management
* Concurrency control
* Modular `.c` / `.h` architecture

---

# 👨‍💻 Author

**Akshay Rao**
B.Tech — Computer Science & Engineering
NIT Puducherry

---

## 🎯 Final Note

This project began with a simple objective:

> **Understand what happens inside a DBMS when an SQL query is executed.**

It started with CSV files and basic `SELECT` queries and was progressively expanded into a functional SQL-like query processor.

The development journey was:

```text
CSV Storage
     ↓
SQL Input
     ↓
Tokenization
     ↓
Parsing
     ↓
Validation
     ↓
Query Execution
     ↓
Filtering / Sorting / Aggregation / JOIN / GROUP BY
     ↓
DISTINCT
     ↓
SQL-style Result
```

The final implementation demonstrates how the different stages of query processing work together to transform a user-written SQL query into a structured result.

**Built from scratch in C to understand DBMS internals.**
