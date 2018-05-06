#ifdef __cplusplus
extern "C" {
#endif
    #define ERR_MSG_SIZE 1000
    static char roulette_err_msg[ERR_MSG_SIZE+1];

    void* roulette_init(void);
    void roulette_free(void* roulette);
    void roulette_insert(void* roulette,void* obj,double chance);
    void* roulette_roll(void* roulette);
    size_t roulette_size(void* roulette);
    void* roulette_init_iterator(void* roulette);
    void roulette_free_iterator(void* roulette_iterator);
    char roulette_iterator_hasNext(void* roulette, void* roulette_iterator);
    void* roulette_deref_iterator(void* roulette_iterator);
    void roulette_iterator_adv(void* roulette_iterator);
    char roulette_is_empty(void* roulette);

#ifdef __cplusplus
}
#endif
