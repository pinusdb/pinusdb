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
