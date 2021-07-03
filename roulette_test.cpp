#define ROULETTE_DEBUG_CPP
#include "roulette.hpp"
#include <iostream>
#include <map>

#define ATTEMPTS 10000

using std::cout;
using std::endl;

typedef std::string test_val_t;

std::map<test_val_t, size_t> run_roulette(Roulette<test_val_t, NewRand> roulette, int attempts, bool print_run = false){

    //const Roulette<test_val_t, NewRand> const_roulette(roulette);
    Roulette<test_val_t, NewRand> const_roulette(roulette);

    std::map<test_val_t, size_t> counter;
    std::map<test_val_t, size_t>::iterator find_iter;


    for (int i = 0 ; i < attempts ; ++i){
        
        test_val_t new_val = const_roulette.roll();

        if ((find_iter = counter.find(new_val)) == counter.end()){
            counter[new_val] = 1;
        }else{
            ++(find_iter->second);
        }
        
        if (print_run)
            cout << new_val << endl;
    }

    return counter;
}

int main(int argc, char* argv[]){

    const char* removable = "little bitch";
    const char* non_existant = "n/a";
    const char* updatable = "suck this dick";

    Roulette<test_val_t, NewRand>::iterator default_iter;
    Roulette<test_val_t, NewRand>::iterator another_default_iter;

    Roulette<test_val_t, NewRand> roulette({{updatable, 3.5}, {"sir farts a lot", 1.5}, {"smell you later", 4.25}, {removable, 0.75}});

    if(default_iter == ++roulette.end()){
        cout << "default is end" << endl;
    }else{
        cout << "default is NOT end" << endl;
    }

    if(default_iter == another_default_iter){
        cout << "default is another" << endl;
    }else{
        cout << "default is NOT another" << endl;
    }
    
    auto counter = run_roulette(roulette, ATTEMPTS);
    
    for( auto const& val : counter){
        cout << "value \"" << val.first << "\" was found " << val.second << " times" << endl;
    }

    cout << endl;

    for (auto iter = roulette.begin() ; iter != roulette.end() ; ++iter)
        cout << "value \"" << iter->get_value() << "\" is between " << iter->get_min() << " and " << iter->get_max() << endl;

    cout << endl << "removing value \"" << non_existant << "\" " << ((roulette.remove(non_existant))? "succeded" : "failed" ) << endl << endl;

    cout << endl << "removing value \"" << removable << "\" " << ((roulette.remove(removable))? "succeded" : "failed" ) << endl << endl;

    cout << endl << "updating value \"" << non_existant << "\" " << ((roulette.update(non_existant, 5))? "succeded" : "failed" ) << endl << endl;
    
    cout << endl << "updating value \"" << updatable << "\" " << ((roulette.update(updatable, 5))? "succeded" : "failed" ) << endl << endl;

    for (auto iter = roulette.begin() ; iter != roulette.end() ; ++iter)
        cout << "value \"" << iter->get_value() << "\" is between " << iter->get_min() << " and " << iter->get_max() << endl;

    cout << endl;
    counter = run_roulette(roulette, ATTEMPTS);
    
    for( auto const& val : counter){
        cout << "value \"" << val.first << "\" was found " << val.second << " times" << endl;
    }

    return 0;
}