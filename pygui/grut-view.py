import gc
import os
import sys
import warnings
import threading
import Tkinter as tk
import time

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True
warnings.filterwarnings(action='ignore',category=RuntimeWarning,
                        message='creating converter.*')
ROOT.PyConfig.StartGuiThread = False
ROOT.PyConfig.IgnoreCommandLineOptions = True

sys.argv = sys.argv[1:] # ROOT appends 'python' to the beginning of sys.argv
sys.argv.insert(0,'grut-view.py')

sys.path.append(os.environ['GRSISYS'])
from pygui.mainwindow import MainWindow

ident = threading.current_thread().ident

def update():
    try:
        if (threading.current_thread().ident == ident and
            window is not None):
            window.Update()
    except tk.TclError:
        pass
    except Exception as e:
        print e

def on_close():
    global window
    window = None
    gc.collect()

window = None
window = MainWindow()
