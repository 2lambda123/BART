#ifndef __gauss_sidel_h__
#define __gauss_sidel_h__

#include "mg_base.h"

template <int dim>
class GaussSidel : public MGBase<dim>
{
public:
  GaussSidel (ParameterHandler);
  ~GaussSidel ();
  
  void mg_iterations
  (std::vector<Vector<double> > &sflxes_proc,
   std::vector<std_cxx11::shared_ptr<EquationBase<dim> > > equ_ptrs);
}

#endif //__gauss_sidel_h__
