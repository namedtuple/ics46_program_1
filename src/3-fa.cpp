#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "array_priority_queue.hpp"
#include "array_set.hpp"
#include "array_map.hpp"

typedef std::string                                     Input;
typedef std::string                                     State;
typedef ics::ArrayQueue         <Input>                 InputsQueue;
typedef ics::ArrayMap           <Input, State>          InputStateMap;
typedef ics::ArrayMap           <State, InputStateMap>  FA;
typedef ics::pair               <State, InputStateMap>  FAEntry;
typedef ics::ArrayPriorityQueue <FAEntry>               FAPQ;
typedef ics::pair               <Input, State>          Transition;
typedef ics::ArrayQueue         <Transition>            TransitionQueue;


const FA read_fa (std::ifstream& file) {
    //Read an open file describing the finite automaton (each line starts with
    //  a state name followed by pairs of transitions from that state: (input
    //  followed by new state) all separated by semicolons), and return a Map
    //  whose keys are states and whose associated values are another Map with
    //  each input in that state (keys) and the resulting state it leads to.
    FA fa;
    InputStateMap ism;
    std::string line;

    while (getline(file, line)) {
        std::vector<std::string> line_as_vector = ics::split(line, ";");
        State main_state = line_as_vector.front();

        for (int i=0; i<line_as_vector.size(); ++i) {
            if (i%2 == 0 && i != 0) {
                Input input = line_as_vector[i-1];
                State state = line_as_vector[i];
                ism[input] = state;
            }
        }
        fa[main_state] = ism;
    }
    return fa;
}

bool fa_entry_alphabetically (const FAEntry& a, const FAEntry& b) {
    return a.first < b.first;
}
void print_fa (const FA& fa) {
    //Print a label and all the entries in the finite automaton Map, in
    //  alphabetical order of the states: each line has a state, the text
    //  "transition:" and the Map of its transitions.
    FAPQ sorted_fa(fa_entry_alphabetically);
    sorted_fa.enqueue(fa.ibegin(), fa.iend());

    std::cout << "\nFinite Automaton Description" << std::endl;
    for (FAEntry kv : sorted_fa)
        std::cout << "  " << kv.first << " transitions: " << kv.second << std::endl;
}

TransitionQueue process (const FA& fa, State state, const InputsQueue& inputs) {
    //Return a queue of the calculated transition pairs, based on the finite
    //  automaton, initial state, and queue of inputs; each pair in the returned
    //  queue is of the form: input, new state.
    //The first pair contains "" as the input and the initial state.
    //If any input i is illegal (does not lead to a state in the finite
    //  automaton, then the last pair in the returned queue is i,"None".
    InputStateMap ism;
    State new_state;
    TransitionQueue trans_queue;
    trans_queue.enqueue(Transition("", state));

    for (Input input : inputs) {
        ism         = fa[state];
        new_state = (ism.has_key(input)) ? ism[input] : "None";
        Transition trans(input, new_state);
        trans_queue.enqueue(trans);
        state = new_state;
    }
    return trans_queue;
}

void interpret (TransitionQueue& tq) {  //or TransitionQueue or TransitionQueue&&
    //Print a TransitionQueue (the result of calling process) in a nice form.
    //Print the Start state on the first line; then print each input and the
    //  resulting new state (or "illegal input: terminated", if the state is
    //  "None") indented on subsequent lines; on the last line, print the Stop
    //  state (which might be "None").
    State last_state;
    for (Transition t : tq) {
        if (t.first == "")
            std::cout << "Start state = " << t.second << std::endl;
        else {
            std::cout << "  Input = " << t.first;
            if (t.second == "None")
                std::cout << "; illegal input: terminated" << std::endl;
            else
                std::cout << "; new state = " << t.second << std::endl;
            last_state = t.second;
        }
    }
    std::cout << "Stop state = " << last_state << std::endl;
}

int main () {
    //Prompt the user for a file, create a finite automaton Map, and print it.
    //Prompt the user for a file containing any number of simulation descriptions
    //  for the finite automaton to process, one description per line; each
    //  description contains a start state followed by inputs, all separated by
    //  semicolons.
    //Repeatedly read a description, print that description, put each input in a
    //  Queue, process the Queue and print the results in a nice form.

    try {
        std::ifstream file;
        ics::safe_open(file, "Enter file name of Finite Automaton", "faparity.txt");
        FA fa = read_fa(file);
        print_fa(fa);

        std::ifstream inputs_file;
        ics::safe_open(inputs_file, "\nEnter file name of start-states and inputs", "fainputparity.txt");

        std::string line;
        while (getline(inputs_file, line)) {
            std::vector<std::string> line_as_vector = ics::split(line, ";");
            State state = line_as_vector[0];
            InputsQueue iq;
            for (std::vector<std::string>::iterator it = line_as_vector.begin()+1; it != line_as_vector.end(); ++it)
                iq.enqueue(*it);
            TransitionQueue tq = process(fa, state, iq);
            std::cout << "\nStarting new simulation with description: " << line << std::endl;
            interpret(tq);
        }
    }

    catch (ics::IcsError& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
