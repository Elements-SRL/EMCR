#ifndef RELEASENOTESDIALOG_H
#define RELEASENOTESDIALOG_H

#include "messagedialog.h"

class ReleaseNotesDialog : public MessageDialog {
    Q_OBJECT

public:
    ReleaseNotesDialog(QWidget* parent = nullptr);

private:
    typedef enum Section {
        NewFeatures,
        Gui,
        Qol,
        BugFixes,
        SectionsNum
    } Section_t;
    typedef struct Version {
        int major = 0;
        int minor = 0;
        int patch = 0;
        QStringList sections[SectionsNum];
    } Version_t;

    void appendVersion(Version_t version);

    QString mainText = "";
};

#endif // RELEASENOTESDIALOG_H
