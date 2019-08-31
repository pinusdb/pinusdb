#include "ker_list.h"

static inline void _list_add(struct list_head *new_node, struct list_head* prev, struct list_head* next)
{
  next->prev = new_node;
  new_node->next = next;
  new_node->prev = prev;
  prev->next = new_node;
}

static inline void _list_del(struct list_head *prev, struct list_head *next)
{
  prev->next = next;
  next->prev = prev;
}

void INIT_LIST_HEAD(struct list_head *list)
{
  list->next = list;
  list->prev = list;
}
void INIT_NODE(struct list_head *node)
{
  node->next = nullptr;
  node->prev = nullptr;
}

void list_add(struct list_head *new_node, struct list_head* head)
{
  _list_add(new_node, head, head->next);
}
void list_add_tail(struct list_head *new_node, struct list_head* head)
{
  _list_add(new_node, head->prev, head);
}
void list_del(struct list_head *entry)
{
  _list_del(entry->prev, entry->next);
  entry->next = nullptr;
  entry->prev = nullptr;
}

void list_move(struct list_head *list, struct list_head *head)
{
  _list_del(list->prev, list->next);
  list_add(list, head);
}
void list_move_tail(struct list_head* list, struct list_head *head)
{
  _list_del(list->prev, list->next);
  list_add_tail(list, head);
}

bool list_empty(const struct list_head* head)
{
  return head->next == head;
}