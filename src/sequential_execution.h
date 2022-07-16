// Copyright 2020 Arnaud Becheler    <arnaud.becheler@gmail.com>

/***********************************************************************                                                                         *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
************************************************************************/

#ifndef __SEQUENTIAL_EXECUTION_H_INCLUDED__
#define __SEQUENTIAL_EXECUTION_H_INCLUDED__

namespace app
{
  ///
  /// @brief Policy class for sequential execution
  ///
  class execution
  {
  private:
    static constexpr int _nb_process = 1;
  public:
    ///
    /// @brief Constructor taking any argument
    ///
    template<typename...Args>
    execution(Args...)
    {
      // Do nothing
    }
    ///
    /// @brief Determines if standard output is allowed
    ///
    /// @note Always true in sequential policy
    ///
    bool can_print()
    {
      return true;
    }
  };
} // namespace app

#endif
