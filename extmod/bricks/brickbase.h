

#ifndef MICROPY_BRICKS_BASE_OBJ_H
#define MICROPY_BRICKS_BASE_OBJ_H

#include "py/obj.h"

// common objects update callback
extern mp_obj_t brickbase_common_update_callback;


// actuators
extern const mp_obj_type_t digital_out_type;
// sensors
extern const mp_obj_type_t lm75_type;
extern const mp_obj_type_t sht21_type;
extern const mp_obj_type_t digital_input_type;
// scenaries
extern const mp_obj_type_t timer_type;
extern const mp_obj_type_t regulator_type;


#endif // MICROPY_BRICKS_BASE_OBJ_H
