#include <stdexcept>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <cstring>

#include "roulette_module.hpp"

class SimpleRand{
    private:
    static bool _is_init;
    public:
        SimpleRand(){
            if(!_is_init){
                _is_init = true;
                srand(time(NULL));
            }
        }

        double operator()(double min,double max){

            if(min >= max)
                throw std::invalid_argument("min cannot be greater or equal to max");

            return (max - min)*(rand()/((double)RAND_MAX+1)) + min;

        }
};

bool SimpleRand::_is_init = false;

template<typename T>
class RangedValue{
private:
    double _min;
    double _max;
    T _val;

public:
    RangedValue(double min,double max,T val)
    :_min(min),_max(max),_val(val){

        if(min >= max){
            throw std::invalid_argument("min cannot be greater or equal to max");
        }
    }

    RangedValue(const RangedValue& other)
    :_min(other._min),_max(other._max),_val(other._val){
    }

    RangedValue& operator=(const RangedValue& rhs){
        _min = rhs._min;
        _max = rhs._max;
        _val = rhs._val;
    }

    virtual ~RangedValue(){}

    bool operator<(const double& val)const {

        if(_max < val)
            return true;

        return false;
    }

    bool operator>(const double& val)const {
        if(_min>val)
            return true;

        return false;
    }

    bool operator==(const double& val)const{

        if(val >= _min && val <= _max)
            return true;

        return false;
    }

    virtual double get_min()const{
        return _min;
    }

    virtual double get_max()const{
        return _max;
    }

    virtual T& get_value(){
        return _val;
    }
};

template <typename T, typename ROLLER = SimpleRand>
class Roulette{
private:
    std::vector<RangedValue<T>> _range_list;
    double _last_val;
    ROLLER _rand_gen;
public:
    Roulette()
    :_last_val(0)
    {}

    Roulette(const Roulette& other)
    :_range_list(other._range_list)
    ,_last_val(other._last_value)
    {}

    virtual ~Roulette()
    {}

    typedef typename std::vector<RangedValue<T>>::iterator iterator;

    virtual iterator begin(){ return _range_list.begin(); }
    virtual iterator end(){ return _range_list.end(); }

    virtual void insert(T val, double chance){
        _range_list.push_back(RangedValue<T>(_last_val,_last_val+chance,val));
        _last_val+=chance;
    }

    virtual size_t size()const{
        return _range_list.size();
    }

    virtual T& roll(){

        double roll = _rand_gen(0,_last_val);
        size_t start = 0, fin = _range_list.size()-1,mid;

        while(start <= fin){
            mid = (start+fin)/2;

            if(_range_list[mid] < roll){
                start = mid+1;
                continue;
            }

            if(_range_list[mid] > roll){
                fin = mid-1;
                continue;
            }

            if(_range_list[mid] == roll){
                return _range_list[mid].get_value();
            }

            printf("roll was %lf out of %lf and reached end of loop\n",roll,_last_val);
            throw std::logic_error("reached the end of the loop, not possible");
        }

        printf("roll was %lf and reached after end of loop\n",roll);
        throw std::logic_error("reached the end of the loop, and not found a value");
    }

    virtual bool isEmpty()const{
        return _range_list.empty();
    }
};

extern "C"{


    void* roulette_init(){
        return new Roulette<void*>();
    }

    void roulette_free(void* roulette){
        delete ((Roulette<void*>*)roulette);
    }

    void roulette_insert(void* roulette,void* obj,double chance){
        ((Roulette<void*>*)roulette)->insert(obj,chance);
    }

    void* roulette_roll(void* roulette){
        try{
            return ((Roulette<void*>*)roulette)->roll();
        }catch(std::exception& ex){
            strncpy(roulette_err_msg,ex.what(),ERR_MSG_SIZE);
            return NULL;
        }
    }

    size_t roulette_size(void* roulette){
        return ((Roulette<void*>*)roulette)->size();
    }

    void* roulette_init_iterator(void* roulette){
        return new Roulette<void*>::iterator(((Roulette<void*>*)roulette)->begin());
    }

    void roulette_free_iterator(void* roulette_iterator){
        delete (Roulette<void*>::iterator*)roulette_iterator;
    }

    char roulette_iterator_hasNext(void* roulette, void* roulette_iterator){
        return *((Roulette<void*>::iterator*)roulette_iterator) != ((Roulette<void*>*)roulette)->end();
    }

    void* roulette_deref_iterator(void* roulette_iterator){
        return (*(*((Roulette<void*>::iterator*)roulette_iterator))).get_value();
    }

    void roulette_iterator_adv(void* roulette_iterator){
        (*((Roulette<void*>::iterator*)roulette_iterator))++;
    }

    char roulette_is_empty(void* roulette){
        return ((Roulette<void*>*)roulette)->isEmpty();
    }
}
