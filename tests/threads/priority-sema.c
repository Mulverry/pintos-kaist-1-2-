/* Tests that the highest-priority thread waiting on a semaphore
   is the first to wake up. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

static thread_func priority_sema_thread;
static struct semaphore sema;

void test_priority_sema(void)
{
  int i;

  /* This test does not work with the MLFQS. */
  ASSERT(!thread_mlfqs);

  sema_init(&sema, 0);
  thread_set_priority(PRI_MIN);
  for (i = 0; i < 10; i++)
  {
    int priority = PRI_DEFAULT - (i + 3) % 10 - 1;
    char name[16];
    snprintf(name, sizeof name, "priority %d", priority);
    thread_create(name, priority, priority_sema_thread, NULL);
    // printf("semaphore value : %d\n", sema.value);
  }
  // struct list_elem *p = sema.waiters.head.next;
  // printf("\n");
  // printf("waiters의 원소 : ");
  // while (1)
  // {
  //   struct thread *t = list_entry(p, struct thread, elem);
  //   printf(" %s,", t->name);
  //   p = p->next;
  //   if (p == NULL)
  //     break;
  // // }
  // printf("\n");
  // printf("\n");
  for (i = 0; i < 10; i++)
  {
    // printf("before sema_up semaphore value : %d\n", sema.value);
    sema_up(&sema);
    // printf("after sema_up semaphore value : %d\n", sema.value);
    msg("Back in main thread.");
  }
}

static void
priority_sema_thread(void *aux UNUSED)
{

  sema_down(&sema);

  msg("Thread %s woke up.", thread_name());
}
