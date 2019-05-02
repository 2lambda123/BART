#ifndef BART_SRC_DATA_SYSTEM_RIGHT_HAND_SIDE_FIXED_H_
#define BART_SRC_DATA_SYSTEM_RIGHT_HAND_SIDE_FIXED_H_

#include <memory>

#include "data/system/rhs_lhs_types.h"
#include "data/system/right_hand_side_i.h"

namespace bart {

namespace data {

namespace system {

class RightHandSide : public RightHandSideI {
 public:
  explicit RightHandSide(std::unordered_set<VariableTerms> = {});
  virtual ~RightHandSide() = default;

  std::unordered_set<VariableTerms> GetVariableTerms() const override {
    return variable_terms_;
  };

  std::shared_ptr<MPIVector> GetFixedPtr(Index index) override {
    try {
      return fixed_right_hand_side_.at(index);
    } catch (std::out_of_range &exc) {
      return nullptr;
    }
  };

  std::shared_ptr<MPIVector> GetFixedPtr(GroupNumber group) override {
    return GetFixedPtr({group, 0});
  };

  void SetFixedPtr(Index index, std::shared_ptr<MPIVector> to_set) override {
    fixed_right_hand_side_[index] = to_set;
  };

  void SetFixedPtr(GroupNumber group, std::shared_ptr<MPIVector> to_set) override {
    SetFixedPtr({group, 0}, to_set);
  }


 private:
  const std::unordered_set<VariableTerms> variable_terms_;
  std::map<Index, std::shared_ptr<MPIVector>> fixed_right_hand_side_;
};


} // namespace system

} // namespace data

} // namespace bart

#endif // BART_SRC_DATA_SYSTEM_RIGHT_HAND_SIDE_FIXED_H_