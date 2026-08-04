/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transformBlend.I
 * @author drose
 * @date 2005-03-24
 */

/**
 *
 */
INLINE TransformBlend::
TransformBlend() {
}

/**
 *
 */
INLINE TransformBlend::
TransformBlend(const VertexTransform *transform0, PN_stdfloat) {
  add_transform(transform0, 1.0f);
}

/**
 *
 */
INLINE TransformBlend::
TransformBlend(const VertexTransform *transform0, PN_stdfloat weight0,
               const VertexTransform *transform1, PN_stdfloat weight1) {
  add_transform(transform0, weight0);
  add_transform(transform1, weight1);
  normalize_weights();
}

/**
 *
 */
INLINE TransformBlend::
TransformBlend(const VertexTransform *transform0, PN_stdfloat weight0,
               const VertexTransform *transform1, PN_stdfloat weight1,
               const VertexTransform *transform2, PN_stdfloat weight2) {
  add_transform(transform0, weight0);
  add_transform(transform1, weight1);
  add_transform(transform2, weight2);
  normalize_weights();
}

/**
 *
 */
INLINE TransformBlend::
TransformBlend(const VertexTransform *transform0, PN_stdfloat weight0,
               const VertexTransform *transform1, PN_stdfloat weight1,
               const VertexTransform *transform2, PN_stdfloat weight2,
               const VertexTransform *transform3, PN_stdfloat weight3) {
  add_transform(transform0, weight0);
  add_transform(transform1, weight1);
  add_transform(transform2, weight2);
  add_transform(transform3, weight3);
  normalize_weights();
}

/**
 *
 */
INLINE TransformBlend::
TransformBlend(const TransformBlend &copy) :
  _entries(copy._entries)
{
}

/**
 *
 */
INLINE void TransformBlend::
operator = (const TransformBlend &copy) {
  _entries = copy._entries;
  Thread *current_thread = Thread::get_current_thread();
  clear_result(current_thread);
}

/**
 *
 */
INLINE TransformBlend::
~TransformBlend() {
}

/**
 *
 */
INLINE bool TransformBlend::
operator < (const TransformBlend &other) const {
  return compare_to(other) < 0;
}

/**
 *
 */
INLINE bool TransformBlend::
operator == (const TransformBlend &other) const {
  return compare_to(other) == 0;
}

/**
 *
 */
INLINE bool TransformBlend::
operator != (const TransformBlend &other) const {
  return compare_to(other) != 0;
}

/**
 * Returns the number of transforms stored in the blend object.
 */
INLINE size_t TransformBlend::
get_num_transforms() const {
  return _entries.size();
}

/**
 * Returns the nth transform stored in the blend object.
 */
INLINE const VertexTransform *TransformBlend::
get_transform(size_t n) const {
  nassertr(n < _entries.size(), nullptr);
  return _entries[n]._transform;
}

/**
 * Returns the weight associated with the nth transform stored in the blend
 * object.
 */
INLINE PN_stdfloat TransformBlend::
get_weight(size_t n) const {
  nassertr(n < _entries.size(), 0.0f);
  return _entries[n]._weight;
}

/**
 * Removes the nth transform stored in the blend object.
 */
INLINE void TransformBlend::
remove_transform(size_t n) {
  nassertv(n < _entries.size());
  _entries.erase(_entries.begin() + n);
  Thread *current_thread = Thread::get_current_thread();
  clear_result(current_thread);
}

/**
 * Replaces the nth transform stored in the blend object.
 */
INLINE void TransformBlend::
set_transform(size_t n, const VertexTransform *transform) {
  nassertv(n < _entries.size());
  _entries[n]._transform = transform;
}

/**
 * Replaces the weight associated with the nth transform stored in the blend
 * object.
 */
INLINE void TransformBlend::
set_weight(size_t n, PN_stdfloat weight) {
  nassertv(n < _entries.size());
  _entries[n]._weight = weight;
}

/**
 * Recomputes the internal representation of the blend value, if necessary.
 * You should call this before calling get_blend() or transform_point().
 */
INLINE void TransformBlend::
update_blend(Thread *current_thread) const {
  CDLockedReader cdata(_cycler, current_thread);
  if (cdata->_global_modified == VertexTransform::get_global_modified(current_thread)) {
    // Nothing anywhere has moved since we last recomputed.
    return;
  }

  // That counter is process-global, so it says almost nothing about *this*
  // blend: a handful of changed joints bumps it every frame while most blends
  // are unaffected.  Ask whether our own transforms moved before opening a
  // writer, which under a threaded pipeline is a copy-on-write plus a retire.
  // Leaving _global_modified stale costs only this re-check next frame.
  UpdateSeq seq = get_entries_modified(current_thread);
  if (cdata->_modified == seq) {
    return;
  }

  CDWriter cdataw(((TransformBlend *)this)->_cycler, cdata, false);
  ((TransformBlend *)this)->recompute_result(cdataw, seq, current_thread);
}

/**
 * Returns the highest modified counter among this blend's own transforms: the
 * value _modified is keyed on, and so the authoritative staleness test.  Each
 * get_modified() is a lock-free cycler read.
 */
INLINE UpdateSeq TransformBlend::
get_entries_modified(Thread *current_thread) const {
  UpdateSeq seq;
  Entries::const_iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    UpdateSeq m = (*ei)._transform->get_modified(current_thread);
    if (seq < m) {
      seq = m;
    }
  }
  return seq;
}

/**
 * Returns the current value of the blend, based on the current value of all
 * of the nested transform objects and their associated weights.
 *
 * You should call update_blend() to ensure that the cache is up-to-date
 * before calling this.
 */
INLINE void TransformBlend::
get_blend(LMatrix4 &result, Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  result = cdata->_result;
}

/**
 * Transforms the indicated point by the blend matrix.
 *
 * You should call update_blend() to ensure that the cache is up-to-date
 * before calling this.
 */
INLINE void TransformBlend::
transform_point(LPoint4 &point, Thread *current_thread) const {
  if (!_entries.empty()) {
    CDReader cdata(_cycler, current_thread);
    point = point * cdata->_result;
  }
}

/**
 * Transforms the indicated point by the blend matrix.
 *
 * You should call update_blend() to ensure that the cache is up-to-date
 * before calling this.
 */
INLINE void TransformBlend::
transform_point(LPoint3 &point, Thread *current_thread) const {
  if (!_entries.empty()) {
    CDReader cdata(_cycler, current_thread);
    point = point * cdata->_result;
  }
}

/**
 * Transforms the indicated vector by the blend matrix.
 *
 * You should call update_blend() to ensure that the cache is up-to-date
 * before calling this.
 */
INLINE void TransformBlend::
transform_vector(LVector3 &vector, Thread *current_thread) const {
  if (!_entries.empty()) {
    CDReader cdata(_cycler, current_thread);
    vector = vector * cdata->_result;
  }
}

#ifndef STDFLOAT_DOUBLE
/**
 * Transforms the indicated point by the blend matrix.
 *
 * You should call update_blend() to ensure that the cache is up-to-date
 * before calling this.
 */
INLINE void TransformBlend::
transform_point(LPoint4d &point, Thread *current_thread) const {
  if (!_entries.empty()) {
    CDReader cdata(_cycler, current_thread);
    point = point * LCAST(double, cdata->_result);
  }
}
#else  // STDFLOAT_DOUBLE
/**
 * Transforms the indicated point by the blend matrix.
 *
 * You should call update_blend() to ensure that the cache is up-to-date
 * before calling this.
 */
INLINE void TransformBlend::
transform_point(LPoint4f &point, Thread *current_thread) const {
  if (!_entries.empty()) {
    CDReader cdata(_cycler, current_thread);
    point = point * LCAST(float, cdata->_result);
  }
}
#endif  // STDFLOAT_DOUBLE

#ifndef STDFLOAT_DOUBLE
/**
 * Transforms the indicated point by the blend matrix.
 *
 * You should call update_blend() to ensure that the cache is up-to-date
 * before calling this.
 */
INLINE void TransformBlend::
transform_point(LPoint3d &point, Thread *current_thread) const {
  if (!_entries.empty()) {
    CDReader cdata(_cycler, current_thread);
    point = point * LCAST(double, cdata->_result);
  }
}
#else  // STDFLOAT_DOUBLE
/**
 * Transforms the indicated point by the blend matrix.
 *
 * You should call update_blend() to ensure that the cache is up-to-date
 * before calling this.
 */
INLINE void TransformBlend::
transform_point(LPoint3f &point, Thread *current_thread) const {
  if (!_entries.empty()) {
    CDReader cdata(_cycler, current_thread);
    point = point * LCAST(float, cdata->_result);
  }
}
#endif  // STDFLOAT_DOUBLE

#ifndef STDFLOAT_DOUBLE
/**
 * Transforms the indicated vector by the blend matrix.
 *
 * You should call update_blend() to ensure that the cache is up-to-date
 * before calling this.
 */
INLINE void TransformBlend::
transform_vector(LVector3d &vector, Thread *current_thread) const {
  if (!_entries.empty()) {
    CDReader cdata(_cycler, current_thread);
    vector = vector * LCAST(double, cdata->_result);
  }
}
#else  // STDFLOAT_DOUBLE
/**
 * Transforms the indicated vector by the blend matrix.
 *
 * You should call update_blend() to ensure that the cache is up-to-date
 * before calling this.
 */
INLINE void TransformBlend::
transform_vector(LVector3f &vector, Thread *current_thread) const {
  if (!_entries.empty()) {
    CDReader cdata(_cycler, current_thread);
    vector = vector * LCAST(float, cdata->_result);
  }
}
#endif  // STDFLOAT_DOUBLE

/**
 * Returns a counter which is guaranteed to increment at least as often as the
 * result of get_blend() changes.
 */
INLINE UpdateSeq TransformBlend::
get_modified(Thread *current_thread) const {
  CDLockedReader cdata(_cycler, current_thread);
  if (cdata->_global_modified != VertexTransform::get_global_modified(current_thread)) {
    // As in update_blend(): only copy-on-write if one of our own transforms
    // moved, not merely because something somewhere did.
    UpdateSeq seq = get_entries_modified(current_thread);
    if (cdata->_modified != seq) {
      CDWriter cdataw(((TransformBlend *)this)->_cycler, cdata, false);
      ((TransformBlend *)this)->recompute_result(cdataw, seq, current_thread);
      return cdataw->_modified;
    }
  }
  return cdata->_modified;
}

/**
 * Provides an ordering of TransformEntries by the VertexTransform pointer
 * only, so we can easily look up in the set to see if a particular transform
 * exists.
 */
INLINE bool TransformBlend::TransformEntry::
operator < (const TransformBlend::TransformEntry &other) const {
  return _transform < other._transform;
}

/**
 *
 */
INLINE TransformBlend::CData::
CData() :
  _result(LMatrix4::ident_mat())
{
}

/**
 *
 */
INLINE TransformBlend::CData::
CData(const TransformBlend::CData &copy) :
  _result(copy._result),
  _modified(copy._modified),
  _global_modified(copy._global_modified)
{
}

INLINE std::ostream &
operator << (std::ostream &out, const TransformBlend &obj) {
  obj.output(out);
  return out;
}
