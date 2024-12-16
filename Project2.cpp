//============================================================================
// Name        : Project2.cpp
// Author      : Taylor Jones
// Date        : 12/15/2024
// Description : CS 300 - Project 2
//============================================================================

#include <algorithm>
#include <climits>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <limits>

using namespace std;

//set default hash table size
const unsigned int DEFAULT_TABLE_SIZE = 8;

//define data structure to store course info
struct Course {
	string courseID;
	string courseTitle;
	vector<string> prereqs;

	Course(string id = "", string title = "", vector<string> prerequisites = {})
		: courseID(id), courseTitle(title), prereqs(prerequisites) {}
};

//define class with data members and methods for creating a hash table with chaining
class HashTable {

private:
	struct Node {
		Course course;
		unsigned int key;
		Node* next;

		//default constructor
		Node() {
			key = UINT_MAX;
			next = nullptr;
		}

		//initialize node with a course
		Node(Course initCourse) : Node() {
			course = initCourse;
		}

		//initialize node with course and key
		Node(Course initCourse, unsigned int initKey) : Node(initCourse) {
			key = initKey;
		}
	};

	vector<Node> nodes;
	unsigned int hashTableSize = DEFAULT_TABLE_SIZE;
	unsigned int hash(const string& key);

public:
	HashTable();
	HashTable(unsigned int size);
	~HashTable();
	void Insert(Course course);
	void PrintCourses();
	Course SearchCourse(const string& courseID);
	size_t Size();
};

//Default constructor
HashTable::HashTable() {
	//initialize node vector via resizing
	nodes.resize(hashTableSize);
}

//constructor to set size of table
HashTable::HashTable(unsigned int size) {
	// invoke local tableSize to size with this->
	this->hashTableSize = size;
	// resize nodes size
	nodes.resize(hashTableSize);
}

//destructor to release dynamically allocated memory
HashTable::~HashTable() {
	for (unsigned int i = 0; i < hashTableSize; ++i) {
		Node* current = nodes[i].next;
		while (current != nullptr) {
			Node* temp = current;
			current = current->next;
			delete temp;
		}
	}
}

//compute and return hash value to ensure it is always in range of the hash table
//since we're working with uses alphanumeric keys we use a string function
unsigned int HashTable::hash(const string& key) {
	unsigned int hashValue = 0;
	for (char ch : key) {
		hashValue = hashValue * 31 + ch;
	}
	return hashValue % hashTableSize; 
}

//insert courses into the hash table
void HashTable::Insert(Course course) {
	//calculate the hash key based on the course ID
	if (course.courseID.empty()) {
		cerr << "Invalid course ID. Skipping insertion." << endl;
		return;
	}

	//hash the string
	unsigned int hashKey = hash(course.courseID);

	//retrieve node at the calculated hash key position
	Node* current = &nodes.at(hashKey);

	//if hash key is empty
	if (current->key == UINT_MAX) {
		//assign hash key to the node
		current->key = hashKey;
		//store bid in node
		current->course = course;
		//point chain to null indicating end of chain
		current->next = nullptr;
	}
	else {
		//traverse linked list to find end of chain
		while (current->next != nullptr) {
			current = current->next;
		}
		//point next to the new node
		current->next = new Node(course, hashKey);
	}
}

//print a list of courses by looping through the nodes with for and while loops
void HashTable::PrintCourses() {
	vector<Course> courses;

	//for node begin to end iterate
	for (unsigned int i = 0; i < hashTableSize; ++i) {
		Node* node = &nodes.at(i);

		//if key not equal to UINT_MAx
		while (node != nullptr && node->key != UINT_MAX) {
			// output key, bidID, title, amount and fund
			courses.push_back(node->course);
			node = node->next;
		}
	}
	//sort courses in order by key
	sort(courses.begin(), courses.end(), [](const Course& a, const Course& b) {
		return a.courseID < b.courseID;
		});
	cout << "Here is a sample schedule: \n" << endl;
	for (const auto& course : courses) {
		cout << course.courseID << ", " << course.courseTitle << endl;		
	}
	cout << endl;
}

// Helper function to convert a string to lowercase
string toLowerCase(const string& str) {
	string lowerStr = str;
	std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
	return lowerStr;
}

size_t HashTable::Size() {
	size_t count = 0;
	for (unsigned int i = 0; i < hashTableSize; ++i) {
		Node* current = &nodes.at(i);
		while (current != nullptr && current->key != UINT_MAX) {
			count++;
			current = current->next;
		}
	}
	return count;
}

//search the hash table for a course
//when a match is found return the course information 
Course HashTable::SearchCourse(const string& courseID) {
	// Convert the input courseID to lowercase
	string lowerCourseID = toLowerCase(courseID);
	
	Node* current = &nodes.at(hash(lowerCourseID));

	// while node not equal to nullptr
	while (current != nullptr) {
		// Convert the stored courseID to lowercase for comparison
		string storedCourseID = toLowerCase(current->course.courseID);
		// If the current node matches, return the course
		if (storedCourseID == lowerCourseID) {
			return current->course;
		}
		//node is equal to next node
		current = current->next;
	}

	// if not found return default empty course
	cout << "Course not found." << endl;
	return Course();
}


//load .csv file data into data structure
void loadCourses(const string& csvPath, HashTable* hashTable) {
	cout << "Opening file: " << csvPath << endl;

	//error handling if file cannot open
	ifstream infile(csvPath);
	if (!infile.is_open()) {
		cerr << "Error: Unable to open file " << csvPath << endl;
		return;
	}

	string line;	

	//skips empty lines while reading file
	while (getline(infile, line)) {
		if (line.empty()) {
			cout << "Empty line skipped." << endl;
			continue;
		}

		stringstream ss(line);
		string token;
		vector<string> tokens;

		//split each line by comma delimiter and capture empty fields
		while (getline(ss, token, ',')) {
			token.erase(0, token.find_first_not_of(" \t"));
			token.erase(token.find_last_not_of(" \t") + 1);
			tokens.push_back(token);
		}

		//row validation
		if (tokens.size() < 2 || tokens[0].empty() || tokens[1].empty()) {
			cerr << "Invalid row: Missing CourseID or CourseTitle. Skipping: " << line << endl;
			continue;
		}

		string courseID = tokens[0];
		string courseTitle = tokens[1];
		vector<string> prereqs;

		//if there are more than two tokens (multiple prereqs) it adds them to prereqs
		if (tokens.size() > 2) {
			prereqs.assign(tokens.begin() + 2, tokens.end());
		}
			
		Course course(courseID, courseTitle, prereqs);
		//insert the course into the hash table
		hashTable->Insert(course);
		
	}
	cout << "\nFile loaded successfully.\n" << endl; 
	infile.close();
}

int main(int argc, char* argv[]) {
	// process command line arguments
	string csvPath, courseKey;
	if (argc > 1) {
		csvPath = argv[1];
	}
	else {
		cout << "Welcome to the course planner. Please load a .csv with course information to continue.\n" << endl;
		cout << "Enter .csv file path: ";
		cin >> csvPath;
	}

	// Define a hash table to hold courses
	HashTable* courseTable = new HashTable();

	int menuChoice = 0;

	//menu loop
	while (menuChoice != 9) {
		cout << "\nWelcome to the course planner\n" << endl;
		cout << "  1. Load Data Structure." << endl;
		cout << "  2. Print Sorted Course List." << endl;
		cout << "  3. Display Course Info." << endl;
		cout << "  9. Exit\n" << endl;
		cout << "Please select a menu option: ";

		//input validation
		if (!(cin >> menuChoice)) {
			cin.clear(); // Clear the error flag
			cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Ignore invalid input
			cout << "Invalid input. Please enter a number." << endl;
			continue;
		}

		switch (menuChoice) {

		case 1:
			//call method to load courses from csvfile into table
			loadCourses(csvPath, courseTable);
			break;
		case 2:
			
			if (courseTable->Size() == 0) {
				cout << "No courses loaded. Please load data first." << endl;
			}
			else {
				//call method to print list of courses from .csv file
				courseTable->PrintCourses();
			}
			break;
		case 3:
			if (courseTable->Size() == 0) {
				cout << "No courses loaded. Please load data first." << endl;
			}  else {
			cout << "Enter course ID to search: ";
			cin >> courseKey;
			//search for a course and print course info if match is found
			{
				Course course = courseTable->SearchCourse(courseKey);

				if (!course.courseID.empty()) {
					cout << "\n***********************" << endl;
					cout << "Course ID: " << course.courseID << endl;
					cout << "Course Title: " << course.courseTitle << endl;
					cout << "Course Prerequirements: ";
					if (course.prereqs.empty()) {
						cout << "None" << endl;
					}
					else {
						for (const string& prereq : course.prereqs) {
							cout << prereq << " ";
						}
						cout << "\n***********************" << endl;
					}
				}
				}
			}
			break;
		case 9:
			cout << "Goodbye." << endl;
			break;
		default:
			cout << "Invalid choice. Please try again." << endl;
			break;
		}
	}

	delete courseTable;
	return 0;
};