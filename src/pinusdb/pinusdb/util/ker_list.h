#pragma once

struct list_head {
  struct list_head *next; //next in chain
  struct list_head *prev; //previous in chain
};

#define list_entry(ptr, type, member) \
  (type*)(((unsigned char*)ptr) - (size_t)(&((type*)0)->member))

#define list_for_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

void INIT_LIST_HEAD(struct list_head *list);
void INIT_NODE(struct list_head *node);

void list_add(struct list_head *new_node, struct list_head* head);
void list_add_tail(struct list_head *new_node, struct list_head* head);
void list_del(struct list_head *entry);
void list_move(struct list_head *list, struct list_head *head);
void list_move_tail(struct list_head* list, struct list_head *head);
bool list_empty(const struct list_head* head);
