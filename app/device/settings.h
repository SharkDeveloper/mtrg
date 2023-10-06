#ifndef SETTINGS_H
#define SETTINGS_H

#include <QList>

const double kDefaultToolLength_nm = 210*10^6; //600mm
const double kDefaultHeadRadius_nm = 10000.648438; //0.01mm
const double kDefaultHeadLength_nm = 10 * 10^6; //10mm

const double kDefaultToolZeroAngleDeg = 1;
const double kDefaultToolMinAngleDeg = -7.5;
const double kDefaultToolMaxAngleDeg = 9.5;


const double kDefaultLengthPerPulseX = 2;
const double kDefaultLengthPerPulseAngle = 2;
const double kDefaultMountPointRadiusMM = 61.5;


class CalibrationData
{
public:
    double tool_length_nm;
    double tool_head_radius_nm;
    double tool_head_lenght_nm;
    double tool_zero_angle_deg;
};

class DeviceParameters
{
public:
    double tool_min_angle_deg;
    double tool_max_angle_deg;
    double length_per_pulse_x_nm;
    double length_per_pulse_angle_nm;
    double mount_point_radius_mm;
};

class MovementSettings
{
public:
    bool  user_defined_axis_step;

    double xplus_step;
    double xminus_step;
    double yplus_step;
    double yminus_step;
    double zplus_step;
    double zminus_step;
    int current_speed;
};

class ViewParameters
{
public:
    enum E_AXIS_UNIT {MM, NM, MK};
    bool angles_in_deg;
    E_AXIS_UNIT axis_unit;
};

class Settings
{
private:
   Settings();

public:
   static Settings& instance()
   {
      static Settings INSTANCE;
      return INSTANCE;
   }

   void save();
   DeviceParameters device;
   CalibrationData calibration;
   ViewParameters view;
   MovementSettings movement;

   QList <QString> controllers;
   int current_controller_index;
};

#endif // CALIBRATIONDATA_H
