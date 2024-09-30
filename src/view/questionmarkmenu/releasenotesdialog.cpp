#include "releasenotesdialog.h"
#include <QTextStream>

ReleaseNotesDialog::ReleaseNotesDialog(QWidget* parent) :
    MessageDialog("Release Notes", true, parent) {
    {
        Version_t version;
        version.major = 0;
        version.minor = 18;
        version.patch = 2;
        version.sections[NewFeatures] << "Digital filters";
        version.sections[NewFeatures] << "Integral rms graph in spectrum plot";
        version.sections[NewFeatures] << "Spectrum and Irms export button";
        version.sections[NewFeatures] << "Auto zoom buttons";
        version.sections[NewFeatures] << "Release notes";
        version.sections[Gui] << "Removed some multichannel features for single channel devices";
        version.sections[Gui] << "Button to rearrange floating windows";
        version.sections[Gui] << "Improved instructions for the offset correction";
        version.sections[BugFixes] << "Fixed bug in event detection analysis";
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

            for (auto item : section) {
                stream << "\n- " << item;
            }
        }
    }
}
