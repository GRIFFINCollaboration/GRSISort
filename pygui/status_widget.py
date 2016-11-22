#!/usr/bin/env python2

from collections import namedtuple
import Tkinter as tk
import ttk

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

class ThreadVars(object):
    def __init__(self):
        self.items_current = tk.DoubleVar()
        self.items_pushed = tk.DoubleVar()
        self.items_popped = tk.DoubleVar()
        self.finished = tk.BooleanVar()

    def Update(self, thread):
        self.items_current.set(thread.GetItemsCurrent())
        self.items_pushed.set(thread.GetItemsPushed())
        self.items_popped.set(thread.GetItemsPopped())
        self.finished.set(not thread.IsRunning())

class ThreadStatusFrame(ttk.Frame, object):
    def __init__(self, parent, *options):
        super(ThreadStatusFrame,self).__init__(parent)

        self.thread_content = {}
        self.after_idle(self._PeriodicThreadCheck)

    def _setup_GUI(self):
        ttk.Separator(self, orient=tk.HORIZONTAL).grid(row=1, column=0, columnspan=10, sticky='ew')
        ttk.Separator(self, orient=tk.VERTICAL).grid(column=1,row=0,rowspan=100,sticky='ns')
        tk.Label(self, text='Output Queue').grid(row=0, column=2)
        ttk.Separator(self, orient=tk.VERTICAL).grid(column=3,row=0,rowspan=100,sticky='ns')
        tk.Label(self, text='Items Processed').grid(row=0, column=4)
        ttk.Separator(self, orient=tk.VERTICAL).grid(column=5,row=0,rowspan=100,sticky='ns')
        tk.Label(self, text='Finished').grid(row=0, column=6)

    def _PeriodicThreadCheck(self):
        self._ThreadCheck()
        self.after(1000, self._PeriodicThreadCheck)

    def _ThreadCheck(self):
        threads = ROOT.StoppableThread.GetAll()
        for thread in threads:
            name = thread.Name()
            if name in self.thread_content:
                var = self.thread_content[name]
            else:
                if not self.thread_content:
                    self._setup_GUI()

                var = ThreadVars()
                row = len(self.thread_content)+2
                self.thread_content[name] = var

                label = tk.Label(self, text=name, justify=tk.LEFT)
                label.grid(row=row, column=0)
                progress = ttk.Progressbar(self, variable=var.items_current, maximum=5e4)
                progress.grid(row=row, column=2)
                total = tk.Label(self, textvariable=var.items_pushed)
                total.grid(row=row, column=4)
                finished = ttk.Checkbutton(self, variable=var.finished, state='disabled')
                finished.grid(row=row, column=6)

            var.Update(thread)
