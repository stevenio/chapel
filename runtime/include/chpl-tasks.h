#ifndef _chpl_tasks_h_
#define _chpl_tasks_h_

#ifndef LAUNCHER

#include <stdint.h>
#include "chpltypes.h"

#ifdef CHPL_TASKS_MODEL_H
#include CHPL_TASKS_MODEL_H
#endif


// Sync variables

void      chpl_sync_lock(chpl_sync_aux_t *);
void      chpl_sync_unlock(chpl_sync_aux_t *);
void      chpl_sync_waitFullAndLock(chpl_sync_aux_t *,
                                       int32_t, chpl_string);
void      chpl_sync_waitEmptyAndLock(chpl_sync_aux_t *,
                                        int32_t, chpl_string);
void      chpl_sync_markAndSignalFull(chpl_sync_aux_t *);     // and unlock
void      chpl_sync_markAndSignalEmpty(chpl_sync_aux_t *);    // and unlock
chpl_bool chpl_sync_isFull(void *, chpl_sync_aux_t *, chpl_bool);
void      chpl_sync_initAux(chpl_sync_aux_t *);
void      chpl_sync_destroyAux(chpl_sync_aux_t *);


// Single variables (currently a synonym for syncs)

typedef chpl_sync_aux_t chpl_single_aux_t;

static ___always_inline
void chpl_single_lock(chpl_sync_aux_t * s) { chpl_sync_lock(s); }
static ___always_inline
void chpl_single_unlock(chpl_sync_aux_t * s) { chpl_sync_unlock(s); }
static ___always_inline
void chpl_single_waitFullAndLock(chpl_sync_aux_t * s,
                                 int32_t lineno, chpl_string filename) {
  chpl_sync_waitFullAndLock(s,lineno,filename);
}
static ___always_inline
void chpl_single_markAndSignalFull(chpl_sync_aux_t * s) {
  chpl_sync_markAndSignalFull(s);
}
static ___always_inline
chpl_bool chpl_single_isFull(void *val_ptr, chpl_sync_aux_t *s,
                                           chpl_bool simple_sync_var) {
  return chpl_sync_isFull(val_ptr, s, simple_sync_var);
}
static ___always_inline
void chpl_single_initAux(chpl_sync_aux_t * s) { chpl_sync_initAux(s); }
static ___always_inline
void chpl_single_destroyAux(chpl_sync_aux_t * s) { chpl_sync_destroyAux(s); }


// Tasks

//
// chpl_task_init() is called by the main task on each locale to initialize
//   the tasking layer
//
void chpl_task_init(void);
void chpl_task_exit(void);        // called by the main task

//
// Have the tasking layer create a dedicated task to help the
// communication layer by running function 'fn' with argument 'arg'.
// This task should be quite dedicated (e.g., get its own system
// thread) in order to be responsive and not be held up by other
// user-level tasks. returns 0 on success, nonzero on failure.
//
int chpl_task_createCommTask(chpl_fn_p fn, void* arg);

//
// Have the tasking layer call the 'chpl_main' function pointer
// representing the entry point for the user's Chapel code.  This
// can either be done by invoking the function directly or by creating
// a task that evaluates the function.
//
void chpl_task_callMain(void (*chpl_main)(void));

//
// The following is an optional callback into the tasking layer from
// the main task indicating that the standard internal modules have
// been initialized.  It gives the tasking layer the ability to make
// use of functionality in the internal modules (like the task
// tracking table) which are not yet available in
// chpl_task_callMain().
//
#ifndef CHPL_TASK_STD_MODULES_INITIALIZED
#define CHPL_TASK_STD_MODULES_INITIALIZED()
#endif


typedef struct chpl_task_list* chpl_task_list_p;

//
// Task list processing.  These are called by the compiler-emitted
// code for all parallel constructs.  addToTaskList() is called for
// each task and builds a list of all of them.  processTaskList()
// actually adds the tasks to the task pool.  executeTasksInList()
// makes sure all the tasks have at least started.  freeTaskList()
// just reclaims space associated with the list.
//
void chpl_task_addToTaskList(
         chpl_fn_int_t,      // function to call for task
         void*,              // argument to the function
         c_sublocid_t,       // desired sublocale
         chpl_task_list_p*,  // task list
         c_nodeid_t,         // locale (node) where task list resides
         chpl_bool,          // is begin{} stmt?  (vs. cobegin or coforall)
         int,                // line at which function begins
         chpl_string);       // name of file containing functions
void chpl_task_processTaskList(chpl_task_list_p);
void chpl_task_executeTasksInList(chpl_task_list_p);
void chpl_task_freeTaskList(chpl_task_list_p);

//
// Launch a task that is the logical continuation of some other task,
// but on a different locale.  This is used to invoke the body of an
// "on" statement.
//
void chpl_task_startMovedTask(chpl_fn_p,          // function to call
                              void*,              // function arg
                              c_sublocid_t,       // desired sublocale
                              chpl_taskID_t,      // task identifier
                              chpl_bool);         // serial state

//
// Get and set the current task's sublocale.  Setting the sublocale
// will actually move the task, if the specified sublocale differs
// from the current one.
//
c_sublocid_t chpl_task_getSubLoc(void);
void chpl_task_setSubLoc(c_sublocid_t);

//
// Get the sublocale the caller specified at the time the task was created.
//
c_sublocid_t chpl_task_getRequestedSubLoc(void);

//
// Get ID.
//
chpl_taskID_t chpl_task_getId(void);

//
// Yield.
//
void chpl_task_yield(void);

//
// Suspend.
//
void chpl_task_sleep(int);

//
// Get and set dynamic serial state.
//
chpl_bool chpl_task_getSerial(void);
void      chpl_task_setSerial(chpl_bool);

//
// Returns the the number of sublocales the tasking layer knows about,
// within the span of hardware it is managing tasks on.
//
c_sublocid_t chpl_task_getNumSubLocales(void);

//
// returns the value of the call stack size limit being used in
// practice; the value returned may potentially differ from one locale
// to the next
//
uint64_t chpl_task_getCallStackSize(void);

//
// returns the number of tasks that are ready to run on the current locale,
// not including any that have already started running.
//
uint32_t chpl_task_getNumQueuedTasks(void);

//
// returns the number of tasks that are running on the current locale,
// including any that may be blocked waiting for something.
// Note that the value returned could be larger than the limit on the maximum
// number of threads, since a thread could be "suspended," particularly if it
// is waiting at the end of a cobegin, e.g.  In this case, it could be
// executing a task inside the cobegin, so in effect the same thread would be
// executing more than one task.
//
uint32_t chpl_task_getNumRunningTasks(void);

//
// returns the number of tasks that are blocked waiting on a sync or single
// variable.
// Note that this information may only available if the program is run with
// the -b switch, which enables block reporting and deadlock detection.
// If this switch is not specified, -1 may be returned.
//
int32_t chpl_task_getNumBlockedTasks(void);


// Threads

//
// returns the total number of threads that currently exist, whether running,
// blocked, or idle
//
uint32_t chpl_task_getNumThreads(void);

//
// returns the number of threads that are currently idle
//
uint32_t chpl_task_getNumIdleThreads(void);


//
// This gets any per-locale thread count specified in the environment.
// It is common to all tasking implementations and so is implemented
// in runtime/src/chpl-tasks.c.
//
int32_t chpl_task_getenvNumThreadsPerLocale(void);

//
// This gets any call stack size specified in the environment.  It
// is common to all tasking implementations and so is implemented
// in runtime/src/chpl-tasks.c.
//
int64_t chpl_task_getenvCallStackSize(void);

#else // LAUNCHER

typedef void chpl_sync_aux_t;
typedef chpl_sync_aux_t chpl_single_aux_t;
#define chpl_task_exit()

#endif // LAUNCHER

#endif
