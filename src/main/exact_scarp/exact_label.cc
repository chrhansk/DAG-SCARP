#include "exact_label.hh"

ExactLabel::ExactLabel(idx current_control,
                       Vertex vertex,
                       double cost,
                       idx prefix_length,
                       ExactLabelPtr predecessor)
  : predecessor(predecessor),
    control_sums(predecessor->get_control_sums()),
    current_control(current_control),
    cost(cost),
    vertex(vertex),
    prefix(prefix_length, std::optional<idx>{})
{
  assert(predecessor);

  const idx pred_size = predecessor->prefix.size();

  assert(pred_size + 1 >= prefix_length);

  auto other_it = std::begin(predecessor->get_prefix());
  auto other_end = std::end(predecessor->get_prefix());

  std::advance(other_it, pred_size - prefix_length + 1);

  auto prefix_it = std::begin(prefix);
  auto prefix_end = std::end(prefix);

  for(;other_it != other_end; ++prefix_it, ++other_it)
  {
    assert(prefix_it != prefix_end);
    (*prefix_it) = (*other_it);
  }

  assert(prefix_it != prefix_end);
  *prefix_it = current_control;

#ifndef NDEBUG
  ++prefix_it;
  assert(prefix_it == prefix_end);
#endif

  control_sums.at(current_control)++;
}

ExactLabel::ExactLabel(idx current_control,
                       Vertex vertex,
                       idx dimension,
                       idx prefix_length,
                       double cost)
  : control_sums(dimension, 0),
    current_control(current_control),
    cost(cost),
    vertex(vertex),
    prefix(prefix_length, std::optional<idx>{})
{
  prefix.back() = current_control;

  control_sums.at(current_control)++;
}

void ExactLabel::replace(const ExactLabel& other)
{
  assert(get_vertex() == other.get_vertex());
  assert(get_current_control() == other.get_current_control());

  predecessor = other.get_predecessor();
  control_sums = other.get_control_sums();
  cost = other.get_cost();
  prefix = other.get_prefix();
}

bool ExactLabel::operator==(const ExactLabel& other) const
{
  assert(get_vertex() == other.get_vertex());
  assert(get_current_control() == other.get_current_control());

  return (prefix == other.get_prefix()) &&
    (get_control_sums() == other.get_control_sums());
}
