#include "sv_abstract_device_interface.h"

svdevifc::SvDeviceInterface::SvAbstractDeviceInterface(svdevifc::SupportedDevices device, QObject *parent) : 
  QObject(parent)
{
  _current_device = device;
  
  switch (_current_device) {
    case NoDevice:
      
      break;
      
    case MAX35101Evaluate:
      _current_device = new SvMAX35101Evaluate();
      break;
      
    default:
      break;
  }
}






/** ------------------------------- **/
svdevifc::SvAbstractDevice SvAbstractDevice(QObject* parent)
{
  
}
