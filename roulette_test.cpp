#include "roulette.hpp"
#include <iostream>
#include <map>

#define ATTEMPTS 7000

using std::cout;
using std::endl;

typedef std::string test_val_t;

int main(int argc, char* argv[]){

    Roulette<test_val_t, NewRand> roulette({{"little bitch", 5}, {"suck this dick", 2}});
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

    return 0;
}