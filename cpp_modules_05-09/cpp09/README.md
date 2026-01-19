# CPP Module 09

*This project has been created as part of the 42 curriculum by viceda-s.*

## Description

CPP Module 09 focuses on **STL containers and advanced algorithms**, emphasizing practical applications of different container types and their performance characteristics. This module demonstrates how to choose appropriate containers for specific use cases and implements various algorithmic approaches. The project consists of three exercises:

- **Exercise 00 (Bitcoin Exchange)**: Using `std::map` for database lookups with date-based queries
- **Exercise 01 (RPN)**: Implementing a Reverse Polish Notation calculator using `std::stack`
- **Exercise 02 (PmergeMe)**: Comparing sorting performance between `std::vector` and `std::deque` using the Ford-Johnson algorithm

### Learning Objectives

- Understanding associative containers (`std::map`)
- Working with container adapters (`std::stack`)
- Comparing container performance characteristics
- Implementing real-world data processing applications
- File parsing and validation
- Time complexity analysis and benchmarking
- Understanding the Ford-Johnson merge-insertion sort algorithm
- Working with the Jacobsthal sequence for optimal insertion order

## Instructions

### Compilation Requirements

- **Compiler**: `c++` (or `g++`, `clang++`)
- **Standard**: C++98
- **Flags**: `-Wall -Wextra -Werror -std=c++98`

### Compilation

Each exercise has its own Makefile. Navigate to the desired exercise directory and run:

```bash
make
```

This will compile the exercise and create an executable.

#### Exercise 00 - Bitcoin Exchange
```bash
cd ex00
make
./btc input.txt
```

#### Exercise 01 - RPN
```bash
cd ex01
make
./RPN "3 4 + 2 *"
```

#### Exercise 02 - PmergeMe
```bash
cd ex02
make
./PmergeMe 3 5 9 7 4
```

### Cleaning

To remove object files:
```bash
make clean
```

To remove object files and executable:
```bash
make fclean
```

To recompile everything:
```bash
make re
```

## Project Structure

```
cpp09/
├── ex00/                    # Bitcoin Exchange
│   ├── BitcoinExchange.hpp # Class declaration
│   ├── BitcoinExchange.cpp # Class implementation
│   ├── main.cpp            # Main program
│   ├── data.csv            # Bitcoin price database
│   ├── input.txt           # Sample input file
│   └── Makefile
├── ex01/                    # RPN Calculator
│   ├── RPN.hpp             # RPN class declaration
│   ├── RPN.cpp             # RPN class implementation
│   ├── main.cpp            # Main program
│   └── Makefile
└── ex02/                    # PmergeMe sorting
    ├── PmergeMe.hpp        # PmergeMe class declaration
    ├── PmergeMe.cpp        # PmergeMe class implementation
    ├── main.cpp            # Main program
    └── Makefile
```

## Features

### Exercise 00: Bitcoin Exchange

A program that evaluates Bitcoin values on specific dates using a historical price database.

**Program Usage**:
```bash
./btc <input_file>
```

**Input File Format**:
```
date | value
2011-01-03 | 3
2011-01-09 | 1
2012-01-11 | 1
```

**Database Format** (data.csv):
```
date,exchange_rate
2009-01-02,0
2011-01-03,162.50
2011-01-09,164.00
```

**Class Interface**:
```cpp
class BitcoinExchange {
private:
    std::map<std::string, float> _database;  // Date -> Exchange rate
    
    bool isValidDate(const std::string& date) const;
    bool isValidValue(const std::string& value) const;
    float stringToFloat(const std::string& str) const;

public:
    bool loadDatabase(const std::string& filename);
    void processInput(const std::string& filename);
};
```

**Key Features**:
- Uses `std::map` for efficient date-based lookups (O(log n))
- Loads historical Bitcoin prices from CSV database
- Validates date format (YYYY-MM-DD)
- Validates value range (0 to 1000)
- Finds closest previous date if exact match doesn't exist
- Calculates: `result = value * exchange_rate`

**Date Lookup Algorithm**:
```cpp
// Uses std::map::upper_bound for efficient date search
std::map<std::string, float>::iterator it = _database.upper_bound(date);
if (it != _database.begin())
    --it;  // Get the closest date <= input date
```

**Why std::map?**
- Stores key-value pairs in sorted order
- Efficient lookup: O(log n)
- `upper_bound()` enables finding closest date
- Red-black tree implementation guarantees balance

**Error Handling**:
- Invalid date format: "Error: bad input => [date]"
- Negative number: "Error: not a positive number."
- Value > 1000: "Error: too large a number."
- Database load failure: "Error: could not load database."
- Input file error: "Error: could not open file."

**Example Output**:
```
2011-01-03 => 3 = 487.50
2011-01-09 => 1 = 164.00
Error: not a positive number.
Error: bad input => 2001-42-42
2012-01-11 => 1 = 196.45
Error: too large a number.
```

### Exercise 01: RPN (Reverse Polish Notation)

A calculator that evaluates mathematical expressions in Reverse Polish Notation using a stack.

**Program Usage**:
```bash
./RPN "expression"
```

**What is RPN?**
Reverse Polish Notation (also called postfix notation) places operators after operands:
- Infix: `(3 + 4) * 2`
- RPN: `3 4 + 2 *`

**Class Interface**:
```cpp
class RPN {
private:
    std::stack<int> _stack;
    
    bool isOperator(const std::string& token) const;
    bool isNumber(const std::string& token) const;
    void performOperation(char op);

public:
    bool evaluate(const std::string& expression);
    int getResult() const;
};
```

**Supported Operations**:
- `+` Addition
- `-` Subtraction
- `*` Multiplication
- `/` Division

**Algorithm**:
1. Parse expression token by token (space-separated)
2. If token is a number: push onto stack
3. If token is an operator:
   - Pop two operands from stack
   - Apply operation: `result = operand1 OP operand2`
   - Push result back onto stack
4. Final result: single value remaining on stack

**Why std::stack?**
- LIFO (Last-In-First-Out) perfectly suits RPN evaluation
- Only need `push()`, `pop()`, and `top()` operations
- Automatically handles operation ordering

**Examples**:

| Expression | Evaluation Steps | Result |
|------------|------------------|--------|
| `"8 9 * 9 - 9 - 9 - 4 - 1 +"` | 8,9→72 72,9→63 63,9→54 54,9→45 45,4→41 41,1→42 | 42 |
| `"7 7 * 7 -"` | 7,7→49 49,7→42 | 42 |
| `"1 2 * 2 / 2 * 2 4 - +"` | 1,2→2 2,2→1 1,2→2 2,4→-2 2,-2→0 | 0 |
| `"(1 + 1)"` | Invalid (not RPN) | Error |

**Step-by-Step Example** (`"3 4 + 2 *"`):
```
Token: "3"  → Stack: [3]
Token: "4"  → Stack: [3, 4]
Token: "+"  → Pop 4,3 → 3+4=7 → Stack: [7]
Token: "2"  → Stack: [7, 2]
Token: "*"  → Pop 2,7 → 7*2=14 → Stack: [14]
Result: 14
```

**Error Cases**:
- Division by zero
- Invalid token (not a digit 0-9 or operator)
- Insufficient operands for operation
- Multiple values left on stack after evaluation
- Empty expression

**Constraints**:
- Only single-digit numbers (0-9)
- Operations always between integers
- Result must be integer

### Exercise 02: PmergeMe

A program that sorts positive integers using the **Ford-Johnson merge-insertion sort** algorithm, comparing performance between `std::vector` and `std::deque`.

**Program Usage**:
```bash
./PmergeMe [positive integers...]
```

**Example**:
```bash
./PmergeMe 3 5 9 7 4
```

**Output**:
```
Before: 3 5 9 7 4
After:  3 4 5 7 9
Time to process a range of 5 elements with std::vector : 0.012 us
Time to process a range of 5 elements with std::deque  : 0.015 us
```

**Class Interface**:
```cpp
class PmergeMe {
private:
    std::vector<int> _vectorData;
    std::deque<int> _dequeData;
    std::vector<int> _originalData;
    double _vectorTime;
    double _dequeTime;

    // Jacobsthal sequence
    size_t jacobsthal(size_t n);
    std::vector<size_t> generateJacobsthalSequence(size_t len);

    // Vector sorting
    void fordJohnsonVector(std::vector<int>& arr);
    void insertPendingVector(std::vector<int>& mainChain, 
                             std::vector<int>& pend, 
                             const std::vector<size_t>& jacobSeq);

    // Deque sorting
    void fordJohnsonDeque(std::deque<int>& arr);
    void insertPendingDeque(std::deque<int>& mainChain, 
                           std::deque<int>& pend, 
                           const std::deque<size_t>& jacobSeq);

public:
    bool parseInput(int argc, char** argv);
    void sort();
    void displayResults() const;
};
```

**Key Features**:
- Implements Ford-Johnson algorithm (merge-insertion sort)
- Uses Jacobsthal sequence for optimal insertion order
- Benchmarks both `std::vector` and `std::deque`
- Displays timing in microseconds (µs)
- Handles large datasets (3000+ elements)
- Falls back to insertion sort for small arrays (≤16 elements)

**Ford-Johnson Algorithm Overview**:

The Ford-Johnson algorithm (also called merge-insertion sort) is one of the most comparison-efficient sorting algorithms, discovered in 1959.

**Steps**:
1. **Pair Elements**: Group array into pairs, sort each pair
2. **Recursively Sort**: Sort the larger elements of pairs
3. **Create Main Chain**: Larger elements form sorted main chain
4. **Insert Pending**: Insert smaller elements using binary search
5. **Jacobsthal Optimization**: Insert in Jacobsthal sequence order

**Why Jacobsthal Sequence?**

The Jacobsthal sequence (0, 1, 1, 3, 5, 11, 21, 43, ...) defines optimal insertion order to minimize comparisons.

Formula: `J(n) = J(n-1) + 2*J(n-2)`, with J(0)=0, J(1)=1

**Jacobsthal Insertion Order**:
- Instead of inserting pending elements sequentially [0,1,2,3,4,5,...]
- Insert in Jacobsthal order: [0,2,1,5,4,3,10,9,8,7,6,...]
- Reduces maximum comparisons needed for binary search

**Example** (sorting [5, 2, 8, 1, 9, 3]):
```
Step 1: Pair and sort
  Pairs: [(5,2), (8,1), (9,3)]
  Sorted: [(5,2), (8,1), (9,3)]

Step 2: Extract larger elements
  Main: [5, 8, 9]
  Pend: [2, 1, 3]

Step 3: Recursively sort main chain
  Main: [5, 8, 9] (already sorted)

Step 4: Insert first pending element
  Main: [2, 5, 8, 9]
  Remaining pend: [1, 3]

Step 5: Insert remaining in Jacobsthal order
  Jacobsthal indices for 2 elements: [1, 0]
  Insert pend[1]=3: [2, 3, 5, 8, 9]
  Insert pend[0]=1: [1, 2, 3, 5, 8, 9]

Result: [1, 2, 3, 5, 8, 9]
```

**Complexity**:
- **Time**: O(n log n) comparisons, but with lower constant factor
- **Space**: O(n) for auxiliary storage
- **Optimal**: Uses near-minimum number of comparisons

**Container Comparison**:

| Feature | std::vector | std::deque |
|---------|-------------|------------|
| **Storage** | Contiguous memory | Chunked blocks |
| **Random Access** | O(1) - fastest | O(1) - slightly slower |
| **Insert at End** | O(1) amortized | O(1) |
| **Insert at Front** | O(n) | O(1) |
| **Insert at Middle** | O(n) | O(n) |
| **Cache Locality** | Excellent | Good |
| **Memory Overhead** | Low | Moderate (chunk pointers) |
| **Reallocation** | Yes (grow exponentially) | No (allocate new chunks) |

**Expected Performance**:
- `std::vector` typically faster for this use case
- Contiguous memory improves cache performance
- Random access during binary search is faster
- Small overhead from occasional reallocation

## Algorithm and Data Structure Explanation

### std::map (Exercise 00)

**Internal Structure**: Red-Black Tree (self-balancing binary search tree)

**Properties**:
- Keys are always sorted
- Each node: Red or Black colored
- Root is black
- No two consecutive red nodes
- All paths from root to NULL have same number of black nodes

**Time Complexity**:
| Operation | Complexity |
|-----------|------------|
| Insert | O(log n) |
| Search | O(log n) |
| Delete | O(log n) |
| Find range | O(log n + k) where k = elements in range |

**Why Use std::map for Bitcoin Exchange?**
1. **Sorted Order**: Dates are stored chronologically
2. **Efficient Lookup**: O(log n) for finding closest date
3. **upper_bound()**: Finds first element > key (perfect for date ranges)
4. **Unique Keys**: Each date maps to one exchange rate

**Alternative Containers**:
- `std::unordered_map`: O(1) lookup but unordered (can't find closest date)
- `std::vector<pair>`: Need O(n) linear search or O(log n) binary search after sorting
- `std::multimap`: Allows duplicate keys (not needed here)

### std::stack (Exercise 01)

**Container Adapter**: Wraps underlying container (default: `std::deque`)

**LIFO Operations**:
```cpp
stack.push(x);      // Add to top
int top = stack.top();  // View top
stack.pop();        // Remove top
bool empty = stack.empty();
size_t size = stack.size();
```

**Why Stack for RPN?**
- RPN naturally uses LIFO ordering
- Most recent operands are used first
- Simple push/pop interface
- No random access needed

**RPN Evaluation Complexity**:
- **Time**: O(n) where n = number of tokens
- **Space**: O(d) where d = maximum stack depth

**Stack Usage in RPN Example** (`"5 1 2 + 4 * + 3 -"`):
```
Token | Stack State      | Operation
------|------------------|------------------
5     | [5]              | Push 5
1     | [5, 1]           | Push 1
2     | [5, 1, 2]        | Push 2
+     | [5, 3]           | Pop 2,1 → 1+2=3 → Push 3
4     | [5, 3, 4]        | Push 4
*     | [5, 12]          | Pop 4,3 → 3*4=12 → Push 12
+     | [17]             | Pop 12,5 → 5+12=17 → Push 17
3     | [17, 3]          | Push 3
-     | [14]             | Pop 3,17 → 17-3=14 → Push 14

Result: 14
```

### Ford-Johnson Algorithm (Exercise 02)

**Historical Context**:
- Invented by Lester Ford Jr. and Selmer Johnson (1959)
- Held record for fewest comparisons for decades
- Still one of the most comparison-efficient algorithms

**Algorithm Phases**:

#### Phase 1: Pairing and Initial Sorting
```cpp
// Pair consecutive elements and sort each pair
std::vector<std::pair<int, int>> pairs;
for (size_t i = 0; i + 1 < arr.size(); i += 2) {
    if (arr[i] > arr[i + 1])
        pairs.push_back(std::make_pair(arr[i], arr[i + 1]));
    else
        pairs.push_back(std::make_pair(arr[i + 1], arr[i]));
}
```

#### Phase 2: Recursive Sort
```cpp
// Extract larger elements and recursively sort them
std::vector<int> mainChain;
std::vector<int> pend;
for (auto& pair : pairs) {
    mainChain.push_back(pair.first);   // Larger element
    pend.push_back(pair.second);       // Smaller element
}
fordJohnsonVector(mainChain);  // Recursive call
```

#### Phase 3: Jacobsthal Insertion
```cpp
// Generate Jacobsthal sequence for insertion order
std::vector<size_t> jacobSeq = generateJacobsthalSequence(pend.size());

// Insert pending elements in Jacobsthal order
for (size_t idx : jacobSeq) {
    int value = pend[idx];
    auto pos = std::upper_bound(mainChain.begin(), mainChain.end(), value);
    mainChain.insert(pos, value);
}
```

**Jacobsthal Sequence Generation**:
```cpp
size_t jacobsthal(size_t n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    
    size_t prev = 0, curr = 1;
    for (size_t i = 2; i <= n; ++i) {
        size_t next = curr + 2 * prev;
        prev = curr;
        curr = next;
    }
    return curr;
}
```

**Jacobsthal Numbers**:
```
J(0) = 0
J(1) = 1
J(2) = 1
J(3) = 3
J(4) = 5
J(5) = 11
J(6) = 21
J(7) = 43
J(8) = 85
```

**Insertion Order Example** (10 elements):
```
Jacobsthal sequence: J(3)=3, J(4)=5, J(5)=11
Insertion indices: [2,1,0, 4,3, 9,8,7,6,5]
                    └─3─┘ └2┘  └────5────┘
```

**Why This Order?**
- Binary search tree is most balanced when inserting in specific patterns
- Jacobsthal order minimizes worst-case comparisons
- Each group fits perfectly in available comparison slots

**Optimization: Small Array Threshold**:
```cpp
if (arr.size() <= 16) {
    // Use insertion sort for small arrays
    for (size_t i = 1; i < arr.size(); ++i) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = key;
    }
    return;
}
```

**Comparison Count**:
- Ford-Johnson: ~n log₂ n - 1.4n comparisons
- Merge Sort: ~n log₂ n comparisons
- Quick Sort: ~1.39n log₂ n average
- Insertion Sort: ~n²/4 average

### Performance Benchmarking

**Timing Method** (using `clock_gettime`):
```cpp
struct timespec start, end;

clock_gettime(CLOCK_MONOTONIC, &start);
fordJohnsonVector(_vectorData);
clock_gettime(CLOCK_MONOTONIC, &end);

_vectorTime = (end.tv_sec - start.tv_sec) * 1e6 
            + (end.tv_nsec - start.tv_nsec) / 1000.0;
```

**Why CLOCK_MONOTONIC?**
- Not affected by system time adjustments
- Guaranteed to be monotonically increasing
- Better for measuring elapsed time

**Microseconds (µs) vs Other Units**:
- 1 second = 1,000,000 µs
- 1 millisecond = 1,000 µs
- Appropriate granularity for sorting operations

**Factors Affecting Performance**:
1. **Container overhead**: vector vs deque memory layout
2. **Cache locality**: contiguous memory vs chunked
3. **Memory allocation**: vector reallocation vs deque chunking
4. **Compiler optimizations**: -O2, -O3 flags
5. **CPU architecture**: cache size, prefetching
6. **Input size**: small vs large datasets
7. **Input distribution**: already sorted, reverse sorted, random

### Container Performance Analysis

**Memory Layout**:

```
std::vector:
[1|2|3|4|5|6|7|8|9|10] (contiguous)
 ↑
 Single pointer

std::deque:
Chunk 1:  [1|2|3|4]
Chunk 2:  [5|6|7|8]
Chunk 3:  [9|10| | ]
   ↑         ↑       ↑
   └─────────┴───────┘
   Map of pointers
```

**Cache Performance**:
- **Vector**: Sequential access = optimal cache usage
- **Deque**: May cross chunk boundaries = cache misses

**When to Use Each**:

**std::vector**:
- Need fast random access
- Frequent iteration over elements
- Memory efficiency important
- Insert/delete mainly at end

**std::deque**:
- Need fast insert/delete at both ends
- Don't want memory reallocation
- Random access still needed
- Memory usage can grow

## Resources

### Documentation
- [cppreference.com - std::map](https://en.cppreference.com/w/cpp/container/map)
- [cppreference.com - std::stack](https://en.cppreference.com/w/cpp/container/stack)
- [cppreference.com - std::vector](https://en.cppreference.com/w/cpp/container/vector)
- [cppreference.com - std::deque](https://en.cppreference.com/w/cpp/container/deque)
- [cppreference.com - upper_bound](https://en.cppreference.com/w/cpp/algorithm/upper_bound)

### Algorithms
- [Ford-Johnson Algorithm - Wikipedia](https://en.wikipedia.org/wiki/Merge-insertion_sort)
- [Reverse Polish Notation - Wikipedia](https://en.wikipedia.org/wiki/Reverse_Polish_notation)
- [Jacobsthal Sequence - OEIS](https://oeis.org/A001045)
- [Red-Black Tree - Wikipedia](https://en.wikipedia.org/wiki/Red%E2%80%93black_tree)

### Articles and Tutorials
- [GeeksforGeeks - RPN](https://www.geeksforgeeks.org/evaluation-of-postfix-expression/)
- [GeeksforGeeks - Ford-Johnson Algorithm](https://www.geeksforgeeks.org/ford-johnson-algorithm/)
- [Understanding std::map](https://www.geeksforgeeks.org/map-associative-containers-the-c-standard-template-library-stl/)

### Books
- "Introduction to Algorithms" by Cormen et al. (CLRS) - Chapter on sorting
- "The Art of Computer Programming, Vol 3: Sorting and Searching" by Donald Knuth
- "Effective STL" by Scott Meyers - Items on container selection
- "The C++ Standard Library" by Nicolai Josuttis - Comprehensive STL reference

### AI Usage

AI was **not** used in the development of this project. All code was written manually to ensure full understanding of:
- STL container selection and usage
- Map operations and binary search trees
- Stack-based algorithm implementation
- Ford-Johnson merge-insertion sort
- Jacobsthal sequence generation
- Performance benchmarking techniques
- File parsing and validation
- Time complexity analysis

The README documentation was also written without AI assistance, based on understanding gained through:
- Implementing the Ford-Johnson algorithm from research papers
- Studying the Jacobsthal sequence properties
- Analyzing container performance characteristics
- Testing with various input sizes
- Understanding RPN evaluation algorithms
- Research on red-black tree properties

This hands-on approach ensured deep comprehension of algorithm design, container selection criteria, and performance optimization techniques.

## Testing

### Exercise 00 Tests

**Basic Tests**:
```bash
# Test with valid input
./btc input.txt

# Test with custom file
echo "date | value" > test.txt
echo "2011-01-03 | 5" >> test.txt
./btc test.txt
```

**Test Cases**:
1. **Valid Dates and Values**:
   ```
   2011-01-03 | 3
   2011-01-09 | 1
   2012-01-11 | 1
   ```

2. **Error Cases**:
   ```
   2012-01-11 | -1           # Negative number
   2012-01-11 | 2147483648   # Too large
   2001-42-42                # Invalid date
   2012-01-11 | 1000.5       # Over limit
   ```

3. **Date Lookup**:
   ```
   2009-01-01 | 1            # Before database start
   2025-12-31 | 1            # Future date (use latest)
   2011-01-05 | 1            # Date not in database (use closest previous)
   ```

4. **Edge Cases**:
   - Empty file
   - File without header
   - Malformed lines (no pipe separator)
   - Whitespace handling

**Validation**:
- Date format: YYYY-MM-DD (exactly 10 chars, hyphens at positions 4 and 7)
- Date validity: month 1-12, day 1-31, year >= 2009
- Value range: 0 <= value <= 1000
- No scientific notation

### Exercise 01 Tests

**Basic Tests**:
```bash
./RPN "8 9 * 9 - 9 - 9 - 4 - 1 +"  # Should output: 42
./RPN "7 7 * 7 -"                   # Should output: 42
./RPN "1 2 * 2 / 2 * 2 4 - +"      # Should output: 0
./RPN "3 4 + 2 *"                   # Should output: 14
```

**Error Tests**:
```bash
./RPN "(1 + 1)"              # Error (infix notation)
./RPN "1 2 +"                # Valid: 3
./RPN "1 2"                  # Error (multiple values left)
./RPN "+"                    # Error (insufficient operands)
./RPN "1 0 /"                # Error (division by zero)
./RPN "1 2 3 4 5 6 7 8 9 * * * * * * * *"  # Valid: 362880
./RPN "10 2 +"               # Error (only single digits)
./RPN "1 2 % 3 +"            # Error (invalid operator)
```

**Complex Expressions**:
```bash
./RPN "9 8 7 6 5 4 3 2 1 + + + + + + + +"  # Should output: 45
./RPN "5 5 5 5 + + +"                       # Should output: 20
./RPN "9 1 - 9 - 9 - 9 - 9 -"              # Should output: -27
```

**Edge Cases**:
- Empty expression
- Only numbers (no operators)
- Only operators (no numbers)
- Mixed valid/invalid tokens

### Exercise 02 Tests

**Small Input Tests**:
```bash
./PmergeMe 3 5 9 7 4
./PmergeMe 1
./PmergeMe 2 1
./PmergeMe 5 4 3 2 1
./PmergeMe 1 2 3 4 5
```

**Large Input Tests**:
```bash
# Generate random sequence
shuf -i 1-3000 -n 3000 | xargs ./PmergeMe

# Test with specific sizes
./PmergeMe $(seq 1 100 | sort -R)
./PmergeMe $(seq 1 500 | sort -R)
./PmergeMe $(seq 1 1000 | sort -R)
```

**Edge Cases**:
```bash
./PmergeMe                    # Error (no input)
./PmergeMe -1                 # Error (negative)
./PmergeMe 0                  # Valid
./PmergeMe 2147483647         # Valid (INT_MAX)
./PmergeMe 2147483648         # Error (overflow)
./PmergeMe 1 2 3 a 4 5        # Error (non-numeric)
./PmergeMe "1 2 3"            # Depends on parsing
```

**Performance Tests**:
```bash
# Measure time difference between containers
for n in 10 100 500 1000 3000; do
    echo "Testing with $n elements:"
    ./PmergeMe $(shuf -i 1-10000 -n $n)
done
```

**Verification**:
- Output should be sorted (ascending order)
- All input elements present in output
- No duplicates added or removed
- Both containers produce same sorted result
- Timing displayed in microseconds

**Expected Behavior**:
- Display first 5 elements of Before/After (or all if < 5)
- If > 5 elements, show "[...]" after fifth element
- Time measurements should be positive
- Vector usually faster than deque for random access patterns

### Memory Testing

Run with memory leak detection:
```bash
# Exercise 00
valgrind --leak-check=full ./btc input.txt

# Exercise 01
valgrind --leak-check=full ./RPN "3 4 + 2 *"

# Exercise 02
valgrind --leak-check=full ./PmergeMe 3 5 9 7 4 1 2
```

Expected: No memory leaks, all allocations freed.

### Automated Testing Script

```bash
#!/bin/bash

# Test Exercise 00
echo "Testing Exercise 00..."
cd ex00
make re
./btc input.txt > output.txt
echo "✓ Ex00 compiled and ran"

# Test Exercise 01
echo "Testing Exercise 01..."
cd ../ex01
make re
./RPN "8 9 * 9 - 9 - 9 - 4 - 1 +" | grep -q "42" && echo "✓ Ex01 test 1 passed"
./RPN "7 7 * 7 -" | grep -q "42" && echo "✓ Ex01 test 2 passed"

# Test Exercise 02
echo "Testing Exercise 02..."
cd ../ex02
make re
./PmergeMe 3 5 9 7 4 | grep -q "3 4 5 7 9" && echo "✓ Ex02 test passed"

echo "All tests completed!"
```

## Notes

- All code follows the **C++98 standard** (no C++11 or later features)
- Must handle edge cases gracefully with appropriate error messages
- Performance comparison should use high-resolution timers
- Container choice affects both correctness and performance
- File parsing must be robust (handle malformed input)
- Date validation must be thorough (leap years, month boundaries)
- RPN only supports single-digit numbers (0-9)
- Ford-Johnson is optimal in terms of comparison count
- Jacobsthal sequence minimizes binary search comparisons
- Large number handling requires overflow checking
- std::map maintains sorted order automatically
- std::stack restricts access to LIFO operations only
- Memory layout differences significantly impact performance

## Author

**viceda-s**  
*42 Luxembourg*

---

*For questions, issues, or suggestions, please refer to the 42 project evaluation guidelines.*
