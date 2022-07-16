// Copyright 2020 Arnaud Becheler    <arnaud.becheler@gmail.com>

/***********************************************************************                                                                         *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
************************************************************************/

#ifndef __PARALLEL_EXECUTION_H_INCLUDED__
#define __PARALLEL_EXECUTION_H_INCLUDED__

# include <mpi.h>

namespace app
{
  ///
  /// @brief Policy class for parallel execution using MPI
  ///
  class execution
  {
  private:
    int _nb_process;
    int _current_process;
  public:
    ///
    /// @brief Constructor
    ///
    execution(int argc, char* argv[])
    {
      // Initialize the MPI execution environment
      MPI_Init(argc, &argv);
      // Determines the size of the group associated with a communicator
      MPI_Comm_size(MPI_COMM_WORLD, &_nb_process);
      // Determines the rank of the calling process in the communicator
      int current_process;
      MPI_Comm_rank(MPI_COMM_WORLD, &_current_process);
    }
    ///
    /// @brief Determines if standard output is allowed
    ///
    /// @note Always true in sequential policy
    ///
    bool can_print()
    {
      return _current_process == 0;
    }
  };
} // namespace app

#endif
