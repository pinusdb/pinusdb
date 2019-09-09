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

#include <assert.h>
#include <stdlib.h>
#include <atomic>
#include <memory>
#include <string>
#include "util/arena.h"
#include "util/random.h"

template<typename Key, typename Val, class Comparator>
class SkipList
{
private:
  struct Node;

public:
  explicit SkipList(Comparator cmp, Arena* pArena);

  bool Insert(const Key key, const Val val);
  bool Contains(const Key key) const;
  bool Erase(const Key key);

  class Iterator {
  public:
    explicit Iterator(const SkipList* pList);

    bool Valid() const;
    Key GetKey() const;
    Val GetVal() const;

    void Next();
    void Prev();
    void Seek(const Key target);
    void SeekToFirst();
    void SeekToLast();

  private:
    const SkipList* pList_;
    Node* pNode_;
  };

private:
  enum { kMaxHeight = 8 };

  Comparator const compare_;
  Arena* pArena_;
  Node* const pHead_;

  volatile int32_t maxHeight_;

  Random rnd_;

  Node* NewNode(const Key key, const Val val, int height);
  int RandomHeight();

  bool Equal(const Key a, const Key b) const { return (compare_(a, b) == 0); }
  bool KeyIsAfterNode(const Key key, Node* pNode) const;

  Node* FindGreaterOrEqual(const Key key, Node** ppPrev) const;
  Node* FindLessThan(const Key key) const;
  Node* FindLast() const;

  //No copying allowed
  SkipList(const SkipList&);
  void operator=(const SkipList&);
};

template<typename Key, typename Val, class Comparator>
struct SkipList<Key, Val, Comparator>::Node {
  Key key_;
  Val val_;
  Node* pNext_[1];
};

template<typename Key, typename Val, class Comparator>
inline SkipList<Key, Val, Comparator>::Iterator::Iterator(const SkipList* pList)
{
  pList_ = pList;
  pNode_ = nullptr;
}

template<typename Key, typename Val, class Comparator>
inline bool SkipList<Key, Val, Comparator>::Iterator::Valid() const
{
  return pNode_ != nullptr;
}

template<typename Key, typename Val, class Comparator>
inline Key SkipList<Key, Val, Comparator>::Iterator::GetKey() const
{
  assert(Valid());
  return pNode_->key_;
}

template<typename Key, typename Val, class Comparator>
inline Val SkipList<Key, Val, Comparator>::Iterator::GetVal() const
{
  assert(Valid());
  return pNode_->val_;
}

template<typename Key, typename Val, class Comparator>
inline void SkipList<Key, Val, Comparator>::Iterator::Next()
{
  assert(Valid());
  pNode_ = pNode_->pNext_[0];
}

template<typename Key, typename Val, class Comparator>
inline void SkipList<Key, Val, Comparator>::Iterator::Prev()
{
  assert(Valid());
  pNode_ = pList_->FindLessThan(pNode_->key_);
  if (pNode_ == pList_->pHead_)
    pNode_ = nullptr;
}

template<typename Key, typename Val, class Comparator>
inline void SkipList<Key, Val, Comparator>::Iterator::Seek(const Key target)
{
  pNode_ = pList_->FindGreaterOrEqual(target, nullptr);
}

template<typename Key, typename Val, class Comparator>
inline void SkipList<Key, Val, Comparator>::Iterator::SeekToFirst()
{
  pNode_ = pList_->pHead_->pNext_[0];
}

template<typename Key, typename Val, class Comparator>
inline void SkipList<Key, Val, Comparator>::Iterator::SeekToLast()
{
  pNode_ = pList_->FindLast();
  if (pNode_ == pList_->pHead_)
    pNode_ = nullptr;
}

template<typename Key, typename Val, class Comparator>
SkipList<Key, Val, Comparator>::SkipList(Comparator cmp, Arena* pArena)
  : compare_(cmp),
  rnd_(0xdeadbeef),
  pArena_(pArena),
  pHead_(NewNode(0, 0, kMaxHeight))
{
  maxHeight_ = 1;
  for (int i = 0; i < kMaxHeight; i++)
  {
    pHead_->pNext_[i] = nullptr;
  }
}

template<typename Key, typename Val, class Comparator>
bool SkipList<Key, Val, Comparator>::Insert(const Key key, const Val val)
{
  Node* ppPrev[kMaxHeight];
  Node* pX = FindGreaterOrEqual(key, ppPrev);

  if (pX != nullptr && Equal(key, pX->key_))
  {
    return false;
  }

  int height = RandomHeight();
  if (height > maxHeight_)
  {
    for (int i = maxHeight_; i < height; i++)
      ppPrev[i] = pHead_;

    maxHeight_ = height;
  }

  pX = NewNode(key, val, height);
  for (int i = 0; i < height; i++)
  {
    pX->pNext_[i] = ppPrev[i]->pNext_[i];
    ppPrev[i]->pNext_[i] = pX;
  }

  return true;
}

template<typename Key, typename Val, class Comparator>
bool SkipList<Key, Val, Comparator>::Contains(const Key key) const
{
  Node* pX = FindGreaterOrEqual(key, nullptr);
  return (pX != nullptr && Equal(key, pX->key_));
}

template<typename Key, typename Val, class Comparator>
bool SkipList<Key, Val, Comparator>::Erase(const Key key)
{
  Node* ppPrev[kMaxHeight] = { 0 };
  Node* pX = FindGreaterOrEqual(key, ppPrev);

  if (pX != nullptr && Equal(key, pX->key_))
  {
    for (int i = 0; i < maxHeight_; i++)
    {
      if (ppPrev[i] != nullptr && ppPrev[i]->pNext_[i] == pX)
      {
        ppPrev[i]->pNext_[i] = pX->pNext_[i];
      }
    }

    return true;
  }
  return false;
}

template<typename Key, typename Val, class Comparator>
typename SkipList<Key, Val, Comparator>::Node*
SkipList<Key, Val, Comparator>::NewNode(const Key key, const Val val, int height)
{
  size_t nodeSize = sizeof(Node) + (sizeof(Node*) * (height - 1));
  char* pMem = pArena_->AllocateAligned(nodeSize);
  if (pMem == nullptr)
    return nullptr;

  memset(pMem, 0, nodeSize);
  Node* pNode = (Node*)pMem;
  pNode->key_ = key;
  pNode->val_ = val;

  return pNode;
}

template<typename Key, typename Val, class Comparator>
int SkipList<Key, Val, Comparator>::RandomHeight()
{
  static const unsigned int kBranching = 4;
  int height = 1;
  while (height < kMaxHeight && ((rnd_.Next() % kBranching) == 0))
  {
    height++;
  }
  assert(height > 0);
  assert(height <= kMaxHeight);
  return height;
}

template<typename Key, typename Val, class Comparator>
bool SkipList<Key, Val, Comparator>::KeyIsAfterNode(const Key key, Node* pNode) const
{
  return (pNode != nullptr) && (compare_(pNode->key_, key) < 0);
}

template<typename Key, typename Val, class Comparator>
typename SkipList<Key, Val, Comparator>::Node*
SkipList<Key, Val, Comparator>::FindGreaterOrEqual(const Key key, Node** ppPrev) const
{
  Node* pX = pHead_;
  int level = maxHeight_ - 1;
  while (true)
  {
    Node* pNext = pX->pNext_[level];
    if (KeyIsAfterNode(key, pNext)) {
      pX = pNext;
    }
    else {
      if (ppPrev != nullptr) ppPrev[level] = pX;
      if (level == 0) {
        return pNext;
      }
      else {
        level--;
      }
    }
  }
}

template<typename Key, typename Val, class Comparator>
typename SkipList<Key, Val, Comparator>::Node*
SkipList<Key, Val, Comparator>::FindLessThan(const Key key) const
{
  Node* pX = pHead_;
  int level = maxHeight_ - 1;
  while (true) {
    assert(pX == pHead_ || compare_(pX->key_, key) < 0);
    Node* pNext = pX->pNext_[level];
    if (pNext == nullptr || compare_(pNext->key_, key) >= 0) {
      if (level == 0) {
        return pX;
      }
      else {
        level--;
      }
    }
    else
    {
      pX = pNext;
    }
  }
}

template<typename Key, typename Val, class Comparator>
typename SkipList<Key, Val, Comparator>::Node*
SkipList<Key, Val, Comparator>::FindLast() const
{
  Node* pX = pHead_;
  int level = maxHeight_ - 1;
  while (true) {
    Node* pNext = pX->pNext_[level];
    if (pNext == nullptr) {
      if (level == 0) {
        return pX;
      }
      else {
        level--;
      }
    }
    else {
      pX = pNext;
    }
  }
}

