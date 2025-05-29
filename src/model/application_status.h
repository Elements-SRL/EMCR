#ifndef APPLICATION_STATUS_H
#define APPLICATION_STATUS_H

#include <channel_mapping.h>
#include "messagedispatcher.h"
#include <map>
#include "channel_and_name.h"

class ApplicationStatus {
//    TODO maybe use a status and act as a state machin, if the configuration is not the default one do we need another way to get channels num and board num?
private:
    std::vector<YAML::ChannelMapping> mappings;
    int voltageChannelsNum = 1;
    int currentChannelsNum = 1;
    int boardsNum = 1;
    MessageDispatcher * msgDisp;
    std::vector<int> filterVisibleChannels(std::vector<int>);
    std::map <uint16_t, bool> expandedTraces;
    std::map <uint16_t, bool> detailedPlots;
    bool plotDetailAuto;

public:
    ApplicationStatus(MessageDispatcher * msgDisp, std::string filepath = "");
    void loadChannelMappingFromYaml(std::string pathTofile);
    void setAllChannelsSelected(bool state);
    //void printEverything();
    int getVoltageChannelsNum();
    int getCurrentChannelsNum();
    int getBoardsNum();
    int getTemperatureChannelsNum();
    std::vector <ChannelModel *> getChannels();
    void setSelectedChannels(std::map<int, bool>);
    std::vector <int> getVisibleChannelsOnBoard(int boardIdx);
    std::vector <int> getVisibleChannelsOnRow(int rowIdx);
    std::vector <bool> getSelectedChannels();
    std::vector <uint16_t> getSelectedChannelsIndexes();
    std::vector <uint16_t> getExpandedChannelsIndexes();
    std::vector <uint16_t> getStimActiveChannelsIndexes();
    std::vector <uint16_t> getExpandedAndStimActiveChannelsIndexes();
    std::vector <uint16_t> getOffsetRecalibratingChannelsIndexes();
    std::vector <uint16_t> getLiquidJunctionCompensatingChannelsIndexes();
    std::vector <YAML::ChannelMapping> getMappings();
    std::set <int> getVisibleBoards();
    MessageDispatcher * getMessageDispatcher();
    std::vector <int> getVisibleChannels();
    std::vector<std::string> getNames();
    Measurement getSamplingRate();
    RangedMeasurement_t getVcVoltageRange();
    std::vector <RangedMeasurement_t> getVcCurrentRange();
    std::vector <RangedMeasurement_t> getCcVoltageRange();

    std::map <uint16_t, bool> getExpandedTraces();
    void setExpandedTraces(std::map <uint16_t, bool>);

    // Plot detail
    std::vector<uint16_t> getDetailedPlotIndexes();
    void setDetailedPlots(std::map <uint16_t, bool>);
    void clearPlotDetails();
    void setPlotDetailAuto(bool);
    bool isPlotDetailAuto();

    RangedMeasurement_t getCcCurrentRange();
    std::vector <RangedMeasurement_t> getVoltageRanges();
    RangedMeasurement_t getMaxVoltageRange();
    std::vector <RangedMeasurement_t> getCurrentRanges();
    RangedMeasurement_t getMaxCurrentRange();
    std::string getSerialNumber();
    std::string getDeviceInfoString();
    ClampingModality_t getClampingModality();
    std::string getClampingModalityString();
    bool isEpisodic();
};


#endif // APPLICATION_STATUS_H
