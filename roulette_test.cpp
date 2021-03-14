#include "roulette.hpp"
#include <iostream>

#define ATTEMPTS 100

using std::cout;
using std::endl;

int main(int argc, char* argv[]){

    Roulette<std::string> word_roulette({{"High five", 5}, {"down low", 2}});
    const Roulette<std::string> const_word_roulette(word_roulette);

    for (int i = 0 ; i < ATTEMPTS ; ++i){

        cout << const_word_roulette.roll() << endl;

    }

    return 0;
}