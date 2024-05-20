#ifndef SOLUTION_H
#define SOLUTION_H
#include "spinlock_ece391.h"


typedef struct ps_enter_exit_lock {
  // Fill this out!!!
  spinlock_t* lock;
  //volatile int student_in_line;
  volatile int student_in_lab;
  volatile int ta_in_line;
  volatile int ta_in_lab;
  volatile int profesor_in_line;
  volatile int profesor_in_lab;
} ps_lock;

ps_lock ps_lock_create(spinlock_t *lock);
void professor_enter(ps_lock *ps);
void professor_exit(ps_lock *ps);
void ta_enter(ps_lock *ps);
void ta_exit(ps_lock *ps);
void student_enter(ps_lock *ps);
void student_exit(ps_lock *ps);

#endif /* SOLUTION_H */