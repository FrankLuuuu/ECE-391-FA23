#include "spinlock_ece391.h"
#include "solution.h"
#include <bits/types.h>

#include <stdio.h>


ps_lock ps_lock_create(spinlock_t *lock) {
  // Fill this out!!!

  // initialize the spinlock and its variables
  
  volatile ps_lock pslock;
  pslock.lock = lock;
  //pslock.student_in_line = 0;
  pslock.student_in_lab = 0;
  pslock.ta_in_line = 0;
  pslock.ta_in_lab = 0;
  pslock.profesor_in_line = 0;
  pslock.profesor_in_lab = 0;
  spinlock_init_ece391(pslock.lock);

  //printf("create");

  return pslock;
}

void professor_enter(ps_lock *ps) {
  // Fill this out!!!

  // add the prof to the line
  spinlock_lock_ece391(ps->lock);
  ps->profesor_in_line++;
  //spinlock_unlock_ece391(ps->lock);

  while(!(ps->profesor_in_lab < 20 && ps->student_in_lab == 0 && ps->ta_in_lab == 0)) {
    spinlock_unlock_ece391(ps->lock);

    spinlock_lock_ece391(ps->lock);
  }

  ps->profesor_in_line--;
  ps->profesor_in_lab++;

  spinlock_unlock_ece391(ps->lock);
  return;
}

void professor_exit(ps_lock *ps) {
  // Fill this out!!!
  spinlock_lock_ece391(ps->lock);

  // check if there is any prof in the lab
  // remove one prof
  if(ps->profesor_in_lab <= 1) {
    ps->profesor_in_lab = 1;
  }
  ps->profesor_in_lab--;

  spinlock_unlock_ece391(ps->lock);
}

void ta_enter(ps_lock *ps) {
  // Fill this out!!!

  // add the ta to the line
  spinlock_lock_ece391(ps->lock);
  ps->ta_in_line++;
  //spinlock_unlock_ece391(ps->lock);

  while(!(ps->ta_in_lab + ps->student_in_lab < 20 && ps->profesor_in_lab == 0 && ps->profesor_in_line == 0)) {
    spinlock_unlock_ece391(ps->lock);

    spinlock_lock_ece391(ps->lock);
  }

  ps->ta_in_line--;
  ps->ta_in_lab++;

  spinlock_unlock_ece391(ps->lock);
  return;
}

void ta_exit(ps_lock *ps) {
  // Fill this out!!!
  spinlock_lock_ece391(ps->lock);

  if(ps->ta_in_lab <= 1) {
    ps->ta_in_lab = 1;
  }
  ps->ta_in_lab--;

  spinlock_unlock_ece391(ps->lock);
}

void student_enter(ps_lock *ps) {
  // Fill this out!!!

  // add the prof to the line
  spinlock_lock_ece391(ps->lock);
  //ps->student_in_line++;
  //spinlock_unlock_ece391(ps->lock);

  while(!(ps->ta_in_lab + ps->student_in_lab < 20 && ps->profesor_in_lab == 0 && ps->profesor_in_line == 0 && ps->ta_in_line == 0)) {
    spinlock_unlock_ece391(ps->lock);

    spinlock_lock_ece391(ps->lock);
  }

  //ps->student_in_line--;
  ps->student_in_lab++;

  spinlock_unlock_ece391(ps->lock);
  return;
}

void student_exit(ps_lock *ps) {
  // Fill this out!!!
  spinlock_lock_ece391(ps->lock);

  if(ps->student_in_lab <= 1) {
    ps->student_in_lab = 1;
  }
  ps->student_in_lab--;

  spinlock_unlock_ece391(ps->lock);
}
