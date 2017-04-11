
#include "telegram-purple.h"

const char *format_time (time_t date) {
  // TODO: Inline this function for better readability?
  struct tm *datetime = localtime(&date);
  // This should be the language's timestamp format. This is preceded by a colon.
  return purple_utf8_strftime (_("%d.%m.%Y %H:%M"), datetime);
}

char *tgp_format_img (int imgstore) {
  const char *br = "<br>";
  
  // <br>'s look ugly in Adium, but no <br> will look ugly in Pidgin
#ifdef __ADIUM_
  br = "";
#endif
  return g_strdup_printf ("%s<img id=\"%u\">", br, imgstore);
}

char *tgp_format_user_status (struct tgl_user_status *status) {
  char *when;
  switch (status->online) {
    case -1:
      when = g_strdup_printf ("%s", format_time (status->when));
      break;
    case -2:
      // This is preceded by a colon.
      when = g_strdup (_("recently"));
      break;
    case -3:
      // This is preceded by a colon.
      when = g_strdup (_("last week"));
      break;
    case -4:
      // This is preceded by a colon.
      when = g_strdup (_("last month"));
      break;
    default:
      // This is preceded by a colon. It refers to a point on time.
      when = g_strdup (_("unknown"));
      break;
  }
  return when;
}

int str_not_empty (const char *string) {
  return string && string[0] != '\0';
}

int tgp_outgoing_msg (struct tgl_state *TLS, struct tgl_message *M) {
  return (M->flags & TGLMF_SESSION_OUTBOUND);
}

int tgp_our_msg (struct tgl_state *TLS, struct tgl_message *M) {
  return tgl_get_peer_id (TLS->our_id) == tgl_get_peer_id (M->from_id);
}

tgl_peer_t *tgp_encr_chat_get_partner (struct tgl_state *TLS, struct tgl_secret_chat *chat) {
  return tgl_peer_get (TLS, TGL_MK_USER(chat->admin_id == tgl_get_peer_id (TLS->our_id) ? chat->user_id : chat->admin_id));
}

long tgp_time_n_days_ago (int days) {
  return time(NULL) - 24 * 3600 * days;
};

void tgp_g_queue_free_full (GQueue *queue, GDestroyNotify free_func) {
  void *entry;
  
  while ((entry = g_queue_pop_head(queue))) {
    free_func (entry);
  }
  g_queue_free (queue);
}

void tgp_g_list_free_full (GList *list, GDestroyNotify free_func) {
  if (list) {
    g_list_foreach (list, (GFunc)free_func, NULL);
    g_list_free (list);
  }
}

const char *tgp_mime_to_filetype (const char *mime) {
  int len = (int) strlen (mime);
  int i;
  for (i = 0; i < len - 1; i ++) {
    if (mime[i] == '/') {
      return mime + i + 1;
    }
  }
  return NULL;
}

int tgp_startswith (const char *str, const char *with) {
  if (! str || !with) {
    return FALSE;
  }
  int slen = strlen (str), wlen = strlen (with);
  if (wlen > slen) {
    return FALSE;
  }
  while (*with) if (*str++ != *with++) {
    return FALSE;
  }
  return TRUE;
}

void tgp_replace (char *string, char what, char with) {
  char *p = string;
  while (*(p ++)) {
    if (*p == what) {
      *p = with;
    }
  }
}
