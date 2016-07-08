/**
*
*	barrier.cpp
*	Chintan Acharya	140070107007
*	Pratik Hadvani	140070107015
*	Ankit Dodiya	140070107012
*
*	Simulates a barrier system for thread synchronisation.
*
**/

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#define BLOCKED 0
#define RUNNING 1
#define TERMINATED 2

using namespace std;

int THREAD_COUNTER = 0; // A counter for threads

map<int, int> BARRIERS; // Stores barrier sizes
map<int, vector<int> > THREADS_WAITING; // Stores the IDs of the threads waiting on all barriers

vector<bool> AVAILABLE; // Threads that are available for running


void create_barrier(int b_id, int value);
/*
*	Creates a barrier of size 'value'
*/


void barrier_wait(int b_id, int thread_id);
/*
*	Causes the thread identified by thread_id to wait on barrier b_id
*/


void barrier_signal(int b_id, int thread_id);
/*
*	Wakes all the threads waiting on the barrier b_id
*/

class Instruction
{
public:
	string type; // Type of the instruction
	int bar_id; // Barrier ID for BE and CB instructions
	int bar_size; // Size of barrier to be created (for CB)
	// TODO: add additional members if required
	Instruction(void)
	{
		cin >> type;
		if (type == "CB" || type == "BW")
			cin >> bar_id;

		if (type == "CB")
			cin >> bar_size;
	}
	Instruction(int not_used)
	{	}
};

ostream & operator<<(ostream & temp, Instruction i)
{
	// Overloaded operator for printing out instruction
	cout << i.type;

	if (i.type == "BW")
		cout << " " << i.bar_id;

	if (i.type == "CB")
		cout << " " << i.bar_id << " " << i.bar_size;
	return temp;
}

class Thread
{
	vector<Instruction> text; // Code of the thread
	int id; // Thread ID
	int pc; // Program Counter for thread
	int state; // State (Running / Blocked)
public:
	Thread(void)
	{
		int n;
		cout << "Enter the no. of executable statements: ";
		cin >> n;
		cout << "Enter the instructions: (NC / CB / BW / CR): " << endl;
		for (int i = 0; i < n; i++)
		{ // Read instructions and add them to text
			Instruction inst;
			text.push_back(inst);
		}
		id = THREAD_COUNTER++; // Assign ID to the thread
		AVAILABLE.push_back(true); // Mark the threas as available
		pc = 0;
		state = RUNNING;
		cout << "Successfully created thread with ID " << id << endl;
	}

	Thread(const char* file)
	{
		ifstream fObj(file);
		int n;
		fObj >> n;
		for (int i = 0; i < n; i++)
		{
			Instruction inst(0);
			fObj >> inst.type;
			if (inst.type == "CB" || inst.type == "BW")
				fObj >> inst.bar_id;
			if(inst.type == "CB")
				fObj >> inst.bar_size;
			text.push_back(inst);

		}
		id = THREAD_COUNTER++; // Assign ID to the thread
		AVAILABLE.push_back(true); // Mark the thread as available
		pc = 0;
		state = RUNNING;
		cout << "Successfully loaded process ID " << id << " from file." << endl;
	}

	void execute(void)
	{
		if(state != BLOCKED && pc < text.size())
		{
			Instruction currentIns = text[pc];
			if (currentIns.type == "NC")
			{	// Non-critical code
				cout << "Executed line #" << pc + 1 << " : " << text[pc] << endl;
				pc++;
			}
			else if (currentIns.type == "CB")
			{	// Call to create barrier
				create_barrier(currentIns.bar_id, currentIns.bar_size);
				cout << "Barrier #" << currentIns.bar_id << " size " << currentIns.bar_size << " created." << endl;
				pc++;
			}
			else if (currentIns.type == "BW")
			{	// Wait for barrier
				barrier_wait(currentIns.bar_id, id);
				cout << "Thread #" << id << " is waiting on barrier #" << currentIns.bar_id << endl;
				pc++;
			}
			else if (currentIns.type == "CR")
			{	// Critical region
				cout << "Executed CRITICAL line #" << pc + 1 << " : " << text[pc] << endl;
				pc++;
			}
			else
			{
				cout << "Syntax error at line #" << pc + 1 << " : " << text[pc] << endl;
				pc++;
			}
		}
		else if (pc >= text.size())
		{
			cout << "The process has been terminated. " << endl;
			state = TERMINATED;
			AVAILABLE[id] = false;
		}
		else 
		{
			cout << "The thread " << id << " is blocked and cannot continue." << endl;
		}
	}
	void block(void)
	{
		state = BLOCKED;
		AVAILABLE[id] = false;
	}
	void unblock(void)
	{
		state = RUNNING;
		AVAILABLE[id] = true;
	}
	void display(void)
	{
		cout << id << "\t";
		cout << pc << "\t";
		if (state == RUNNING)
			cout << "Running" << "\t";
		else if (state == BLOCKED)
			cout << "Blocked" << "\t";
		else
			cout << "Terminated" << "\t";
	}
};

vector<Thread> THREADS;

void create_barrier(int b_id, int value)
{
	//TODO: if barrier exists, dont create; else create
	BARRIERS[b_id] = value;
}

void barrier_wait(int b_id, int thread_id)
{
	THREADS_WAITING[b_id].push_back(thread_id);
	THREADS[thread_id].block();
	// Open the barrier if no. of threads waiting == size
	if (BARRIERS[b_id] == THREADS_WAITING[b_id].size())
		barrier_signal(b_id, thread_id);
}

void barrier_signal(int b_id, int thread_id)
{
	// Unblock all threads waiting on the barrier b_id
	vector<int> threads = THREADS_WAITING[b_id];
	cout << "Barrier #" << b_id << " released." << endl; 
	vector<int>::iterator i = threads.begin();
	while (i != threads.end())
	{
		THREADS[*i].unblock();
		i++;
	}
	// Remove the barrier entry
	map<int, vector<int> >::iterator it = THREADS_WAITING.find(b_id);
	THREADS_WAITING.erase(it);
	cout << "Barrier #" << b_id << "RELEASED!" << endl;
}

int main(void)
{
	/*
	int n;
	cout << "Enter no. of threads: ";
	cin >> n;
	for (int i = 0; i < n; i++)
	{
		Thread t;
		THREADS.push_back(t);
	}
	*/

	int ch;
	do
	{
		cout << "Menu :" << endl;
		cout << "1. Create thread" << endl;
		cout << "2. Load thread from file" << endl;
		cout << "3. Execute a thread" << endl;
		cout << "4. View all threads" << endl;
		cout << "5. View running threads" << endl;
		cout << "6. View blocked threads" << endl;
		cout << "7. View help" << endl;
		cout << "0. Exit" << endl;
		cout << "Enter your choice: ";
		cin >> ch;
		if(ch == 1)
		{
			Thread t;
			THREADS.push_back(t);
		}
		else if (ch == 2)
		{
			//load process data from file
			char filename[100];
			cout << "Enter file name: ";
			cin >> filename;
			Thread t(filename);
			THREADS.push_back(t);
		}
		else if (ch == 3)
		{
			cout << "Available threads: " << endl;
			cout << "ID\tPC\tStatus" << endl;
			for (int i = 0; i < AVAILABLE.size(); i++)
			{
				if (AVAILABLE[i])
				{
					THREADS[i].display();
					cout << endl;
				}
			}
			int t_id;
			cout << "Select a thread (ID): ";
			cin >> t_id;
			THREADS[t_id].execute();
		}
		else if (ch == 4)
		{
			cout << "ID\tPC\tStatus" << endl;
			for (int i = 0; i < THREAD_COUNTER; i++)
			{
				THREADS[i].display();
				cout << endl;
			}
		}
		else if (ch == 5)
		{
			cout << "ID\tPC\tStatus" << endl;
			for (int i = 0; i < THREAD_COUNTER; i++)
			{
				if(AVAILABLE[i])
				{
					THREADS[i].display();
					cout << endl;
				}
			}
		}
		else if (ch == 6)
		{
			cout << "ID\tPC\tStatus" << endl;
			for (int i = 0; i < THREAD_COUNTER; i++)
			{
				if(!AVAILABLE[i])
				{
					THREADS[i].display();
					cout << endl;
				}
			}
		}
		else if (ch == 7)
		{
			cout << "HELP: " << endl;
			cout << "This program simulates thread execution and barrier system." << endl;
			cout << "Each thread is an array of instructions." << endl;
			cout << "Instruction format: " << endl;
			cout << "\tNC\t\tDenotes instructions not critical to barrier." << endl;
			cout << "\tCB <id> <size>\tDenotes the creation of a barrier of ID <id> and size <size>." << endl;
			cout << "\tBW <id>\t\tCauses the thread to wait until the barrier of ID <id> is not opened." << endl;
			cout << "\tCR\t\tDenotes the critical region of the thread i.e. the code that must be executed only after the barrier is passed." << endl;
			cout << endl;
			cout << "The file format for loading threads from file is:" << endl;
			cout << "\tThe first line contains the no. of instructions (n)." << endl;
			cout << "\tn lines follow, each containing an instruction." << endl;
			cout << "Thread execution: " << endl;
			cout << "Threads must be executed manually as no scheduling is present." << endl;
			cout << endl; 
		}
		cout << "\nPress any key to continue... ";
		cin.get();
		cin.get(); 
	}
	while (ch != 0);

	return 0;
}