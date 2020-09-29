#ifndef BART_SRC_CALCULATOR_FOURIER_FOURIER_TRANSFORM_FFTW_H_
#define BART_SRC_CALCULATOR_FOURIER_FOURIER_TRANSFORM_FFTW_H_

#include <complex>
#include <vector>

#include "calculator/fourier/fourier_transform_i.h"

namespace bart {

namespace calculator {

namespace fourier {

namespace fftw {
#include <fftw3.h>
} // namespace fftw

class FourierTransformFFTW : public FourierTransformI {
 public:
  FourierTransformFFTW(const int n_samples);
  int n_samples() const { return n_samples_; }
 private:
  const int n_samples_;
  std::vector<std::complex<double>> input_, output_;
  fftw::fftw_complex* input_ptr_;
  fftw::fftw_complex* output_ptr_;
  fftw::fftw_plan_s* plan_;
};

} // namespace fourier

} // namespace calculator

} // namespace bart

#endif //BART_SRC_CALCULATOR_FOURIER_FOURIER_TRANSFORM_FFTW_H_
