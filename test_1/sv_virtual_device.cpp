#include "sv_virtual_device.h"

SvVirtualDevice::SvVirtualDevice()
{

}

SvVirtualDevice::~SvVirtualDevice()
{
  stop();
  close();
  deleteLater();
}

bool SvVirtualDevice::open()
{
  setOpened(true);
  return true;
}

void SvVirtualDevice::close()
{
  stop();
  setOpened(false);
}

bool SvVirtualDevice::start(quint32 msecs)
{
  if(!isOpened()) {
    setLastError(QString("%1: device is not opened").arg(deviceInfo().name));
    return false;
  }
  
  if(_thr)
    delete _thr;
  
  _thr = new SvPullVirtualDevice(msecs, &mutex);
  connect(_thr, SIGNAL(new_data(svidev::MeasuredData )), this, SIGNAL(new_data(svidev::MeasuredData )));
  _thr->start();
  
  
}

bool SvVirtualDevice::stop()
{
  if(_thr)
    delete _thr;
  
  _thr = nullptr;
}

/** ******************************* **/

SvPullVirtualDevice::~SvPullVirtualDevice()
{ 
  stop();
  deleteLater();
}

void SvPullVirtualDevice::run()
{
  _started = true;
  _finished = false;
  
  svidev::MeasuredData measured_data;
  
  while(_started)
  {
    
    _mutex->lock();
    
    measured_data.tof1 = (rand()%10 + 1) / 10.0 * 78000.0; //, &a.hit_up_average);
    measured_data.tof2 = (rand()%10 + 2) / 10.0 * 78000.0; //, &a.hit_down_average);
     
    _mutex->unlock();
    
    emit new_data(measured_data); 
    
    msleep(_timeout);
    
  }
  
  _finished = true;
  
}

void SvPullVirtualDevice::stop()
{
  _started = false;
  while(!_finished) QApplication::processEvents();
}
