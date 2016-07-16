//
// Created by Jakub Klama on 15.07.2016.
//

#ifndef FREENAS_VM_TOOLS_UNIX_DEVICE_HH
#define FREENAS_VM_TOOLS_UNIX_DEVICE_HH

#include "../src/device.hh"

class unix_device: public device
{
public:
    virtual void open(const std::string &devnode = "");
    virtual void close();
    virtual bool connected();
    virtual int read(void *buf, int count);
    virtual int write(void *buf, int count);

private:
    const std::string &find_device_node();

    std::string m_path;
    int m_fd = -1;
};

#endif //FREENAS_VM_TOOLS_UNIX_DEVICE_HH
