#include "roulette.hpp"
#include <iostream>
#include <map>

#define ATTEMPTS 10000

using std::cout;
using std::endl;

typedef std::string test_val_t;

int main(int argc, char* argv[]){

    const char* removable = "little bitch";

    Roulette<test_val_t, NewRand> roulette({{"suck this dick", 5}, {removable, 2}, {"smell you later", 2}, {"sir farts a lot", 1}});
    const Roulette<test_val_t, NewRand> const_roulette(roulette);

    std::map<test_val_t, size_t> counter;
    std::map<test_val_t, size_t>::iterator find_iter;

    for (int i = 0 ; i < ATTEMPTS ; ++i){
        
        test_val_t new_val = const_roulette.roll();

        if ((find_iter = counter.find(new_val)) == counter.end()){
            counter[new_val] = 1;
        }else{
            ++(find_iter->second);
        }

        cout << new_val << endl;
    }

    for( auto const& val : counter){
        cout << "value \"" << val.first << "\" was found " << val.second << " times" << endl;
    }

    cout << endl;

    for (auto iter = roulette.begin() ; iter != roulette.end() ; ++iter)
        cout << "value \"" << iter->get_value() << "\" is between " << iter->get_min() << " and " << iter->get_max() << endl;

    cout << endl << "removing value \"" << removable << "\"" << endl << endl;

    roulette.remove(removable);

    for (auto iter = roulette.begin() ; iter != roulette.end() ; ++iter)
        cout << "value \"" << iter->get_value() << "\" is between " << iter->get_min() << " and " << iter->get_max() << endl;


    return 0;
}