
#include <tgl.h>
#include <stdlib.h>
#include <glib.h>
#include <eventloop.h>

struct tgl_timer {
  struct tgl_state *TLS;
  void (*cb)(struct tgl_state *, void *);
  void *arg;
  int fd;
};

static int timer_alarm (gpointer arg) {
  struct tgl_timer *t = arg;
  t->fd = -1;
  t->cb (t->TLS, t->arg);
  return FALSE;
}

static struct tgl_timer *tgl_timer_alloc (struct tgl_state *TLS, void (*cb)(struct tgl_state *TLS, void *arg), void *arg) {
  struct tgl_timer *t = malloc (sizeof (*t));
  t->TLS = TLS;
  t->cb = cb;
  t->arg = arg;
  t->fd = -1;
  return t;
}

static void tgl_timer_delete (struct tgl_timer *t);

static void tgl_timer_insert (struct tgl_timer *t, double p) {
  tgl_timer_delete (t);
  if (p < 0) { p = 0; }
  if (p < 1) {
    t->fd = purple_timeout_add (1000 * p, timer_alarm, t);
  } else {
    t->fd = purple_timeout_add_seconds (p, timer_alarm, t);
  }
}

static void tgl_timer_delete (struct tgl_timer *t) {
  if (t->fd >= 0) {
    purple_timeout_remove (t->fd);
    t->fd = -1;
  }
}

static void tgl_timer_free (struct tgl_timer *t) {
  if (t->fd >= 0) {
    tgl_timer_delete (t);
  }
  free (t);
}

struct tgl_timer_methods tgp_timers = {
  .alloc = tgl_timer_alloc, 
  .insert = tgl_timer_insert,
  .remove = tgl_timer_delete,
  .free = tgl_timer_free
};
