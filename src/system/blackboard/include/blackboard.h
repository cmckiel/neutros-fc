#ifndef _BLACKBOARD_H
#define _BLACKBOARD_H

#include "blackboard_topic_ids.h"

void *blackboard_get_publisher_handle(blackboard_topic_id_t topic_id);

const void *blackboard_get_subscriber_handle(blackboard_topic_id_t topic_id);

#endif /* _BLACKBOARD_H */
