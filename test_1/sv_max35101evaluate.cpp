#include "sv_max35101evaluate.h"

SvMAX35101Evaluate::SvMAX35101Evaluate()
{

}

SvMAX35101Evaluate::~SvMAX35101Evaluate()
{
  libusb_release_interface(_handle, 0); // отпускаем интерфейс 0
  libusb_close(_handle);  // закрываем устройство
  libusb_exit(NULL);  // завершаем работу с библиотекой 
}

bool SvMAX35101Evaluate::open()
{
  libusb_context *ctx = NULL;  
  libusb_init(NULL);  // инициализируем библиотеку
  
  // открываем устройство
  _handle = libusb_open_device_with_vid_pid(ctx, _device_info.idVendor, _device_info.idProduct);
    
  if (!_handle)
  {
    libusb_exit(NULL);
    _last_error = QString("%1: libusb_open_device_with_vid_pid error. (idVendor = %2, idProduct = %3")
                  .arg(deviceInfo().name).arg(deviceInfo().idVendor).arg(deviceInfo().idProduct);
    return false;
  }
  
  // запрашиваем интерфейс 0 для себя
  int err = libusb_claim_interface(_handle, 0); 
  if(err < 0) { 
    _last_error = QString("%1: libusb_claim_interface error: %2").arg(deviceInfo().name).arg(err);
    return false;
  }
  
  return true;
}

fres *request(libusb_device_handle *handle/*, pullusb::MAX35101EV_ANSWER &databuf*/)
{

  /** первый запрос **/  
  QByteArray b1 = QByteArray::fromHex("3200020000000000000000000000000000000000000000000000000000000000"
                                      "0000000000000000000000000000000000000000000000000000000000000000");
  
  /** второй запрос **/ 
  QByteArray b2 = QByteArray::fromHex("3222C40000000000000000000000000000000000000000000000000000000000"
                                      "0000000000000000000000000000000000000000000000000000000000000000");
  
  /** третий запрос **/  
  QByteArray b3 = QByteArray::fromHex("3422C40000000000000000000000000000000000000000000000000000000000"
                                      "0000000000000000000000000000000000000000000000000000000000000000");
  
  fres *result2 = nullptr;
  
  fres *result1 = pullusb::pullData(handle, b1);
  if(result1->code == 0)
  {
    
    result2 = pullusb::pullData(handle, b2);
    if((result2->code == 0) & (nullptr != result2->data))
    { 
//      memcpy(&databuf, result2->data, sizeof(pullusb::MAX35101EV_ANSWER));
//      qDebug() << "22: "<< int(result->data[0]) << int(result->data[1]) << int(result->data[2]) << int(result->data[3]) << int(result->data[4]);
      
      
      fres *result3 = pullusb::pullData(handle, b3);
      if(result3->code == 0)
      {
        
      }
      delete result3;
    }
  }
  delete result1;

  return result2;
  
}


fres *pullData(libusb_device_handle *handle, QByteArray &ba)
{
  fres *result = new fres;
  result->code = 0;
  result->data = nullptr;
    
  int transfered;
  
  result->code = libusb_bulk_transfer(handle, 0x01, (unsigned char*)ba.data(), ba.size(), &transfered, 1000);
  
  if(result->code != 0)
  {
    result->message = "Ошибка при отправке\0";
    return result;
  }
  
  // принимаем ответ
  result->data = (char*)malloc(sizeof(MAX35101EV_ANSWER));
  
  result->code = libusb_bulk_transfer(handle, 0x81, (unsigned char*)result->data, sizeof(MAX35101EV_ANSWER), &transfered, 1000);

  if((result->code != 0)/* | (transfered != b.size())*/)
  {
    result->message = "Ошибка при получении ответа\0";
    return result;
  }

//  qDebug() << "11:" << int(result->data[0]) << int(result->data[1]) << int(result->data[2]) << int(result->data[3]) << int(result->data[4]);
  
  return result;
}
