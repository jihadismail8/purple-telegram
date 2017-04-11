#include "telegram-purple.h"

#ifdef HAVE_LIBWEBP
#include <webp/decode.h>
#endif

PurpleAccount *tls_get_pa (struct tgl_state *TLS) {
  return tls_get_data (TLS)->pa;
}

PurpleConnection *tls_get_conn (struct tgl_state *TLS) {
  return tls_get_data (TLS)->gc;
}

connection_data *tls_get_data (struct tgl_state *TLS) {
  return TLS->ev_base;
}

int tls_get_media_threshold (struct tgl_state *TLS) {
  return purple_account_get_int (tls_get_pa (TLS),
             TGP_KEY_MEDIA_SIZE, TGP_DEFAULT_MEDIA_SIZE) << 10;
}

connection_data *gc_get_data (PurpleConnection *gc) {
  return purple_connection_get_protocol_data (gc);
}

struct tgl_state *gc_get_tls (PurpleConnection *gc) {
  return ((connection_data *)purple_connection_get_protocol_data (gc))->TLS;
}

connection_data *pa_get_data (PurpleAccount *pa) {
  return purple_connection_get_protocol_data (purple_account_get_connection (pa));
}

connection_data *pbn_get_data (PurpleBlistNode *node) {
  if (PURPLE_BLIST_NODE_IS_CHAT (node)) {
    return pa_get_data (purple_chat_get_account ((PurpleChat *) node));
  }
  if (PURPLE_BLIST_NODE_IS_BUDDY (node)) {
    return pa_get_data (purple_buddy_get_account ((PurpleBuddy *) node));
  }
  return NULL;
}

int p2tgl_status_is_present (PurpleStatus *status) {
  const char *name = purple_status_get_id (status);
  return !(strcmp (name, "unavailable") == 0 || strcmp (name, "away") == 0);
}

void tgp_chat_got_in (struct tgl_state *TLS, tgl_peer_t *chat, tgl_peer_id_t from, const char *message,
    int flags, time_t when) {
  g_return_if_fail(chat);
  if (tgp_chat_show (TLS, chat)) {
    
    // channel messages in non-megagroups are always sent by the channel itself
    if (tgl_get_peer_type (chat->id) == TGL_PEER_CHANNEL && !(chat->channel.flags & TGLCHF_MEGAGROUP)) {
      from = chat->id;
    }
    
    serv_got_chat_in (tls_get_conn (TLS), tgl_get_peer_id (chat->id), tgp_blist_lookup_purple_name (TLS, from),
        flags, message, when);
  } else {
    g_warn_if_reached();
  }
}

void p2tgl_got_im_combo (struct tgl_state *TLS, tgl_peer_id_t who, const char *msg, int flags, time_t when) {
  connection_data *conn = TLS->ev_base;
  
  if (flags & PURPLE_MESSAGE_SYSTEM) {
    tgp_msg_special_out (TLS, msg, who, flags & PURPLE_MESSAGE_NO_LOG);
    return;
  }
  
  /*
    Outgoing messages are not well supported in different libpurple clients,
    purple_conv_im_write should have the best among different versions. Unfortunately
    this causes buggy formatting in Adium, so we don't use this workaround in that case.
   
    NOTE: Outgoing messages will not work in Adium <= 1.6.0, there is no way to print outgoing
    messages in those versions at all.
  */
#ifndef __ADIUM_
  if (flags & PURPLE_MESSAGE_SEND) {
    PurpleConversation *conv = p2tgl_find_conversation_with_account (TLS, who);
    if (!conv) {
      conv = purple_conversation_new (PURPLE_CONV_TYPE_IM, tls_get_pa (TLS),
          tgp_blist_lookup_purple_name (TLS, who));
    }
    purple_conv_im_write (purple_conversation_get_im_data (conv), tgp_blist_lookup_purple_name (TLS, who),
        msg, PURPLE_MESSAGE_SEND, when);
    return;
  }
#endif
  serv_got_im (conn->gc, tgp_blist_lookup_purple_name (TLS, who), msg, flags, when);
}

PurpleConversation *p2tgl_find_conversation_with_account (struct tgl_state *TLS, tgl_peer_id_t peer) {
  int type = PURPLE_CONV_TYPE_IM;
  if (tgl_get_peer_type (peer) == TGL_PEER_CHAT || tgl_get_peer_type (peer) == TGL_PEER_CHANNEL) {
    type = PURPLE_CONV_TYPE_CHAT;
  }
  PurpleConversation *conv = purple_find_conversation_with_account (type,
      tgp_blist_lookup_purple_name (TLS, peer), tls_get_pa (TLS));
  return conv;
}

void p2tgl_prpl_got_user_status (struct tgl_state *TLS, tgl_peer_id_t user, struct tgl_user_status *status) {
  connection_data *data = TLS->ev_base;
  
  if (status->online == 1) {
    purple_prpl_got_user_status (tls_get_pa (TLS), tgp_blist_lookup_purple_name (TLS, user), "available", NULL);
  } else {
    debug ("%d: when=%d", tgl_get_peer_id (user), status->when);
    if (tgp_time_n_days_ago (
          purple_account_get_int (data->pa, TGP_KEY_INACTIVE_DAYS_OFFLINE, TGP_DEFAULT_INACTIVE_DAYS_OFFLINE)) > status->when && status->when) {
      debug ("offline");
      purple_prpl_got_user_status (tls_get_pa (TLS), tgp_blist_lookup_purple_name (TLS, user), "offline", NULL);
    } else {
      debug ("mobile");
      purple_prpl_got_user_status (tls_get_pa (TLS), tgp_blist_lookup_purple_name (TLS, user), "mobile", NULL);
    }
  }
}

void p2tgl_conv_add_user (struct tgl_state *TLS, PurpleConversation *conv, int user, char *message, int flags,
    int new_arrival) {
  const char *name = tgp_blist_lookup_purple_name (TLS, TGL_MK_USER (user));
  g_return_if_fail (name);
  purple_conv_chat_add_user (purple_conversation_get_chat_data (conv), name, message, flags, new_arrival);
}

int p2tgl_imgstore_add_with_id (const char* filename) {
  gchar *data = NULL;
  size_t len;
  GError *err = NULL;
  g_file_get_contents (filename, &data, &len, &err);
  
  int id = purple_imgstore_add_with_id (data, len, NULL);
  return id;
}

int p2tgl_imgstore_add_with_id_raw (const unsigned char *raw_bgra, unsigned width, unsigned height) {
  // Heavily inspired by: https://github.com/EionRobb/pidgin-opensteamworks/blob/master/libsteamworks.cpp#L113
  const unsigned char tga_header[] = {
      // No ID; no color map; uncompressed true color
      0,0,2,
      // No color map metadata
      0,0,0,0,0,
      // No offsets
      0,0,0,0,
      // Dimensions
      width&0xFF,(width/256)&0xFF,height&0xFF,(height/256)&0xFF,
      // 32 bits per pixel
      32,
      // "Origin in upper left-hand corner"
      32};
  // Will be owned by libpurple imgstore, which uses glib functions for managing memory
  const unsigned tga_len = sizeof(tga_header) + width * height * 4;
  unsigned char *tga = g_malloc(tga_len);
  memcpy(tga, tga_header, sizeof(tga_header));
  // From the documentation: "The 4 byte entry contains 1 byte each of blue, green, red, and attribute."
  memcpy(tga + sizeof(tga_header), raw_bgra, width * height * 4);
  return purple_imgstore_add_with_id (tga, tga_len, NULL);
}

#ifdef HAVE_LIBWEBP

static const int MAX_W = 256;
static const int MAX_H = 256;

int p2tgl_imgstore_add_with_id_webp (const char *filename) {
  const uint8_t *data = NULL;
  size_t len;
  GError *err = NULL;
  g_file_get_contents (filename, (gchar **) &data, &len, &err);
  if (err) { warning ("cannot open file %s: %s.", filename, err->message); return 0; }
  
  // downscale oversized sticker images displayed in chat, otherwise it would harm readabillity
  WebPDecoderConfig config;
  WebPInitDecoderConfig (&config);
  if (WebPGetFeatures(data, len, &config.input) != VP8_STATUS_OK) {
    warning ("error reading webp bitstream: %s", filename);
    g_free ((gchar *)data);
    return 0;
  }

  config.options.use_scaling = 0;
  config.options.scaled_width = config.input.width;
  config.options.scaled_height = config.input.height;
  if (config.options.scaled_width > MAX_W || config.options.scaled_height > MAX_H) {
    const float max_scale_width = MAX_W * 1.0f / config.options.scaled_width;
    const float max_scale_height = MAX_H * 1.0f / config.options.scaled_height;
    if (max_scale_width < max_scale_height) {
      // => the width is most limiting
      config.options.scaled_width = MAX_W;
      // Can't use ' *= ', because we need to do the multiplication in float
      // (or double), and only THEN cast back to int.
      config.options.scaled_height = (int) (config.options.scaled_height * max_scale_width);
    } else {
      // => the height is most limiting
      config.options.scaled_height = MAX_H;
      // Can't use ' *= ', because we need to do the multiplication in float
      // (or double), and only THEN cast back to int.
      config.options.scaled_width = (int) (config.options.scaled_width * max_scale_height);
    }
    config.options.use_scaling = 1;
  }
  config.output.colorspace = MODE_BGRA;
  if (WebPDecode(data, len, &config) != VP8_STATUS_OK) {
    warning ("error decoding webp: %s", filename);
    g_free ((gchar *)data);
    return 0;
  }
  g_free ((gchar *)data);
  const uint8_t *decoded = config.output.u.RGBA.rgba;

  // convert and add
  int imgStoreId = p2tgl_imgstore_add_with_id_raw(decoded, config.options.scaled_width, config.options.scaled_height);
  WebPFreeDecBuffer (&config.output);
  return imgStoreId;
}
#endif
