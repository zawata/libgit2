/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */

#include "global.h"
#include "pthread.h"

static void *git_unix__threadproc(void *arg)
{
  void *result;
  git_thread *thread = arg;
  GIT_GLOBAL->current_thread = thread;

  if (thread->tls.set_storage_on_thread) {
    thread->tls.set_storage_on_thread(thread->tls.payload);
  }

  result = thread->proc(thread->param);

  if (thread->tls.teardown_storage_on_thread) {
    thread->tls.teardown_storage_on_thread();
  }

  return result;
}

int git_thread_create(
	git_thread *thread,
	void *(*start_routine)(void*),
	void *arg)
{

  thread->proc = start_routine;
  thread->param = arg;
  if (git_custom_tls__init(&thread->tls) < 0)
    return -1;

  return pthread_create(&thread->thread, NULL, git_unix__threadproc, thread);
}

void git_thread_exit(void *value)
{
  git_thread *current_thread = GIT_GLOBAL->current_thread;
  assert(current_thread);
  if (current_thread->tls.teardown_storage_on_thread)
    current_thread->tls.teardown_storage_on_thread();

  return pthread_exit(value);
}
