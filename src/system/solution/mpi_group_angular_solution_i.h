#ifndef BART_SRC_SYSTEM_SOLUTION_MPI_GROUP_ANGULAR_SOLUTION_I_H_
#define BART_SRC_SYSTEM_SOLUTION_MPI_GROUP_ANGULAR_SOLUTION_I_H_

#include <map>

#include "system/system_types.h"
#include "system/solution/solution_i.h"

namespace bart {

namespace system {

namespace solution {

/*! \brief Interface for classes that store angular system solutions as PETSc MPI vectors.
 *
 * System solutions are indentified by a unique index made up of the energy group
 * number and angle index. They are stored as MPI Vectors using the deal.II
 * wrapper for PETSc MPI Vectors.
 */
class MPIGroupAngularSolutionI :
    public SolutionI<system::Index, system::MPIVector> {
 public:
  using SolutionMap = std::map<system::Index, system::MPIVector>;
  using MPIVector = system::MPIVector;
  using Index = system::Index;

  virtual ~MPIGroupAngularSolutionI() = default;

  /*! \brief Returns the total number of energy groups. */
  virtual int total_groups() const = 0;
  /*! \brief Returns the total number of angles */
  virtual int total_angles() const = 0;
};


} // namespace solution

} // namespace system

} // namespace bart

#endif //BART_SRC_SYSTEM_SOLUTION_MPI_GROUP_ANGULAR_SOLUTION_I_H_