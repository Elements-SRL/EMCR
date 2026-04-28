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
    QLabel* copyrightLbl = new QLabel("Copyright (c) 2015-" + yearStr + " by Elements s.r.l.");
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
    QString htmlText =
        "<h3>Third-party libraries</h3>"
        "<table width='100%' cellpadding='0' cellspacing='0' style='border: none;'>"
        "  <tr>"
        // Qt
        "    <td align='left' style='vertical-align: middle;'>"
        "       <span><a href='https://qt.io'>Qt Framework</a> v"
        QT_VERSION_STR
        "       </span>"
        "    </td>"
        "    <td align='right' style='vertical-align: middle;'>"
        "       <a href='https://doc.qt.io/qt-6/lgpl.html'>LGPL 3</a>"
        "    </td>"
        "  </tr>"
        // Qwt
        "  <tr>"
        "    <td align='left' style='vertical-align: middle;'>"
        "       <span><a href='https://qwt.sourceforge.io'>Qwt</a> v"
        + QString("%1.%2.%3").arg(QWT_VER_MAJOR).arg(QWT_VER_MINOR).arg(QWT_VER_PATCH) +
        "       </span>"
        "    </td>"
        "    <td align='right' style='vertical-align: middle;'>"
        "       <a href='https://qwt.sourceforge.io/qwtlicense.html'>Qwt 1.0</a>"
        "    </td>"
        "  </tr>"
        // FFTW
        "  <tr>"
        "    <td align='left' style='vertical-align: middle;'>"
        "       <span><a href='https://www.fftw.org/'>FFTW</a> v"
        + QString("%1.%2.%3").arg(FFTW_VER_MAJOR).arg(FFTW_VER_MINOR).arg(FFTW_VER_PATCH) +
        "       </span>"
        "    </td>"
        "    <td align='right' style='vertical-align: middle;'>"
        "       <a href='https://www.fftw.org/doc/License-and-Copyright.html'>GPL 2</a>"
        "    </td>"
        "  </tr>"
        "</table>"
        "<hr>"
        "<p>This software is provided under the terms of the MIT License</p>"
        "<p>Copyright (c) " + yearStr + " Elements s.r.l.</p>"
        "<p>Permission is hereby granted, free of charge, to any person obtaining a copy "
        "of this software and associated documentation files (the ""Software""), to deal "
        "in the Software without restriction, including without limitation the rights "
        "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
        "copies of the Software, and to permit persons to whom the Software is "
        "furnished to do so, subject to the following conditions:</p>"
        "<p>The above copyright notice and this permission notice shall be included in all "
        "copies or substantial portions of the Software.</p>"
        "<p>THE SOFTWARE IS PROVIDED ""AS IS"", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
        "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
        "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE "
        "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
        "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
        "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE "
        "SOFTWARE.</p>";
    ackTb->setHtml(htmlText);
    mainVl->addWidget(ackTb);

    this->addDefaultButtonBox();

    this->centerOnParent(parent);
}
