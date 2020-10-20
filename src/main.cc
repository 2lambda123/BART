#include <memory>
#include <iostream>
#include <fstream>

#include <deal.II/base/conditional_ostream.h>
#include <deal.II/base/parameter_handler.h>
#include <deal.II/base/mpi.h>

#include "framework/builder/framework_builder.hpp"
#include "problem/parameters_dealii_handler.h"
#include "utility/runtime/runtime_helper.h"

int main(int argc, char* argv[]) {
  try {
    bart::utility::runtime::RuntimeHelper runtime_helper("0.2.0");
    runtime_helper.ParseArguments(argc, argv);

    if (runtime_helper.show_help()) {
      std::cout << runtime_helper.HelpMessage() << std::endl;
      return EXIT_FAILURE;
    }

    std::cout << runtime_helper.ProgramHeader() << std::endl;
    std::cout << "\nInitializing MPI\n";
    dealii::Utilities::MPI::MPI_InitFinalize mpi_initialization(argc, argv, 1);
    std::cout << "\nMPI Initialized\n";

    namespace MPI = dealii::Utilities::MPI;
    const std::string filename{runtime_helper.filename()};
    const int n_processes = MPI::n_mpi_processes(MPI_COMM_WORLD);
    const int process_id =  MPI::this_mpi_process(MPI_COMM_WORLD);

    bart::problem::ParametersDealiiHandler prm;
    dealii::ParameterHandler d2_prm;

    prm.SetUp(d2_prm);
    d2_prm.parse_input(filename, "");
    prm.Parse(d2_prm);

    double k_eff_final;

    // Open file for output, if there are multiple processes they will end with
    // a number indicating the process number.
    std::ofstream output_stream, master_output_stream;
    const std::string output_filename_base{prm.OutputFilenameBase()};
    if (n_processes > 1) {
      const std::string full_filename = output_filename_base + dealii::Utilities::int_to_string(process_id, 4);
      output_stream.open((full_filename + ".vtu").c_str());
      master_output_stream.open(output_filename_base + ".pvtu");
    } else {
      output_stream.open((output_filename_base + ".vtu").c_str());
    }
    std::vector<std::string> filenames;
    // Write master pvtu record if multiple files are required
    if ((n_processes > 1) && (process_id == 0)) {
      for (int process = 0; process < n_processes; ++process) {
        const std::string full_filename =
            output_filename_base + dealii::Utilities::int_to_string(process, 4);
        filenames.push_back(full_filename + ".vtu");
      }
    }

    // Framework pointer
    std::unique_ptr<bart::framework::FrameworkI> framework_ptr;

    switch(prm.SpatialDimension()) {
      case 1: {
        bart::framework::builder::FrameworkBuilder<1> builder;
        framework_ptr = builder.BuildFramework("main", prm);
        break;
      }
      case 2: {
        bart::framework::builder::FrameworkBuilder<2> builder;
        framework_ptr = builder.BuildFramework("main", prm);
        break;
      }
      case 3: {
        bart::framework::builder::FrameworkBuilder<3> builder;
        framework_ptr = builder.BuildFramework("main", prm);
        break;
      }
    }

    // Pause if needed before solve
    if (runtime_helper.do_pause()) {
      std::cout << "Press <Enter> to begin solve...";
      std::cin.ignore();
    }

    framework_ptr->SolveSystem();
    std::unique_ptr<bart::framework::FrameworkI> fourier_framework_ptr;
    switch(prm.SpatialDimension()) {
      case 1: {
        bart::framework::builder::FrameworkBuilder<1> builder;
        fourier_framework_ptr = builder.BuildFramework("fourier", prm,
                                                       framework_ptr->system()->current_moments.get());
        break;
      }
      case 2: {
        bart::framework::builder::FrameworkBuilder<2> builder;
        fourier_framework_ptr = builder.BuildFramework("fourier", prm,
                                                       framework_ptr->system()->current_moments.get());
        break;
      }
      case 3: {
        bart::framework::builder::FrameworkBuilder<3> builder;
        fourier_framework_ptr = builder.BuildFramework("fourier", prm,
                                                       framework_ptr->system()->current_moments.get());
        break;
      }
    }
    framework_ptr.release();

    fourier_framework_ptr->SolveSystem();
    fourier_framework_ptr->OutputResults(output_stream);
    if (n_processes > 1)
      fourier_framework_ptr->OutputMasterFile(master_output_stream, filenames, process_id);
    k_eff_final = fourier_framework_ptr->system()->k_effective.value_or(0);

    std::cout << "Final k_effective: " << k_eff_final << std::endl;

  } catch (std::exception &exc) {
    std::cerr << std::endl << std::endl
              << "----------------------------------------------------"
              << std::endl;
    std::cerr << "Exception on processing: " << std::endl
              << exc.what() << std::endl
              << "Aborting!" << std::endl
              << "----------------------------------------------------"
              << std::endl;
    return 1;
  } catch (...) {
    std::cerr << std::endl << std::endl
              << "----------------------------------------------------"
              << std::endl;
    std::cerr << "Unknown exception!" << std::endl
              << "Aborting!" << std::endl
              << "----------------------------------------------------"
              << std::endl;
    return 1;
  }
  return 0;
}
