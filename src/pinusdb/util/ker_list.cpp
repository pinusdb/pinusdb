/*
* Copyright (c) 2019 ChangSha JuSong Soft Inc. <service@pinusdb.cn>.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; version 3 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; If not, see <http://www.gnu.org/licenses>
*/

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