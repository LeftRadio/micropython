

#include "py/obj.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "extmod/bricks/brickbase.h"


typedef struct _timer_obj_t {
    mp_obj_base_t base;
    const char *name;
    size_t name_len;
    bool enable;
    bool binded;
    mp_int_t start;
    mp_uint_t on;
    mp_uint_t off;
    mp_uint_t period;
    bool out_state;
} timer_obj_t;

const mp_obj_type_t timer_type;


STATIC void timer_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    timer_obj_t *self = self_in;
    mp_printf(print, "Scenaries UserTimer(%p)", &self->base);
}


STATIC void timer_init_helper(timer_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_name, ARG_enable, ARG_binded, ARG_start, ARG_on, ARG_off, ARG_period, ARG_seconds, ARG_interval };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_name, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_enable, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_binded, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_start, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_on, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_off, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_period, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if (args[ARG_name].u_obj != MP_OBJ_NULL) {
        self->name = mp_obj_str_get_data(args[ARG_name].u_obj, &self->name_len);
    }
    if (args[ARG_enable].u_obj != MP_OBJ_NULL) {
        self->enable = mp_obj_is_true(args[ARG_enable].u_obj);
    }
    if (args[ARG_binded].u_obj != MP_OBJ_NULL) {
        self->binded = mp_obj_is_true(args[ARG_binded].u_obj);
    }
    if (args[ARG_start].u_int != -1) {
        self->start = args[ARG_start].u_int;
    }
    if (args[ARG_on].u_int != -1) {
        self->on = args[ARG_on].u_int;
    }
    if (args[ARG_off].u_int != -1) {
        self->off = args[ARG_off].u_int;
    }
    if (args[ARG_period].u_int != -1) {
        self->period = args[ARG_period].u_int;
    }
}


STATIC mp_obj_t timer_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 255, true);
    // create
    timer_obj_t *self = m_new_obj(timer_obj_t);
    //
    self->base.type = &timer_type;
    self->name = "timer-1";
    self->name_len = 7;
    self->enable = false;
    self->binded = false;
    self->start = -1;
    self->on = 0;
    self->off = 0;
    self->period = 0;
    //
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
    timer_init_helper(self, n_args, args, &kw_args);
    //
    return MP_OBJ_FROM_PTR(self);
}


STATIC mp_obj_t timer_init(mp_obj_t self_in) {
    return mp_const_none;
}


STATIC mp_obj_t timer_update(mp_obj_t self_in, mp_obj_t seconds_in, mp_obj_t interval_in) {

    timer_obj_t *self = self_in;
    mp_int_t seconds = mp_obj_get_int(seconds_in);
    mp_int_t interval = mp_obj_get_int(interval_in);

    bool newstate;

    if (self->enable && seconds != -1) {
        if (self->start == -1) {
            self->start = seconds;
        }
        // calc delta from start time
        int tdelta = seconds - self->start;
        if (tdelta < 0) {
            tdelta += self->period;
        }
        if (tdelta >= self->period - interval) {
            self->start = seconds + interval;
        }
        // on = 12:00, off = 13:00
        if (self->off >= self->on) {
            if (self->off <= tdelta) {
                newstate = false;
            }
            else if (self->on <= tdelta) {
                newstate = true;
            }
            else {
                newstate = false;
            }
        }
        // on = 13:00, off = 7:00
        else {
            if (self->on <= tdelta) {
                newstate = true;
            }
            else if (self->on >= tdelta && self->off <= tdelta) {
                newstate = false;
            }
            else if (self->off >= tdelta) {
                newstate = true;
            }
            else {
                newstate = false;
            }
        }
    }
    else {
        self->start = -1;
        newstate = false;
    }

    // if tim out changed state
    if (self->out_state != newstate) {
        self->out_state = newstate;
        //
        if (brickbase_common_update_callback != MP_OBJ_NULL) {
            mp_call_function_2(
                brickbase_common_update_callback,
                self,
                mp_obj_new_bool(self->out_state)
            );
        }
    }

    return mp_const_none;
}


STATIC mp_obj_t timer_update_slot(mp_obj_t self_in, mp_obj_t sender, mp_obj_t data) {
    timer_obj_t *self = (timer_obj_t*)self_in;
    self->enable = mp_obj_is_true(data);
    return mp_const_none;
}


STATIC mp_obj_t timer_name(mp_obj_t self_in) {
    timer_obj_t *self = (timer_obj_t*)self_in;
    return mp_obj_new_str(self->name, self->name_len);
}


STATIC mp_obj_t timer_group(mp_obj_t self_in) {
    return MP_ROM_QSTR(MP_QSTR_scenaries);
}


STATIC mp_obj_t timer_properties(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {

    timer_obj_t *self = (timer_obj_t*)args[0];
    mp_uint_t kw_len = kw_args->alloc;

    if (kw_len == 0) {
        mp_obj_dict_t *dict = mp_obj_new_dict(8);
        mp_map_elem_t *t = dict->map.table;
        t[0].key = MP_ROM_QSTR(MP_QSTR_classname);      t[0].value = MP_ROM_QSTR(MP_QSTR_UserTimer);
        t[1].key = MP_ROM_QSTR(MP_QSTR_name);           t[1].value = mp_obj_new_str(self->name, self->name_len);
        t[2].key = MP_ROM_QSTR(MP_QSTR_enable);         t[2].value = mp_obj_new_bool(self->enable);
        t[3].key = MP_ROM_QSTR(MP_QSTR_binded);         t[3].value = mp_obj_new_bool(self->binded);
        t[4].key = mp_obj_new_str("start", 5);          t[4].value = mp_obj_new_int(self->start);
        t[5].key = mp_obj_new_str("on", 2);             t[5].value = mp_obj_new_int(self->on);
        t[6].key = mp_obj_new_str("off", 3);            t[6].value = mp_obj_new_int(self->off);
        t[7].key = mp_obj_new_str("period", 6);         t[7].value = mp_obj_new_int(self->period);

        return MP_OBJ_FROM_PTR(dict);
    }

    timer_init_helper(self, n_args-1, args+1, kw_args);
    return mp_const_none;
}


STATIC MP_DEFINE_CONST_FUN_OBJ_1(timer_init_obj, timer_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(timer_update_obj, timer_update);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(timer_update_slot_obj, timer_update_slot);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(timer_name_obj, timer_name);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(timer_group_obj, timer_group);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(timer_properties_obj, 1, timer_properties);

STATIC const mp_rom_map_elem_t timer_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&timer_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_update), MP_ROM_PTR(&timer_update_obj) },
    { MP_ROM_QSTR(MP_QSTR_update_slot), MP_ROM_PTR(&timer_update_slot_obj) },
    { MP_ROM_QSTR(MP_QSTR_name), MP_ROM_PTR(&timer_name_obj) },
    { MP_ROM_QSTR(MP_QSTR_group), MP_ROM_PTR(&timer_group_obj) },
    { MP_ROM_QSTR(MP_QSTR_properties), MP_ROM_PTR(&timer_properties_obj) },
};
STATIC MP_DEFINE_CONST_DICT(timer_locals_dict, timer_locals_dict_table);


const mp_obj_type_t timer_type = {
    { &mp_type_type },
    .name = MP_QSTR_UserTimer,
    .print = timer_print,
    .make_new = timer_make_new,
    .locals_dict = (mp_obj_dict_t*)&timer_locals_dict,
};
