#ifndef _C2_TYPES_H
#define _C2_TYPES_H

#define C2_BLACKBOARD_RAW_DATA_SIZE 64

typedef struct {

} c2_blackboard_private_data_t;

typedef struct {
  uint8_t received_data_raw[C2_BLACKBOARD_RAW_DATA_SIZE];
} c2_blackboard_public_data_t;

typedef struct {
  c2_blackboard_private_data_t private_data;
  c2_blackboard_public_data_t public_data;
} c2_blackboard_data_t;

#endif /* _C2_TYPES_H */
