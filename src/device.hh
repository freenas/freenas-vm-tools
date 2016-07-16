//
// Created by Jakub Klama on 15.07.2016.
//

#ifndef FREENAS_VM_TOOLS_DEVICE_HH
#define FREENAS_VM_TOOLS_DEVICE_HH

class device
{
public:
    virtual void open(const std::string &devnode = "") = 0;
    virtual void close() = 0;
    virtual bool connected() = 0;
    virtual int read(void *buf, int size) = 0;
    virtual int write(void *buf, int size) = 0;
};

#endif //FREENAS_VM_TOOLS_DEVICE_HH
