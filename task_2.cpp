#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

using namespace std;
const string FILE_NAME = "students.txt";

// ---------- Result: the base class ----------
// Inheritance: TheoryResult and LabResult are derived from this class.
class Result {
protected:
    string subject;
    int marks;
    int maxMarks;
    int passMark;
public:
    Result(const string& subj, int m, int maxM, int passM)
        : subject(subj), marks(m), maxMarks(maxM), passMark(passM) {
        // Exception handling: refuse marks outside the allowed range
        if (m < 0 || m > maxM) {
            ostringstream oss;
            oss << "marks must be between 0 and " << maxM;
            throw invalid_argument(oss.str());
        }
    }
    virtual ~Result() {}
    // Virtual functions: each derived class gives its own answer
    virtual string typeName() const = 0;
    virtual string code() const = 0;   // letter saved in the file
    bool passed() const { return marks >= passMark; }
    string getSubject() const { return subject; }
    int getMarks() const { return marks; }
    int getMaxMarks() const { return maxMarks; }
};

// Theory subject: out of 100, pass mark 40
class TheoryResult : public Result {
public:
    TheoryResult(const string& subj, int m) : Result(subj, m, 100, 40) {}
    string typeName() const override { return "Theory"; }
    string code() const override { return "T"; }
};

// Lab subject: out of 50, pass mark 20
class LabResult : public Result {
public:
    LabResult(const string& subj, int m) : Result(subj, m, 50, 20) {}
    string typeName() const override { return "Lab"; }
    string code() const override { return "L"; }
};

// Factory: creates the correct kind of Result object
unique_ptr<Result> makeResult(const string& type, const string& subject, int marks) {
    if (type == "T") return make_unique<TheoryResult>(subject, marks);
    if (type == "L") return make_unique<LabResult>(subject, marks);
    throw invalid_argument("unknown subject type");
}

// ---------- Student ----------
class Student {
public:
    int roll;
    string name;
    vector<unique_ptr<Result>> results;   // smart pointers stored in a vector
    Student(int r, const string& n) : roll(r), name(n) {}
    int totalMarks() const {
        int total = 0;
        for (const auto& r : results) total += r->getMarks();
        return total;
    }
    int totalMaxMarks() const {
        int total = 0;
        for (const auto& r : results) total += r->getMaxMarks();
        return total;
    }
    double percentage() const {
        if (totalMaxMarks() == 0) return 0.0;
        return 100.0 * totalMarks() / totalMaxMarks();
    }
    // A student passes only if every subject is passed
    bool passed() const {
        if (results.empty()) return false;
        for (const auto& r : results)
            if (!r->passed()) return false;
        return true;
    }
};

// All students are kept in this vector
vector<unique_ptr<Student>> students;

// ---------- Helper functions ----------
// Template: reads a value of any type (int, double, ...) and asks again if it is wrong
template <typename T>
T readValue(const string& prompt) {
    T value;
    cout << prompt;
    while (!(cin >> value)) {
        if (cin.eof()) {
            cout << "\nInput closed. Exiting.\n";
            exit(0);
        }
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Please enter a valid number: ";
    }
    cin.ignore(10000, '\n');   // skip the rest of the line
    return value;
}

// Reads one line of text (for names and subjects)
string readText(const string& prompt) {
    cout << prompt;
    string text;
    getline(cin, text);
    if (text.empty()) throw invalid_argument("text cannot be empty");
    if (text.find(',') != string::npos) throw invalid_argument("commas are not allowed");
    return text;
}

// Converts text to an int, with a clear message if it is not a number
int toNumber(const string& text) {
    istringstream iss(text);
    int value;
    char extra;
    if (!(iss >> value) || (iss >> extra))
        throw invalid_argument("'" + text + "' is not a number");
    return value;
}

// Splits one line of the file at each comma
vector<string> split(const string& line) {
    vector<string> parts;
    stringstream ss(line);
    string part;
    while (getline(ss, part, ',')) parts.push_back(part);
    return parts;
}

// Finds a student by roll number (STL find_if with a lambda)
Student* findStudent(int roll) {
    auto it = find_if(students.begin(), students.end(),
                      [roll](const unique_ptr<Student>& s) { return s->roll == roll; });
    if (it == students.end()) return nullptr;
    return it->get();
}

// Table printing
void printHeader() {
    cout << left << setw(8) << "Roll" << setw(22) << "Name"
         << setw(12) << "Percentage" << "Status\n";
    cout << string(48, '-') << "\n";
}

void printRow(const Student& s) {
    cout << left << setw(8) << s.roll << setw(22) << s.name
         << setw(12) << s.percentage() << (s.passed() ? "PASS" : "FAIL") << "\n";
}

// ---------- Menu actions ----------
void addStudent() {
    int roll = readValue<int>("Enter roll number: ");
    if (findStudent(roll) != nullptr)
        throw runtime_error("roll number already exists");
    string name = readText("Enter student name: ");
    students.push_back(make_unique<Student>(roll, name));
    cout << "Student added.\n";
}

void addMarks() {
    int roll = readValue<int>("Enter roll number: ");
    Student* s = findStudent(roll);
    if (s == nullptr) throw runtime_error("no student has this roll number");
    cout << "Subject type: 1) Theory (out of 100)   2) Lab (out of 50)\n";
    int type = readValue<int>("Choose 1 or 2: ");
    if (type != 1 && type != 2) throw invalid_argument("type must be 1 or 2");
    string subject = readText("Enter subject name: ");
    int marks = readValue<int>("Enter marks: ");
    // makeResult() throws an exception if the marks are out of range
    s->results.push_back(makeResult(type == 1 ? "T" : "L", subject, marks));
    cout << "Marks added.\n";
}

void showStudentDetails() {
    int roll = readValue<int>("Enter roll number: ");
    Student* s = findStudent(roll);
    if (s == nullptr) throw runtime_error("no student has this roll number");
    cout << "\nRoll: " << s->roll << "   Name: " << s->name << "\n";
    for (const auto& r : s->results) {
        // typeName() is virtual, so each subject prints its own type
        cout << "  " << left << setw(22) << r->getSubject()
             << setw(8) << r->typeName()
             << right << setw(4) << r->getMarks() << " / " << setw(3) << r->getMaxMarks()
             << (r->passed() ? "   PASS" : "   FAIL") << "\n";
    }
    cout << "Total: " << s->totalMarks() << " / " << s->totalMaxMarks()
         << "   Percentage: " << s->percentage() << "%"
         << "   Status: " << (s->passed() ? "PASS" : "FAIL") << "\n";
}

void showAllStudents() {
    if (students.empty()) {
        cout << "No students yet.\n";
        return;
    }
    // Make a list of pointers, so the students themselves are not moved
    vector<Student*> list;
    for (const auto& s : students) list.push_back(s.get());
    // STL sort with a lambda: highest percentage first
    sort(list.begin(), list.end(), [](Student* a, Student* b) {
        return a->percentage() > b->percentage();
    });
    printHeader();
    for (Student* s : list) printRow(*s);
    // STL count_if with a lambda: how many students passed
    int passCount = count_if(students.begin(), students.end(),
                             [](const unique_ptr<Student>& s) { return s->passed(); });
    cout << "Total students: " << students.size() << ", passed: " << passCount << "\n";
}

void showPassedStudents() {
    printHeader();
    // STL for_each with a lambda: print only the students who passed
    for_each(students.begin(), students.end(), [](const unique_ptr<Student>& s) {
        if (s->passed()) printRow(*s);
    });
}

// ---------- File handling ----------
// Writes all students to the file.  Format:
//   S,roll,name
//   R,type,subject,marks      (type is T or L)
void writeFile() {
    ofstream out(FILE_NAME);
    if (!out) throw runtime_error("could not open " + FILE_NAME + " for writing");
    for (const auto& s : students) {
        out << "S," << s->roll << "," << s->name << "\n";
        for (const auto& r : s->results)
            out << "R," << r->code() << "," << r->getSubject() << "," << r->getMarks() << "\n";
    }
}

// Reads students from the file.  Bad lines are reported and skipped.
void readFile() {
    ifstream in(FILE_NAME);
    if (!in) throw runtime_error("could not open " + FILE_NAME + ". Save first.");
    students.clear();
    Student* current = nullptr;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        try {
            vector<string> p = split(line);
            if (p[0] == "S" && p.size() == 3) {               // student line
                current = nullptr;
                int roll = toNumber(p[1]);
                if (findStudent(roll) != nullptr)
                    throw runtime_error("duplicate roll number");
                students.push_back(make_unique<Student>(roll, p[2]));
                current = students.back().get();
            } else if (p[0] == "R" && p.size() == 4) {        // result line
                if (current == nullptr)
                    throw runtime_error("result line without a student");
                current->results.push_back(makeResult(p[1], p[2], toNumber(p[3])));
            } else {
                throw runtime_error("unknown line");
            }
        } catch (const exception& e) {
            cout << "Skipped line \"" << line << "\": " << e.what() << "\n";
        }
    }
}

// Data passed to the Windows worker thread
struct ThreadTask {
    void (*job)();
    string error;
};

DWORD WINAPI threadWorker(LPVOID param) {
    ThreadTask* task = static_cast<ThreadTask*>(param);
    try {
        task->job();
    } catch (const exception& e) {
        task->error = e.what();
    }
    return 0;
}

// Runs a job on a separate Windows thread and waits for it.
// Any error inside the thread is saved and thrown again on the main thread.
void runInThread(void (*job)()) {
    ThreadTask task{job, ""};
    HANDLE worker = CreateThread(nullptr, 0, threadWorker, &task, 0, nullptr);
    if (worker == nullptr)
        throw runtime_error("could not create worker thread");
    WaitForSingleObject(worker, INFINITE);   // the main thread waits here
    CloseHandle(worker);
    if (!task.error.empty()) throw runtime_error(task.error);
}

void saveToFile() {
    runInThread(writeFile);
    cout << "Saved to " << FILE_NAME << ".\n";
}

void loadFromFile() {
    runInThread(readFile);
    cout << "Loaded " << students.size() << " student(s) from " << FILE_NAME << ".\n";
}

// ---------- Main menu ----------
void showMenu() {
    cout << "\n===== STUDENT RESULT MANAGEMENT SYSTEM =====\n"
         << "1. Add student\n"
         << "2. Add marks for a student\n"
         << "3. Show one student's marks\n"
         << "4. Show all students (sorted by percentage)\n"
         << "5. Show only passed students\n"
         << "6. Save to file\n"
         << "7. Load from file\n"
         << "0. Exit\n";
}

int main() {
    cout << fixed << setprecision(2);
    int choice = -1;
    while (choice != 0) {
        showMenu();
        choice = readValue<int>("Enter choice: ");
        // Any error thrown by an action is shown here, and the menu comes back
        try {
            switch (choice) {
                case 1: addStudent();         break;
                case 2: addMarks();           break;
                case 3: showStudentDetails(); break;
                case 4: showAllStudents();    break;
                case 5: showPassedStudents(); break;
                case 6: saveToFile();         break;
                case 7: loadFromFile();       break;
                case 0: break;
                default: cout << "Please choose a number from the menu.\n";
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }
    cout << "Goodbye!\n";
    return 0;
}