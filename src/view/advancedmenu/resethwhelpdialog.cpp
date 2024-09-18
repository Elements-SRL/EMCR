#include "resethwhelpdialog.h"
#include <QLabel>

ResetHwHelpDialog::ResetHwHelpDialog(QWidget* parent) :
    MessageDialog("HW reset usage", true, parent) {

    this->addMainText("The HW reset should be used if the device seems stuck in saturation.\n"
                      "Before using the HW reset feature please check that the saturation is\n"
                      "actually due to a front end problem."
                      "In order to do so check that:\n"
                      "- you are using a proper current range in Voltage clamp; increase the current\n"
                      "  range and zoom out vertically to verify that the current trace is always stuck\n"
                      "  on the top or bottom value of the selected range;\n"
                      "- the resistance of your DUT is not too low in voltage clamp or too high in current\n"
                      "  clamp; in particular short circuits always saturate the voltage clamp front end\n"
                      "  and open circuits always saturate the current clamp front end;\n"
                      "- you are using a proper zoom to visualize the traces; in order to completely reset\n"
                      "  the zoom and center it around the traces double right click on any plot and click\n"
                      "  the Full trace zoom in the higher left corner of any plot window.");

    this->addDefaultButtonBox();

    this->centerOnParent(parent);
}
