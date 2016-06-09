#
# Copyright 2016 iXsystems, Inc.
# All rights reserved
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted providing that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
#####################################################################

import os
import logging
import argparse
import platform
from threading import Thread
from freenas.dispatcher.rpc import RpcContext, RpcException
from freenas.dispatcher.server import Server
import transport


DEVICE_NODES = {
    'Linux': '/dev/virtio-ports/org.freenas.vm-tools',
    'FreeBSD': '/dev/vtcon/org.freenas.vm-tools'
}


class Context(object):
    def __init__(self):
        self.server = Server(self)
        self.rpc = RpcContext()
        self.server_thread = None

    @staticmethod
    def find_device_node():
        try:
            node = DEVICE_NODES[platform.system()]
            if not os.path.exists(node):
                return
        except KeyError:
            return

        return node

    def init_server(self):
        node = self.find_device_node()
        if not node:
            raise RuntimeError('Cannot find vm-tools virtio port')

        self.server.start('virtio://{0}'.format(node))
        self.server_thread = Thread(target=self.server.serve_forever)
        self.server_thread.name = 'ServerThread'
        self.server_thread.daemon = True
        self.server_thread.start()

    def main(self):
        logging.basicConfig(level=logging.DEBUG)
        self.init_server()


if __name__ == '__main__':
    m = Context()
    m.main()
