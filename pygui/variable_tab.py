#!/usr/bin/env python2

import Tkinter as tk
import ttk

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

from .util import PreserveGDir

class VariableTab(object):

    def __init__(self, main, frame):
        self.main = main
        self._setup_GUI(frame)
        self.variables = {}
        self.OnUpdateAll_Click()

    def _setup_GUI(self, frame):
        self.frame = frame
        self._MakeButtons(frame)
        self._MakeTreeView(frame)

    def _MakeButtons(self, parent):
        # Text entries
        frame = tk.Frame(parent)
        tk.Label(frame, text='Name').grid(row=0, column=0)
        tk.Label(frame, text='Value').grid(row=0, column=1)
        self.var_name = tk.StringVar()
        tk.Entry(frame, textvariable=self.var_name).grid(row=1, column=0)
        self.var_value = tk.StringVar()
        entry = tk.Entry(frame, textvariable=self.var_value)
        entry.bind('<Return>',self.OnSetReplaceVariable_Click)
        entry.grid(row=1, column=1)
        frame.pack(fill=tk.X,expand=False)

        # Buttons
        frame = tk.Frame(parent)
        tk.Button(frame, text='Set/Replace',
                  command=self.OnSetReplaceVariable_Click).pack(side=tk.LEFT)
        tk.Button(frame, text='Delete',
                  command=self.OnDeleteVariable_Click).pack(side=tk.LEFT)
        tk.Button(frame, text='Update',
                  command=self.OnUpdateAll_Click).pack(side=tk.LEFT)
        tk.Button(frame, text='Save',
                  command=self.OnSave_Click).pack(side=tk.LEFT)
        frame.pack(fill=tk.X,expand=False)

    def _MakeTreeView(self, parent):
        self.treeview = ttk.Treeview(parent, columns=('value',))
        self.treeview.heading('#0', text='Name')
        self.treeview.heading('value', text='Value')
        self.treeview.pack(fill=tk.BOTH,expand=True)
        self.treeview.bind("<Double-1>",self.VariableSelection)

    def OnUpdateAll_Click(self, *args):
        t = ROOT.GValue.AllValues()
        for val in t:
            self.SetReplaceVariable(val.GetName(), val.GetValue(),
                                    update_cpp = False)

    def OnSave_Click(self, *args):
        ROOT.GValue.WriteValFile("myvalues.val")


    def SetReplaceVariable(self, name, value, update_cpp = True):
        self.variables[name] = value
        if name in self.treeview.get_children():
            self.treeview.item(name, values=(str(value),))
        else:
            self.treeview.insert('','end',name, text=name, values=(str(value),))

        if update_cpp:
            ROOT.GValue.SetReplaceValue(name, value)


    def DeleteVariable(self, name):
        self.variables.pop(name, None)
        self.treeview.delete(name)

    def OnSetReplaceVariable_Click(self, *args):
        name = self.var_name.get()
        if not name:
            return

        try:
            value = float(self.var_value.get())
        except ValueError:
            print 'Could not convert "{}" to float'.format(self.var_value.get())
            return

        self.SetReplaceVariable(name, value)

    def OnDeleteVariable_Click(self):
        selection = self.treeview.selection()
        if not selection:
            return
        self.DeleteVariable(selection[0])

    def VariableSelection(self, event):
        name = event.widget.selection()[0]
        self.var_name.set(name)
        self.var_value.set(str(self.variables[name]))

    def _dump_to_tfile(self):
        #pipeline = ROOT.GetPipeline(0)
        #if not pipeline:
        #    return

        tdir = ROOT.gDirectory.mkdir('variables')
        #with PreserveGDir(tdir):
        #    for obj in pipeline.GetVariables():
        #        obj.Write()

    def _variable_patterns(self):
        return self.variables

    def _load_variable_patterns(self, patterns):
        for name, value in patterns.items():
            self.SetReplaceVariable(name, value)

    def AddFile(self, tfile):
        tdir = tfile.GetListOfKeys().FindObject('variables')
        if (tdir and
            issubclass(getattr(ROOT, tdir.GetClassName()), ROOT.TDirectory)):
            tdir = tdir.ReadObj()
            for key in tdir.GetListOfKeys():
                obj = key.ReadObj()
                #print " Name : " + obj.GetName()
                #print " Value : " + str(obj.GetValue())
                self.SetReplaceVariable(obj.GetName(),
                                        obj.GetValue())
