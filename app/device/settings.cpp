#include "settings.h"
#include <QSettings>

Settings::Settings()
{
    QSettings settings("settings.ini",
                    QSettings::IniFormat);

    calibration.tool_length_nm =  settings.value("calibration/tool_length_nm", kDefaultToolLength_nm).toDouble();
    calibration.tool_head_radius_nm = settings.value("calibration/tool_head_radius_nm", kDefaultHeadRadius_nm).toDouble();
    calibration.tool_head_lenght_nm = settings.value("calibration/tool_head_length_nm", kDefaultHeadLength_nm).toDouble();
    calibration.tool_zero_angle_deg = settings.value("calibration/tool_zero_angle_deg", kDefaultToolZeroAngleDeg).toDouble();

    device.tool_max_angle_deg = settings.value("device/tool_max_angle_deg", kDefaultToolMaxAngleDeg).toDouble();
    device.tool_min_angle_deg = settings.value("device/tool_min_angle_deg", kDefaultToolMinAngleDeg).toDouble();
    device.length_per_pulse_x_nm = settings.value("device/length_per_pulse_x_nm", kDefaultLengthPerPulseX).toDouble();
    device.length_per_pulse_angle_nm = settings.value("device/length_per_pulse_angle_nm", kDefaultLengthPerPulseAngle).toDouble();
    device.mount_point_radius_mm = settings.value("device/mount_point_radius_mm", kDefaultMountPointRadiusMM).toDouble();
    view.axis_unit = (ViewParameters::E_AXIS_UNIT) settings.value("view/axis_unit", ViewParameters::E_AXIS_UNIT::MM).toInt();
    view.angles_in_deg = settings.value("view/angles_in_deg", false).toBool();


    movement.xplus_step = settings.value("movement/xplus_step", 0.1).toDouble();
    movement.xminus_step = settings.value("movement/xminus_step", 0.1).toDouble();
    movement.yplus_step = settings.value("movement/yplus_step", 0.1).toDouble();
    movement.yminus_step = settings.value("movement/yminus_step", 0.1).toDouble();
    movement.zplus_step = settings.value("movement/zplus_step", 0.1).toDouble();
    movement.zminus_step = settings.value("movement/zminus_step", 0.1).toDouble();
    movement.user_defined_axis_step = settings.value("movement/user_defined_axis_step", false).toBool();
    movement.current_speed = settings.value("movement/current_speed", 0).toInt();

    int controller_count = settings.value("controllers/count",0).toInt();

    for (int i =0; i < controller_count; i++)
    {
        controllers << settings.value("controllers/c_"+QString::number(i), "").toString();
    }

    current_controller_index = settings.value("controllers/current_index", -1).toInt();
}

void Settings::save()
{
    QSettings settings("settings.ini",
                    QSettings::IniFormat);

    settings.setValue("calibration/tool_length_nm", calibration.tool_length_nm );
    settings.setValue("calibration/tool_head_radius_nm", calibration.tool_head_radius_nm);
    settings.setValue("calibration/tool_head_length_nm", calibration.tool_head_lenght_nm);
    settings.setValue("calibration/tool_zero_angle_deg", calibration.tool_zero_angle_deg);

    settings.setValue("device/tool_max_angle_deg", device.tool_max_angle_deg);
    settings.setValue("device/tool_min_angle_deg", device.tool_min_angle_deg);
    settings.setValue("device/length_per_pulse_x_nm", device.length_per_pulse_x_nm);
    settings.setValue("device/length_per_pulse_angle_nm", device.length_per_pulse_angle_nm);
    settings.setValue("device/mount_point_radius_mm", device.mount_point_radius_mm);

    settings.setValue("view/axis_unit", view.axis_unit);
    settings.setValue("view/angles_in_deg", view.angles_in_deg);

    settings.setValue("controllers/count",QString::number(controllers.size()));

    for (int i =0; i < controllers.size(); i++)
    {
        settings.setValue("controllers/c_"+QString::number(i), controllers.at(i));
    }

    settings.setValue("controllers/current_index", current_controller_index);

    settings.setValue("movement/xplus_step", movement.xplus_step) ;
    settings.setValue("movement/xminus_step", movement.xminus_step);
    settings.setValue("movement/yplus_step", movement.yplus_step);
    settings.setValue("movement/yminus_step", movement.yminus_step);
    settings.setValue("movement/zplus_step", movement.zplus_step);
    settings.setValue("movement/zminus_step", movement.zminus_step);
    settings.setValue("movement/user_defined_axis_step", movement.user_defined_axis_step);
    settings.setValue("movement/current_speed", movement.current_speed);
}
