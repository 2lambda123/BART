#include "instrumentation/builder/instrument_builder.hpp"

#include <deal.II/lac/vector.h>
#include <deal.II/base/mpi.h>
#include <deal.II/base/conditional_ostream.h>

#include <fstream>

#include "calculator/fourier/fourier_transform_fftw.hpp"
#include "instrumentation/basic_instrument.h"
#include "instrumentation/instrument.h"
#include "instrumentation/converter/calculator/vector_subtractor.h"
#include "instrumentation/converter/multi_converter.hpp"
#include "instrumentation/converter/factory.hpp"
#include "instrumentation/outstream/factory.hpp"
#include "instrumentation/instrument_array.hpp"
#include "system/moments/spherical_harmonic_i.h"
#include "utility/colors.hpp"

namespace bart::convergence {
struct Status;
} // namespace bart::convergence

namespace bart::instrumentation::builder {

namespace  {

using ConditionalOstream = dealii::ConditionalOStream;
using ConditionalOstreamPtr = std::unique_ptr<ConditionalOstream>;
using ConvergenceStatus = convergence::Status;
using ConvergenceInstrument = InstrumentI<convergence::Status>;
using ConverterName = instrumentation::converter::ConverterName;
using DealiiVector = dealii::Vector<double>;
using IntDoublePair = std::pair<int, double>;
using OutstreamName = outstream::OutstreamName;
using StringInstrument = InstrumentI<std::string>;
using StringColorPair = std::pair<std::string, utility::Color>;

auto GetConditionalOstream = []() {
  return outstream::OutstreamIFactory<std::string, ConditionalOstreamPtr>::get()
    .GetConstructor(OutstreamName::kToConditionalOstream)(
        std::make_unique<ConditionalOstream>(
            std::cout,
            dealii::Utilities::MPI::this_mpi_process(MPI_COMM_WORLD) == 0));
};

} // namespace

// Convergence Status ==========================================================

template <>
auto InstrumentBuilder::BuildInstrument<ConvergenceStatus>(
    const InstrumentName name) -> std::unique_ptr<ConvergenceInstrument> {
  switch (name) {
    case InstrumentName::kConvergenceStatusToConditionalOstream: {
      return std::make_unique<Instrument<ConvergenceStatus, std::string>>(
          converter::ConverterIFactory<ConvergenceStatus, std::string>::get()
              .GetConstructor(ConverterName::kConvergenceToString)(),
          GetConditionalOstream());
    }
    default:
    AssertThrow(false,
                dealii::ExcMessage("Bad instrument name passed to builder"))
  }
}

//  SphericalHarmonicI =========================================================

template <>
auto InstrumentBuilder::BuildInstrument<system::moments::SphericalHarmonicI>(
    const InstrumentName name,
    const int group_number,
    const DealiiVector error,
    const std::string filename)
-> std::unique_ptr<InstrumentI<system::moments::SphericalHarmonicI>> {
  switch (name) {
    case InstrumentName::kFourierTransformOfSingleGroupScalarFluxErrorToFile: {
      using ComplexVector = std::vector<std::complex<double>>;
      using IntComplexVectorPair = std::pair<int, ComplexVector>;
      using AbsoluteValue = instrumentation::converter::calculator::AbsoluteValue;
      using FourierCalculator = calculator::fourier::FourierTransformI;
      using FourierCalculatorFFTW = calculator::fourier::FourierTransformFFTW;
      using Normalized = calculator::fourier::Normalized;
      using FirstStageConverter = instrumentation::converter::MultiConverter<DealiiVector, DealiiVector, ComplexVector>;
      using SphericalHarmonics = system::moments::SphericalHarmonicI;

      auto group_scalar_flux_extractor_ptr =
          converter::ConverterIFactory<system::moments::SphericalHarmonicI,
                                       system::moments::MomentVector, const int>::get()
              .GetConstructor(ConverterName::kGroupScalarFluxExtractor)
              (group_number);

      auto vector_subtractor_ptr =
          converter::ConverterIFactory<DealiiVector, DealiiVector, DealiiVector, AbsoluteValue>::get()
          .GetConstructor(ConverterName::kCalculatorVectorSubtractor)
              (error, AbsoluteValue(false));
      auto dealii_to_complex_vector_converter_ptr =
          converter::ConverterIFactory<DealiiVector, ComplexVector>::get()
          .GetConstructor(ConverterName::kDealiiToComplexVector)();
      auto fourier_transform_calculator = std::make_unique<FourierCalculatorFFTW>(error.size());
      auto fourier_calculator_ptr =
          converter::ConverterIFactory<ComplexVector, ComplexVector, std::unique_ptr<FourierCalculator>, Normalized>::get()
              .GetConstructor(ConverterName::kFourierTransform)
                  (std::move(fourier_transform_calculator), Normalized(true));
      auto pair_incrementer_ptr =
               converter::ConverterIFactory<ComplexVector, std::pair<int, ComplexVector>>::get()
                   .GetConstructor(converter::ConverterName::kPairIncrementer)();
      auto pair_to_string_ptr =
          converter::ConverterIFactory<IntComplexVectorPair, std::string>::get()
              .GetConstructor(converter::ConverterName::kIntVectorComplexPairToString)();

      auto full_converter = std::move(group_scalar_flux_extractor_ptr) +
          std::move(vector_subtractor_ptr) +
          std::move(dealii_to_complex_vector_converter_ptr) +
          std::move(fourier_calculator_ptr) +
          std::move(pair_incrementer_ptr) +
          std::move(pair_to_string_ptr);

      std::unique_ptr<std::ostream> file_stream = std::make_unique<std::ofstream>(filename);
      return std::make_unique<Instrument<system::moments::SphericalHarmonicI, std::string>>(
          std::move(full_converter),
          outstream::OutstreamIFactory<std::string, std::unique_ptr<std::ostream>>::get()
              .GetConstructor(OutstreamName::kToOstream)
                  (std::move(file_stream)));
    }
    default:
    AssertThrow(false,
                dealii::ExcMessage("Bad instrument name passed to builder"))
  }
}

template <>
auto InstrumentBuilder::BuildInstrument<system::moments::SphericalHarmonicI>(
    const InstrumentName name,
    system::moments::SphericalHarmonicI* previous_iteration,
    const std::string filename_base)
-> std::unique_ptr<InstrumentI<system::moments::SphericalHarmonicI>> {
  switch (name) {
    case InstrumentName::kFourierTransformOfAllGroupScalarFluxErrorToFile: {
      using ReturnType = instrumentation::InstrumentArray<system::moments::SphericalHarmonicI>;
      auto return_ptr = std::make_unique<ReturnType>();
      const int total_groups = previous_iteration->total_groups();

      for (int group = 0; group < total_groups; ++group) {
        auto group_instrument = InstrumentBuilder::BuildInstrument<system::moments::SphericalHarmonicI>(
            InstrumentName::kFourierTransformOfSingleGroupScalarFluxErrorToFile,
            group, previous_iteration->GetMoment({group, 0, 0}),
            filename_base + "_" + std::to_string(group) + ".csv");
        return_ptr->AddInstrument(std::move(group_instrument));
      }

      return return_ptr;
    };
    default:
    AssertThrow(false,
                dealii::ExcMessage("Bad instrument name passed to builder"))
  }
}

// INT-DOUBLE-PAIR =============================================================
template <>
auto InstrumentBuilder::BuildInstrument<IntDoublePair>(
    const InstrumentName name, const std::string filename)
-> std::unique_ptr<InstrumentI<IntDoublePair> > {
  switch (name) {
    case InstrumentName::kIntDoublePairToFile: {
      std::unique_ptr<std::ostream> file_stream = std::make_unique<std::ofstream>(filename);
      return std::make_unique<Instrument<IntDoublePair, std::string>>(
          converter::ConverterIFactory<IntDoublePair, std::string>::get()
              .GetConstructor(ConverterName::kIntDoublePairToString)(),
          outstream::OutstreamIFactory<std::string, std::unique_ptr<std::ostream>>::get()
              .GetConstructor(OutstreamName::kToOstream)(
                  std::move(file_stream)));
    }
    default:
    AssertThrow(false,
                dealii::ExcMessage("Bad instrument name passed to builder"))
  }
}

// STRING-COLOR-PAIR ===========================================================

template<>
std::unique_ptr<InstrumentI<StringColorPair>> InstrumentBuilder::BuildInstrument(
    const InstrumentName name) {

  switch (name) {
    case InstrumentName::kColorStatusToConditionalOstream: {
      return std::make_unique<Instrument<StringColorPair, std::string>>(
          converter::ConverterIFactory<StringColorPair, std::string>::get()
              .GetConstructor(converter::ConverterName::kStringColorPairToString)(),
          outstream::OutstreamIFactory<std::string, ConditionalOstreamPtr>::get()
              .GetConstructor(outstream::OutstreamName::kToConditionalOstream)(
                  std::make_unique<ConditionalOstream>(
                      std::cout,
                      dealii::Utilities::MPI::this_mpi_process(MPI_COMM_WORLD) == 0)));
    }
    default:
      AssertThrow(false,
                  dealii::ExcMessage("Bad instrument name passed to builder"))
  }
}

// STRING ======================================================================
template <>
auto InstrumentBuilder::BuildInstrument(const InstrumentName name)
-> std::unique_ptr<StringInstrument> {
  switch (name) {
    case InstrumentName::kStringToConditionalOstream: {
      return std::make_unique<BasicInstrument<std::string>>(
          outstream::OutstreamIFactory<std::string, ConditionalOstreamPtr>::get()
              .GetConstructor(outstream::OutstreamName::kToConditionalOstream)(
                  std::make_unique<ConditionalOstream>(
                      std::cout,
                      dealii::Utilities::MPI::this_mpi_process(MPI_COMM_WORLD) == 0)));
    };
    default:
    AssertThrow(false,
                dealii::ExcMessage("Bad instrument name passed to builder"))
  }
}

} // namespace bart::instrumentation::builder
