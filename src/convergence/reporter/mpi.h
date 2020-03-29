#ifndef BART_SRC_CONVERGENCE_REPORTER_MPI_NOISY_H_
#define BART_SRC_POST_REPORTER_MPI_NOISY_H_

#include <memory>

#include <deal.II/base/conditional_ostream.h>
#include <ostream>

#include "convergence/reporter/mpi_i.h"
#include "utility/uncopyable.h"

namespace bart {

namespace convergence {

namespace reporter {

/*! \brief Noisy reporter, will report each time Report is called for
 * convergence, or a passed string.
 */

class MpiNoisy : public MpiI, private utility::Uncopyable {
 public:
  MpiNoisy(std::unique_ptr<dealii::ConditionalOStream> pout_ptr)
      : pout_ptr_(std::move(pout_ptr)) {};
  ~MpiNoisy() = default;

  void Report(const bart::convergence::Status &to_report) override;
  void Report(const std::string &to_report) override {
    *pout_ptr_ << to_report;
  }

 private:
  std::unique_ptr<dealii::ConditionalOStream> pout_ptr_;


};
} // namespace reporter

} // namespace convergence

} // namespace bart

#endif // BART_SRC_CONVERGENCE_REPORTER_MPI_NOISY_H_