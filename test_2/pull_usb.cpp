#include "pull_usb.h"

using namespace pullusb;

fres *pullusb::request(libusb_device_handle *handle/*, pullusb::MAX35101EV_ANSWER &databuf*/)
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


fres *pullusb::request_texas(libusb_device_handle *handle/*, pullusb::MAX35101EV_ANSWER &databuf*/)
{

  /** первый запрос **/  
  QByteArray b1 = QByteArray::fromHex("30353030303030303030303030303030"
                                      "30303030303030303030303030303030");

//  fres *result2 = nullptr;
  
  fres *result1 = pullusb::pullData_texas(handle, b1);

  return result1;
  
}

fres *pullusb::pullData_texas(libusb_device_handle *handle, QByteArray &ba)
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
  result->data = (char*)malloc(sizeof(TDC1000_ANSWER));
  
  // читаем повтор запроса
  result->code = libusb_bulk_transfer(handle, 0x81, (unsigned char*)result->data, sizeof(TDC1000_ANSWER), &transfered, 1000);
  
  // читаем данные
  result->code = libusb_bulk_transfer(handle, 0x81, (unsigned char*)result->data, sizeof(TDC1000_ANSWER), &transfered, 1000);

  if((result->code != 0)/* | (transfered != b.size())*/)
  {
    result->message = "Ошибка при получении ответа\0";
    return result;
  }

//  qDebug() << "11:" << int(result->data[0]) << int(result->data[1]) << int(result->data[2]) << int(result->data[3]) << int(result->data[4]);
  
  return result;
}
