#ifndef ASSIGNMENT1_H_DEFINED
#define ASSIGNMENT1_H_DEFINED

#define MAX_AVAIL_LEN 50
#define MAX_NAME_LEN 50
#define MAX_APPLIC_PER_DAY 10
#define MAX_APPLICANTS 50

#define MAX_WORKERS_PER_BUS 5
#define MAX_BUSES 2

typedef struct People {
    char name[MAX_NAME_LEN+1];
    char avail[MAX_AVAIL_LEN+1];
} People;

void add_applicant();
void delete_applicant();
void modify_applicant();
void print_applicants();
void load_applicants();

void bus_handler(int signum);
void start_bus();

#endif