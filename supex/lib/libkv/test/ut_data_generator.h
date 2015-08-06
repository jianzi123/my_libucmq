#ifndef UT_DATA_GENERATOR_H_
#define UT_DATA_GENERATOR_H_










typedef struct case_data {
        const char *ptr;
        unsigned int ptrlen;
}case_data_t;


void case_data_release(struct case_data *dc);
case_data_t* generator_rand_str(unsigned int len);
case_data_t* generator_rand_bytes(unsigned int len);
unsigned int gen_rand_num(unsigned int max);

#endif
