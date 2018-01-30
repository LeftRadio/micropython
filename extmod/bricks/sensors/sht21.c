
#include <string.h>

#include "py/mpconfig.h"
#include "py/obj.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "extmod/machine_i2c.h"
#include "extmod/bricks/brickbase.h"


extern mp_obj_t bricks_sensors_req_i2c_peripheral(mp_int_t id, mp_int_t scl, mp_int_t sda, mp_int_t freq);


typedef struct _sht21_obj_t {
    mp_obj_base_t base;
    const char *name;
    size_t name_len;
    bool enable;
    mp_obj_t i2c_obj;
    mp_int_t i2c_id;
    mp_int_t i2c_scl;
    mp_int_t i2c_sda;
    mp_int_t i2c_freq;
    mp_int_t i2c_addr;
    mp_int_t data[2];
} sht21_obj_t;

const mp_obj_type_t sht21_type;


STATIC void sht21_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    sht21_obj_t *self = self_in;
    mp_printf(print, "Bricks Sensor SHT21(%p)", &self->base);
}


STATIC void sht21_init_helper(sht21_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_name, ARG_enable, ARG_i2c_id, ARG_i2c_sda, ARG_i2c_scl, ARG_i2c_freq, ARG_i2c_addr, ARG_thermo };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_name, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_enable, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_i2c_id, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_i2c_sda, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_i2c_scl, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_i2c_freq, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 10000} },
        { MP_QSTR_i2c_addr, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if (args[ARG_name].u_obj != MP_OBJ_NULL) {
        self->name = mp_obj_str_get_data(args[ARG_name].u_obj, &self->name_len);
    }
    if (args[ARG_enable].u_obj != MP_OBJ_NULL) {
        self->enable = mp_obj_is_true(args[ARG_enable].u_obj);
    }
    if (args[ARG_i2c_sda].u_int != -1 && args[ARG_i2c_scl].u_int != -1 && args[ARG_i2c_addr].u_int != -1) {
        self->i2c_id = args[ARG_i2c_id].u_int;
        self->i2c_scl = args[ARG_i2c_scl].u_int;
        self->i2c_sda = args[ARG_i2c_sda].u_int;
        self->i2c_freq = args[ARG_i2c_freq].u_int;
        self->i2c_addr = args[ARG_i2c_addr].u_int;
        self->i2c_obj = bricks_sensors_req_i2c_peripheral(
            self->i2c_id,
            self->i2c_scl,
            self->i2c_sda,
            self->i2c_freq
        );
    }
}


STATIC mp_obj_t sht21_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 255, true);
    // create
    sht21_obj_t *self = m_new_obj(sht21_obj_t);
    //
    self->base.type = &sht21_type;
    self->name = "sht21-1";
    self->name_len = 7;
    self->enable = false;
    self->i2c_id = -1;
    self->i2c_scl = -1;
    self->i2c_sda = -1;
    self->i2c_freq = 10000;
    self->i2c_addr = -1;
    memset(self->data, 2, -65535);
    //
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
    sht21_init_helper(self, n_args, args, &kw_args);
    //
    return MP_OBJ_FROM_PTR(self);
}


STATIC mp_obj_t sht21_init(mp_obj_t self_in) {
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(sht21_init_obj, sht21_init);


STATIC mp_obj_t sht21_update(mp_obj_t self_in, mp_obj_t seconds_in, mp_obj_t interval_in) {

    sht21_obj_t *self = (sht21_obj_t*)self_in;

    mp_obj_base_t *i2c_obj = (mp_obj_base_t*)MP_OBJ_TO_PTR(self->i2c_obj);
    mp_machine_i2c_p_t *i2c_p = (mp_machine_i2c_p_t*)(i2c_obj->type->protocol);

    vstr_t vstr;
    vstr_init_len(&vstr, 3);
    bool stop = false;
    int ret = -1;
    int t;

    vstr.buf[0] = 0xF3;
    ret = i2c_p->writeto(i2c_obj, self->i2c_addr, (uint8_t*)vstr.buf, 1, stop);
    // sleep(0.1)
    ret = i2c_p->readfrom(i2c_obj, self->i2c_addr, (uint8_t*)vstr.buf, vstr.len, stop);
    if (ret < 0) {
        return mp_obj_new_int(ret);
    }
    t = (((uint16_t)vstr.buf[0] << 8) + vstr.buf[1]) & 0xFFFC;
    self->data[0] = (mp_int_t)(-46.85 + (175.72 * ((float)t / (0x01 << 16))));

    vstr.buf[0] = 0xF5;
    ret = i2c_p->writeto(i2c_obj, self->i2c_addr, (uint8_t*)vstr.buf, 1, stop);
    // sleep(0.1)
    ret = i2c_p->readfrom(i2c_obj, self->i2c_addr, (uint8_t*)vstr.buf, vstr.len, stop);
    if (ret < 0) {
        return mp_obj_new_int(ret);
    }
    t = ((vstr.buf[0] << 8) + vstr.buf[1]) & 0xFFFC;
    self->data[1] = (mp_int_t)(-6 + (125 * ((float)t / (0x01 << 16))));

    //
    if (brickbase_common_update_callback != MP_OBJ_NULL) {
        mp_call_function_2(
            brickbase_common_update_callback,
            self,
            mp_obj_new_bytes((const byte*)self->data, 2)
        );
    }

    return mp_obj_new_int(0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(sht21_update_obj, sht21_update);


STATIC mp_obj_t sht21_name(mp_obj_t self_in) {
    sht21_obj_t *self = (sht21_obj_t*)self_in;
    return mp_obj_new_str(self->name, self->name_len);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(sht21_name_obj, sht21_name);


STATIC mp_obj_t sht21_group(mp_obj_t self_in) {
    return MP_ROM_QSTR(MP_QSTR_sensors);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(sht21_group_obj, sht21_group);


STATIC mp_obj_t sht21_properties(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {

    sht21_obj_t *self = (sht21_obj_t*)args[0];
    mp_uint_t kw_len = kw_args->alloc;

    if (kw_len == 0) {
        mp_obj_dict_t *dict = mp_obj_new_dict(8);
        mp_map_elem_t *t = dict->map.table;
        t[0].key = MP_ROM_QSTR(MP_QSTR_classname);  t[0].value = MP_ROM_QSTR(MP_QSTR_LM75);
        t[1].key = MP_ROM_QSTR(MP_QSTR_name);       t[1].value = mp_obj_new_str(self->name, self->name_len);
        t[2].key = MP_ROM_QSTR(MP_QSTR_enable);     t[2].value = mp_obj_new_bool(self->enable);
        t[3].key = mp_obj_new_str("i2c_id", 6);     t[3].value = mp_obj_new_int(self->i2c_id);
        t[4].key = mp_obj_new_str("i2c_scl", 7);    t[4].value = mp_obj_new_int(self->i2c_scl);
        t[5].key = mp_obj_new_str("i2c_sda", 7);    t[5].value = mp_obj_new_int(self->i2c_sda);
        t[6].key = mp_obj_new_str("i2c_freq", 8);   t[6].value = mp_obj_new_int(self->i2c_freq);
        t[7].key = mp_obj_new_str("i2c_addr", 8);   t[7].value = mp_obj_new_int(self->i2c_addr);

        return MP_OBJ_FROM_PTR(dict);
    }

    sht21_init_helper(self, n_args-1, args+1, kw_args);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(sht21_properties_obj, 1, sht21_properties);


STATIC const mp_rom_map_elem_t sht21_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&sht21_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_update), MP_ROM_PTR(&sht21_update_obj) },
    { MP_ROM_QSTR(MP_QSTR_name), MP_ROM_PTR(&sht21_name_obj) },
    { MP_ROM_QSTR(MP_QSTR_group), MP_ROM_PTR(&sht21_group_obj) },
    { MP_ROM_QSTR(MP_QSTR_properties), MP_ROM_PTR(&sht21_properties_obj) },
};
STATIC MP_DEFINE_CONST_DICT(sht21_locals_dict, sht21_locals_dict_table);


const mp_obj_type_t sht21_type = {
    { &mp_type_type },
    .name = MP_QSTR_SHT21,
    .print = sht21_print,
    .make_new = sht21_make_new,
    .locals_dict = (mp_obj_dict_t*)&sht21_locals_dict,
};
