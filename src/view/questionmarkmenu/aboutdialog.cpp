#include "aboutdialog.h"
#include <QLabel>
#include "globaldefines.h"

#include <QDate>
#include <QTextBrowser>

AboutDialog::AboutDialog(QWidget* parent) :
    MessageDialog(QString("About ") + GLB_SOFTWARE_NAME, true, parent) {

    QLabel* versionLbl = new QLabel(QString("Version ") + GLB_SOFTWARE_VERSION_NUMBER);
    versionLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(versionLbl);

    QString yearStr = QString("%1").arg(QDate().currentDate().year());
    QLabel* copyrightLbl = new QLabel("Copyright (c) 2023-" + yearStr + " by Elements s.r.l.");
    copyrightLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(copyrightLbl);

    QLabel* emailLbl = new QLabel("info@elements-ic.com");
    emailLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(emailLbl);

    QLabel* websiteLbl = new QLabel("www.elements-ic.com");
    websiteLbl->setAlignment(Qt::AlignCenter);
    mainVl->addWidget(websiteLbl);

    QTextBrowser * ackTb = new QTextBrowser();
    ackTb->setReadOnly(true);
    ackTb->setOpenExternalLinks(true);

    QString librariesText = "<h2>" + GLB_SOFTWARE_NAME + " is based on the following libraries</h2>";
    librariesText += "<h3>Elements open source libraries</h3><table width='100%' cellpadding='0' cellspacing='0' style='border: none;'>";
    librariesText += addLibrary("e384commlib",
                                "https://github.com/Elements-SRL/e384commLib",
                                GLB_COMMLIB_VERSION_NUMBER,
                                "MIT",
                                "https://github.com/Elements-SRL/e384commLib?tab=MIT-1-ov-file");
    librariesText += addLibrary("ftdi_utils",
                                "https://github.com/Elements-SRL/ftdi_utils/",
                                FTDIUTILS_VER_MAJOR,
                                FTDIUTILS_VER_MINOR,
                                FTDIUTILS_VER_PATCH,
                                "MIT",
                                "https://github.com/Elements-SRL/ftdi_utils/?tab=MIT-1-ov-file");
    librariesText += addLibrary("protocol-serializer",
                                "https://github.com/Elements-SRL/protocol-serializer/",
                                PROTSER_VER_MAJOR,
                                PROTSER_VER_MINOR,
                                PROTSER_VER_PATCH,
                                "MIT",
                                "https://github.com/Elements-SRL/protocol-serializer/?tab=MIT-1-ov-file");
    librariesText += "</table>";
    librariesText += "<hr>";

    librariesText += "<h3>Third-party open source libraries</h3><table width='100%' cellpadding='0' cellspacing='0' style='border: none;'>";
    librariesText += addLibrary("Qt Framework",
                                "https://qt.io",
                                QT_VERSION_STR,
                                "LGPLv3",
                                "https://doc.qt.io/qt-6/lgpl.html");
    librariesText += addLibrary("Qwt",
                                "https://qwt.sourceforge.io",
                                QWT_VER_MAJOR,
                                QWT_VER_MINOR,
                                QWT_VER_PATCH,
                                "Qwt 1.0",
                                "https://qwt.sourceforge.io/qwtlicense.html");
    librariesText += addLibrary("POCKETFFT",
                                "https://github.com/mreineck/pocketfft",
                                POCKETFFT_VER_MAJOR,
                                POCKETFFT_VER_MINOR,
                                POCKETFFT_VER_PATCH,
                                "3-clause BSD",
                                "https://github.com/mreineck/pocketfft?tab=BSD-3-Clause-1-ov-file");
    librariesText += addLibrary("HDF5 Software Library and Utilities",
                                "https://github.com/HDFGroup/hdf5",
                                H5_VER_MAJOR,
                                H5_VER_MINOR,
                                H5_VER_PATCH,
                                "3-clause BSD",
                                "https://github.com/HDFGroup/hdf5/?tab=License-1-ov-file");
    librariesText += addLibrary("Axon Library",
                                "https://sourceforge.net/projects/libaxon/",
                                AXONLIB_VER_MAJOR,
                                AXONLIB_VER_MINOR,
                                AXONLIB_VER_PATCH,
                                "LGPLv3",
                                "https://www.gnu.org/licenses/lgpl-3.0.txt");
    librariesText += addLibrary("yaml-cpp",
                                "https://github.com/jbeder/yaml-cpp",
                                YAML_VER_MAJOR,
                                YAML_VER_MINOR,
                                YAML_VER_PATCH,
                                "MIT",
                                "https://github.com/jbeder/yaml-cpp?tab=MIT-1-ov-file");
    librariesText += addLibrary("toml++",
                                "https://marzer.github.io/tomlplusplus/",
                                TOML_VER_MAJOR,
                                TOML_VER_MINOR,
                                TOML_VER_PATCH,
                                "MIT",
                                "https://github.com/marzer/tomlplusplus/blob/master/LICENSE");
    librariesText += "</table>";
    librariesText += "<hr>";

    librariesText += "<h3>Third-party closed source libraries</h3><table width='100%' cellpadding='0' cellspacing='0' style='border: none;'>";
    librariesText += addLibrary("FrontPanel",
                                "https://pins.opalkelly.com/downloads",
                                OK_VER_MAJOR,
                                OK_VER_MINOR,
                                OK_VER_PATCH,
                                "",
                                "");
    librariesText += addLibrary("D2XX",
                                "https://ftdichip.com/drivers/d2xx-drivers/",
                                FT_VER_MAJOR,
                                FT_VER_MINOR,
                                FT_VER_BUILD,
                                "",
                                "");
    librariesText += addLibrary("LibMPSSE-SPI",
                                "https://ftdichip.com/software-examples/mpsse-projects/libmpsse-spi-examples/",
                                MPSSE_VER_MAJOR,
                                MPSSE_VER_MINOR,
                                MPSSE_VER_PATCH,
                                "",
                                "");
    librariesText += addLibrary("EZ-USB FX3 SDK",
                                "https://www.infineon.com/design-resources/development-tools/sdk/usb-controllers-sdk/ez-usb-fx3-software-development-kit",
                                CY_VER_MAJOR,
                                CY_VER_MINOR,
                                CY_VER_PATCH,
                                "",
                                "");
    librariesText += "</table>";

    ackTb->setHtml(librariesText);
    mainVl->addWidget(ackTb);

    this->addDefaultButtonBox();

    this->centerOnParent(parent);
}

QString AboutDialog::addLibrary(QString libName, QString libUrl, QString libVer, QString licenseName, QString licenseUrl) {
    QString appendedText =
    "  <tr>"
    "    <td align='left' style='vertical-align: middle;'>"
    "       <span><a href='" + libUrl + "'>" + libName + "</a> v" + libVer +  "</span>"
    "    </td>"
    "    <td align='right' style='vertical-align: middle;'>"
    "       <a href='" + licenseUrl + "'>" + licenseName + "</a>"
    "    </td>"
    "  </tr>";
    return appendedText;
}

QString AboutDialog::addLibrary(QString libName, QString libUrl, int libMaj, int libMin, int libPat, QString licenseName, QString licenseUrl) {
    return addLibrary(libName, libUrl, QString("%1.%2.%3").arg(libMaj).arg(libMin).arg(libPat), licenseName, licenseUrl);
}
