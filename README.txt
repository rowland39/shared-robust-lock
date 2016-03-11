This code demonstrates how to create a shared robust pthread lock between
separate processes. The pthread lock has to be stored in shared memory between
the processes, and it should be robust in case any of the processes die. We
need to check the return value from pthread_lock() for the value EOWNERDEAD
from errno.h. If that is returned, we have successfully acquired the lock, but
we must fix its consistency with pthread_mutex_consistency().

