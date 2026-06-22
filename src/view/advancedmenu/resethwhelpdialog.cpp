#include "resethwhelpdialog.h"
#include <QLabel>

ResetHwHelpDialog::ResetHwHelpDialog(QWidget* parent) :
    MessageDialog("HW reset usage", true, parent) {

    this->addMainText(
        "<p style='margin-bottom: 10px; font-size: 11px; font-weight: bold;'>"
        "⚠️ The HW reset should be used if the device seems stuck in saturation."
        "</p>"

        "<p style='margin-bottom: 12px;'>"
        "Before using the HW reset feature please check that the saturation is "
        "actually due to a front end problem. In order to do so check that:"
        "</p>"

        "<ul style='margin-left: 12px;'>"
        "  <li style='margin-bottom: 6px;'>"
        "    <b>Voltage clamp range:</b> You are using a proper current range in Voltage clamp; "
        "    increase the current range and zoom out vertically to verify that the current trace "
        "    is always stuck on the top or bottom value of the selected range."
        "  </li>"
        "  <li style='margin-bottom: 6px;'>"
        "    <b>DUT Resistance:</b> The resistance of your DUT is not too low in voltage clamp "
        "    or too high in current clamp; in particular short circuits always saturate the "
        "    voltage clamp front end and open circuits always saturate the current clamp front end."
        "  </li>"
        "  <li style='margin-bottom: 6px;'>"
        "    <b>Traces Zoom:</b> You are using a proper zoom to visualize the traces; in order to "
        "    completely reset the zoom and center it around the traces, <b>double right-click</b> on "
        "    any plot and click the <i>Full trace zoom</i> in the higher left corner of any plot window."
        "  </li>"
        "</ul>"
        );

    this->addDefaultButtonBox();

    this->centerOnParent(parent);
}
