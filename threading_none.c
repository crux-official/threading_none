// The threading PRIMITIVE's declared ABSENCE: the pure-logic composition.
// No thread is ever created — Thread_Create runs the function INLINE on the
// calling thread and returns an already-finished handle (so a later Join is a
// no-op), and every hardware fact reads 0 (the concurrency ceiling is 0, the
// fact a worker pool keys its serial degradation off). This is the mode where
// a cell composes threading but wants NO parallelism at all — not even the
// web driver's named degrade path — so the binary carries no pthread code and
// no -pthread flags.
//
// No concern bit is registered: Engine_Supports(CRUX_CONCERN_THREADING) is
// false for it (the honest absence — the cell has no threading capability).
// The served no-ops still exist so a static consumer of the concern's headers
// links against the honest absence, and so the thread_pool standalone (which
// keys its serial path off Thread_Concurrency == 0) degrades correctly when
// this is the composed implementation.
#include "crux_threading.h"
#include "extension.h"
#include "kernel_types.h"
#include "threading_api.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// The opaque handle: a shared sentinel marking an already-finished (inline)
// unit of work. The none driver is the ZERO-COST absence — it makes no
// allocation, so the handle is a static token the caller passes back to
// Join/Detach, which ignore it (there is no real thread to wait on or reclaim).
struct CruxThread {
  int _pad; // the opaque handle needs a field (C has no empty struct); never read
};

static struct CruxThread s_none_finished = {0};

CruxThread *Thread_Create(void (*func)(void *arg), void *arg) {
  if (func == NULL) {
    return NULL;
  }
  func(arg);
  return &s_none_finished; // an already-finished handle (the sentinel; Join/Detach ignore it)
}

void Thread_Join(CruxThread *thread) {
  (void)thread; // already finished (it ran inline); the sentinel is shared, nothing to reclaim
}

void Thread_Detach(CruxThread *thread) {
  (void)thread; // nothing to detach (it ran inline); the sentinel is shared, nothing to reclaim
}

int Thread_Count_Logical(void) {
  return 0; // the honest absence: no threads, no cores to report
}

int Thread_Count_Physical(void) {
  return 0;
}

int Thread_Concurrency(void) {
  return 0; // the ceiling is 0 — a pool keyed on this runs serial
}

// The served struct: the threading concern's contract, filled with this
// driver's no-op function addresses. Valid from load, before Construct.
static ThreadingAPI g_threading_api = {
    .version = {1, 0, 0},
    .Thread_Create = Thread_Create,
    .Thread_Join = Thread_Join,
    .Thread_Detach = Thread_Detach,
    .Thread_Count_Logical = Thread_Count_Logical,
    .Thread_Count_Physical = Thread_Count_Physical,
    .Thread_Concurrency = Thread_Concurrency,
};

// The extension interface. The NONE driver registers NO concern bit (the cell
// has no threading capability) — Engine_Supports sees the absence.
// GetSpecificAPI still serves the empty contract so the registry walk finds a
// valid served struct.
static bool threading_none_construct(void) {
  return true;
}

static void *threading_none_get_api(void) {
  return &g_threading_api;
}

ExtensionInterface g_extension_threading_none = {
    "threading_none",
    {1, 0, 0},
    threading_none_construct,
    NULL,
    NULL,
    NULL,
    NULL,
    threading_none_get_api,
};
