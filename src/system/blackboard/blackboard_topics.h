#ifndef _BLACKBOARD_TOPICS_H
#define _BLACKBOARD_TOPICS_H

#include <stdlib.h>
#include <stdbool.h>

#include "blackboard_topic_ids.h"

#define MAX_TOPIC_NAME_SIZE 50

typedef struct {
  char name[MAX_TOPIC_NAME_SIZE];
  void* topic_address;
  bool topic_is_published;
} blackboard_topic_t;

extern blackboard_topic_t blackboard_topics[_BLACKBOARD_TOPIC_ID_MAX];

#endif /* _BLACKBOARD_TOPICS_H */
