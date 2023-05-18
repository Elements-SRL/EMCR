#ifndef PROTOCOLDEFS_H
#define PROTOCOLDEFS_H

typedef enum {
    ProtocolTypeGapfree = 0,
    ProtocolTypeEpisodic = 1
} ProtocolType_t;

typedef enum {
    ProtocolConsumerNone, /*!< This is a marker, must not be used to identify single consumer types */
    ProtocolConsumerDataWriter,
    ProtocolConsumerAnalysis, /*!< This is a marker, must not be used to identify single consumer types */
    ProtocolConsumerNoiseReport,
    ProtocolConsumerHistogram,
    ProtocolConsumerSpectrum,
    ProtocolConsumerResistanceEstimation,
    ProtocolConsumerMembraneTest,
    ProtocolConsumerIvGraph,
    ProtocolConsumerVoltageTracking,
    ProtocolConsumerApThreshold,
    ProtocolConsumerApStatistics,
    ProtocolConsumerTypesNum /*!< This is a marker, must not be used to identify single consumer types */
} ProtocolConsumerType_t;

#define PTD_PROTOCOL_ANALYSIS_OFFSET (ProtocolConsumerAnalysis+1)
#define PTD_PROTOCOL_ANALYSIS_NUM (ProtocolConsumerTypesNum-PTD_PROTOCOL_ANALYSIS_OFFSET)

#endif // PROTOCOLDEFS_H
