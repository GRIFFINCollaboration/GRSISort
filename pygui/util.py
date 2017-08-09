#!/usr/bin/env python2

import re

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

def unpack_tdirectory(tdir):
    if tdir.GetListOfKeys():
        for key in tdir.GetListOfKeys():
            yield key.ReadObj()
    else:
        for obj in tdir.GetList():
            yield obj

def update_tcanvases(objects=None):
    if objects is None:
        objects = ROOT.gROOT.GetListOfCanvases()

    for obj in objects:
        if isinstance(obj, ROOT.TPad):
            obj.Modified()
            obj.Update()
            update_tcanvases(obj.GetListOfPrimitives())


def increment_name(name):
    res = re.search('[0-9]+$', name)
    if res:
        prefix = name[:-len(res.group())]
        number = int(res.group()) + 1
        return prefix + str(number)
    else:
        return name + '_1'


class PreserveGDir(object):

    def __init__(self, directory = None):
        self.directory = directory

    def __enter__(self):
        self.gdir = ROOT.gDirectory
        if self.directory is not None:
            self.directory.cd()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.gdir.cd()


class TKeyDict(dict):
    def __getitem__(self, key):
        output = super(TKeyDict,self).__getitem__(key)
        if isinstance(output, ROOT.TKey):
            output = output.ReadObj()
            if (isinstance(output, ROOT.TH2) and
                not isinstance(output, ROOT.GH2Base)):
                output = ROOT.GH2D(output)
            self[key] = output

        return output

    def is_tkey(self, key):
        output = super(TKeyDict,self).__getitem__(key)
        try:
            return isinstance(output, ROOT.TKey)
        except AttributeError:
            return False
