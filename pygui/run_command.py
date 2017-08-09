#!/usr/bin/env python2

import ctypes

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True


def run_command(command, host, port):
    sock = ROOT.TSocket(host,port)
    sock.Send(command)
    message = ROOT.TMessage()
    bytes_received = sock.Recv(message)
    if bytes_received <= 0:
        return None

    if message.What()==ROOT.kMESS_STRING:
        arr = ctypes.create_string_buffer(256)
        message.ReadString(arr,256)
        return arr.value
    elif message.What()==ROOT.kMESS_OBJECT:
        obj = message.ReadObject(message.GetClass())
        return obj
    else:
        return None
