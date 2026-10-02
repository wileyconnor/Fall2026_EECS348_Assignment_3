/*
 * Name of Program:    EECS 348 Assignment 3
 * Description:        C++ program that prioritizes a busy CEO's email inbox
 *                      using a hand-written, list-based MaxHeap that
 *                      implements a priority queue. Emails are queued by
 *                      sender category, read in this order: Boss,
 *                      Subordinate, Peer, ImportantPerson, OtherPerson.
 *                      Within the same category, the newest email (by
 *                      MM-DD-YYYY date) is read before older ones.
 *                      Supports EMAIL, NEXT, READ, and COUNT commands.
 * Input:               A test file (given as argv[1]) or stdin containing
 *                      EMAIL commands (<sender category>,<subject line>,
 *                      <date>) followed by any mix of NEXT, READ, and
 *                      COUNT commands, one command per line.
 * Output:              Terminal output: how many emails are left to read,
 *                      the next email's sender/subject/date on NEXT, and
 *                      a warning for any malformed EMAIL line.
 * Collaborators:       Claude, Connor Wiley
 * Other Sources:       N/A
 *                      
 *                      
 * Author:              Claude, Connor Wiley
 * Creation Date:       10/1/2026
 * Revision Date:       10/1/2026
 * Revisions:           N/A
 *                      
 */
 
 #include <iostream> // for cin, cout, and basic terminal input/output
#include <fstream> // for ifstream, used to open the file passed in on the command line
#include <sstream> // for istringstream, used to pick apart each line
#include <vector> // for vector, used as the backing array for the heap
using namespace std; // so we don't have to type std:: in front of everything

struct Email { // creates the struct Email for a single email
    string sender, subject, date;  // instantiates sender for the person sending, subject for the topic, and date for the year-month-day they sent
    int priorityKey = 0; // instantiates a cached key so key() doesn't have to redo the lookup and parsing on every single heap comparison

    static int categoryPriority(const string& category) { // instantiates categoryPriority as a plain if-chain instead of a map, O(1) since its at most 4 comparisons, and it cant silently insert a bad entry the way map's operator[] used to
        if (category == "Boss") return 5; // Boss is the highest priority
        if (category == "Subordinate") return 4; // Subordinate is next
        if (category == "Peer") return 3; // then Peer
        if (category == "ImportantPerson") return 2; // then ImportantPerson
        return 1; // OtherPerson or anything unrecognized safely falls back to the lowest priority, with no side effects
    } // ends categoryPriority function

    void computeKey() { // instantiates computeKey to fill in priorityKey once, right after sender and date get set, O(1) since the date is a fixed length
        int yyyymmdd = stoi(date.substr(6)) * 10000 + stoi(date.substr(0, 2)) * 100 + stoi(date.substr(3, 2)); // creates the date into YYYYMMDD as a plain int
        priorityKey = categoryPriority(sender) * 100000000 + yyyymmdd; // combines the category first, then newest date as the value
    } // ends computeKey function

    int key() const { return priorityKey; } // creates the key function that just returns the cached value, O(1), instead of redoing the lookup and parsing every time
}; // ends Email struct

class MaxHeap {  // creates the class MaxHeap as an array based binary max-heap
    vector<Email> h; // instantiates h as a vector instead of a raw array so it resizes itself
public: // everything below here is public
    int size() const { return h.size(); } // returns how many emails are in the heap, O(1)
    const Email& top() const { // creates the top function that looks at the top email without removing it, O(1) since its always at the root
        static Email empty; // instantiates a fallback empty email for the edge case where somebody calls top() on an empty heap
        if (h.empty()) return empty; // checks if the heap is actually empty first now, instead of just grabbing h[0] no matter what
        return h[0]; // returns the top email
    } // ends top function
    void push(const Email& e) { // creates the push function that adds an email to the heap, O(log n) since key() is cached now, each comparison during the sift is just a cheap int compare instead of redoing the lookup
        h.push_back(e); // sticks the new email on the end of the vector
        for (size_t i = h.size() - 1; i > 0 && h[i].key() > h[(i - 1) / 2].key(); i = (i - 1) / 2) // keeps comparing to the parent while we outrank it
            swap(h[i], h[(i - 1) / 2]); // swaps up since we outrank the parent
    } // ends push function
    void pop() { // creates the pop function that removes the top email from the heap, O(log n) for the sift down loop
        if (h.empty()) return; // checks if the heap is actually empty first now, instead of touching h[0] and h.back() no matter what
        h[0] = h.back(); // moves the last email into the root
        h.pop_back(); // shrinks the vector by one
        for (size_t i = 0, c; (c = 2 * i + 1) < h.size(); i = c) { // walks down from the root while theres at least a left child
            if (c + 1 < h.size() && h[c + 1].key() > h[c].key()) c++; // checks if the right child outranks the left, if so use the right child instead
            if (h[c].key() <= h[i].key()) break; // if neither child outranks us, the heap is good, stop here
            swap(h[i], h[c]); // swaps down to whichever child won
        } // ends for loop
    } // ends pop function
}; // ends MaxHeap class

class Inbox { // creates the class Inbox that wraps the heap and handles reading commands
    MaxHeap q; // instantiates q as the heap that holds all the CEO's emails
    bool started = false; // instantiates started to track whether weve printed anything yet, used to space out the output
    void print(const string& s) { cout << (started ? "\n" : "") << s; started = true; } // prints a blank line before every message except the first one
public: // everything below here is public
    void run(istream& in) { // creates the run function that reads commands from a stream and dispatches them, O(m log n) overall for m lines since EMAIL and READ are each O(log n) and COUNT and NEXT are O(1)
        string line, cmd; // holds each line, and the first word of that line
        while (getline(in, line)) { // reads lines until the input runs out
            if (!line.empty() && line.back() == '\r') line.pop_back(); // strips a trailing carriage return if theres one
            istringstream ss(line); // builds a stream out of the line so we can pick it apart
            ss >> cmd; // reads the first word of the line as the command
            ss.ignore(); // skips the single space right after the command word
            if (cmd == "EMAIL") { // checks if the command is EMAIL
                Email e; // new email to fill in
                getline(ss, e.sender, ','); // grabs everything up to the first comma as the sender
                getline(ss, e.subject, ','); // grabs everything up to the second comma as the subject
                getline(ss, e.date); // grabs whatever is left as the date
                e.computeKey(); // fills in the cached priority key now that sender and date are set
                q.push(e); // pushes the new email onto the heap
            } else if (cmd == "COUNT") { // checks if the command is COUNT
                print("There are " + to_string(q.size()) + " emails to read.\n"); // prints how many emails are left
            } else if (cmd == "NEXT" && q.size()) { // checks if the command is NEXT and theres actually something to show
                print("Next email:\n\tSender: " + q.top().sender + "\n\tSubject: " + q.top().subject +
                      "\n\tDate: " + q.top().date + "\n"); // prints out the sender, subject, and date of the top email
            } else if (cmd == "READ" && q.size()) { // checks if the command is READ and theres actually something to remove
                q.pop(); // pops the top email off the heap
            } // ends else if statement
        } // ends while statement
    } // ends run function
}; // ends Inbox class

int main(int argc, char** argv) { // instantiates the main function for running the program
    ifstream file(argc > 1 ? argv[1] : ""); // tries to open a filename if one was given, otherwise tries to open an empty name which just fails
    Inbox ceo; // creates the inbox that holds the heap and does all the work
    ceo.run(file ? static_cast<istream&>(file) : cin); // uses the file if it actually opened okay, otherwise falls back to stdin
} // ends main function
 
