/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/**
 * Copyright (C) 2014-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version, with the additional exemption that
 * compiling, linking, and/or using OpenSSL is allowed.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU Lesser General Public License is in the file COPYING.
 */

#ifndef NDN_CHANGE_COUNTER_HPP
#define NDN_CHANGE_COUNTER_HPP

#include "../common.hpp"

namespace ndn {

/**
 * A ChangeCounter keeps a target object whose change count is tracked by a local
 * change count.  You can set to a new target which updates the local change count, and you can call checkChanged
 * to check if the target (or one of the target's targets) has been changed.
 * The target type T must have a method getChangeCount.
 * If you need the target to be a shared_ptr, see SharedPointerChangeCounter.
 */
template<class T>
class ChangeCounter {
public:
  /**
   * Create a new ChangeCounter with a default value for the target.  This sets the local change counter to target_.getChangeCount().
   */
  ChangeCounter()
  {
    changeCount_ = target_.getChangeCount();
  }

  /**
   * Create a new ChangeCounter, calling the copy constructor on T with the given target.
   * This sets the local change counter to target_.getChangeCount().
   * @param target The target to copy.
   */
  ChangeCounter(const T& target)
  : target_(target)
  {
    changeCount_ = target_.getChangeCount();
  }

  /**
   * Get a const reference to the target object.
   * @return A const reference to the target.
   */
  const T&
  get() const { return target_; }

  /**
   * Get a reference to the target object.  If the target is changed, then checkChanged will detect it.
   * @return A reference to the target.
   */
  T&
  get() { return target_; }

  /**
   * Set the target to the given target.  This sets the local change counter to target.getChangeCount().
   * @param target A reference to the target object whose assignment operator is called to copy it.
   */
  void
  set(const T& target)
  {
    target_ = target;
    changeCount_ = target_.getChangeCount();
  }

  /**
   * If the target's change count is different than the local change count, then update the local change count
   * and return true.  Otherwise return false, meaning that the target has not changed.  This is useful since the
   * target (or one of the target's targets) may be changed and you need to find out.
   * @return True if the change count has been updated, false if not.
   */
  bool
  checkChanged() const
  {
    uint64_t targetChangeCount = target_.getChangeCount();
    if (changeCount_ != targetChangeCount) {
      // This method can be called on a const object, but we want to be able to update the changeCount_.
      const_cast<ChangeCounter<T>*>(this)->changeCount_ = targetChangeCount;
      return true;
    }
    else
      return false;
  }

private:
  T target_;
  uint64_t changeCount_;
};

/**
 * A ChangeCounter keeps a shared_ptr to a target object whose change count is tracked by a local
 * change count.  You can set to a new target which updates the local change count, and you can call checkChanged
 * to check if the target (or one of the target's targets) has been changed.
 * The target type T must have a method getChangeCount.
 * If you need the target to be a normal value type, see ChangeCounter.
 */
template<class T>
class SharedPointerChangeCounter {
public:
  /**
   * Create a new SharedPointerChangeCounter with a default a null shared_ptr for the target.
   * This sets the local change counter to 0.
   */
  SharedPointerChangeCounter()
  {
    changeCount_ = 0;
  }

  /**
   * Create a new SharedPointerChangeCounter with the given target.  This sets the local change counter to target_->getChangeCount().
   * @param target A new object for initializing the shared_ptr to the target.
   */
  SharedPointerChangeCounter(T *target)
  : target_(target)
  {
    if (target_)
      changeCount_ = target_->getChangeCount();
    else
      changeCount_ = 0;
  }

  /**
   * Get a const pointer to the target object.
   * @return A const pointer to the target.
   */
  const T*
  get() const { return target_.get(); }

  /**
   * Get a pointer to the target object.  If the target is changed, then checkChanged will detect it.
   * @return A pointer to the target.
   */
  T*
  get() { return target_.get(); }

  /**
   * Set the shared_ptr to the target to the given value.  If target is not null, this sets the local change counter to target->getChangeCount().
   * @param target A reference to the shared_ptr to the target object whose assignment operator is called to copy the shared_ptr.
   */
  void
  set(const ptr_lib::shared_ptr<T>& target)
  {
    target_ = target;
    if (target_)
      changeCount_ = target_->getChangeCount();
    else
      changeCount_ = 0;
  }

  /**
   * If the target's change count is different than the local change count, then update the local change count
   * and return true.  Otherwise return false, meaning that the target has not changed.
   * However, if the shared_ptr to the target is null, return false.
   * This is useful since the target (or one of the target's targets) may be changed and you need to find out.
   * @return True if the change count has been updated, false if not.
   */
  bool
  checkChanged() const
  {
    if (!target_)
      return false;

    uint64_t targetChangeCount = target_->getChangeCount();
    if (changeCount_ != targetChangeCount) {
      // This method can be called on a const object, but we want to be able to update the changeCount_.
      const_cast<SharedPointerChangeCounter<T>*>(this)->changeCount_ = targetChangeCount;
      return true;
    }
    else
      return false;
  }

private:
  ptr_lib::shared_ptr<T> target_;
  uint64_t changeCount_;
};

}

#endif
