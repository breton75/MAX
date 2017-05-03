#include "pull_usb.h"

using namespace pullusb;

fres *pullusb::request(libusb_device_handle *handle, pullusb::MAX35101EV_ANSWER &databuf)
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
  
  fres *result = pullusb::pullData(handle, b1);
  if(result->code == 0)
  {
    result = pullusb::pullData(handle, b2);
    if((result->code == 0) & (nullptr != result->data))
    { 
      memcpy(&databuf, result->data, sizeof(pullusb::MAX35101EV_ANSWER));
//      qDebug() << "22: "<< int(result->data[0]) << int(result->data[1]) << int(result->data[2]) << int(result->data[3]) << int(result->data[4]);

      result = pullusb::pullData(handle, b3);
      if(result->code == 0)
      {
        
      }
    }
  }

  return result;
  
}


fres *pullusb::pullData(libusb_device_handle *handle, QByteArray &ba)
{
  fres *result = new fres;
  result->code = 0;
  result->data = nullptr;
    
  int transfered;
  
  result->code = libusb_bulk_transfer(handle, 0x01, (unsigned char*)ba.data(), ba.size(), &transfered, 1000);
  
  if(result->code != 0)
  {
    result->message = "Ошибка при отправке\0";
//    QMessageBox::critical(0, "Error", QString("Ошибка при отправке: %1").arg(result), QMessageBox::Ok);
    return result;
  }
  
  // принимаем ответ
  result->data = (char*)malloc(sizeof(MAX35101EV_ANSWER));
  
  result->code = libusb_bulk_transfer(handle, 0x81, (unsigned char*)result->data, sizeof(MAX35101EV_ANSWER), &transfered, 1000);

  if((result->code != 0)/* | (transfered != b.size())*/)
  {
    result->message = "Ошибка при получении ответа\0";
//    QMessageBox::critical(0, "Error", QString("Ошибка при получении ответа: %1").arg(result), QMessageBox::Ok);
    return result;
  }

//  qDebug() << "11:" << int(result->data[0]) << int(result->data[1]) << int(result->data[2]) << int(result->data[3]) << int(result->data[4]);
  
  return result;
}

