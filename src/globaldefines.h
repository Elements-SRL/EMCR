#ifndef GLOBALDEFINES_H
#define GLOBALDEFINES_H

#include <math.h>

#include <QDebug>
#include <QCoreApplication>

/*! Sw info */
#define GLB_SOFTWARE_NAME "EMCR"
#define GLB_SOFTWARE_VERSION_NUMBER (QString("%1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_PATCH))

/*! Settings tags */
#define GLB_PROTOCOL_RECORD_PATH_TAG "Protocol/Settings/recordPath"
#define GLB_PROTOCOL_RECORD_NAME_TAG "Protocol/Settings/recordName"
#define GLB_PROTOCOL_ADD_DATE_TAG "Protocol/Settings/addDate"
#define GLB_PROTOCOL_RECORD_FORMAT_TAG "Protocol/Settings/recordFormat"
#define GLB_PROTOCOL_RECORD_DURATION_TAG "Protocol/Settings/recordDuration"
#define GLB_PROTOCOL_CHUNK_DURATION_TAG "Protocol/Settings/chunkDuration"

/*! Utility defines */

/*! Global defines */
#define STAMP_PLOT_SIZE 35

/*! Utility to debug by printf */
#define GLB_HERE { qDebug()<<__FILE__<<__LINE__; }

/*! Undef to see all controls */
#ifndef GLB_SHOW_DEBUG_CTRLS
#define GLB_HIDE_DEBUG_CTRLS
#endif

#endif // GLOBALDEFINES_H
