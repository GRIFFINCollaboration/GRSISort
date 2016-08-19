#!/usr/bin/env python2

import itertools
import Tkinter as tk
import ttk

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

from .util import unpack_tdirectory, update_tcanvases, TKeyDict

class HistTab(object):

    def __init__(self, main, frame):
        self.main = main
        self._setup_GUI(frame)
        self.active_dirs = []

        self._requires_resort = False
        self.CheckOnlineHists()
        self.main.window.after_idle(self._PeriodicHistogramCheck)

    def _setup_GUI(self, frame):
        self.frame = frame
        self._MakeHistView(frame)

    def _MakeHistView(self,parent):
        self.treeview = ttk.Treeview(parent)
        self.treeview.pack(fill=tk.BOTH,expand=True)
        # Map from treeview name to ROOT object
        self.hist_lookup = TKeyDict()
        self.treeview.bind("<Double-1>", self.OnHistClick)

    def AddActiveDirectory(self, tdir):
        if tdir.GetName() not in [d.GetName() for d in self.active_dirs]:
            self.active_dirs.append(tdir)

    def OnHistClick(self,event):
        objects = {name:self.hist_lookup[name]
                   for name in event.widget.selection()}
        histograms = {name:h for name,h in objects.items()
                      if isinstance(h, ROOT.TH1)}

        color = 1;
        for name,obj in sorted(histograms.items()):
            if isinstance(obj, ROOT.GH2Base):
                self._refresh(name, obj)

            self.main._draw_single(obj,color,len(histograms))
            if self.main.plotlocation.get()=='Overlay':
                color+=1
                if color == 5:
                    color+=1

        update_tcanvases()

    def _dump_to_tfile(self):
        for key in self.hist_lookup:
            self.hist_lookup[key].Write()

    def _load_compiled_histograms(self, filename):
        print 'load compiled histograms called.'
        #pipeline = ROOT.GetPipeline(0)
        #if outfile:
        #    pipeline.SetHistogramLibrary(filename)

    def _compiled_histogram_filename(self):
        print ' _compiled_histogram_filename  called.\n'
        #pipeline = ROOT.GetPipeline(0)
        #if pipeline:
        #    return pipeline.GetLibraryName()
        #else:
        return ''

    def _refresh(self, name, obj):
        parent = self._find_parent(name)
        if parent is not None:
            self.Insert(obj, parent)

    def _find_parent(self, name, base=''):
        children = self.treeview.get_children(base)
        if name in children:
            # Found it, done
            return base
        elif children:
            # Search in the children
            for child in children:
                parent = self._find_parent(name, child)
                if parent:
                    return parent
        else:
            # Not here.
            return None

    def Insert(self,obj,parent='',objname=None,icon=None):
        if not obj:
            return

        if objname is None:
            objname = obj.GetName()

        if (isinstance(obj, ROOT.TKey) and
            not issubclass(getattr(ROOT, obj.GetClassName()), ROOT.TH1)):
            obj = obj.ReadObj()

        if (isinstance(obj, ROOT.TTree) or
            isinstance(obj, ROOT.TCutG) or
            isinstance(obj, ROOT.TProcessID)):
            return

        if parent:
            name = parent + '/' + objname
        else:
            name = objname

        self._insert_single_nonrecursive(obj, parent, name, objname, icon)

        if isinstance(obj, ROOT.TList):
            iterable = obj
        elif isinstance(obj, ROOT.TDirectory):
            iterable = obj.GetListOfKeys()
            if not iterable:
                iterable = obj.GetList()
        elif isinstance(obj, ROOT.GH2Base):
            iterable = itertools.chain(obj.GetProjections(),
                                       obj.GetSummaryProjections())
        else:
            iterable = None

        if iterable is not None:
            for obj in iterable:
                self.Insert(obj, name)

    def _insert_single_nonrecursive(self, obj, parent, name,
                                    objname=None, icon=None):
        is_histogram_key = (isinstance(obj, ROOT.TKey) and
                            issubclass(getattr(ROOT, obj.GetClassName()), ROOT.TH1))
        is_histogram = isinstance(obj, ROOT.TH1)

        if objname is None:
            objname = obj.GetName()

        if (is_histogram_key and
            name in self.hist_lookup and
            not self.hist_lookup.is_tkey(name)):
            # If the histogram has already been read, copy it in
            orig = self.hist_lookup[name]
            obj.Copy(orig)
            orig.SetDirectory(0)
        else:
            # TKeyDict makes a Clone, so the histograms updating won't
            # require an update of the canvas.
            self.hist_lookup[name] = obj

        if name not in self.treeview.get_children(parent):
            #print 'Performing insertion of',name,'into',parent
            self._requires_resort = True
            if icon is None:
                icon = self.main._PickIcon(obj)
            self.treeview.insert(parent,'end', name, text=objname,image=icon)

    def _PeriodicHistogramCheck(self):
        self.CheckOnlineHists()
        self.main.window.after(1000, self._PeriodicHistogramCheck)

    def CheckOnlineHists(self):
        for tdir in self.active_dirs:
            if tdir.GetList():
                self.Insert(tdir.GetList(),
                            objname=tdir.GetName(), icon=self.main.icons['tfile'])
            if tdir.GetListOfKeys():
                self.Insert(tdir.GetListOfKeys(),
                            objname=tdir.GetName(), icon=self.main.icons['tfile'])

        if self._requires_resort:
            self.Resort()
            self._requires_resort = False

    def Resort(self, parent=''):
        children = list(self.treeview.get_children(parent))
        if not children:
            return

        children.sort()
        for index, name in enumerate(children):
            self.treeview.move(name, parent, index)

        for child in children:
            self.Resort(child)


    def InsertHist(self, hist):
        dirname = hist.GetDirectory().GetName()

        if dirname in self.treeview.get_children(''):
            icon = self.main._PickIcon(hist)
            name = dirname + '/' + hist.GetName()
            self._insert_single_nonrecursive(hist, dirname, name)
