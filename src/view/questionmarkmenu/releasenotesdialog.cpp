#include "releasenotesdialog.h"
#include <QTextStream>

ReleaseNotesDialog::ReleaseNotesDialog(QWidget* parent) :
    MessageDialog("Release Notes", true, parent) {
    {
        Version_t version;
        version.major = VERSION_MAJOR;
        version.minor = VERSION_MINOR;
        version.patch = VERSION_PATCH;
        version.sections[NewFeatures] << "Linear fit lines drawn on top of I/V graphs";
        this->appendVersion(version);
    }
    {
        Version_t version;
        version.major = 0;
        version.minor = 24;
        version.patch = 16;
        version.sections[NewFeatures] << "Compatibility with new devices";
        version.sections[NewFeatures] << "Current tracking";
        version.sections[Gui] << "UI theme changed";
        version.sections[BugFixes] << "Made offset recalibration more robust in presence of high noise";
        version.sections[BugFixes] << "Fixed sinusoidal protocol in e192";
        version.sections[BugFixes] << "Fixed zap not being applied";
        version.sections[BugFixes] << "Fixed crash when a corrupted protocol file is loaded on startup";
        this->appendVersion(version);
    }
    {
        Version_t version;
        version.major = 0;
        version.minor = 23;
        version.patch = 3;
        version.sections[NewFeatures] << "Resistance estimation in current clamp";
        version.sections[NewFeatures] << "Increased max number of applicable sweeps";
        version.sections[BugFixes] << "Fixed recording in event detection tab that could not be stopped";
        version.sections[BugFixes] << "Fixed downsampling not correctly decimating the data";
        version.sections[BugFixes] << "Fixed default status of AUTO buttons";
        version.sections[BugFixes] << "Fixed ranges shown in I0 clamp";
        version.sections[BugFixes] << "Current protocols set correctly the range";
        version.sections[BugFixes] << "Fixed crash happening when a voltage and current protocols have the same name";
        version.sections[BugFixes] << "Device info working for all devices";
        version.sections[BugFixes] << "Fixed driver installer not running during SW installation";
        this->appendVersion(version);
    }
    {
        Version_t version;
        version.major = 0;
        version.minor = 22;
        version.patch = 2;
        version.sections[NewFeatures] << "Handles new 2x10MHz and 4x10MHz nanopore readers";
        version.sections[NewFeatures] << "Added Plot Detail feature: plots a single trace in a separate resizable window";
        version.sections[BugFixes] << "Fixed crash in I/V graph";
        version.sections[BugFixes] << "Fixed crash during clamping modality change";
        this->appendVersion(version);
    }
    {
        Version_t version;
        version.major = 0;
        version.minor = 20;
        version.patch = 1;
        version.sections[NewFeatures] << "Added episodic plots and recordings for patch clamp experiments";
        version.sections[NewFeatures] << "Added compensations for patch clamp experiments";
        version.sections[NewFeatures] << "Added autodeclogger for nanopore experiments";
        version.sections[NewFeatures] << "Online resistance estimation";
        version.sections[NewFeatures] << "Online pipette capacitance estimation";
        version.sections[NewFeatures] << "Online membrane estimation";
        version.sections[Gui] << "Added slidebar and collapsible sections to device controls";
        version.sections[Qol] << "Implemented auto buttons for stimulus and trace expand as well";
        version.sections[BugFixes] << "Fixed measurement overview buggy behaviour";
        version.sections[BugFixes] << "Improved accuracy of timers";
        version.sections[BugFixes] << "Fixed resistance estimation crash on single channel devices";
        version.sections[BugFixes] << "Fixed recordings not starting";
        this->appendVersion(version);
    }
    {
        Version_t version;
        version.major = 0;
        version.minor = 18;
        version.patch = 3;
        version.sections[NewFeatures] << "Digital filters";
        version.sections[NewFeatures] << "Integral rms graph in spectrum plot";
        version.sections[NewFeatures] << "Spectrum and Irms export button";
        version.sections[NewFeatures] << "Auto zoom buttons";
        version.sections[NewFeatures] << "Release notes";
        version.sections[Gui] << "Removed some multichannel features for single channel devices";
        version.sections[Gui] << "Button to rearrange floating windows";
        version.sections[Gui] << "Improved instructions for the offset correction";
        version.sections[BugFixes] << "Fixed a bug in event detection analysis that caused some events to be missed";
        version.sections[BugFixes] << "Fixed a bug in event detection analysis that caused the recorded stimulus to be wrong";
        version.sections[BugFixes] << "Fixed spectrum not starting for multichannel devices";
        version.sections[BugFixes] << "Fixed channel number in Measurement overview exported csv";
        this->appendVersion(version);
    }
    {
        Version_t version;
        version.major = 0;
        version.minor = 17;
        version.patch = 1;
        version.sections[NewFeatures] << "Event detection";
        version.sections[NewFeatures] << "Spectrum";
        version.sections[NewFeatures] << "Current offset recalibration";
        version.sections[NewFeatures] << "Reset HW button";
        version.sections[Gui] << "Moved controls close to relevant plots";
        version.sections[Gui] << "Digital offset compensation renamed Liquid junction compensation";
        version.sections[Gui] << "Offset correction wizard";
        version.sections[Qol] << "Liquid junction compensation stops by itself";
        this->appendVersion(version);
    }

    this->addMainText(mainText);

    this->addDefaultButtonBox();

    this->centerOnParent(parent);
}

void ReleaseNotesDialog::appendVersion(Version_t version) {
    QTextStream stream(&mainText);
    if (!mainText.isEmpty()) {
        stream << "\n\n";
    }
    stream << "Version " << version.major << "." << version.minor << "." << version.patch;

    for (int sectionIdx = 0; sectionIdx < SectionsNum; sectionIdx++) {
        auto section = version.sections[sectionIdx];
        if (!section.isEmpty()) {
            stream << "\n\n";
            switch (sectionIdx) {
            case NewFeatures:
                stream << "New features";
                break;

            case Gui:
                stream << "GUI";
                break;

            case Qol:
                stream << "QoL";
                break;

            case BugFixes:
                stream << "Bug fixes";
                break;
            }

            for (auto &item : section) {
                stream << "\n- " << item;
            }
        }
    }
}
