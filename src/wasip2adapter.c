#include "env.h"
#include <emcore/emcore.h>

static Callback *cb_tab = NULL;
static uint32_t cb_cap = 0;

static uint32_t *free_stack = NULL;
static uint32_t free_len = 0;
static uint32_t free_cap = 0;

static int ensure_cb_capacity(uint32_t need) {
  if (cb_cap >= need)
    return 1;
  uint32_t new_cap = cb_cap ? cb_cap : 16;
  while (new_cap < need)
    new_cap <<= 1;
  void *p = emlite_realloc(cb_tab, new_cap * sizeof(Callback));
  if (!p)
    return 0;

  memset((uint8_t *)p + cb_cap * sizeof(Callback), 0,
         (new_cap - cb_cap) * sizeof(Callback));
  cb_tab = (Callback *)p;
  cb_cap = new_cap;
  return 1;
}
static int free_push(uint32_t idx) {
  if (free_len == free_cap) {
    uint32_t nc = free_cap ? free_cap << 1 : 16;
    void *p = emlite_realloc(free_stack, nc * sizeof(uint32_t));
    if (!p)
      return 0;
    free_stack = (uint32_t *)p;
    free_cap = nc;
  }
  free_stack[free_len++] = idx;
  return 1;
}
static int free_pop(uint32_t *out) {
  if (!free_len)
    return 0;
  *out = free_stack[--free_len];
  return 1;
}

EMLITE_USED Handle emlite_register_callback(Callback fn) {
  if (!fn)
    return EMLITE_UNDEFINED;
  uint32_t idx;
  if (!free_pop(&idx)) {
    idx = cb_cap;
    if (!ensure_cb_capacity(idx + 1))
      return EMLITE_UNDEFINED;
  }
  cb_tab[idx] = fn;
  return idx; // fidx
}

EMLITE_USED void emlite_unregister_callback(Handle fidx) {
  if (fidx < cb_cap && cb_tab[fidx]) {
    cb_tab[fidx] = NULL;
    (void)free_push(fidx);
  }
}

EMLITE_USED Handle emlite_env_dyncall_apply(uint32_t fidx, uint32_t argv,
                                            uint32_t data) {
  if (fidx >= cb_cap)
    return EMLITE_UNDEFINED;
  Callback fn = cb_tab[fidx];
  if (!fn)
    return EMLITE_UNDEFINED;
  return fn(argv, data);
}

#define FWD0(ret, pub, gen)                                                    \
  EMLITE_USED ret pub(void) { return gen(); }
#define FWD1(ret, pub, gen, T1)                                                \
  EMLITE_USED ret pub(T1 a) { return gen(a); }
#define FWD2(ret, pub, gen, T1, T2)                                            \
  EMLITE_USED ret pub(T1 a, T2 b) { return gen(a, b); }
#define FWD3(ret, pub, gen, T1, T2, T3)                                        \
  EMLITE_USED ret pub(T1 a, T2 b, T3 c) { return gen(a, b, c); }

FWD0(void, emlite_init_handle_table, emlite_env_host_emlite_init_handle_table);

FWD0(Handle, emlite_val_new_array, emlite_env_host_emlite_val_new_array);
FWD0(Handle, emlite_val_new_object, emlite_env_host_emlite_val_new_object);
FWD1(Handle, emlite_val_make_bool, emlite_env_host_emlite_val_make_bool, bool);
FWD1(Handle, emlite_val_make_int, emlite_env_host_emlite_val_make_int, int32_t);
FWD1(Handle, emlite_val_make_uint, emlite_env_host_emlite_val_make_uint,
     uint32_t);
FWD1(Handle, emlite_val_make_bigint, emlite_env_host_emlite_val_make_bigint,
     int64_t);
FWD1(Handle, emlite_val_make_biguint, emlite_env_host_emlite_val_make_biguint,
     uint64_t);
FWD1(Handle, emlite_val_make_double, emlite_env_host_emlite_val_make_double,
     double);

FWD1(bool, emlite_val_get_value_bool, emlite_env_host_emlite_val_get_value_bool,
     Handle);
FWD1(int32_t, emlite_val_get_value_int,
     emlite_env_host_emlite_val_get_value_int, Handle);
FWD1(uint32_t, emlite_val_get_value_uint,
     emlite_env_host_emlite_val_get_value_uint, Handle);
FWD1(int64_t, emlite_val_get_value_bigint,
     emlite_env_host_emlite_val_get_value_bigint, Handle);
FWD1(uint64_t, emlite_val_get_value_biguint,
     emlite_env_host_emlite_val_get_value_biguint, Handle);
FWD1(double, emlite_val_get_value_double,
     emlite_env_host_emlite_val_get_value_double, Handle);

FWD2(void, emlite_val_push, emlite_env_host_emlite_val_push, Handle, Handle);
FWD2(Handle, emlite_val_get, emlite_env_host_emlite_val_get, Handle, Handle);
FWD3(void, emlite_val_set, emlite_env_host_emlite_val_set, Handle, Handle,
     Handle);
FWD2(bool, emlite_val_has, emlite_env_host_emlite_val_has, Handle, Handle);
FWD1(bool, emlite_val_not, emlite_env_host_emlite_val_not, Handle);
FWD1(bool, emlite_val_is_string, emlite_env_host_emlite_val_is_string, Handle);
FWD1(bool, emlite_val_is_number, emlite_env_host_emlite_val_is_number, Handle);
FWD1(bool, emlite_val_is_bool, emlite_env_host_emlite_val_is_bool, Handle);
FWD2(bool, emlite_val_gt, emlite_env_host_emlite_val_gt, Handle, Handle);
FWD2(bool, emlite_val_gte, emlite_env_host_emlite_val_gte, Handle, Handle);
FWD2(bool, emlite_val_lt, emlite_env_host_emlite_val_lt, Handle, Handle);
FWD2(bool, emlite_val_lte, emlite_env_host_emlite_val_lte, Handle, Handle);
FWD2(bool, emlite_val_equals, emlite_env_host_emlite_val_equals, Handle,
     Handle);
FWD2(bool, emlite_val_strictly_equals,
     emlite_env_host_emlite_val_strictly_equals, Handle, Handle);
FWD2(bool, emlite_val_instanceof, emlite_env_host_emlite_val_instanceof, Handle,
     Handle);

FWD2(Handle, emlite_val_construct_new, emlite_env_host_emlite_val_construct_new,
     Handle, Handle);
FWD2(Handle, emlite_val_func_call, emlite_env_host_emlite_val_func_call, Handle,
     Handle);

FWD1(void, emlite_val_inc_ref, emlite_env_host_emlite_val_inc_ref, Handle);
FWD1(void, emlite_val_dec_ref, emlite_env_host_emlite_val_dec_ref, Handle);
FWD1(void, emlite_val_throw, emlite_env_host_emlite_val_throw, Handle);
FWD0(void, emlite_print_object_map, emlite_env_host_emlite_print_object_map);
FWD0(void, emlite_reset_object_map, emlite_env_host_emlite_reset_object_map);

FWD2(Handle, emlite_val_make_callback, emlite_env_host_emlite_val_make_callback,
     Handle, Handle);

EMLITE_USED Handle emlite_val_make_str(const char *s, size_t n) {
  env_string_t tmp = {(uint8_t *)s, n};
  return emlite_env_host_emlite_val_make_str(&tmp);
}

EMLITE_USED Handle emlite_val_make_str_utf16(const uint16_t *p, size_t n) {
  env_list_u16_t tmp = {(uint16_t *)p, n};
  return emlite_env_host_emlite_val_make_str_utf16(&tmp);
}

EMLITE_USED char *emlite_val_get_value_string(Handle h) {
  env_string_t out = {0};
  emlite_env_host_emlite_val_get_value_string(h, &out);
  char *s = (char *)emlite_malloc(out.len + 1);
  if (!s) {
    env_string_free(&out);
    return NULL;
  }
  memcpy(s, out.ptr, out.len);
  s[out.len] = '\0';
  env_string_free(&out);
  return s;
}

EMLITE_USED uint16_t *emlite_val_get_value_string_utf16(Handle h) {
  env_list_u16_t out = {0};
  emlite_env_host_emlite_val_get_value_string_utf16(h, &out);
  uint16_t *p = (uint16_t *)emlite_malloc((out.len + 1) * sizeof(uint16_t));
  if (!p) {
    env_list_u16_free(&out);
    return NULL;
  }
  memcpy(p, out.ptr, out.len * sizeof(uint16_t));
  p[out.len] = 0;
  env_list_u16_free(&out);
  return p;
}

EMLITE_USED char *emlite_val_typeof(Handle h) {
  env_string_t out = {0};
  emlite_env_host_emlite_val_typeof(h, &out);
  char *s = (char *)emlite_malloc(out.len + 1);
  if (!s) {
    env_string_free(&out);
    return NULL;
  }
  memcpy(s, out.ptr, out.len);
  s[out.len] = '\0';
  env_string_free(&out);
  return s;
}

EMLITE_USED Handle emlite_val_obj_call(Handle obj, const char *name, size_t len,
                                       Handle argv) {
  env_string_t m = {(uint8_t *)name, len};
  return emlite_env_host_emlite_val_obj_call(obj, &m, argv);
}

EMLITE_USED bool emlite_val_obj_has_own_prop(Handle obj, const char *prop,
                                             size_t len) {
  env_string_t p = {(uint8_t *)prop, len};
  return emlite_env_host_emlite_val_obj_has_own_prop(obj, &p);
}
