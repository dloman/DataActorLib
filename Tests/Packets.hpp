#pragma once

#include <boost/hana/define_struct.hpp>
#include <cstdint>

namespace dal::test
{
  struct MotorCommand
  {
    BOOST_HANA_DEFINE_STRUCT(
      MotorCommand,
      (uint8_t, mMotor0),
      (uint8_t, mMotor1),
      (uint8_t, mMotor2)
      );
  };

  struct Position
  {
    BOOST_HANA_DEFINE_STRUCT(
      Position,
      (double, mPostionX),
      (double, mPostionY),
      (double, mPostionZ),
      (double, mTime)
      );
  };
}
