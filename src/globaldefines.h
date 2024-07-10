#ifndef GLOBALDEFINES_H
#define GLOBALDEFINES_H

#include <math.h>

#include <QDebug>
#include <QCoreApplication>

/*! Recordings settings*/
#define PSD_DEFAULT_RECORD_PATH QString(QDir::homePath() + "/" + GLB_SOFTWARE_NAME + "/Recordings/")
#define PSD_DEFAULT_RECORD_NAME QString("file")

/*! Sw info */
#define GLB_SOFTWARE_NAME QString("EMCR")
#define GLB_SOFTWARE_VERSION_NUMBER (QString("%1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_PATCH))

/*! Settings tags */
#define GLB_PROTOCOL_RECORD_PATH_TAG "Protocol/Settings/recordPath"
#define GLB_PROTOCOL_RECORD_NAME_TAG "Protocol/Settings/recordName"
#define GLB_PROTOCOL_ADD_DATE_TAG "Protocol/Settings/addDate"
#define GLB_PROTOCOL_RECORD_FORMAT_TAG "Protocol/Settings/recordFormat"
#define GLB_PROTOCOL_VOLTAGE_FORMAT_TAG "Protocol/Settings/voltageFormat"
#define GLB_PROTOCOL_VOLTAGE_DECIMATOR_FACTOR_TAG "Protocol/Settings/voltageDecimationFactor"
#define GLB_PROTOCOL_RECORD_DURATION_TAG "Protocol/Settings/recordDuration"
#define GLB_PROTOCOL_CHUNK_DURATION_TAG "Protocol/Settings/chunkDuration"
#define GLB_PROTOCOL_FOLDER_TAG "Protocol/folder"
#define GLB_PREFERENCES_CURRENT_CHANNEL_X_COLOR_TAG "Preferences/Plot/currentChannel%1Color"
#define GLB_PREFERENCES_VOLTAGE_CHANNEL_X_COLOR_TAG "Preferences/Plot/voltageChannel%1Color"
#define GLB_PREFERENCES_DARK_MODE_TAG "Preferences/Plot/darkMode"
/*! Utility defines */

/*! Global defines */
#define STAMP_PLOT_MIN_WIDTH 50
#define STAMP_PLOT_MIN_HEIGHT 35
#define GLB_MAX_PROT_ID 0xFFFF

/*! Utility to debug by printf */
#define GLB_HERE { qDebug()<<__FILE__<<__LINE__; }

/*! Undef to see all controls */
#ifndef GLB_SHOW_DEBUG_CTRLS
#define GLB_HIDE_DEBUG_CTRLS
#endif

#endif // GLOBALDEFINES_H
