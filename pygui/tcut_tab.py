#!/usr/bin/env python2

import Tkinter as tk
import ttk
import re

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

from .util import increment_name

class TCutTab(object):

    def __init__(self, main, frame):
        self.main = main
        self._setup_GUI(frame)

        self.cuts = {}
        self.detector_classes = {}

        #frame.after_idle(self._repeatedly_check)

    def _setup_GUI(self, frame):
        self.frame = frame
        self._MakeNaming(frame)
        self._MakeTreeView(frame)

    def _repeatedly_check(self):
        self._check_for_tcut()
        self.frame.after(1000,self._repeatedly_check)

    def _dump_to_tfile(self):
        for cut in self.cuts.values():
            cut.Write()

    def _tcut_patterns(self):
        output = []
        for name, cut in self.cuts.items():
            points = []
            for i in xrange(cut.GetN()):
                points.append((cut.GetX()[i], cut.GetY()[i]))

            pattern = {'name':name,
                       'points':points,
                       'varx':cut.GetVarX(),
                       'vary':cut.GetVarY(),
                }
            output.append(pattern)
        return output

    def _load_tcut_patterns(self, patterns):
        for pattern in patterns:
            cut = ROOT.TCutG(pattern['name'],len(pattern['points']))
            cut.SetVarX(pattern['varx'])
            cut.SetVarY(pattern['vary'])
            for i,(x,y) in enumerate(pattern['points']):
                cut.SetPoint(i, x, y)
            self.AddCut(cut)

    def _check_for_tcut(self):
        # Does CUTG exist?
        cutg = ROOT.gROOT.GetListOfSpecials().FindObject('CUTG')
        if not cutg:
            return None

        # Does it have 3 or more points?
        npoints = cutg.GetN()
        if npoints < 3:
            return None

        # Is the first point equal to the last point?
        xi = cutg.GetX()[0]
        yi = cutg.GetY()[0]
        xf = cutg.GetX()[npoints-1]
        yf = cutg.GetY()[npoints-1]
        if xi!=xf or yi!=yf:
            return None

        return cutg

    def AddDirectory(self, tdir):
        gchain = ROOT.gFragment
        # If the chain doesn't exist, or if it is empty.
        if not gchain or gchain.GetEntries()==0:
            return

        for branch in gchain.GetListOfBranches():
            cls_name = branch.GetName()
            cls = getattr(ROOT, cls_name)
            if cls_name not in self.detector_classes:
                top = self.tree.insert('','end',
                                       cls_name, text=cls_name)
                self.tree.insert(top,'end',
                                 '{}/Draw PID'.format(cls_name),
                                 text='Draw PID')
                self.detector_classes[cls_name] = cls

    def AddFile(self, tfile):
        for key in tfile.GetListOfKeys():
            if key.GetClassName()=='TCutG':
                self.AddCut(key.ReadObj())

    def AddCut(self, cut, det_type = None):
        name = cut.GetName()
        full_name = '{}/{}'.format(det_type,name) if det_type is not None else name
        cut.SetLineColor(ROOT.kRed)
        cut.SetLineWidth(3)
        self.cuts[full_name] = cut

        if det_type is None:
            parent = ''
        else:
            parent = det_type
            self.detector_classes[det_type].AddGate(cut)

        self.tree.insert(parent, 'end', full_name, text=name, values='2D Cut',
                         image = self.main.icons['tcutg'])

        if ROOT.gPad:
            ROOT.gPad.Modified()
            ROOT.gPad.Update()

    def StartCut(self):
        ROOT.gROOT.SetEditorMode('CutG')

    def SaveCut(self):
        cutg = self._check_for_tcut()
        if cutg is None:
            return

        ROOT.gROOT.GetListOfSpecials().Remove(cutg)

        cutg.SetName(self._increment_name())
        for prim in ROOT.gPad.GetListOfPrimitives():
            if isinstance(prim, ROOT.TH2):
                title = prim.GetName()
                det_type = title[:title.index('_')]
                break
        else:
            det_type = None

        self.AddCut(cutg, det_type)

    def DeleteCut(self):
        cutg = self._check_for_tcut()
        if(cutg is None):
            return

        cutg.Delete()

    def CopyCut(self):
        cuts = self.tree.selection()
        if not cuts:
            return
        cutname = cuts[0]
        tcutg = self.cuts[cutname]
        newcut = tcutg.Clone(self._increment_name())

        xmin = min(newcut.GetX()[i] for i in range(newcut.GetN()))
        xmax = max(newcut.GetX()[i] for i in range(newcut.GetN()))
        xshift = 0.5*(xmax-xmin)
        for i in range(newcut.GetN()):
            newcut.GetX()[i] += xshift


        newcut.Draw('same')
        self.AddCut(newcut)


    def _increment_name(self):
        name = self.next_name.get()
        self.next_name.set(increment_name(name))
        return name

    def _MakeNaming(self, parent):
        self.next_name = tk.StringVar(value='tcutg_0')
        frame = tk.Frame(parent)
        tk.Label(frame, text='Name:').pack(side=tk.LEFT)
        tk.Entry(frame, textvariable=self.next_name).pack(side=tk.LEFT)
        frame.pack(fill=tk.X,expand=False)

        frame = tk.Frame(parent)
        tk.Button(frame, text='Make Gate', fg="black",bg="light goldenrod", command=self.StartCut).pack(side=tk.LEFT)
        tk.Button(frame, text='Save Gate', fg="black",bg="light goldenrod", command=self.SaveCut).pack(side=tk.LEFT)
        tk.Button(frame, text='Copy Gate',fg="black",bg="light goldenrod", command=self.CopyCut).pack(side=tk.LEFT)
        tk.Button(frame, text='Delete Gate',fg="black",bg="firebrick", command=self.DeleteCut).pack(side=tk.LEFT)
        frame.pack(fill=tk.X,expand=False)

    def _MakeTreeView(self, parent):
        self.tree = ttk.Treeview(parent, columns=('type',))
        self.tree.column('type', width=50, anchor='e')
        self.tree.heading('type', text='Type')
        self.tree.pack(fill=tk.BOTH,expand=True)
        self.tree.bind("<Double-1>",self.TreeView_OnDoubleClick)

    def _draw_cut(self,cut):
        tcut.Draw('same')
        if ROOT.gPad:
            ROOT.gPad.Modified()
            ROOT.gPad.Update()

    def TreeView_OnDoubleClick(self, event):
        selection = event.widget.selection()[0]
        if selection in self.cuts:
            self._draw_cut(self.cuts[selection])
        else:
            if selection.endswith('Draw PID'):
                cls_name = selection[:selection.index('/')]
                try:
                    self.detector_classes[cls_name].DrawPID('','',1000)
                except AttributeError:
                    print 'You did not implement {}::DrawPID.\nPlease do so.'.format(cls_name)
