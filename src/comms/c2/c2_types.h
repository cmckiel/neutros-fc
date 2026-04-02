#ifndef _C2_TYPES_H
#define _C2_TYPES_H

typedef struct {

} c2_blackboard_private_data_t;

typedef struct {

} c2_blackboard_public_data_t;

typedef struct {
  c2_blackboard_private_data_t private_data;
  c2_blackboard_public_data_t public_data;
} c2_blackboard_data_t;

#endif /* _C2_TYPES_H */
