#ifndef SOLUTION_H
#define SOLUTION_H
#include "spinlock_ece391.h"


typedef struct ps_enter_exit_lock {
  // Fill this out!!!
  spinlock_t* lock;
  volatile int prof_count;
  volatile int ta_count;
  volatile int student_count;
  volatile int tot_count;
} ps_lock;

ps_lock ps_lock_create(spinlock_t *lock);
void professor_enter(ps_lock *ps);
void professor_exit(ps_lock *ps);
void ta_enter(ps_lock *ps);
void ta_exit(ps_lock *ps);
void student_enter(ps_lock *ps);
void student_exit(ps_lock *ps);

#endif /* SOLUTION_H */
