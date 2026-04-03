#include "blackboard.h"
#include "blackboard_topics.h"

#include <stdbool.h>

void *blackboard_get_publisher_handle(blackboard_topic_id_t topic_id)
{
  void *handle = NULL;

  if (_BLACKBOARD_TOPIC_ID_MIN <= topic_id && topic_id < _BLACKBOARD_TOPIC_ID_MAX &&
      blackboard_topics[topic_id].topic_address != NULL &&
      blackboard_topics[topic_id].topic_is_published == false)
  {
    handle = blackboard_topics[topic_id].topic_address;
    blackboard_topics[topic_id].topic_is_published = true;
  }

  return handle;
}

const void *blackboard_get_subscriber_handle(blackboard_topic_id_t topic_id)
{
  const void *handle = NULL;

  if (_BLACKBOARD_TOPIC_ID_MIN <= topic_id && topic_id < _BLACKBOARD_TOPIC_ID_MAX &&
      blackboard_topics[topic_id].topic_address != NULL)
  {
    handle = (const void*)blackboard_topics[topic_id].topic_address;
  }

  return handle;
}
