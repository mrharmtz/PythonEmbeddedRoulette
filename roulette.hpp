#ifndef __ROULETTE_HPP__
#define __ROULETTE_HPP__


#include <stdexcept>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <cstring>
#include <initializer_list>
#include <utility>


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

        double operator()(double min,double max)const{

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
		
		return *this;
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

    virtual T const& get_value()const {
        return _val;
    }
};

template <typename T, typename ROLLER = SimpleRand>
class Roulette{
private:
    std::vector<RangedValue<T> > _range_list;
    double _last_val;
    ROLLER _rand_gen;

protected:
    virtual size_t _find_index(double roll)const {
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
                return mid;
            }

            throw std::logic_error("reached the end of the loop, not possible");
        }

        throw std::logic_error("reached the end of the loop, and not found a value");
    }

public:

    Roulette()
    :_last_val(0)
    {}
    
    Roulette(const std::initializer_list<std::pair<T, double> >& list)
    :_last_val(0)
    {
        for(const auto& val : list){
            double shadow = _last_val;
            _range_list.push_back(RangedValue<T>(shadow, _last_val+=val.second, val.first));
        }
    }

    Roulette(const Roulette& other)
    :_range_list(other._range_list)
    ,_last_val(other._last_val)
    {}

    virtual ~Roulette()
    {}

    typedef typename std::vector<RangedValue<T> >::iterator iterator;

    virtual iterator begin(){ return _range_list.begin(); }
    virtual iterator end(){ return _range_list.end(); }

    virtual void insert(T val, double chance){
        double shadow = _last_val;
        _range_list.push_back(RangedValue<T>(shadow, (_last_val+=chance), val));
    }

    virtual size_t size()const{
        return _range_list.size();
    }


    virtual T const & roll() const{
        return _range_list[_find_index(_rand_gen(0,_last_val))].get_value();
    }

    virtual T& roll(){
        return _range_list[_find_index(_rand_gen(0,_last_val))].get_value();
    }

    virtual bool isEmpty()const{
        return _range_list.empty();
    }
};


#endif //__ROULETTE_HPP__