#ifndef PHYSIC_CONSTANTS_HPP
#define PHYSIC_CONSTANTS_HPP

///
/// @brief Components for orbital propagation
///
namespace spock
{
  ///
  /// @brief Compile-time physic constants
  ///
  namespace physic_constant
  {
    ///
    /// @brief Number of days in a year
    ///
    inline constexpr int DAYS_IN_ONE_YEAR = 365.;
    ///
    /// @brief Number of seconds in a day
    ///
    inline constexpr int SECONDS_IN_ONE_DAY = 86400.;
    ///
    /// @brief Number of hours in one day
    ///
    inline constexpr int HOURS_IN_ONE_DAY = 24.;
    ///
    /// @brief Number of metres in a kilometer
    ///
    inline constexpr int METRES_IN_ONE_KILOMETRE = 1000.;
    ///
    /// @brief Number of seconds in one hour
    ///
    inline constexpr int SECONDS_IN_ONE_HOUR = 3600.;
  }
}

#endif
